#include <algorithm>
#include <array>
#include <functional>
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
  FILE *file;
  int num;
  bool has_num;
};

struct entry_cmp_s {
  bool operator()(entry_s *x, entry_s *y) { return x->num > y->num; }
};

FILE *open_or_die_file(const char *name, const char *modes) {
  FILE *f = fopen(name, modes);
  if (!f) {
    DIE("failed to open file: %s\n", name);
  }
  return f;
}

FILE *create_or_die_file(const char *name) {
  FILE *f = fopen(name, "w+");
  if (!f) {
    DIE("failed to create file: %s\n", name);
  }
  return f;
}

size_t size_of_file(FILE *f) {
  fseek(f, 0L, SEEK_END);
  auto res = static_cast<size_t>(ftell(f));
  fseek(f, 0L, SEEK_SET);
  return res;
}

std::vector<FILE *> split_file(const char *name, size_t mem_size,
                               const char *out_dir) {
  FILE *fp = open_or_die_file(name, "rb+");
  size_t fsize = size_of_file(fp);
  size_t len = fsize / mem_size;
  if (fsize % mem_size > 0) {
    ++len;
  }
  std::vector<FILE *> res(len);
  size_t buf_size = (mem_size - mem_size % NUM_SIZE);
  size_t buf_len = buf_size / NUM_SIZE;
  int *buf = new int[buf_len];
  size_t n = 0;
  size_t i = 0;
  while ((n = fread(buf, NUM_SIZE, buf_len, fp)) > 0) {
    std::sort(buf, buf + n);
    char *dest_name;
    asprintf(&dest_name, "%s/%zd_%s", out_dir, i, name);
    FILE *fp_dest = create_or_die_file(dest_name);
    fwrite(buf, NUM_SIZE, n, fp_dest);
    fseek(fp_dest, 0L, SEEK_SET);
    res[i++] = fp_dest;
    memset(buf, 0, buf_size);
  }
  delete[] buf;
  fclose(fp);
  return res;
}

void create_src_file(const char *name, size_t size, int max_value) {
  time_t t;
  srand(static_cast<unsigned>(time(&t)));
  size_t nnums = size / NUM_SIZE;
  FILE *fd = create_or_die_file(name);
  for (size_t i = 0; i < nnums; ++i) {
    int num = rand() % max_value;
    fwrite(&num, NUM_SIZE, 1, fd);
  }
  fclose(fd);
}

void print_file(const char *name) {
  FILE *fd = open_or_die_file(name, "rb");
  size_t fsize = size_of_file(fd);
  size_t nnums = fsize / NUM_SIZE;
  for (size_t i = 0; i < nnums; ++i) {
    int num;
    fread(&num, NUM_SIZE, 1, fd);
    printf("\n%d", num);
  }
  fclose(fd);
}

void merge_files(std::vector<FILE *> files, const char *dest_file) {
  FILE *dest_fd = create_or_die_file(dest_file);
  std::priority_queue<entry_s *, std::vector<entry_s *>, entry_cmp_s> pq;
  std::size_t nfiles = files.size();
  for (size_t i = 0; i < nfiles; ++i) {
    entry_s *e = new entry_s;
    e->file = files[i];
    size_t ret = fread(&(e->num), NUM_SIZE, 1, files[i]);
    if (ret != 0) {
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
      fwrite(&(x->num), NUM_SIZE, 1, dest_fd);
      size_t ret = fread(&(x->num), NUM_SIZE, 1, x->file);
      if (ret != 0) {
        x->has_num = true;
        pq.push(x);
      } else {
        x->has_num = false;
      }
    }
  }
  fclose(dest_fd);
}

void sort_file(size_t mem_size, const char *src_file, const char *dest_file,
               const char *out_dir) {
  auto files = split_file(src_file, mem_size, out_dir);
  merge_files(files, dest_file);
  for (FILE *file : files) {
    fclose(file);
  }
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
