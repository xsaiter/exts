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

static const size_t num_size = sizeof(int);
using size_u = std::size_t;

struct file_deleter_s {
  void operator()(FILE *fp) {
    if (fp) {
      fclose(fp);
    }
  }
};

using file_ptr_u = std::unique_ptr<FILE, file_deleter_s>;
using files_ptr_u = std::vector<file_ptr_u>;

file_ptr_u open_or_die_file(const std::string &name, const std::string &modes) {
  FILE *fp = std::fopen(name.c_str(), modes.c_str());
  if (!fp) {
    DIE("failed to open file: %s\n", name.c_str());
  }
  return file_ptr_u(fp);
}

inline file_ptr_u create_or_die_file(const std::string &name) {
  return open_or_die_file(name, "w+");
}

size_u size_of_file(FILE *fp) {
  std::fseek(fp, 0L, SEEK_END);
  auto res = static_cast<size_u>(std::ftell(fp));
  std::fseek(fp, 0L, SEEK_SET);
  return res;
}

void create_src_file(const std::string &name, size_u size, int max_value) {
  std::time_t t;
  std::srand(static_cast<unsigned>(std::time(&t)));
  size_u nnums = size / num_size;
  auto fp = create_or_die_file(name);
  for (size_u i = 0; i < nnums; ++i) {
    auto num = rand() % max_value;
    std::fwrite(&num, num_size, 1, fp.get());
  }
}

void print_file(const std::string &name) {
  auto fp = open_or_die_file(name, "rb");
  auto fsize = size_of_file(fp.get());
  auto nnums = fsize / num_size;
  for (size_u i = 0; i < nnums; ++i) {
    int num;
    std::fread(&num, num_size, 1, fp.get());
    printf("\n%d", num);
  }
}

struct entry_s {
  FILE *fp;
  int num;
  bool has_num;
};

using entry_ptr_u = std::shared_ptr<entry_s>;

struct entry_cmp_s {
  bool operator()(const entry_ptr_u &x, const entry_ptr_u y) const {
    return x->num > y->num;
  }
};

void merge_files(const files_ptr_u &files, const std::string &out_file) {
  std::priority_queue<entry_ptr_u, std::vector<entry_ptr_u>, entry_cmp_s> pq;
  const size_u nfiles = files.size();
  for (size_u i = 0; i < nfiles; ++i) {
    auto e = std::make_shared<entry_s>();
    e->fp = files[i].get();
    auto ret = std::fread(&(e->num), num_size, 1, files[i].get());
    if (ret != 0) {
      e->has_num = true;
    } else {
      e->has_num = false;
    }
    if (e->has_num) {
      pq.push(e);
    }
  }

  auto out_fp = create_or_die_file(out_file);

  while (!pq.empty()) {
    auto x = pq.top();
    if (x) {
      pq.pop();
      std::fwrite(&(x->num), num_size, 1, out_fp.get());
      auto ret = fread(&(x->num), num_size, 1, x->fp);
      if (ret != 0) {
        x->has_num = true;
        pq.push(x);
      } else {
        x->has_num = false;
      }
    }
  }
}

files_ptr_u split_file(const std::string &name, size_u mem_size,
                       const std::string &tmp_dir) {
  auto fp = open_or_die_file(name, "rb+");
  auto fsize = size_of_file(fp.get());
  auto len = fsize / mem_size;
  if (fsize % mem_size > 0) {
    ++len;
  }
  files_ptr_u res(len);
  auto buf_size = mem_size - mem_size % num_size;
  auto buf_len = buf_size / num_size;
  std::vector<int> buf(buf_len);
  size_u n = 0;
  size_u i = 0;
  while ((n = std::fread(&buf[0], num_size, buf_len, fp.get())) > 0) {
    std::sort(std::begin(buf), std::next(std::begin(buf), static_cast<int>(n)));
    std::string s(tmp_dir);
    s.append("/" + std::to_string(i) + "_");
    s.append(name);
    auto tmp_fp = create_or_die_file(s);
    std::fwrite(&buf[0], num_size, n, tmp_fp.get());
    std::fseek(tmp_fp.get(), 0L, SEEK_SET);
    res[i++] = std::move(tmp_fp);
    std::fill(std::begin(buf), std::end(buf), 0);
  }
  return res;
}

void sort_file(size_u mem_size, const std::string &file,
               const std::string &out_file, const std::string &tmp_dir) {
  auto files = split_file(file, mem_size, tmp_dir);
  merge_files(files, out_file);
}

int main(int argc, char **argv) {
  PRINT("DATA\n");
  print_file("data2.dat");
  PRINT("DATA END\n");

  // create_src_file("data3.dat", 20, 20);

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
