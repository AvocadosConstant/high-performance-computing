#ifndef _TREE_HPP_
#define _TREE_HPP_

typedef std::vector<std::vector<float>> points_t;

struct Node {
  int index;
};

class KDTree {
  public:
    KDTree(points_t *points) {
      points_ = points;
    }

  private:
    Node root_;
    points_t *points_;
};
#endif
