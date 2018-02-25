#include <vector>
#include "include/reader.hpp"
#include "include/ingest.hpp"
#include "include/tree.hpp"

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
  std::cout << "Running with " << n_cores << " cores." << std::endl;

  auto training = parse_data(argv[2]);
  auto queries = parse_data(argv[3]);

  //print_data(training);
  //print_data(queries);

  std::cout << training.points.size() << " training points" << std::endl;

  KDTree tree(&training.points);
  std::cout << "Tree size: " << sizeof(tree) << std::endl;
}
