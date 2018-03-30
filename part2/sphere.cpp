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
#include <iomanip>

const int N = 100'000;
const int MIN_DIM = 2;
const int MAX_DIM = 50;
const int HIST_BUCKETS = 100;
const int THREADS = 16;

std::vector<std::vector<int>> generate_histograms(bool parallel) {
  std::vector<std::vector<int>> hists;
  std::vector<int> hist(HIST_BUCKETS);

  // Need MAX_DIM + 2 size for points due to sampling formula from
  // section 3.2 of http://compneuro.uwaterloo.ca/files/publications/voelker.2017.pdf
  std::vector<std::vector<float>> points(N, std::vector<float>(MAX_DIM + 2));
  std::vector<float> dists(N);

  std::minstd_rand eng;
  std::normal_distribution<float> dist(0, 1);

  for (int dims = MIN_DIM; dims <= MAX_DIM; dims++) {
    std::fill(dists.begin(), dists.end(), 0);

    #pragma omp parallel for num_threads(THREADS) schedule(guided) private(dist, eng, hist) shared(hists, points, dists) if(parallel)
    for (std::size_t i = 0; i < points.size(); i++) {
      auto &pt = points[i];

      // Generate uniform points on the dims+1 sphere
      float norm = 0;
      for (int dim = 0; dim < dims + 2; dim++) {
        pt[dim] = dist(eng);
        //std::cout << pt[dim] << std::endl;
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

    std::fill(hist.begin(), hist.end(), 0);
    // Histogram and stuff and graphs
    for (size_t i = 0; i < dists.size(); i++) {
      hist[(int) (dists[i] * hist.size())] += 1;
    }

    hists.push_back(hist);
  }
  return hists;
}

void print_hists(std::vector<std::vector<int>> hists) {
  for (int dims = MIN_DIM; dims <= MAX_DIM; dims++) {
    std::cout << "\n\t" << dims << "-D" << std::endl;
    std::cout << "Histogram with " << HIST_BUCKETS << " buckets" << std::endl;
    std::cout << "Bucket\t\tPercentage\tCount" << std::endl;
    for (size_t i = 0; i < hists[dims-2].size(); i++) {
      std::cout.precision(2);
      std::cout << std::fixed
        << ((float) i) / hists[dims-2].size() << "\t\t"
        << (int)(100.0 * hists[dims-2][i] / N) << "%\t\t"
        << hists[dims-2][i] << std::endl;
    }
  }
}

int main() {
  // warmup
  generate_histograms(true);

  auto start = std::chrono::system_clock::now();
  auto hists = generate_histograms(true);
  auto stop = std::chrono::system_clock::now();
  std::cout << "Parallel:\t"
    << std::chrono::duration<double>(stop - start).count() << "s" << std::endl;

  start = std::chrono::system_clock::now();
  generate_histograms(false);
  stop = std::chrono::system_clock::now();
  std::cout << "Serial:\t\t"
    << std::chrono::duration<double>(stop - start).count() << "s" << std::endl;

  print_hists(hists);
}
