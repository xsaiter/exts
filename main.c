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

int open_or_die_file(const char *name, int oflag) {
  int fd = open(name, oflag);
  if (fd == -1) {
    DIE("failed to open file: %s\n", name);
  }
  return fd;
}

int create_or_die_file(const char *name) {
  // int fd = creat(name, 0644);
  int fd = open(name, O_RDWR | O_CREAT | O_TRUNC, 0644);
  if (fd == -1) {
    DIE("failed to create file: %s\n", name);
  }
  return fd;
}

size_t size_of_file(int fd) {
  struct stat buf;
  fstat(fd, &buf);
  return (size_t)buf.st_size;
}

int *split_src_file(const char *src_file, size_t mem_size, const char *out_dir,
                    size_t *nfiles) {
  int src_fd = open_or_die_file(src_file, O_RDONLY);
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
    qsort(buf, n / NUM_SIZE, NUM_SIZE, &qsa_cmp_int);
    char *dest_name;
    asprintf(&dest_name, "%s/%zd_%s", out_dir, i, src_file);
    int fp_dest = create_or_die_file(dest_name);
    write(fp_dest, buf, n);
    off_t pos = lseek(fp_dest, 0L, SEEK_SET);
    res[i++] = fp_dest;
    memset(buf, 0, buf_size);
  }
  free(buf);
  close(src_fd);
  *nfiles = i;
  return res;
}

void create_src_file(const char *name, size_t size, int max_value) {
  time_t t;
  srand((unsigned)time(&t));
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

struct feed {
  int fd;
  int num;
  bool has_num;
};

int cmp_feed(const void *l, const void *r) {
  struct feed *lf = (struct feed *)l;
  struct feed *rf = (struct feed *)r;
  if (lf->num > rf->num) {
    return 1;
  }
  if (lf->num < rf->num) {
    return -1;
  }
  return 0;
}

void merge_files(int *files, size_t nfiles, const char *dest_file) {
  int dest_fd = create_or_die_file(dest_file);
  qsa_heap_s *pq = qsa_heap_make(16, sizeof(struct feed), &cmp_feed);
  for (size_t i = 0; i < nfiles; ++i) {
    struct feed fe;
    fe.fd = files[i];
    ssize_t c = read(files[i], &(fe.num), NUM_SIZE);
    if (c != -1L) {
      fe.has_num = true;
    } else {
      fe.has_num = false;
    }
    if (fe.has_num) {
      qsa_heap_enq(pq, &fe);
    }
  }
  while (!qsa_heap_empty(pq)) {
    struct feed *x = qsa_heap_peek(pq);
    if (x != NULL) {
      write(dest_fd, &(x->num), NUM_SIZE);
      qsa_heap_deq(pq);
      ssize_t ret = read(x->fd, &(x->num), NUM_SIZE);
      if (ret != -1 && ret != 0) {
        x->has_num = true;
        qsa_heap_enq(pq, x);
      } else {
        x->has_num = false;
      }
    }
  }
  qsa_heap_free(pq);
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

  sort_file(8, "data2.dat", "res.dat", "tmp");

  // create_src_file("data2.dat", 20, 20);

  // size_t nfiles;
  // split_src_file("data.dat", 256, "tmp", &nfiles);

  PRINT("DATA\n");
  print_file("data2.dat");

  PRINT("\nOUT");
  print_file("tmp/0_data.dat");

  PRINT("\nOUT");
  print_file("tmp/1_data.dat");

  PRINT("\nEND");

  sort_file(256, "data.dat", "out.dat", "tmp");

  return EXIT_SUCCESS;
}
