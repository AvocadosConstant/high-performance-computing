#include "tree.hpp"

KDTree::KDTree(points_t &p, int d, int num_threads) :
    points_(p), dims_(d), num_nodes_(1) {

  int_vec indices(points_.size());
  std::iota(indices.begin(), indices.end(), 0);


  // TODO Put this in a function for reuse with grow_branch()

  int median = sample_median_index(indices, 0);

  int_vec left, right;
  std::tie(left, right) = split(median, indices, 0);

  root_ = std::make_unique<Node>(0);
  root_->index_ = median;


  if (!left.empty()) {
    root_->left_ = std::make_unique<Node>(1);
    ++num_nodes_;
    job_q_.emplace(root_->left_.get(), left);
  }

  if (!right.empty()) {
    root_->right_ = std::make_unique<Node>(1);
    ++num_nodes_;
    job_q_.emplace(root_->right_.get(), right);
  }

  std::vector<std::thread> threads;
  for (int i = 0; i < num_threads; ++i) {
    threads.push_back(std::thread(&KDTree::grow_branch, this, i));
  }
  for (int i = 0; i < num_threads; ++i) {
    threads[i].join();
  }
}


void KDTree::query(points_t &p, int d, int num_threads) {
  assert(d == dims_);
  std::cout << p.size() << ", " << num_threads << std::endl;

  // batch queries into chunks
  // each thread pulls first chunk free and processes, writing to memory
}


////////////////////////
// Private Functions //
//////////////////////


int KDTree::sample_median_index(const int_vec &indices, const int dim) {
  int_vec sample;
  std::sample(indices.begin(), indices.end(),
      std::back_inserter(sample), std::min(MIN_SAMPLE_SIZE, (int)indices.size()),
      std::mt19937{std::random_device{}()});

  std::sort(sample.begin(), sample.end(), [&](int a, int b) {
    return points_[a][dim] < points_[b][dim];
  });

  return sample[sample.size() / 2];
}


std::pair<int_vec, int_vec> KDTree::split(int pivot_i, int_vec &indices, int dim) {
  int_vec left;
  int_vec right;
  float pivot = points_[pivot_i][dim];

  for (auto i : indices) {
    if (i != pivot_i) {
      float cur = points_[i][dim];
      if (cur < pivot) left.push_back(i);
      else right.push_back(i);
    }
  }
  assert(
      (indices.size() == left.size() + right.size() + 1) ||
      !(std::cerr << left.size() << " + " << right.size() << " + 1 = " <<
        (left.size() + right.size() + 1) << " =/= "
        << indices.size() << std::endl));
  return std::pair<int_vec, int_vec>(left, right);
}


void KDTree::grow_branch(int tid) {

  std::unique_lock<std::mutex> num_lock(num_mtx_, std::defer_lock);

  for (;;) {
    std::unique_lock<std::mutex> job_q_lock(job_mtx_);
    while (job_q_.empty()) {

      num_lock.lock();
      if (num_nodes_ == points_.size()) {
        // Exit condition
        num_lock.unlock();
        return;
      }
      num_lock.unlock();

      job_cv_.wait(job_q_lock);
    }
    Job j = std::move(job_q_.front());
    job_q_.pop();


    job_q_lock.unlock();
    job_cv_.notify_all();



    assert(!j.indices.empty());
    if (j.indices.size() == 1) {
      // Leaf node
      // TODO Replace with leaf nodes of a larger size (10 nodes?)
      j.node->index_ = j.indices[0];
      continue;
    }

    int median = sample_median_index(j.indices, j.node->level_ % dims_);

    // TODO Replace with std::partition
    int_vec left, right;
    std::tie(left, right) = split(median, j.indices, 0);




    if (!left.empty()) {
      j.node->left_ = std::make_unique<Node>(j.node->level_ + 1);
      num_lock.lock();
      ++num_nodes_;
      num_lock.unlock();
    }
    if (!right.empty()) {
      j.node->right_ = std::make_unique<Node>(j.node->level_ + 1);
      num_lock.lock();
      ++num_nodes_;
      num_lock.unlock();
    }



    // Acquire lock again to add jobs
    job_q_lock.lock();

    if (!left.empty()) {
      job_q_.emplace(j.node->left_.get(), left);
    }
    if (!right.empty()) {
      job_q_.emplace(j.node->right_.get(), right);
    }

    job_q_lock.unlock();
  }
}
