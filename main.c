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

#define NSIZE (sizeof(int))
#define NMAX 1000

#define DIE(...) \
  fprintf(stderr, __VA_ARGS__); \
  perror("reason"); \
  exit(EXIT_FAILURE)

static void mirror_str(char *s)
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

static void int_to_str(int n, char *s)
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
    mirror_str(s);
}

static int cmp_int(const void *p1, const void *p2)
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

static FILE *open_or_die_file(const char *name, const char *mode)
{
    FILE * fp = fopen(name, mode);
    if (!fp) {
        DIE("failed to open file: %s\n", name);
    }
    return fp;
}

static size_t get_file_size(const char *name)
{
    FILE *fp = open_or_die_file(name, "r");
    fseek(fp, 0L, SEEK_END);
    size_t size = ftell(fp);
    fclose(fp);
    return size;
}

static bool eq_file_sizes(const char *name1, const char *name2)
{
    return get_file_size(name1) == get_file_size(name2);
}

static void print_file(const char *name)
{
    FILE *fp = open_or_die_file(name, "rb");
    long fsize = get_file_size(name);
    size_t nnums = fsize / NSIZE;
    for (long i = 0; i < nnums; ++i) {
        int n;
        fread(&n, NSIZE, 1, fp);
        printf("\n%d", n);
    }
    fclose(fp);
}

static void merge_files(const char *name1, const char *name2)
{
    FILE *fp1 = open_or_die_file(name1, "rb");
    FILE *fp2 = open_or_die_file(name2, "rb");

    char *name3;
    asprintf(&name3, "data/_%s_%s", name1, name2);
    FILE *fp3 = open_or_die_file(name3, "wb");
    free(name3);

    size_t p1 = 0;
    size_t p2 = 0;

    while (1) {
        int n1, n2;

        fseek(fp1, p1, SEEK_SET);
        fseek(fp2, p2, SEEK_SET);

        size_t nr1 = fread(&n1, NSIZE, 1, fp1);
        size_t nr2 = fread(&n2, NSIZE, 1, fp2);

        if (nr1 > 0 && nr2 > 0) {
            if (n1 > n2) {
                fwrite(&n2, NSIZE, 1, fp3);
                p2 += nr2*NSIZE;
            } else if (n1 < n2) {
                fwrite(&n1, NSIZE, 1, fp3);
                p1 += nr1*NSIZE;
            } else {
                fwrite(&n2, NSIZE, 1, fp3);
                p1 += nr1*NSIZE;
                p2 += nr2*NSIZE;
            }
        } else if (nr1 == 0 && nr2 == 0) {
            break;
        } else if (nr1 > 0) {
            fwrite(&n1, NSIZE, 1, fp3);
            p1 += nr1*NSIZE;
        } else if (nr2 > 0) {
            fwrite(&n2, NSIZE, 1, fp3);
            p2 += nr2*NSIZE;
        }
    }

    fclose(fp3);
    fclose(fp2);
    fclose(fp1);

    remove(name2);
    remove(name1);
}

static size_t split_file(const char *name, size_t mem_size)
{
    size_t i = 0;
    size_t nnums = mem_size / NSIZE;

    FILE *fp_r = open_or_die_file(name, "rb");
    int *buf = malloc(mem_size);
    char *s;
    size_t nr = 1;
    do {
        memset(buf, 0, mem_size);
        nr = fread(buf, NSIZE, nnums, fp_r);
        if (nr > 0) {
            qsort(buf, nr, NSIZE, &cmp_int);

            asprintf(&s, "data/out/_%lu", i++);
            FILE *fp_w = open_or_die_file(s, "wb");
            fwrite(buf, NSIZE, nr, fp_w);
            fclose(fp_w);

            free(s);
        }
    } while (nr > 0);

    free(buf);
    fclose(fp_r);

    return i;
}

static void sort_file(const char *name, size_t mem_size)
{
    size_t n = split_file(name, mem_size);
    /*for (size_t i = 0; i < n; ++i) {
        merge_files("data/out/_0", "data/out/_1");
    }*/
}

static void create_and_fill_file(const char *name, size_t size)
{
    FILE *fp = open_or_die_file(name, "wb");
    time_t t;
    srand((unsigned) time(&t));
    size_t nnums = size / NSIZE;
    for (size_t i = 0; i < nnums; ++i) {
        int num = rand() % NMAX;
        fwrite(&num, NSIZE, 1, fp);
    }
    fclose(fp);
}

int main(int argc, char** argv)
{
    const char *filename = "data/input.dat";
    size_t filesize = 100 * 4;
    size_t mem_size = 256;

    create_and_fill_file(filename, filesize);
    print_file(filename);

    sort_file(filename, mem_size);

    return (EXIT_SUCCESS);
}