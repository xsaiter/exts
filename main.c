#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

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


static FILE *xfopen(const char *filename, const char *mode)
{
    FILE * fp = fopen(filename, mode);
    if (!fp) {
        DIE("failed to open file %s", filename);
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

static size_t get_file_size(const char *filename)
{
    FILE *f = xfopen(filename, "r");
    fseek(f, 0L, SEEK_END);
    size_t size = ftell(f);
    fclose(f);
    return size;
}

static inline bool eq_file_sizes(const char *filename1, const char *filename2)
{
    return get_file_size(filename1) == get_file_size(filename2);
}

int cmp_int(const void *p1, const void *p2)
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

void split_file(const char *filename, long mem_nums)
{
    long file_size = get_file_size(filename);

    long nnums = file_size / N;

    FILE *f = xfopen(filename, "rb");

    long n = nnums / mem_nums;

    for (long i = 0; i < n; ++i) {
        char s[50];
        sprintf(s, "_%lu", i);

        int *buf = malloc(N * mem_nums);

        int nr = fread(buf, N, mem_nums, f);

        qsort(buf, nr, N, &cmp_int);

        FILE *fout = xfopen(s, "wb");

        fwrite(buf, N, nr, fout);

        fclose(fout);

        free(buf);
    }

    fclose(f);
}

void merge_files(const char *filename1, const char *filename2, int level)
{
    FILE *fp1 = xfopen(filename1, "rb");
    FILE *fp2 = xfopen(filename2, "rb");

    char fname3[50];

    sprintf(fname3, "%d_%s_%s", level, filename1, filename2);

    FILE *fp_res = xfopen(fname3, "wb");   

    long pos1 = 0;
    long pos2 = 0;

    while (1) {
        int n1, n2;

        fseek(fp1, pos1, SEEK_SET);
        fseek(fp2, pos2, SEEK_SET);

        int nr1 = fread(&n1, N, 1, fp1);
        int nr2 = fread(&n2, N, 1, fp2);

        if (nr1 > 0 && nr2 > 0) {
            if (n1 > n2) {
                fwrite(&n2, N, 1, fp_res);
                pos2 += nr2*N;
            } else if (n1 < n2) {
                fwrite(&n1, N, 1, fp_res);
                pos1 += nr1*N;
            } else {
                fwrite(&n2, N, 1, fp_res);
                pos1 += nr1*N;
                pos2 += nr2*N;
            }
        } else if (nr1 == 0 && nr2 == 0) {
            break;
        } else if (nr1 > 0) {
            fwrite(&n1, N, 1, fp_res);
            pos1 += nr1*N;
        } else if (nr2 > 0) {
            fwrite(&n2, N, 1, fp_res);
            pos2 += nr2*N;
        }
    }

    fclose(fp_res);
    fclose(fp2);
    fclose(fp1);
}

void create_file(const char *filename, size_t nnums)
{
    FILE *fp = xfopen(filename, "wb");    

    time_t t;
    srand((unsigned) time(&t));

    printf("\ncreate");

    for (size_t i = 0; i < nnums; ++i) {
        int num = rand();
        fwrite(&num, N, 1, fp);
    }

    fclose(fp);
}

void print_file(const char *filename)
{
    FILE *fp = xfopen(filename, "rb");

    long fsize = get_file_size(filename);
    size_t nnums = fsize / N;

    for (long i = 0; i < nnums; ++i) {
        int n;
        fread(&n, N, 1, fp);
        printf("\n%d", n);
    }

    fclose(fp);
}

void ext_sort(const char *filename, long mem_nums)
{
    split_file(filename, mem_nums);

    merge_files("_0", "_1", 1);

    printf("success");
}

int main(int argc, char** argv)
{
    const char *filename = "data/_in";

    long nnums = 10;
    long mem_nums = 5;

    create_file(filename, nnums);

    print_file(filename);

    ext_sort(filename, mem_nums);

    printf("check...");

    print_file(filename);
    print_file("1__0__1");

    return (EXIT_SUCCESS);
}
