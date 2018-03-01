#ifndef _TREE_HPP_
#define _TREE_HPP_

#include <iostream>
#include <cassert>
#include <memory>
#include <numeric>
#include <algorithm>
#include <random>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>


using points_t = std::vector<std::vector<float>>;
using int_vec = std::vector<int>;

const int MIN_SAMPLE_SIZE = 80;
const int BATCH_SIZE = 5;


struct Node {
  Node() :
    level_{0}, left_{nullptr}, right_{nullptr} {}

  Node(int l) :
    level_{l}, left_{nullptr}, right_{nullptr} {}

  int level_;
  int index_;
  std::unique_ptr<Node> left_;
  std::unique_ptr<Node> right_;
};


struct Job {
  Node *node;
  int_vec indices;

  Job(Node *n, int_vec i) :
    node{n}, indices{i} {};
};


class KDTree {
  public:
    KDTree(points_t &p, int d, int num_threads);
    void query(points_t *qs, int d, int k, int num_threads);

  private:
    std::unique_ptr<Node> root_;
    points_t &points_;
    points_t *queries_;
    int dims_;
    std::condition_variable job_cv_;
    std::queue<Job> job_q_;
    std::mutex job_mtx_;

    std::atomic<size_t> num_nodes_;
    std::atomic<size_t> next_batch_;


    int sample_median_index(const int_vec &indices, const int dim);

    std::pair<int_vec, int_vec> split(int pivot_i, int_vec &indices, int dim);

    //TODO rename this godawful function
    void grow_branch(int tid);

    void process_query_batch(int tid);

};

#endif
