#ifndef _TREE_HPP_
#define _TREE_HPP_

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
    KDTree(points_t &p, int d) : points_(p), dims_(d) {

      int_vec indices(points_.size());
      std::iota(indices.begin(), indices.end(), 0);

      int median = sample_median_index(indices, 0);
      std::cout << "Sample median:\t" << "p[" << median << "] = " << points_[median][0] << std::endl;

      int_vec left, right;
      std::tie(left, right) = split(median, indices, 0);

    }


  private:
    Node *root_;
    points_t &points_;
    int dims_;

    int sample_median_index(const int_vec &indices, const int dim) {
      int_vec sample;
      std::sample(indices.begin(), indices.end(),
          std::back_inserter(sample), std::min(MIN_SAMPLE_SIZE, (int)indices.size()),
          std::mt19937{std::random_device{}()});

      std::sort(sample.begin(), sample.end(), [&](int a, int b) {
        return points_[a][dim] < points_[b][dim];
      });

      return sample[sample.size() / 2];
    }


    std::pair<int_vec, int_vec> split(int pivot_i, int_vec &indices, int dim) {
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


    void grow_branch(Job job) {
      // sample some points to find median
      int level = job.parent->level_ + 1;
      int dim = level % dims_;
      auto cur_node = std::make_unique<Node>(
          job.parent, sample_median_index(job.indices, dim), level);

      //std::cout << "cur_node unique ptr size: " << sizeof(cur_node) << std::endl;

      // create node with median

      // set parent as parent, and set parent's correct child as curnode

      // filter into a left and right indices vector based on median

      // create two jobs with curnode as parent and with one of the sub indices

      // done
    }


};

#endif
