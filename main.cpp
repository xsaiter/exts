#include <algorithm>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <memory>
#include <queue>
#include <string>
#include <vector>

#define PRINT(...)                                                             \
  std::printf(__VA_ARGS__);                                                    \
  std::fflush(stdout);

#define DIE(...)                                                               \
  std::fprintf(stderr, __VA_ARGS__);                                           \
  std::perror("reason");                                                       \
  exit(EXIT_FAILURE)

static const size_t NUM_SIZE = sizeof(int);

struct entry_s {
  FILE *fp;
  int num;
  bool has_num;
};

struct entry_cmp_s {
  bool operator()(entry_s *x, entry_s *y) const { return x->num > y->num; }
};

struct file_deleter_s {
  void operator()(FILE *fp) {
    if (fp) {
      fclose(fp);
    }
  }
};

using file_ptr_u = std::unique_ptr<FILE, file_deleter_s>;

FILE *open_or_die_file(const std::string &name, const char *modes) {
  FILE *fp = std::fopen(name.c_str(), modes);
  if (!fp) {
    DIE("failed to open file: %s\n", name.c_str());
  }
  return fp;
}

inline FILE *create_or_die_file(const std::string &name) {
  return open_or_die_file(name, "w+");
}

size_t size_of_file(FILE *fp) {
  std::fseek(fp, 0L, SEEK_END);
  auto res = static_cast<size_t>(std::ftell(fp));
  std::fseek(fp, 0L, SEEK_SET);
  return res;
}

std::vector<FILE *> split_file(const std::string &name, size_t mem_size,
                               const std::string &out_dir) {
  file_ptr_u fp(open_or_die_file(name, "rb+"));
  size_t fsize = size_of_file(fp.get());
  size_t len = fsize / mem_size;
  if (fsize % mem_size > 0) {
    ++len;
  }
  std::vector<FILE *> res(len);
  size_t buf_size = mem_size - mem_size % NUM_SIZE;
  size_t buf_len = buf_size / NUM_SIZE;
  std::vector<int> buf(buf_len);
  size_t n = 0;
  size_t i = 0;
  while ((n = std::fread(&buf[0], NUM_SIZE, buf_len, fp.get())) > 0) {
    std::sort(std::begin(buf), std::next(std::begin(buf), static_cast<int>(n)));
    std::string s(out_dir);
    s.append("/" + std::to_string(i) + "_");
    s.append(name);
    FILE *fp_dest = create_or_die_file(s.c_str());
    std::fwrite(&buf[0], NUM_SIZE, n, fp_dest);
    std::fseek(fp_dest, 0L, SEEK_SET);
    res[i++] = fp_dest;
    std::fill(std::begin(buf), std::end(buf), 0);
  }
  return res;
}

void create_src_file(const char *name, size_t size, int max_value) {
  std::time_t t;
  std::srand(static_cast<unsigned>(std::time(&t)));
  size_t nnums = size / NUM_SIZE;
  file_ptr_u fp(create_or_die_file(name));
  for (size_t i = 0; i < nnums; ++i) {
    int num = rand() % max_value;
    std::fwrite(&num, NUM_SIZE, 1, fp.get());
  }
}

void print_file(const char *name) {
  file_ptr_u fp(open_or_die_file(name, "rb"));
  size_t fsize = size_of_file(fp.get());
  size_t nnums = fsize / NUM_SIZE;
  for (size_t i = 0; i < nnums; ++i) {
    int num;
    std::fread(&num, NUM_SIZE, 1, fp.get());
    printf("\n%d", num);
  }
}

void merge_files(const std::vector<FILE *> &files,
                 const std::string &dest_file) {
  file_ptr_u dest_fp(create_or_die_file(dest_file));
  std::priority_queue<entry_s *, std::vector<entry_s *>, entry_cmp_s> pq;
  size_t nfiles = files.size();
  for (size_t i = 0; i < nfiles; ++i) {
    entry_s *e = new entry_s;
    e->fp = files[i];
    size_t ret = std::fread(&(e->num), NUM_SIZE, 1, files[i]);
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
    if (x) {
      pq.pop();
      std::fwrite(&(x->num), NUM_SIZE, 1, dest_fp.get());
      size_t ret = fread(&(x->num), NUM_SIZE, 1, x->fp);
      if (ret != 0) {
        x->has_num = true;
        pq.push(x);
      } else {
        x->has_num = false;
      }
    }
  }
}

void sort_file(size_t mem_size, const std::string &src_file,
               const std::string &dest_file, const std::string &out_dir) {
  auto files = split_file(src_file, mem_size, out_dir);
  merge_files(files, dest_file);
  for (FILE *file : files) {
    std::fclose(file);
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

  create_src_file("data3.dat", 20, 20);

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
