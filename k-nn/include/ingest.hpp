#ifndef _INGEST_HPP_
#define _INGEST_HPP_

#include <iostream>
#include <cstdlib>
#include <string>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <iomanip>
#include <vector>

using points_t = std::vector<std::vector<float>>;

void assert_openable_file(const std::string fn, int fd) {
  if (fd < 0) {
    int en = errno;
    std::cerr << "Couldn't open " << fn << ": " <<
      strerror(en) << "." << std::endl;
    exit(2);
  }
}


off_t get_file_size(int fd) {
  struct stat sb;
  int rv = fstat(fd, &sb);
  assert(rv == 0);
  return sb.st_size;
}


void *mmap_wrapper(off_t file_size, int fd) {
  void *vp = mmap(nullptr, file_size, PROT_READ, MAP_SHARED, fd, 0);
  if (vp == MAP_FAILED) {
    int en = errno;
    fprintf(stderr, "mmap() failed: %s\n", strerror(en));
    exit(3);
  }
  return vp;
}


points_t ingest_points(Reader &reader, uint64_t n_points, uint64_t n_dims) {
  points_t points;
  for (uint64_t i = 0; i < n_points; i++) {
    std::vector<float> point;
    for (uint64_t j = 0; j < n_dims; j++) {
      float f;
      reader >> f;
      point.push_back(f);
    }
    points.push_back(point);
  }
  return points;
}


void print_points(points_t points) {
  for (auto p : points) {
    std::cout << "\tPoint: ";
    for (auto v : p) {
      std::cout << std::fixed << std::setprecision(6) << std::setw(15) << std::setfill(' ') << v;
    }
    std::cout << std::endl;
  }
}


struct Data {
  std::string file_type;
  uint64_t id;
  uint64_t n_points;
  uint64_t n_dims;
  uint64_t k;   // Defaults to 0 for training data
  points_t points;
};


Data parse_data(const std::string &fn) {
  //std::cout << "\nParsing file " << fn << std::endl;

  int fd = open(fn.c_str(), O_RDONLY);
  assert_openable_file(fn, fd);

  auto file_size = get_file_size(fd);

  void *vp = mmap_wrapper(file_size, fd);

  char *file_mem = (char *) vp;

  // Tell the kernel that it should evict the pages as soon as possible.
  int rv = madvise(vp, file_size, MADV_SEQUENTIAL|MADV_WILLNEED); assert(rv == 0);
  rv = close(fd); assert(rv == 0);


  // Read file type string.
  int n = strnlen(file_mem, 8);
  std::string file_type(file_mem, n);

  // Start to read data, skip the file type string.
  Reader reader{file_mem + 8};

  uint64_t id, n_points, n_dims, k = 0;
  reader >> id >> n_points >> n_dims;
  if (file_type == "QUERY") reader >> k;

  auto points = ingest_points(reader, n_points, n_dims);

  Data data = {file_type, id, n_points, n_dims, k, points};
  return data;
}


void print_data(Data d) {
  std::cout << "\nPrinting some data" << std::endl;
  std::cout << "\tFile type string: " << d.file_type << std::endl;
  std::cout << "\tTraining file ID: " << std::hex << std::setw(16) <<
    std::setfill('0') << d.id << std::dec << std::endl;
  std::cout << "\tNumber of points: " << d.n_points << std::endl;
  std::cout << "\tNumber of dimensions: " << d.n_dims << std::endl;

  if (d.file_type == "QUERY") {
    std::cout << "\tNumber of neighbors: " << d.k << std::endl;
  }

  print_points(d.points);
  std::cout << std::endl;
}

#endif
