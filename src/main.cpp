#include <vector>
#include <chrono>
#include "reader.hpp"
#include "ingest.hpp"
#include "tree.hpp"

const auto ARGC = 5;
const auto USAGE_STRING =
  "./k-nn n_cores input_file queries_file results_file";


void assert_usage(int argc) {
  if (argc != ARGC) {
    std::cerr << "Execution format: " << USAGE_STRING
      << std::endl << std::endl;
    exit(1);
  }
}


int main(const int argc, char **argv) {
  assert_usage(argc);

  //int n_cores = std::stoi(argv[1]);
  //std::cout << "Running with " << n_cores << " cores." << std::endl;

  auto training = parse_data(argv[2]);
  auto queries = parse_data(argv[3]);

  points_t points = training.points;
  uint64_t dims = training.n_dims;


  auto start = std::chrono::high_resolution_clock::now();

  KDTree tree(points, dims);

  auto stop = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> dt = stop - start;

  std::cout << "Tree building took " << dt.count() << std::endl;
}
