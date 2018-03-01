#include "tree.hpp"

KDTree::KDTree(points_t &p, int d) : points_(p), dims_(d) {

  int_vec indices(points_.size());
  std::iota(indices.begin(), indices.end(), 0);

  int median = sample_median_index(indices, 0);
  std::cout << "Sample median:\t" << "p[" << median << "] = " << points_[median][0] << std::endl;

  int_vec left, right;
  std::tie(left, right) = split(median, indices, 0);

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


void KDTree::grow_branch(Job job) {
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
