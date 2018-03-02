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


void KDTree::query(points_t *qs, int d, int k, int num_threads) {
  assert(d == dims_);
  queries_ = qs;
  //std::cout << queries_->size() << ", " << num_threads << std::endl;

  next_batch_ = 0;

  std::vector<std::thread> threads;
  for (int i = 0; i < num_threads; ++i) {
    threads.push_back(std::thread(&KDTree::process_query_batch, this, i, k));
  }
  for (int i = 0; i < num_threads; ++i) {
    threads[i].join();
  }
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

  for (;;) {
    std::unique_lock<std::mutex> job_q_lock(job_mtx_);
    while (job_q_.empty()) {

      // Exit condition
      if (num_nodes_ == points_.size()) return;

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
      ++num_nodes_;
    }
    if (!right.empty()) {
      j.node->right_ = std::make_unique<Node>(j.node->level_ + 1);
      ++num_nodes_;
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


std::vector<float> KDTree::node_val(Node *node) {
  return points_[node->index_];
}


void KDTree::process_query_batch(int tid, int k) {
  for (;;) {
    size_t batch_start = next_batch_;
    do {
      if (batch_start >= queries_->size()) return;
    } while (!std::atomic_compare_exchange_weak(
      &next_batch_, &batch_start, batch_start + BATCH_SIZE));


    //std::cout << "Thread " << tid << " handling batch "
    //  << batch_start << std::endl;

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    auto batch_end = std::min(batch_start + BATCH_SIZE, queries_->size());
    for (auto i = batch_start; i < batch_end; i++) {
      //std::cout << "Thread " << tid << "\tBatch " << batch_start
      //  << "\tquery " << i << std::endl;

      auto result = process_query(root_.get(), i, 0, k);
    }
  }
}


float KDTree::distance(std::vector<float> a, std::vector<float> b) {
  assert(a.size() == b.size());

  float sum = 0;
  for (size_t i = 0; i < a.size(); i++) {
    float dif = a[i] - b[i];
    sum += dif * dif;
  }
  return std::sqrt(sum);
}

Node *KDTree::closer_node(std::vector<float> target, Node *a, Node *b) {

  if (!a) {
    return b;
  } else if (!b) {
    return a;
  }

  auto da = distance(target, node_val(a));
  auto db = distance(target, node_val(b));

  if (da < db) {
    return a;
  }
  return b;
}


Node *KDTree::process_query(Node *node,
    size_t qi, int depth, int k) {

  if (!node) return nullptr;

  int dim = depth % k;

  Node *next_branch = nullptr;
  Node *oppo_branch = nullptr;

  
  std::vector<float> target = (*queries_)[qi];
  //for (auto f : target) std::cout << f << " ";
  //std::cout << std::endl;


  std::ostringstream os;

  os << "query = " << qi
     << "\tdepth = " << depth
     << "\tdim = " << dim
     << "\ttarget[dim] = " << target[dim]
     << "\tnode[dim] = " << node_val(node)[dim];

  if (target[dim] < node_val(node)[dim]) {
    next_branch = node->left_.get();
    oppo_branch = node->right_.get();
    os << "\tNext branch is left\n";
  } else {
    next_branch = node->right_.get();
    oppo_branch = node->left_.get();
    os << "\tNext branch is right\n";
  }
  std::cout << os.str();

  std::cout << "Checking if split node or subtree is closer" << std::endl;
  Node *best = closer_node(target,
      node,
      process_query(next_branch, qi, depth + 1, k));

  auto dist_to_best = distance(target, node_val(best));
  auto dist_to_split = std::abs(target[dim] - node_val(node)[dim]);

  std::cout << "Checking if need to check oppo branch" << std::endl;
  if (dist_to_best > dist_to_split) {
    std::cout << "Opposite branch could be better!" << std::endl;
    Node *best = closer_node(target,
        best,
        process_query(oppo_branch, qi, depth + 1, k));
  }

  dist_to_best = distance(target, node_val(best));
  std::cout << "Best dist so far: " << dist_to_best << std::endl;
  return best;
}
