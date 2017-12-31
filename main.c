#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

#define NUM_SIZE (sizeof(int))

static void die(const char *s)
{
    perror(s);
    exit(EXIT_FAILURE);
}

static FILE *xfopen(const char *fname, const char *mode){
    FILE * f = fopen(fname, mode);
    if(!f){
        die(fname);
    }
    return f;
}

static void _str_reverse(char *s)
{
    int i, j;
    i = 0;
    j = strlen(s) - 1;
    while (i < j) {
        char c = s[i];
        s[i] = s[j];
        s[j] = c;
        ++i;
        --j;
    }
}

static void _itoa(int n, char *s)
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

    _str_reverse(s);
}

int size_file(const char *filename){
    FILE *f = xfopen(filename, "r");
    fseek(f, 0L, SEEK_END);
    int size = ftell(f);
    fclose(f);
    return size;
}

int file_eq_size(const char *filename1, const char *filename2){
    int size1 = size_file(filename1);
    int size2 = size_file(filename2);
    return size1 == size2;
}

int cmp_int(const void *v1, const void *v2){
    int a = *((int*)v1);
    int b = *((int*)v2);
    if(a < b){
        return -1;
    }
    if(a > b){
        return 1;
    }
    return 0;
}

void file_create(const char *filename, size_t nnums){
    FILE *f = fopen(filename, "wb");
    if(!f){
        perror(filename);
        exit(EXIT_FAILURE);
    }
    
    time_t t;
    srand((unsigned)time(&t));
    
    printf("\ncreate");
    
    for(size_t i = 0; i < nnums; ++i){
        int num = rand();
        fwrite(&num, NUM_SIZE, 1, f);
    }
    
    fclose(f);
}

void file_display(const char *filename, size_t nnums){
    FILE *f = xfopen(filename, "rb");
    
    for(size_t i = 0; i < nnums; ++i){
        int num;
        
        fread(&num, NUM_SIZE, 1, f);
        
        printf("\n%d", num);
    }
    
    fclose(f);
}

void init_split_file(const char *filename, size_t nnums, size_t mem_nums){
    FILE *f = xfopen(filename, "rb");
    
    int n = nnums / mem_nums;
    
    for(size_t i = 0; i < n; ++i){
        char s[50];
        sprintf(s, "_%d", i);
        
        int *buf = malloc(NUM_SIZE*mem_nums);
        
        int nr = fread(buf, NUM_SIZE, mem_nums, f);
        
        qsort(buf, nr, NUM_SIZE, &cmp_int);
        
        FILE *fout = xfopen(s, "wb");
        
        fwrite(buf, NUM_SIZE, nr, fout);
        
        fclose(fout);
        
        free(buf);
    }
    
    fclose(f);
}

void merge_files(const char *filename1, const char *filename2, int level, int mem_nums){
    FILE *f1 = xfopen(filename1, "rb");
    FILE *f2 = xfopen(filename2, "rb");
    
    char fname3[50];
    
    sprintf(fname3, "%d_%s_%s", level, filename1, filename2);
    
    FILE *f3 = xfopen(fname3, "wb");
    
    FILE *f;
    
    int pos1 = 0;
    int pos2 = 0;
    
    while(1){
        int n1, n2;
        
        fseek(f1, pos1, SEEK_SET);
        fseek(f2, pos2, SEEK_SET);
        
        int nr1 = fread(&n1, NUM_SIZE, 1, f1);
        int nr2 = fread(&n2, NUM_SIZE, 1, f2);
        
        if(nr1 > 0 && nr2 > 0){
            if(n1 > n2){
                fwrite(&n2, NUM_SIZE, 1, f3);
                pos2 += nr2*NUM_SIZE;
            }else if(n1 < n2){
                fwrite(&n1, NUM_SIZE, 1, f3);
                pos1 += nr1*NUM_SIZE;
            }else{
                fwrite(&n2, NUM_SIZE, 1, f3);
                pos1 += nr1*NUM_SIZE;
                pos2 += nr2*NUM_SIZE;
            }
        }else if(nr1 == 0 && nr2 == 0){
            break;
        }else if(nr1 > 0){
            fwrite(&n1, NUM_SIZE, 1, f3);
            pos1 += nr1*NUM_SIZE;
        }else if(nr2 > 0){
            fwrite(&n2, NUM_SIZE, 1, f3);
            pos2 += nr2*NUM_SIZE;
        }
    }
    
    fclose(f3);
    fclose(f2);
    fclose(f1);
}

void ext_sort(const char *filename, size_t nnums, size_t mem_nums){
    init_split_file(filename, nnums, mem_nums);
    
    merge_files("_0", "_1", 1, mem_nums);
    
    printf("success");
}


int main(int argc, char** argv)
{
    const char *filename = "_in";
    size_t nnums = 10;
    size_t mem_nums = 5;
    
    file_create(filename, nnums);
    file_display(filename, nnums);
    
    ext_sort(filename, nnums, mem_nums);
    
    printf("check...");
    
    file_display(filename, nnums);
    file_display("1__0__1", nnums);

    return (EXIT_SUCCESS);
}
