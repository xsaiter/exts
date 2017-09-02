#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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

static FILE *xfopen(const char *filename, char *modes)
{
    FILE *f = fopen(filename, modes);
    if (!f) {
        die(filename);
    }
    return f;
}

int main(int argc, char** argv)
{    
    return (EXIT_SUCCESS);
}
