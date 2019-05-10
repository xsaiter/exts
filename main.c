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

#include "common.h"

static const size_t NUM_SIZE = sizeof(int);

/* begin prototypes */

size_t size_of_file(int fd);
void print_file(const char *name);

int *split_src_file(const char *src_file, size_t mem_size, const char *out_dir,
                    size_t *nfiles);
void merge_files(int *files, size_t nfiles, const char *dest_file);
void sort_file(size_t mem_size, const char *src_file, const char *dest_file,
               const char *out_dir);

void create_src_file(const char *name, size_t size, int max_value);

/* end prototypes */

int open_or_die_file(const char *name, int flag, ...) {
  int fp = open(name, flag);
  if (fp == -1) {
    DIE("failed to open file: %s\n", name);
  }
  return fp;
}

size_t size_of_file(int fd) {
  struct stat buf;
  fstat(fd, &buf);
  return (size_t)buf.st_size;
}

int *split_src_file(const char *src_file, size_t mem_size, const char *out_dir,
                    size_t *nfiles) {
  int src_fd = open(src_file, O_RDONLY);
  if (src_fd == -1) {
    perror("open");
  }
  size_t src_file_size = size_of_file(src_fd);
  size_t len = src_file_size / mem_size;
  if (src_file_size % mem_size > 0) {
    ++len;
  }
  int *res = qsa_malloc(len * sizeof(int));
  size_t buf_size = mem_size - mem_size % NUM_SIZE;
  int *buf = qsa_malloc0(buf_size);
  size_t n = 0;
  size_t i = 0;
  while ((n = read(src_fd, buf, buf_size)) > 0) {
    qsort(buf, n, NUM_SIZE, &qsa_cmp_int);
    char *dest_name;
    asprintf(&dest_name, "%s/%zd_%s", out_dir, i, src_file);
    int fp_dest = creat(dest_name, 0644);
    write(fp_dest, buf, n);
    lseek(fp_dest, 0L, SEEK_END);
    res[i++] = fp_dest;
    memset(buf, 0, buf_size);
  }
  free(buf);
  close(src_fd);
  *nfiles = i;
  return res;
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

void merge_files(int *files, size_t nfiles, const char *dest_file) {}

void create_src_file(const char *name, size_t size, int max_value) {
  time_t t;
  srand((unsigned)time(&t));
  size_t nnums = size / NUM_SIZE;
  int fd = creat(name, 0644);
  if (fd == -1) {
    DIE("failed to open file: %s\n", name);
  }
  for (size_t i = 0; i < nnums; ++i) {
    int num = rand() % max_value;
    write(fd, &num, sizeof(num));
  }
  close(fd);
}

void print_file(const char *name) {
  int fd = open(name, O_RDONLY);
  size_t fsize = size_of_file(fd);
  size_t nnums = fsize / NUM_SIZE;
  for (size_t i = 0; i < nnums; ++i) {
    int num;
    read(fd, &num, NUM_SIZE);
    printf("\n%d", num);
  }
  close(fd);
}

int main(int argc, char **argv) {
  /*int fd = open("data.dat", O_RDONLY);
  size_t size = get_size_of_file(fd);
  size_t tmp = size;
  close(fd);*/

  create_source_file("data2.dat", 400, 100);
  size_t nfiles;
  split_src_file("data.dat", 256, "tmp", &nfiles);

  PRINT("DATA");
  print_file("data.dat");

  PRINT("0-OUT");
  print_file("tmp/0_data.dat");

  PRINT("1-OUT");
  print_file("tmp/1_data.dat");

  PRINT("END");

  sort_file(256, "data.dat", "out.dat", "tmp");

  return EXIT_SUCCESS;
}
