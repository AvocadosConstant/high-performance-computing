#include <omp.h>
#include <stdio.h>
#include <cmath>
#include <iostream>
#include <functional>
#include <chrono>
#include <random>
#include <array>
#include <assert.h>
#include <cmath>

const int N = 10;
//const int N = 100'000'000;
const int MIN_DIM = 2;
const int MAX_DIM = 16;

double time(const std::function<void ()> &f) {
  f(); // Run once to warmup.
  // Now time it for real.
  auto start = std::chrono::system_clock::now();
  f();
  auto stop = std::chrono::system_clock::now();
  return std::chrono::duration<double>(stop - start).count();
}

int main() {

  std::vector<std::vector<float>> hists(MAX_DIM - MIN_DIM);
  std::vector<std::vector<float>> points(N, std::vector<float>(MAX_DIM + 2));
  std::vector<float> dists(N);


  std::minstd_rand eng;
  std::uniform_real_distribution<float> dist(-1, 1);

  for (int dims = 2; dims <= 16; dims++) {
    std::cout << "\n\t" << dims << "-D" << std::endl;

    std::fill(dists.begin(), dists.end(), 0);

    for (std::size_t i = 0; i < points.size(); i++) {
      auto &pt = points[i];

      // Generate uniform points on the dims+1 sphere
      float norm = 0;
      for (int dim = 0; dim < dims + 2; dim++) {
        pt[dim] = dist(eng);
        norm += pt[dim] * pt[dim];
      }

      // Calculate l2-norm
      norm = std::sqrt(norm);

      // Normalize those points
      for (int dim = 0; dim < dims + 2; dim++) {
        pt[dim] /= norm;
      }

      // "Drop" last two dims for uniform points in dims-ball

      // Now pt[0:dims] consists of uniform points in the dims-ball

      // Calculate distance to surface of each point
      for (int dim = 0; dim < dims; dim++) {
        dists[i] += pt[dim] * pt[dim];
      }
      dists[i] = std::sqrt(dists[i]);
    }
    // dists should consist of distances of the N uniform points on thedims-ball

    // Histogram and stuff and graphs
    for (auto dist : dists) {
      std::cout << dist << std::endl;
    }
  }
}
