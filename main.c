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

#define QSA_NUM_SIZE (sizeof(int))

struct sort_file_cfg_s {
  size_t mem_size;
  char *src_file;
  char *dest_file;
  char *out_dir;
};

struct make_file_cfg {
  char *name;
  size_t size;
  int min_value;
  int max_value;
};

/* begin prototypes */

FILE *open_or_die_file(const char *name, const char *mode);
size_t get_file_size(const char *name);
bool eq_file_sizes(const char *name1, const char *name2);
void print_file(const char *name);

FILE **split_file(const char *src_file, size_t mem_size, const char *out_dir,
                  size_t *nfiles);
void merge_files(FILE **files, size_t nfiles, const char *dest_file);
void sort_file(struct sort_file_cfg_s *cfg);

void make_source_file(const struct make_file_cfg *cfg);

/* end prototypes */

FILE *open_or_die_file(const char *name, const char *mode) {
  FILE *fp = fopen(name, mode);
  if (!fp) {
    DIE("failed to open file: %s\n", name);
  }
  return fp;
}

size_t get_file_size(const char *name) {
  FILE *fp = open_or_die_file(name, "r");
  fseek(fp, 0L, SEEK_END);
  size_t size = (size_t)ftell(fp);
  fclose(fp);
  return size;
}

bool eq_file_sizes(const char *name1, const char *name2) {
  return get_file_size(name1) == get_file_size(name2);
}

FILE **split_file(const char *src_file, size_t mem_size, const char *out_dir,
                  size_t *nfiles) {
  size_t src_file_size = get_file_size(src_file);
  size_t len = src_file_size / mem_size;
  if (src_file_size % mem_size > 0) {
    ++len;
  }
  FILE **res = qsa_malloc(len * sizeof(FILE *));
  FILE *fp_src = open_or_die_file(src_file, "rb");
  size_t buf_size = mem_size - mem_size % QSA_NUM_SIZE;
  int *buf = qsa_malloc0(buf_size);
  size_t n = 0;
  size_t i = 0;
  while ((n = fread(buf, 1, buf_size, fp_src)) > 0) {
    // qsort(buf, n, QSA_NUM_SIZE, &qsa_cmp_int);
    char *dest_name;
    asprintf(&dest_name, "%s/%zd_%s", out_dir, i, src_file);
    FILE *fp_dest = open_or_die_file(dest_name, "wb");
    fwrite(buf, QSA_NUM_SIZE, n, fp_dest);
    fseek(fp_dest, 0L, SEEK_END);
    res[i++] = fp_dest;
    memset(buf, 0, buf_size);
  }
  free(buf);
  fclose(fp_src);
  *nfiles = i;
  return res;
}

void sort_file(struct sort_file_cfg_s *cfg) {
  char *src_file = cfg->src_file;
  size_t mem_size = cfg->mem_size;
  char *out_dir = cfg->out_dir;
  size_t nfiles;
  FILE **files = split_file(src_file, mem_size, out_dir, &nfiles);
  merge_files(files, nfiles, cfg->dest_file);
  for (size_t i = 0; i < nfiles; ++i) {
    fclose(files[i]);
  }
  free(files);
}

void merge_files(FILE **files, size_t nfiles, const char *dest_file) {
  FILE *fp_dest = open_or_die_file(dest_file, "wb");
  for (size_t i = 0; i < nfiles; ++i) {
  }
  fclose(fp_dest);
}

void make_source_file(const struct make_file_cfg *cfg) {
  FILE *fp = open_or_die_file(cfg->name, "wb");
  time_t t;
  srand((unsigned)time(&t));
  size_t nnums = cfg->size / QSA_NUM_SIZE;
  for (size_t i = 0; i < nnums; ++i) {
    int num = rand() % cfg->max_value;
    fwrite(&num, QSA_NUM_SIZE, 1, fp);
  }
  fclose(fp);
}

void print_file(const char *name) {
  FILE *fp = open_or_die_file(name, "rb");
  size_t fsize = get_file_size(name);
  size_t nnums = fsize / QSA_NUM_SIZE;
  for (size_t i = 0; i < nnums; ++i) {
    int n;
    fread(&n, QSA_NUM_SIZE, 1, fp);
    printf("\n%d", n);
  }
  fclose(fp);
}

int main(int argc, char **argv) {
  /*struct make_file_cfg make_cfg;
  make_cfg.name = "data.dat";
  make_cfg.size = sizeof(int) * 100;
  make_cfg.max_value = 100;
  make_cfg.min_value = 0;

  make_source_file(&make_cfg);*/

  PRINT("DATA");
  print_file("data.dat");

  PRINT("0-OUT");
  print_file("tmp/0_data.dat");

  PRINT("1-OUT");
  print_file("tmp/1_data.dat");

  PRINT("END");

  struct sort_file_cfg_s cfg;
  cfg.mem_size = 256;
  cfg.src_file = "data.dat";
  cfg.dest_file = "out.dat";
  cfg.out_dir = "tmp";

  sort_file(&cfg);

  return EXIT_SUCCESS;
}
