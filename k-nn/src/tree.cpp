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
  // batch queries into chunks
  // each thread pulls first chunk free and processes, writing to memory
  assert(d == dims_);
  queries_ = qs;

  next_batch_ = 0;

  results_.resize(queries_->size() * k);
  //std::cout << "results_.size() = " << results_.size() << std::endl;

  std::vector<std::thread> threads;
  for (int i = 0; i < num_threads; ++i) {
    threads.push_back(std::thread(&KDTree::process_query_batch, this, i, k));
  }
  for (int i = 0; i < num_threads; ++i) {
    threads[i].join();
  }

}

void KDTree::write(std::string results_file, uint64_t tid, uint64_t qid, uint64_t nqs, uint64_t k) {

  std::ofstream file;
  file.open(results_file, std::ios_base::binary);
  assert(file.is_open());

  file.write("RESULT", 8);
  file.write((char*) &tid, 8);
  file.write((char*) &qid, 8);
  file.write((char*) &qid + 1, 8);
  file.write((char*) &nqs, 8);
  file.write((char*) &dims_, 8);
  file.write((char*) &k, 8);

  for (auto result : results_) {
    for (auto val : result) {
      file.write((char*) &val, sizeof(float));
    }
  }

  file.close();
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
  assert(tid >= 0);

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



    j.node->index_ = median;

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
  assert(tid >= 0);

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

      std::vector<Node*> heap;
      process_query(root_.get(), i, 0, k, heap);
      /*
      std::cout << "result->index_: " << result->index_ << std::endl;
      std::cout << "distance: " << distance((*queries_)[i], node_val(result)) << std::endl;
      */
      for (size_t j = 0; j < heap.size(); j++) {
        results_[i * k + j] = node_val(heap[j]);
      }
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
    size_t qi, int depth, int k, std::vector<Node*> &heap) {

  //std::cout << "\n\nin process_query()" << std::endl;

  if (!node) return nullptr;

  std::vector<float> target = (*queries_)[qi];

  auto comp = [&](Node *a, Node *b){
    if (a == closer_node(target, a, b)) {
      return true;
    }
    return false;
  };


  if (!node->left_ && !node->right_) {
    if (node == closer_node(target, node, heap[0])) {
      heap[0] = node;
    }
    std::make_heap(heap.begin(), heap.end(), comp);
    return node;
  }

  int dim = depth % dims_;

  Node *next_branch = nullptr;
  Node *oppo_branch = nullptr;


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
  //std::cout << os.str();

  //std::cout << "Checking if split node or subtree is closer" << std::endl;

  if (heap.size() < (unsigned)k) {
    heap.push_back(node);
  } else if (heap.size() == (unsigned)k) {
    if (node == closer_node(target, node, heap[0])) {
      heap[0] = node;
    }
  }
  std::make_heap(heap.begin(), heap.end(), comp);

  Node *best = closer_node(target,
      node,
      process_query(next_branch, qi, depth + 1, k, heap));

  auto dist_to_best = distance(target, node_val(best));
  auto dist_to_split = std::abs(target[dim] - node_val(node)[dim]);

  if (dist_to_best > dist_to_split) {
    //std::cout << "Opposite branch could be better!" << std::endl;
    best = closer_node(target,
        best,
        process_query(oppo_branch, qi, depth + 1, k, heap));
  }

  /*
  if (best == closer_node(target, best, heap[0])) {
    heap[0] = node;
  }
  std::make_heap(heap.begin(), heap.end(), comp);
  std::cout << "HEAP" << std::endl;
  for (auto e : heap) {
    std::cout << e->index_ << "\tdistance: " << distance(target, node_val(e)) << std::endl;
  }
  */

  return best;
}
