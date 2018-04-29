#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <math.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

#define N (sizeof(int))

#define DIE(...) \
  fprintf(stderr, __VA_ARGS__); \
  perror("reason"); \
  exit(EXIT_FAILURE)

static FILE *ext_fopen(const char *fn, const char *mode)
{
    FILE * fp = fopen(fn, mode);
    if (!fp) {
        DIE("failed to open file: %s\n", fn);
    }
    return fp;
}

static void ext_str_reverse(char *s)
{
    int i = 0;
    int j = strlen(s) - 1;
    while (i < j) {
        char c = s[i];
        s[i] = s[j];
        s[j] = c;
        ++i;
        --j;
    }
}

static void ext_itoa(int n, char *s)
{
    int i, sign;

    if ((sign = n) < 0) {
        n = -n;
    }

    i = 0;
    do {
        s[i++] = n % 10 + '0';
    } while ((n /= 10) > 0);

    if (sign < 0) {
        s[i++] = '-';
    }

    s[i] = '\0';

    ext_str_reverse(s);
}

static size_t ext_file_size(const char *fn)
{
    FILE *f = ext_fopen(fn, "r");
    fseek(f, 0L, SEEK_END);
    size_t size = ftell(f);
    fclose(f);
    return size;
}

static inline bool ext_eq_file_sizes(const char *fn1, const char *fn2)
{
    return ext_file_size(fn1) == ext_file_size(fn2);
}

int ext_cmp_int(const void *p1, const void *p2)
{
    int v1 = *((int*) p1);
    int v2 = *((int*) p2);
    if (v1 < v2) {
        return -1;
    }
    if (v1 > v2) {
        return 1;
    }
    return 0;
}

void ext_split_file(const char *fn, long mem_nums)
{
    long fsize = ext_file_size(fn);

    long nnums = fsize / N;

    FILE *fp_r = ext_fopen(fn, "rb");

    long n = nnums / mem_nums;

    int *buf = malloc(N * mem_nums);

    char *s;

    for (long i = 0; i < n; ++i) {
        asprintf(&s, "data/_%lu", i);

        memset(buf, 0, N * mem_nums);

        int nr = fread(buf, N, mem_nums, fp_r);

        qsort(buf, nr, N, &ext_cmp_int);

        FILE *fp_w = ext_fopen(s, "wb");

        fwrite(buf, N, nr, fp_w);

        fclose(fp_w);

        free(s);
    }

    free(buf);

    fclose(fp_r);
}

void ext_merge_files(const char *fn1, const char *fn2, int depth)
{
    FILE *fp1 = ext_fopen(fn1, "rb");
    FILE *fp2 = ext_fopen(fn2, "rb");

    char *fn3;

    asprintf(&fn3, "data/%d_%s_%s", depth, fn1, fn2);

    FILE *fp3 = ext_fopen(fn3, "wb");

    free(fn3);

    long p1 = 0;
    long p2 = 0;

    while (1) {
        int n1, n2;

        fseek(fp1, p1, SEEK_SET);
        fseek(fp2, p2, SEEK_SET);

        long nr1 = fread(&n1, N, 1, fp1);
        long nr2 = fread(&n2, N, 1, fp2);

        if (nr1 > 0 && nr2 > 0) {
            if (n1 > n2) {
                fwrite(&n2, N, 1, fp3);
                p2 += nr2*N;
            } else if (n1 < n2) {
                fwrite(&n1, N, 1, fp3);
                p1 += nr1*N;
            } else {
                fwrite(&n2, N, 1, fp3);
                p1 += nr1*N;
                p2 += nr2*N;
            }
        } else if (nr1 == 0 && nr2 == 0) {
            break;
        } else if (nr1 > 0) {
            fwrite(&n1, N, 1, fp3);
            p1 += nr1*N;
        } else if (nr2 > 0) {
            fwrite(&n2, N, 1, fp3);
            p2 += nr2*N;
        }
    }

    fclose(fp3);
    fclose(fp2);
    fclose(fp1);
}

void ext_create_file(const char *fn, size_t nnums)
{
    FILE *fp = ext_fopen(fn, "wb");

    time_t t;
    srand((unsigned) time(&t));

    printf("\ncreate");

    for (size_t i = 0; i < nnums; ++i) {
        int num = rand() % 1000;
        fwrite(&num, N, 1, fp);
    }

    fclose(fp);
}

void ext_print_file(const char *fn)
{
    FILE *fp = ext_fopen(fn, "rb");

    long fsize = ext_file_size(fn);
    size_t nnums = fsize / N;

    for (long i = 0; i < nnums; ++i) {
        int n;
        fread(&n, N, 1, fp);
        printf("\n%d", n);
    }

    fclose(fp);
}

void ext_sort(const char *fn, long mem_nums)
{
    ext_split_file(fn, mem_nums);

    ext_merge_files("data/_0", "data/_1", 1);
}

int main(int argc, char** argv)
{
    long nnums = 10;
    long mem_nums = 5;

    const char *filename = "data/input.dat";

    ext_create_file(filename, nnums);

    ext_print_file(filename);

    ext_sort(filename, mem_nums);

    //e_print_file(filename);
    ext_print_file("data/1__0__1");

    return (EXIT_SUCCESS);
}