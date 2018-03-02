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

  int n_cores = std::stoi(argv[1]);

  std::cout << "\nParsing training data..." << std::endl;
  auto training = parse_data(argv[2]);

  std::cout << "\nBuilding tree..." << std::endl;
  auto start = std::chrono::high_resolution_clock::now();

  KDTree tree(training.points, training.n_dims, n_cores);

  auto stop = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> dt = stop - start;
  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(dt);

  std::cout << "\nTree building took " << ms.count() << " ms."<< std::endl;



  std::cout << "\nParsing query data..." << std::endl;
  auto queries = parse_data(argv[3]);



  start = std::chrono::high_resolution_clock::now();

  tree.query(&(queries.points), queries.n_dims, queries.k, n_cores);

  stop = std::chrono::high_resolution_clock::now();
  dt = stop - start;
  ms = std::chrono::duration_cast<std::chrono::milliseconds>(dt);

  std::cout << "\nQuery parsing took " << ms.count() << " ms."<< std::endl;
}
