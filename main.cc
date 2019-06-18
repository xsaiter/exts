#include <algorithm>
#include <iostream>
#include <memory>
#include <queue>
#include <vector>

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#define CONST_VOID_PTR_TO_INT(p) (*(const int *)(p))
#define CONST_VOID_PTR_TO_LONG(p) (*(const long *)(p))

#define LOG_ERR(...) fprintf(stderr, __VA_ARGS__)
#define LOG_INFO(...) printf(__VA_ARGS__)

#define PRINT(...)                                                             \
  printf(__VA_ARGS__);                                                         \
  fflush(stdout);

#define DIE(...)                                                               \
  fprintf(stderr, __VA_ARGS__);                                                \
  perror("reason");                                                            \
  exit(EXIT_FAILURE)

static const size_t NUM_SIZE = sizeof(int);

void qsa_flush(void) { fflush(stdout); }

struct entry_s {
  int fd;
  int num;
  bool has_num;
};

struct entry_cmp_s {
  bool operator()(entry_s *x, entry_s *y) { return x->num > y->num; }
};

size_t size_of_file(int fd);
void print_file(const char *name);

int *split_src_file(const char *src_file, size_t mem_size, const char *out_dir,
                    size_t *nfiles);
void merge_files(int *files, size_t nfiles, const char *dest_file);
void sort_file(size_t mem_size, const char *src_file, const char *dest_file,
               const char *out_dir);

void create_src_file(const char *name, size_t size, int max_value);

int open_or_die_file(const char *name, int oflag) {
  int fd = open(name, oflag);
  if (fd == -1) {
    DIE("failed to open file: %s\n", name);
  }
  return fd;
}

int create_or_die_file(const char *name) {
  int fd = open(name, O_RDWR | O_CREAT | O_TRUNC, 0644);
  if (fd == -1) {
    DIE("failed to create file: %s\n", name);
  }
  return fd;
}

size_t size_of_file(int fd) {
  struct stat buf;
  fstat(fd, &buf);
  return static_cast<size_t>(buf.st_size);
}

int *split_src_file(const char *src_file, size_t mem_size, const char *out_dir,
                    size_t *nfiles) {
  int src_fd = open_or_die_file(src_file, O_RDONLY);
  size_t src_file_size = size_of_file(src_fd);
  size_t len = src_file_size / mem_size;
  if (src_file_size % mem_size > 0) {
    ++len;
  }
  int *res = new int[len];
  size_t buf_size = (mem_size - mem_size % NUM_SIZE);
  int *buf = new int[buf_size / NUM_SIZE];
  size_t n = 0;
  size_t i = 0;
  while ((n = static_cast<size_t>(read(src_fd, buf, buf_size))) > 0) {
    std::sort(buf, buf + n / NUM_SIZE);
    char *dest_name;
    asprintf(&dest_name, "%s/%zd_%s", out_dir, i, src_file);
    int fp_dest = create_or_die_file(dest_name);
    write(fp_dest, buf, n);
    off_t pos = lseek(fp_dest, 0L, SEEK_SET);
    res[i++] = fp_dest;
    memset(buf, 0, buf_size);
  }
  delete[] buf;
  close(src_fd);
  *nfiles = i;
  return res;
}

void create_src_file(const char *name, size_t size, int max_value) {
  time_t t;
  srand(static_cast<unsigned>(time(&t)));
  size_t nnums = size / NUM_SIZE;
  int fd = create_or_die_file(name);
  for (size_t i = 0; i < nnums; ++i) {
    int num = rand() % max_value;
    write(fd, &num, sizeof(num));
  }
  close(fd);
}

void print_file(const char *name) {
  int fd = open_or_die_file(name, O_RDONLY);
  size_t fsize = size_of_file(fd);
  size_t nnums = fsize / NUM_SIZE;
  for (size_t i = 0; i < nnums; ++i) {
    int num;
    read(fd, &num, NUM_SIZE);
    printf("\n%d", num);
  }
  close(fd);
}

void merge_files(int *files, size_t nfiles, const char *dest_file) {
  int dest_fd = create_or_die_file(dest_file);
  std::priority_queue<entry_s *, std::vector<entry_s *>, entry_cmp_s> pq;
  for (size_t i = 0; i < nfiles; ++i) {
    entry_s *e = new entry_s;
    e->fd = files[i];
    ssize_t ret = read(files[i], &(e->num), NUM_SIZE);
    if (ret != -1 && ret != 0) {
      e->has_num = true;
    } else {
      e->has_num = false;
    }
    if (e->has_num) {
      pq.push(e);
    }
  }
  while (!pq.empty()) {
    entry_s *x = pq.top();
    if (x != nullptr) {
      pq.pop();
      write(dest_fd, &(x->num), NUM_SIZE);
      ssize_t ret = read(x->fd, &(x->num), NUM_SIZE);
      if (ret != -1 && ret != 0) {
        x->has_num = true;
        pq.push(x);
      } else {
        x->has_num = false;
      }
    }
  }
  close(dest_fd);
}

void sort_file(size_t mem_size, const char *src_file, const char *dest_file,
               const char *out_dir) {
  size_t nfiles;
  int *files = split_src_file(src_file, mem_size, out_dir, &nfiles);
  merge_files(files, nfiles, dest_file);
  for (size_t i = 0; i < nfiles; ++i) {
    close(files[i]);
  }
  free(files);
}

int main(int argc, char **argv) {
  /*int fd = open("data.dat", O_RDONLY);
  size_t size = get_size_of_file(fd);
  size_t tmp = size;
  close(fd);*/

  PRINT("DATA\n");
  print_file("data2.dat");
  PRINT("DATA END\n");

  // create_src_file("data2.dat", 20, 20);

  sort_file(8, "data2.dat", "res.dat", "tmp");

  // size_t nfiles;
  // split_src_file("data.dat", 256, "tmp", &nfiles);

  PRINT("\nOUT_0\n");
  print_file("tmp/0_data2.dat");

  PRINT("\nOUT_1\n");
  print_file("tmp/1_data2.dat");

  PRINT("\nOUT_2\n");
  print_file("tmp/2_data2.dat");

  PRINT("\nRESULT\n");
  print_file("res.dat");

  PRINT("\nEND");

  sort_file(256, "data.dat", "out.dat", "tmp");

  return EXIT_SUCCESS;
}
