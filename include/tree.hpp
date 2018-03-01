#ifndef _TREE_HPP_
#define _TREE_HPP_

#include <iostream>
#include <cassert>
#include <memory>
#include <numeric>
#include <algorithm>
#include <random>


using points_t = std::vector<std::vector<float>>;
using int_vec = std::vector<int>;

const int MIN_SAMPLE_SIZE = 100;


struct Node {
  Node() :
    parent_{nullptr}, left_{nullptr}, right_{nullptr},
    index_{-1}, level_{0} {}

  Node(Node *p, int i, int l) :
    parent_{p}, left_{nullptr}, right_{nullptr},
    index_{i}, level_{l} {}

  Node *parent_;
  Node *left_;
  Node *right_;
  int index_;
  int level_;
};


struct Job {
  Node *parent;
  int_vec indices;
};


class KDTree {
  public:
    KDTree(points_t &p, int d);

  private:
    Node *root_;
    points_t &points_;
    int dims_;

    int sample_median_index(const int_vec &indices, const int dim);

    std::pair<int_vec, int_vec> split(int pivot_i, int_vec &indices, int dim);

    void grow_branch(Job job);

};

#endif
