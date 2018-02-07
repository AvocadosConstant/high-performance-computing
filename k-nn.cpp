#include <iostream>
#include <cstdlib>
#include <string>

const auto ARGC = 5;
const auto USAGE_STRING =
  "./k-nn n_cores input_file queries_file results_file";

int main(const int argc, char **argv) {
  if (argc != ARGC) {
    std::cerr << "Execution format: " << USAGE_STRING
      << std::endl << std::endl;
    exit(1);
  }
  int n_cores = std::stoi(argv[1]);
  std::string input_file = argv[2];
  std::string queries_file = argv[3];
  std::string results_file = argv[4];

  std::cout << n_cores << " core"
    << (n_cores > 1 ? "s" : "") << std::endl;
}
