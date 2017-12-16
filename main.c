#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#define NUM_SIZE (sizeof(int))

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

static void die(const char *s)
{
    perror(s);
    exit(EXIT_FAILURE);
}

void save(const char *file, int *buf, size_t buf_len)
{
    int fd = creat(file, S_IRWXO);
    if (fd == -1) {
        die("create");
    }
    
    int nr = write(fd, buf, buf_len);
    if(nr == -1){
        die("write");
    }
    
    close(fd);
}

int main(int argc, char** argv)
{
    int fd = open("test.txt", O_RDONLY);
    if (fd == -1) {
        die("open");
    }

    const size_t buf_len = 2048;

    ssize_t nr;

    int buf[buf_len];
    int i = 0;

    char file[10];
    
    while ((nr = read(fd, buf, buf_len * NUM_SIZE)) != 0) {        
        sprintf(file, "%d", i);
        save(file, buf, nr);
        ++i;
    }

    close(fd);

    return (EXIT_SUCCESS);
}
