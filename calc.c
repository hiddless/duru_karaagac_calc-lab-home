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

//builded paths without assuming a fixed maximum path length.
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

// we will use this to extract the final name from a file or directory patjj.

static char *part_of_path(const char *path){
    size_t n=strlen(path);

    while (n && path [n-1]== '/') n--;

    size_t start = n;

    while(start && path[start-1] != '/') start--; //using this to find where the final name starts

    char *out = malloc(n - start + 1);
    if (!out) return NULL;

    memcpy(out, path + start, n- start);  //copying the final name into out
    out [n - start]= '\0';

    return out;
    
}

static int ends_txt(const char *s){
    size_t n=strlen(s);
    return n>= 4 && strcmp(s + n -4, ".txt") == 0; //this checks if the file last 4 digits is .txt
}


static void skip_ws(Parser *p){
    while (p->at <p-> len &&
    (p->s[p->at] == ' ' ||
    p->s[p->at] == '\t' ||
    p->s[p->at] == '\r')){
        p->at++;
    }
}

// AI (ChatGPT): I used it for tokenizer and position tracking design.

static size_t position(const Parser *p){
    return p->base + p->at + 1;
}

static void fail(Parser *p, size_t pos){   //preserves the first error instead of overwriting it
    if(!p-> error) p->error =pos;
}

static Value integer(long long n) {
    Value v={0.0, n, 0};
    return v;
}

static Value floating(double n){
    Value v={n, 0, 1};
    return v;
}

static double as_double(Value v){ //converts the value to double if needed

    return v.is_float ? v.f: (double)v.i;
}

static Value expr(Parser *p);
static Value power(Parser *p);

static Value primary(Parser *p){
    skip_ws(p);  //skips whitespace chars

    if (p->at >= p->len || p->s[p->at]== '\n'){    //checking if we reached at the end of te input or newline
        fail(p, position(p));
        return integer(0);
    }

    if(p->s[p->at] == '('){
        p->at++;

        Value v=expr(p);
        if (p->error) return v;  //skipping the parenthesis

        skip_ws(p);

        if(p->at >= p->len || p->s[p->at] != ')'){
            fail(p,position(p));
        }else{
            p->at++;
        }

        return v;
    }

    if (!isdigit((unsigned char)p->s[p->at])){
        fail(p, position(p));
        return integer(0);
    }

    long long num= 0;
    size_t start = p->at;
    
    //reading digits until we reach a non-digit char
    while (p->at < p->len &&
    isdigit((unsigned char)p->s[p->at])){
        num = num * 10 + (p->s[p->at] - '0');
        p->at++;
    }
    
    
}


