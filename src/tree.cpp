#include "tree.hpp"

KDTree::KDTree(points_t &p, int d) : points_(p), dims_(d), num_nodes_(1) {

  int_vec indices(points_.size());
  std::iota(indices.begin(), indices.end(), 0);


  int median = sample_median_index(indices, 0);
  std::cout << "Sample median:\t" << "p[" << median
    << "] = " << points_[median][0] << std::endl;

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


  const int num_threads = 1;
  std::thread t[num_threads];
  for (int i = 0; i < num_threads; ++i) {
    t[i] = std::thread(&KDTree::grow_branch, this, i);
  }
  for (int i = 0; i < num_threads; ++i) {
    t[i].join();
  }
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
    float cur = points_[i][dim];
    if (cur < pivot) left.push_back(i);
    else if (cur > pivot) right.push_back(i);
  }
  assert(indices.size() == left.size() + right.size() + 1);
  return std::pair<int_vec, int_vec>(left, right);
}


void KDTree::grow_branch(int tid) {

  std::unique_lock<std::mutex> num_lock(num_mtx_, std::defer_lock);

  for (;;) {
    std::unique_lock lock(job_mtx_);
    while (job_q_.empty()) {

      num_lock.lock();
      if (num_nodes_ == points_.size()) {
        num_lock.unlock();
        return;
      }
      num_lock.unlock();

      job_cv_.wait(lock);
    }

    Job j = std::move(job_q_.front());
    job_q_.pop();

    std::cout << "Job queue size: " << job_q_.size() << std::endl;

    lock.unlock();
    job_cv_.notify_all();




    if (j.indices.size() == 0) continue;

    if (j.indices.size() == 1) {
      std::cout << "Case with 1 index only" << std::endl;
      std::cout << "Value is " << j.indices[0] << std::endl;

      j.node->index_ = j.indices[0];

      std::cout << "num nodes: " << num_nodes_ << std::endl;
      continue;
    }

    std::cout << "Thread " << tid << " processing job with "
      << j.indices.size() << " indices" << std::endl;


    int dim = j.node->level_ % dims_;

    int median = sample_median_index(j.indices, dim);

    std::cout << "Sample median:\t" << "p[" << median << "] = "
      << points_[median][0] << std::endl;

    int_vec left, right;
    std::tie(left, right) = split(median, j.indices, 0);

    for (auto i : left) { std::cout << i << " "; } std::cout << std::endl;
    for (auto i : right) { std::cout << i << " "; } std::cout << std::endl;



    if (!left.empty()) {
      root_->left_ = std::make_unique<Node>(j.node->level_ + 1);
      num_lock.lock();
      ++num_nodes_;
      num_lock.unlock();
    }

    if (!right.empty()) {
      root_->right_ = std::make_unique<Node>(j.node->level_ + 1);
      num_lock.lock();
      ++num_nodes_;
      num_lock.unlock();
    }




    lock.lock();
    std::cout << "Thread " << tid << " acquired lock again " << std::endl;

    if (!left.empty()) {
      job_q_.emplace(root_->left_.get(), left);
    }
    if (!right.empty()) {
      job_q_.emplace(root_->right_.get(), right);
    }

    std::cout << "num nodes: " << num_nodes_ << std::endl;
    std::cout << "Job queue size: " << job_q_.size() << std::endl;
    std::cout << "----------------------" << std::endl << std::endl;

    lock.unlock();
  }
}
