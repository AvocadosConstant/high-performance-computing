#include <iostream>
#include <cstdlib>
#include <string>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

const auto ARGC = 5;
const auto USAGE_STRING =
  "./k-nn n_cores input_file queries_file results_file";

struct InputHeader {
  uint64_t input_id;
  uint64_t n_pts;
  uint64_t n_dims;
};

struct QueriesHeader {
  uint64_t input_id;
  uint64_t query_id;
  uint64_t n_queries;
  uint64_t n_dims;
  uint64_t n_neighbors;
};

InputHeader *parse_input_header(char *path) {
  int input_fd = open(path, O_RDONLY);
  InputHeader *input = static_cast<InputHeader*>
    (mmap(NULL, sizeof(InputHeader), PROT_READ, MAP_SHARED, input_fd, 0));

  std::cout << "\nINPUT DATA" << std::endl
    << "Input set ID:\t"    << input->input_id << std::endl
    << "Num points:\t"      << input->n_pts << std::endl
    << "Num dimensions:\t"  << input->n_dims << std::endl;

  return input;
}

QueriesHeader *parse_queries_header(char *path) {
  int queries_fd = open(path, O_RDONLY);
  QueriesHeader *queries = static_cast<QueriesHeader*>
    (mmap(NULL, sizeof(QueriesHeader), PROT_READ, MAP_SHARED, queries_fd, 0));

  std::cout << "\nQUERY DATA" << std::endl
    << "Input set ID:\t"    << queries->input_id << std::endl
    << "Query set ID:\t"    << queries->query_id << std::endl
    << "Num queries:\t"     << queries->n_queries << std::endl
    << "Num dimensions:\t"  << queries->n_dims << std::endl
    << "Num neighbors:\t"   << queries->n_neighbors << std::endl;

  return queries;
}

int main(const int argc, char **argv) {
  if (argc != ARGC) {
    std::cerr << "Execution format: " << USAGE_STRING
      << std::endl << std::endl;
    exit(1);
  }

  int n_cores = std::stoi(argv[1]);
  auto input_header = parse_input_header(argv[2]);
  auto queries_header = parse_queries_header(argv[3]);
}
