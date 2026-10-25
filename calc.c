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
    if (p->at <p->len && p->s[p->at] == '.'){
        size_t dot= p->at++;

        if(p->at == p->len ||      //checking if there is a digit after our decimal pnt
        !isdigit((unsigned char)p->s[p->at])){
            fail(p,p->base + dot +1);    //recording the error position
            return integer(0);
        }

        while (p->at < p->len &&
        isdigit((unsigned char) p->s[p->at])){
            p->at++;
        }

        size_t n = p->at - start;
        char *copy = malloc(n+1);

        if (!copy) {
            fail(p, position(p));
            return integer(0);
        }

        memcpy(copy, p->s + start, n);
        copy[n] = '\0';

        double d = strtod(copy, NULL);
        free(copy);

        return floating(d);
    }

    return integer(num);
    
}

//calling power recursively on the rigt makes ** right associative. so 2 ** 3 ** 2 means 2**(3**2)
static Value power(Parser *p){
    Value left =primary(p);

    if (p->error) return left;

    skip_ws(p);

    if (p->at +1 < p->len &&
    p->s[p->at] == '*' &&
    p->s[p->at +1]== '*'){

        size_t op=position(p);
        p->at +=2;

        skip_ws(p);
        size_t operand = position(p);

        Value right =power(p);

        if (p->error) return left;

        if (!left.is_float && !right.is_float){
            //no negative integers
            if(right.i <0){
                fail(p, operand);
                return left;
            }

            long long result =1;
            long long factor = left.i;
            unsigned long long exponent = (unsigned long long)right.i;

            while(exponent){           //this avoids huge loops
                if(exponent & 1U){
                    result*=factor;
                }

                exponent >>= 1U;

                if(exponent){
                    factor*=factor;
                }
            }

            return integer(result);
        }

        double d =pow(as_double(left), as_double(right));

        //NaN and infinity are errors at the operator position
        if (!isfinite(d)){
            fail(p , op);
        }

        return floating(d);
    }

    return left;
}

//Multiplication and division bind more tightly than addition, because expr() calls term() before processing + or -
static Value term(Parser *p){
    Value left = power(p);

    while(!p->error){
        skip_ws(p);

        if (p->at >= p->len||
        (p->s[p->at] != '*' && p->s[p->at] != '/')){
            break;
        }

        size_t op_pos = position(p);
        char op = p->s[p->at++];

        if(op == '*'&&
        p->at <p->len &&
        p->s[p->at] == '*'){
            fail(p, op_pos);
            break;
        }

        skip_ws(p);
        size_t operand_pos= position(p);

        Value right = power(p);

        if(p->error) break;

        //division by 0 reports the start of the divisor
        if(op== '/' && as_double(right) == 0.0){
            fail(p, operand_pos);
            break;
        }

        if(left.is_float || right.is_float){
            double d;       //a float operand makes the entire operation use double

            if(op == '*'){
                d = as_double(left) * as_double(right);
            }else{
                d = as_double(left)/ as_double(right);
            }
            if(!isfinite(d)){
                fail(p, op_pos);
                break;
            }

            left =floating(d);
        }else{
            if (op == '*'){
                left= integer(left.i * right.i);
            }else{
                left = integer(left.i / right.i); 
            }
        }
    }

    return left;
}

static Value expr(Parser *p){
    Value left= term(p);

    while(!p->error){
        skip_ws(p);

        if (p->at >= p->len||
        (p->s[p->at] != '+' && p->s[p->at] != '-')){
            break;
        }

        size_t op_pos = position(p);
        char op = p->s[p->at++];

        Value right = term(p);

        if(p->error) break;

        if(left.is_float || right.is_float){
            double d;

            if(op == '+'){
                d = as_double(left) + as_double(right);
            }else{
                d= as_double(left) - as_double(right);
            }

            if(!isfinite(d)){
                fail(p, op_pos);
                break;
            }
            left =floating(d);
        }else{
            if(op == '+'){
                left= integer(left.i + right.i);
            }else{
                left = integer(left.i - right.i);
            }
        }
    }

    return left;

}

    static int evaluate_file(const char *input, const char *output) {
        FILE *in=fopen(input, "rb");

        if(!in){
            fprintf(stderr, "cant open input: %s\n", input);
            return 1;
        }

        FILE *temp= tmpfile();

        if (!temp){
            fprintf(stderr, "cant create temporary file\n");
            fclose(in);
            return 1;
        }

        char *line= NULL;
        size_t cap= 0;
        size_t used=0;
        size_t base=0;
        size_t error =0;

        int ch;
        int io_error=0;

        while((ch = fgetc(in)) !=EOF){
            if(used +2 > cap){
                size_t next= cap ? cap * 2:256;
                char *grown = realloc(line , next);

                if(!grown){
                    io_error = 1;
                    break;
                }

                line =grown;
                cap=next;
            }

            line[used++] = (char)ch;

            if (ch != '\n') continue;
            line[used]= '\0';

            Parser p= {line, used, 0, base, 0};
            skip_ws(&p);

            //ignoring blank lines and full line comments
            if (p.at <used&&
            line[p.at] !='#'&&
            line[p.at] != '\n'){

                Value result = expr(&p);
                skip_ws(&p);

                if(!p.error&&
                p.at < used&&
                line[p.at] != '\n'){
                    fail(&p, position(&p));
                }

                if (!p.error){
                    if(result.is_float){
                        if(fprintf(temp, "%.15g\n", result.f) < 0) {
                            io_error =1;
                        }
                    }else{
                        if(fprintf(temp,"%lld\n", result.i) <0){
                            io_error =1;
                        }
                    }
                }

                error= p.error;
            }
            base +=used;
            used=0;

            if (error || io_error) break;
        }

        if(!error && !io_error &&used){
            line[used]='\0';

            Parser p= {line, used,0,base,0};
            skip_ws(&p);

            if(p.at <used && line[p.at] != '#'){
                Value result =expr(&p);
                skip_ws(&p);

                if(!p.error && p.at <used){
                    fail(&p, position(&p));
                }

                if(!p.error){
                    if(result.is_float){
                        if(fprintf(temp, "%.15g\n", result.f) < 0){
                            io_error = 1;
                        }
                    }else{
                        if (fprintf(temp, "%lld\n", result.i) <0){
                            io_error= 1;
                        }
                    }
                }

                error= p.error;
            }
        }

        if (ferror(in)) io_error =1;

        free(line);
        fclose(in);

        if(io_error){
            fprintf(stderr, "I/O or memory error: %s\n" , input);
            fclose(temp);
            return 1;
        }

        FILE *out = fopen(output, "wb");

        if(!out){
            fprintf(stderr, "cant write output: %s\n", output);
            fclose(temp);
            return 1;
        }

        if(error) {
            if(fprintf(out, "ERROR:%zu\n", error) < 0){
                io_error =1;
            }
        }else{
            rewind(temp);
            char block[8192];
            size_t n;

            while((n=fread(block, 1, sizeof block, temp)) > 0){
                if (fwrite(block, 1, n, out) != n){
                    io_error =1;
                    break;
                }
            }

            if (ferror(temp)) io_error = 1;
        }

        if(fclose(out) !=0){
            io_error =1;
        }

        fclose(temp);

        if(io_error){
            fprintf(stderr, "cant finish writing: %s\n", output);
        }

        return io_error;
    }

    // constructinf the required output filename using student identifiers
    static int process_file(const char * input, const char *outdir){
        char *base = part_of_path(input);

        if(!base) return 1;

        if(!ends_txt(base)){
            fprintf(stderr, "Input must be .txt: %s}\n", input);
            free(base);
            return 1;
        }

        base[strlen(base) - 4]= '\0';

        size_t n = strlen(base) +
        sizeof STUDENT_NAME +
        sizeof STUDENT_LASTNAME +
        sizeof STUDENT_ID +8;

        char * filename = malloc(n);

        if(!filename){
            free(base);
            return 1;
        }

        snprintf(filename, n, "%s_%s_%s_%s.txt",
        base, STUDENT_NAME, STUDENT_LASTNAME, STUDENT_ID);

        char *destination = join_path(outdir, filename);

        free(base);
        free(filename);

        if(!destination) return 1;

        int status = evaluate_file(input, destination);

        free(destination);
        return status;
    }

    int main(int argc, char **argv){
        const char *directory= NULL;
        const char *outopt= NULL;
        const char *input= NULL;

        for(int i= 1; i <argc; i++){
            if (!strcmp(argv[i], "-d")||
            !strcmp(argv[i], "--dir")){
                if(++i >= argc || directory || input){
                fprintf(stderr, "invaild -d option \n");
                return 1;
                }

            directory= argv[i];
             } else if (!strcmp(argv[i], "-o") ||
                    !strcmp(argv[i], "--output-dir")) {

             if (++i >= argc || outopt) {
                 fprintf(stderr, "Invalid -o option\n");
                 return 1;
             }

             outopt=argv[i];
                
        
        } else if (argv[i][0] == '-' || input || directory) {
            fprintf(stderr, "Unknown or extra argument: %s\n", argv[i]);
            return 1;
        } else {
            input = argv[i];
        }
    }

    if ((!input && !directory) || (input && directory)) {
        fprintf(stderr,
                "Usage: calc [-d DIR] [-o OUTDIR] [input.txt]\n");
        return 1;
    }

    struct stat st;


         if (stat(directory ? directory : input, &st) != 0 ||
         (directory ? !S_ISDIR(st.st_mode) : !S_ISREG(st.st_mode))) {
            fprintf(stderr, "missing or invalid input: %s\n",
            directory ? directory: input);

            return 1;
    }

    char *owned_out = NULL;

    if(!outopt){
        char *base = part_of_path(directory ? directory : input);

        if(!base) return 1;

        if(!directory && ends_txt(base)){
            base[strlen(base) - 4] = '\0';
        }

        size_t n= strlen(base) +
        sizeof STUDENT_NAME +
        sizeof STUDENT_ID +4;

        owned_out = malloc(n);

        if (!owned_out) {
            free(base);
            return 1;
        }

        snprintf(owned_out, n, "%s_%s_%s", base, STUDENT_NAME,STUDENT_ID);

        free(base);
        outopt = owned_out;
    }

    if(mkdir(outopt, 0777) != 0 && errno != EEXIST){
        fprintf(stderr, "cant create output directory; %s\n", outopt);
        free(owned_out);
        return 1;
    }

    if (stat(outopt, &st) !=0 || !S_ISDIR(st.st_mode)){
        fprintf(stderr, "invalid output directory: %s\n", outopt);
        free(owned_out);
        return 1;
    } 

    int status= 0;

    if(input){
        status = process_file(input, outopt);
    }else{
        DIR *dir= opendir(directory);

        if(!dir){
            fprintf(stderr, "cant open directory: %s\n", directory);
            free(owned_out);
            return 1;
        }

        struct dirent *ent;

        //onlyy process .txt files directly inside the directory
        while ((ent= readdir(dir)) !=NULL){
            if(!ends_txt(ent->d_name))continue;

            char *path = join_path(directory, ent->d_name);

            if(!path){
                status=1;
                break;
            }

            if (stat(path, &st)== 0&&
        S_ISREG(st.st_mode)&&
    process_file(path, outopt)){
        status = 1;
    }

    free(path);

    if (status) break;
        }

        closedir(dir);
    }

    free(owned_out);
    return status;

}
