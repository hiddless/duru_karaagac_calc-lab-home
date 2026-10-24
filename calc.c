// Name Lastname StudentID Duru Karaagac 251ADB089
// (optional) https://github.com/hiddless/duru_karaagac_calc-lab-home
// Compile with: gcc -O2 -Wall -Wextra -std=c17 -o calc calc.c -lm
// Resources used: <every AI tool, website and book you used; details in comments where you used them>
// Resources i mostly used RTU Programming Languages lectures 2,3,4,5,6 presentations.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>

#define STUDENT_NAME "duru"
#define STUDENT_LASTNAME "karaagac"
#define STUDENT_ID "251ADB089"

typedef struct 
{
    double f;
    long long i;
    int is_float;
}Value;

typedef struct {
    const char *s;
    size_t len;
    size_t at;
    size_t base;
    size_t error;
}Parser;

static char *join_path(const char *dir, const char *file){
    size_t a= strlen(dir);
    size_t b= strlen(file);

    char *s= malloc(a +b +2);  // we use it to allocate memory for both strings the / seperator and the null terminator \0
    if (!s) return NULL;

    memcpy(s, dir, a);

    if(a && dir[a-1] != '/'){    //for dir string length
        s[a++]= '/';
    }

    memcpy(s + a, file, b + 1);    //for file string length
    return s;
}




