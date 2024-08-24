/*
    RETOOR
*/
// Found (local) include: rmath.h
// Found (local) include: rmalloc.h
// Found (local) include: rtime.h
// Found (local) include: arena.h
// Found (local) include: rmalloc.h
// Found (local) include: rio.h
// Found (local) include: rprint.h
// Found (local) include: rtime.h
// Found (local) include: rstring.h
// Found (local) include: rmath.h
// Found (local) include: rterminal.h
// Found (local) include: rtest.h
// Found (local) include: rmalloc.h
// Found (local) include: rprint.h
// Found (local) include: rtest.h
// Found (local) include: rtree.h
// Found (local) include: rmalloc.h
// Found (local) include: rlexer.h
// Found (local) include: rstring.h
// Found (local) include: rbench.h
// Found (local) include: rprint.h
// Found (local) include: rtime.h
// Found (local) include: rstring.h
// Found (local) include: rterminal.h
#ifndef RLIB_H
#define RLIB_H
// BEGIN OF RLIB
#ifndef RMATH_H
#define RMATH_H
#include <math.h>

#ifndef ceil
double ceil(double x) {
    if (x == (double)(long long)x) {
        return x;
    } else if (x > 0.0) {
        return (double)(long long)x + 1.0;
    } else {
        return (double)(long long)x;
    }
}
#endif

#ifndef floor
double floor(double x) {
    if (x >= 0.0) {
        return (double)(long long)x;
    } else {
        double result = (double)(long long)x;
        return (result == x) ? result : result - 1.0;
    }
}
#endif

#ifndef modf
double modf(double x, double *iptr) {
    double int_part = (x >= 0.0) ? floor(x) : ceil(x);
    *iptr = int_part;
    return x - int_part;
}
#endif
#endif
#ifndef RMALLOC_H
#define RMALLOC_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned int rmalloc_count = 0;
unsigned int rmalloc_alloc_count = 0;
unsigned int rmalloc_free_count = 0;

void *rmalloc(size_t size) {
    rmalloc_count++;
    rmalloc_alloc_count++;
    return malloc(size);
}
void *rrealloc(void *obj, size_t size) { return realloc(obj, size); }
void *rfree(void *obj) {
    rmalloc_count--;
    rmalloc_free_count++;
    free(obj);
    return NULL;
}

char *rmalloc_stats() {
    static char res[100] = {0};
    sprintf(res, "Memory usage: %d allocated, %d freed, %d in use.",
            rmalloc_alloc_count, rmalloc_free_count, rmalloc_count);
    return res;
}

char *rstrdup(char *str) {

    char *res = (char *)strdup(str);
    rmalloc_alloc_count++;
    rmalloc_count++;
    return res;
}

#endif

#ifndef RLIB_TIME
#define RLIB_TIME

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#ifndef CLOCK_MONOTONIC
#define CLOCK_MONOTONIC 1
#endif

typedef unsigned long long msecs_t;
typedef uint64_t nsecs_t;

nsecs_t nsecs() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (int64_t)ts.tv_sec * 1000000000LL + (int64_t)ts.tv_nsec;
}

msecs_t rnsecs_to_msecs(nsecs_t nsecs) { return nsecs / 1000 / 1000; }

nsecs_t rmsecs_to_nsecs(msecs_t msecs) { return msecs * 1000 * 1000; }

msecs_t usecs() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (long long)(tv.tv_sec) * 1000000 + (long long)(tv.tv_usec);
}

msecs_t msecs() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (long long)(tv.tv_sec) * 1000 + (tv.tv_usec / 1000);
}
char *msecs_strs(msecs_t ms) {
    static char str[22];
    str[0] = 0;
    sprintf(str, "%f", ms * 0.001);
    for (int i = strlen(str); i > 0; i--) {
        if (str[i] > '0')
            break;
        str[i] = 0;
    }
    return str;
}
char *msecs_strms(msecs_t ms) {
    static char str[22];
    str[0] = 0;
    sprintf(str, "%lld", ms);
    return str;
}
char *msecs_str(long long ms) {
    static char result[30];
    result[0] = 0;
    if (ms > 999) {
        char *s = msecs_strs(ms);
        sprintf(result, "%ss", s);
    } else {
        char *s = msecs_strms(ms);
        sprintf(result, "%sMs", s);
    }
    return result;
}

void nsleep(long nanoseconds) {
    // long nanoseconds = (long)(1000000000 * s);

    long seconds = 0;

    struct timespec req = {seconds, nanoseconds};
    struct timespec rem;

    if (nanosleep(&req, &rem) == -1) {
        if (errno == EINTR) {
            printf("Sleep was interrupted. Remaining time: %ld.%09ld seconds\n",
                   rem.tv_sec, rem.tv_nsec);
        } else {
            perror("nanosleep");
        }
    } else {
        // printf("Slept for %ld.%09ld seconds\n", req.tv_sec, req.tv_nsec);
    }
}

void ssleep(double s) {
    long nanoseconds = (long)(1000000000 * s);

    long seconds = 0;

    struct timespec req = {seconds, nanoseconds};
    struct timespec rem;

    if (nanosleep(&req, &rem) == -1) {
        if (errno == EINTR) {
            printf("Sleep was interrupted. Remaining time: %ld.%09ld seconds\n",
                   rem.tv_sec, rem.tv_nsec);
        } else {
            perror("nanosleep");
        }
    } else {
        // printf("Slept for %ld.%09ld seconds\n", req.tv_sec, req.tv_nsec);
    }
}
void msleep(long miliseonds) {
    long nanoseconds = miliseonds * 1000000;
    nsleep(nanoseconds);
}

char *format_time(int64_t nanoseconds) {
    static char output[1024];
    size_t output_size = sizeof(output);
    output[0] = 0;
    if (nanoseconds < 1000) {
        // Less than 1 microsecond
        snprintf(output, output_size, "%ldns", nanoseconds);
    } else if (nanoseconds < 1000000) {
        // Less than 1 millisecond
        double us = nanoseconds / 1000.0;
        snprintf(output, output_size, "%.2fµs", us);
    } else if (nanoseconds < 1000000000) {
        // Less than 1 second
        double ms = nanoseconds / 1000000.0;
        snprintf(output, output_size, "%.2fms", ms);
    } else {
        // 1 second or more
        double s = nanoseconds / 1000000000.0;
        snprintf(output, output_size, "%.2fs", s);
    }
    return output;
}

#endif
#ifndef RARENA_H
#define RARENA_H

#include <assert.h>
#include <stdlib.h>
#include <string.h>

typedef struct arena_t {
    unsigned char *memory;
    unsigned int pointer;
    unsigned int size;
} arena_t;

arena_t *arena_construct() {
    arena_t *arena = (arena_t *)rmalloc(sizeof(arena_t));
    arena->memory = NULL;
    arena->pointer = 0;
    arena->size = 0;
    return arena;
}

arena_t *arena_new(size_t size) {
    arena_t *arena = arena_construct();
    arena->memory = (unsigned char *)rmalloc(size);
    arena->size = size;
    return arena;
}

void *arena_alloc(arena_t *arena, size_t size) {
    if (arena->pointer + size > arena->size) {
        return NULL;
    }
    void *p = arena->memory + arena->pointer;
    arena->pointer += size;
    return p;
}

void arena_free(arena_t *arena) {
    // Just constructed and unused arena memory is NULL so no free needed
    if (arena->memory) {
        rfree(arena->memory);
    }
    rfree(arena);
}

void arena_reset(arena_t *arena) { arena->pointer = 0; }
#endif
#ifndef RLIB_RIO
#define RLIB_RIO
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

bool rfile_exists(char *path) {
    struct stat s;
    return !stat(path, &s);
}

size_t rfile_size(char *path) {
    struct stat s;
    stat(path, &s);
    return s.st_size;
}

size_t rfile_readb(char *path, void *data, size_t size) {
    FILE *fd = fopen(path, "rb");
    if (!fd) {
        return 0;
    }
    __attribute__((unused)) size_t bytes_read =
        fread(data, size, sizeof(char), fd);

    fclose(fd);
    return size;
}

#endif
#ifndef RPRINT_H
#define RPRINT_H
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

long rpline_number = 0;
nsecs_t rprtime = 0;

int8_t _env_rdisable_colors = -1;
bool _rprint_enable_colors = true;

bool rprint_is_color_enabled() {
    if (_env_rdisable_colors == -1) {
        _env_rdisable_colors = getenv("RDISABLE_COLORS") != NULL;
    }
    if (_env_rdisable_colors) {
        _rprint_enable_colors = false;
    }
    return _rprint_enable_colors;
}

void rprint_disable_colors() { _rprint_enable_colors = false; }
void rprint_enable_colors() { _rprint_enable_colors = true; }
void rprint_toggle_colors() { _rprint_enable_colors = !_rprint_enable_colors; }

void rclear() { printf("\033[2J"); }

void rprintpf(FILE *f, const char *prefix, const char *format, va_list args) {
    char *pprefix = (char *)prefix;
    char *pformat = (char *)format;
    bool reset_color = true;
    bool press_any_key = false;
    char new_format[4096];
    bool enable_color = rprint_is_color_enabled();
    memset(new_format, 0, 4096);
    int new_format_length = 0;
    char temp[1000];
    memset(temp, 0, 1000);
    if (enable_color && pprefix[0]) {
        strcat(new_format, pprefix);
        new_format_length += strlen(pprefix);
    }
    while (true) {
        if (pformat[0] == '\\' && pformat[1] == 'i') {
            strcat(new_format, "\e[3m");
            new_format_length += strlen("\e[3m");
            pformat++;
            pformat++;
        } else if (pformat[0] == '\\' && pformat[1] == 'u') {
            strcat(new_format, "\e[4m");
            new_format_length += strlen("\e[4m");
            pformat++;
            pformat++;
        } else if (pformat[0] == '\\' && pformat[1] == 'b') {
            strcat(new_format, "\e[1m");
            new_format_length += strlen("\e[1m");
            pformat++;
            pformat++;
        } else if (pformat[0] == '\\' && pformat[1] == 'C') {
            press_any_key = true;
            rpline_number++;
            pformat++;
            pformat++;
            reset_color = false;
        } else if (pformat[0] == '\\' && pformat[1] == 'k') {
            press_any_key = true;
            rpline_number++;
            pformat++;
            pformat++;
        } else if (pformat[0] == '\\' && pformat[1] == 'c') {
            rpline_number++;
            strcat(new_format, "\e[2J\e[H");
            new_format_length += strlen("\e[2J\e[H");
            pformat++;
            pformat++;
        } else if (pformat[0] == '\\' && pformat[1] == 'L') {
            rpline_number++;
            temp[0] = 0;
            sprintf(temp, "%ld", rpline_number);
            strcat(new_format, temp);
            new_format_length += strlen(temp);
            pformat++;
            pformat++;
        } else if (pformat[0] == '\\' && pformat[1] == 'l') {
            rpline_number++;
            temp[0] = 0;
            sprintf(temp, "%.5ld", rpline_number);
            strcat(new_format, temp);
            new_format_length += strlen(temp);
            pformat++;
            pformat++;
        } else if (pformat[0] == '\\' && pformat[1] == 'T') {
            nsecs_t nsecs_now = nsecs();
            nsecs_t end = rprtime ? nsecs_now - rprtime : 0;
            temp[0] = 0;
            sprintf(temp, "%s", format_time(end));
            strcat(new_format, temp);
            new_format_length += strlen(temp);
            rprtime = nsecs_now;
            pformat++;
            pformat++;
        } else if (pformat[0] == '\\' && pformat[1] == 't') {
            rprtime = nsecs();
            pformat++;
            pformat++;
        } else {
            new_format[new_format_length] = *pformat;
            new_format_length++;
            if (!*pformat)
                break;

            // printf("%c",*pformat);
            pformat++;
        }
    }
    if (enable_color && reset_color && pprefix[0]) {
        strcat(new_format, "\e[0m");
        new_format_length += strlen("\e[0m");
    }

    new_format[new_format_length] = 0;
    vfprintf(f, new_format, args);

    fflush(stdout);
    if (press_any_key) {
        nsecs_t s = nsecs();
        fgetc(stdin);
        rprtime += nsecs() - s;
    }
}

void rprintp(char *format, ...) {
    va_list args;
    va_start(args, format);
    rprintpf(stdout, "", format, args);
    va_end(args);
}

void rprintf(FILE *f, char *format, ...) {
    va_list args;
    va_start(args, format);
    rprintpf(f, "", format, args);
    va_end(args);
}
void rprint(char *format, ...) {
    va_list args;
    va_start(args, format);
    rprintpf(stdout, "", format, args);
    va_end(args);
}

// Print line
void rprintlf(FILE *f, char *format, ...) {
    va_list args;
    va_start(args, format);
    rprintpf(f, "\\l", format, args);
    va_end(args);
}
void rprintl(char *format, ...) {
    va_list args;
    va_start(args, format);
    rprintlf(stdout, format, args);
    va_end(args);
}

// Black
void rprintkf(FILE *f, char *format, ...) {
    va_list args;
    va_start(args, format);
    rprintpf(f, "\e[30m", format, args);
    va_end(args);
}
void rprintk(char *format, ...) {
    va_list args;
    va_start(args, format);
    rprintkf(stdout, format, args);
    va_end(args);
}

// Red
void rprintrf(FILE *f, char *format, ...) {
    va_list args;
    va_start(args, format);
    rprintpf(f, "\e[31m", format, args);
    va_end(args);
}
void rprintr(char *format, ...) {
    va_list args;
    va_start(args, format);
    rprintrf(stdout, format, args);
    va_end(args);
}

// Green
void rprintgf(FILE *f, char *format, ...) {
    va_list args;
    va_start(args, format);
    rprintpf(f, "\e[32m", format, args);
    va_end(args);
}
void rprintg(char *format, ...) {
    va_list args;
    va_start(args, format);
    rprintgf(stdout, format, args);
    va_end(args);
}

// Yellow
void rprintyf(FILE *f, char *format, ...) {
    va_list args;
    va_start(args, format);
    rprintpf(f, "\e[33m", format, args);
    va_end(args);
}
void rprinty(char *format, ...) {
    va_list args;
    va_start(args, format);
    rprintyf(stdout, format, args);
    va_end(args);
}

// Blue
void rprintbf(FILE *f, char *format, ...) {
    va_list args;
    va_start(args, format);
    rprintpf(f, "\e[34m", format, args);
    va_end(args);
}

void rprintb(char *format, ...) {
    va_list args;
    va_start(args, format);
    rprintbf(stdout, format, args);
    va_end(args);
}

// Magenta
void rprintmf(FILE *f, char *format, ...) {
    va_list args;
    va_start(args, format);
    rprintpf(f, "\e[35m", format, args);
    va_end(args);
}
void rprintm(char *format, ...) {
    va_list args;
    va_start(args, format);
    rprintmf(stdout, format, args);
    va_end(args);
}

// Cyan
void rprintcf(FILE *f, char *format, ...) {
    va_list args;
    va_start(args, format);
    rprintpf(f, "\e[36m", format, args);
    va_end(args);
}
void rprintc(char *format, ...) {
    va_list args;
    va_start(args, format);
    rprintcf(stdout, format, args);
    va_end(args);
}

// White
void rprintwf(FILE *f, char *format, ...) {
    va_list args;
    va_start(args, format);
    rprintpf(f, "\e[37m", format, args);
    va_end(args);
}
void rprintw(char *format, ...) {
    va_list args;
    va_start(args, format);
    rprintwf(stdout, format, args);
    va_end(args);
}
#endif
#ifndef RSTRING_H
#define RSTRING_H
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned long _r_generate_key_current = 0;

char *rgenerate_key() {
    _r_generate_key_current++;
    static char key[100];
    key[0] = 0;
    sprintf(key, "%ld", _r_generate_key_current);
    return key;
}

char *rformat_number(long lnumber) {
    static char formatted[1024];

    char number[1024];
    sprintf(number, "%ld", lnumber);

    int len = strlen(number);
    int commas_needed = (len - 1) / 3;
    int new_len = len + commas_needed;

    formatted[new_len] = '\0';

    int i = len - 1;
    int j = new_len - 1;
    int count = 0;

    while (i >= 0) {
        if (count == 3) {
            formatted[j--] = '.';
            count = 0;
        }
        formatted[j--] = number[i--];
        count++;
    }
    return formatted;
}

bool rstrextractdouble(char *str, double *d1) {
    for (size_t i = 0; i < strlen(str); i++) {
        if (isdigit(str[i])) {
            str += i;
            sscanf(str, "%lf", d1);
            return true;
        }
    }
    return false;
}

void rstrstripslashes(const char *content, char *result) {
    size_t content_length = strlen((char *)content);
    unsigned int index = 0;
    for (unsigned int i = 0; i < content_length; i++) {
        char c = content[i];
        if (c == '\\') {
            i++;
            c = content[i];
            if (c == 'r') {
                c = '\r';
            } else if (c == 't') {
                c = '\t';
            } else if (c == 'b') {
                c = '\b';
            } else if (c == 'n') {
                c = '\n';
            } else if (c == 'f') {
                c = '\f';
            } else if (c == '\\') {
                // No need tbh
                c = '\\';
            }
        }
        result[index] = c;
        index++;
    }
    result[index] = 0;
}

int rstrstartswith(const char *s1, const char *s2) {
    if (s1 == NULL)
        return s2 == NULL;
    if (s1 == s2 || s2 == NULL || *s2 == 0)
        return true;
    size_t len_s2 = strlen(s2);
    size_t len_s1 = strlen(s1);
    if (len_s2 > len_s1)
        return false;
    return !strncmp(s1, s2, len_s2);
}

bool rstrendswith(const char *s1, const char *s2) {
    if (s1 == NULL)
        return s2 == NULL;
    if (s1 == s2 || s2 == NULL || *s2 == 0)
        return true;
    size_t len_s2 = strlen(s2);
    size_t len_s1 = strlen(s1);
    if (len_s2 > len_s1) {
        return false;
    }
    s1 += len_s1 - len_s2;
    return !strncmp(s1, s2, len_s2);
}

void rstraddslashes(const char *content, char *result) {
    size_t content_length = strlen((char *)content);
    unsigned int index = 0;
    for (unsigned int i = 0; i < content_length; i++) {
        if (content[i] == '\r') {
            result[index] = '\\';
            index++;
            result[index] = 'r';
            index++;
            continue;
        } else if (content[i] == '\t') {
            result[index] = '\\';
            index++;
            result[index] = 't';
            index++;
            continue;
        } else if (content[i] == '\n') {
            result[index] = '\\';
            index++;
            result[index] = 'n';
            index++;
            continue;
        } else if (content[i] == '\\') {
            result[index] = '\\';
            index++;
            result[index] = '\\';
            index++;
            continue;
        } else if (content[i] == '\b') {
            result[index] = '\\';
            index++;
            result[index] = 'b';
            index++;
            continue;
        } else if (content[i] == '\f') {
            result[index] = '\\';
            index++;
            result[index] = 'f';
            index++;
            continue;
        }
        result[index] = content[i];
        index++;
    }
    result[index] = 0;
}

int rstrip_whitespace(char *input, char *output) {
    output[0] = 0;
    int count = 0;
    size_t len = strlen(input);
    for (size_t i = 0; i < len; i++) {
        if (input[i] == '\t' || input[i] == ' ') {
            continue;
        }
        count = i;
        size_t j;
        for (j = 0; j < len - count; j++) {
            output[j] = input[j + count];
        }
        output[j] = '\0';
        break;
    }
    return count;
}
size_t rstrtokline(char *input, char *output, size_t offset, bool strip_nl) {

    size_t len = strlen(input);
    output[0] = 0;
    size_t new_offset = 0;
    size_t j;
    size_t index = 0;

    for (j = offset; j < len + offset; j++) {
        if (input[j] == 0) {
            index++;
            break;
        }
        index = j - offset;
        output[index] = input[j];

        if (output[index] == '\n') {
            index++;
            break;
        }
    }
    output[index] = 0;

    new_offset = index + offset;

    if (strip_nl) {
        if (output[index - 1] == '\n') {
            output[index - 1] = 0;
        }
    }
    return new_offset;
}

void rstrjoin(char **lines, size_t count, char *glue, char *output) {
    output[0] = 0;
    for (size_t i = 0; i < count; i++) {
        strcat(output, lines[i]);
        if (i != count - 1)
            strcat(output, glue);
    }
}

int rstrsplit(char *input, char **lines) {
    int index = 0;
    size_t offset = 0;
    char line[1024];
    while ((offset = rstrtokline(input, line, offset, false)) && *line) {
        if (!*line) {
            break;
        }
        lines[index] = (char *)malloc(strlen(line) + 1);
        strcpy(lines[index], line);
        index++;
    }
    return index;
}

bool rstartswithnumber(char *str) { return isdigit(str[0]); }

void rstrmove2(char *str, unsigned int start, size_t length,
               unsigned int new_pos) {
    size_t str_len = strlen(str);
    char new_str[str_len + 1];
    memset(new_str, 0, str_len);
    if (start < new_pos) {
        strncat(new_str, str + length, str_len - length - start);
        new_str[new_pos] = 0;
        strncat(new_str, str + start, length);
        strcat(new_str, str + strlen(new_str));
        memset(str, 0, str_len);
        strcpy(str, new_str);
    } else {
        strncat(new_str, str + start, length);
        strncat(new_str, str, start);
        strncat(new_str, str + start + length, str_len - start);
        memset(str, 0, str_len);
        strcpy(str, new_str);
    }
    new_str[str_len] = 0;
}

void rstrmove(char *str, unsigned int start, size_t length,
              unsigned int new_pos) {
    size_t str_len = strlen(str);
    if (start >= str_len || new_pos >= str_len || start + length > str_len) {
        return;
    }
    char temp[length + 1];
    strncpy(temp, str + start, length);
    temp[length] = 0;
    if (start < new_pos) {
        memmove(str + start, str + start + length, new_pos - start);
        strncpy(str + new_pos - length + 1, temp, length);
    } else {
        memmove(str + new_pos + length, str + new_pos, start - new_pos);
        strncpy(str + new_pos, temp, length);
    }
}

int cmp_line(const void *left, const void *right) {
    char *l = *(char **)left;
    char *r = *(char **)right;

    char lstripped[strlen(l) + 1];
    rstrip_whitespace(l, lstripped);
    char rstripped[strlen(r) + 1];
    rstrip_whitespace(r, rstripped);

    double d1, d2;
    bool found_d1 = rstrextractdouble(lstripped, &d1);
    bool found_d2 = rstrextractdouble(rstripped, &d2);

    if (found_d1 && found_d2) {
        double frac_part1;
        double int_part1;
        frac_part1 = modf(d1, &int_part1);
        double frac_part2;
        double int_part2;
        frac_part2 = modf(d2, &int_part2);
        if (d1 == d2) {
            return strcmp(lstripped, rstripped);
        } else if (frac_part1 && frac_part2) {
            return d1 > d2;
        } else if (frac_part1 && !frac_part2) {
            return 1;
        } else if (frac_part2 && !frac_part1) {
            return -1;
        } else if (!frac_part1 && !frac_part2) {
            return d1 > d2;
        }
    }
    return 0;
}

int rstrsort(char *input, char *output) {
    char **lines = (char **)malloc(strlen(input) * 10);
    int line_count = rstrsplit(input, lines);
    qsort(lines, line_count, sizeof(char *), cmp_line);
    rstrjoin(lines, line_count, "", output);
    free(lines);
    return line_count;
}

#endif
#ifndef RLIB_TERMINAL_H
#define RLIB_TERMINAL_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef RTEST_H
#define RTEST_H
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#define debug(fmt, ...) printf("%s:%d: " fmt, __FILE__, __LINE__, __VA_ARGS__);

char *rcurrent_banner;
int rassert_count = 0;
unsigned short rtest_is_first = 1;
unsigned int rtest_fail_count = 0;

int rtest_end(char *content) {
    // Returns application exit code. 0 == success
    printf("%s", content);
    printf("\n@assertions: %d\n", rassert_count);
    printf("@memory: %s\n", rmalloc_stats());

    if (rmalloc_count != 0) {
        printf("MEMORY ERROR\n");
        return rtest_fail_count > 0;
    }
    return rtest_fail_count > 0;
}

void rtest_test_banner(char *content, char *file) {
    if (rtest_is_first == 1) {
        char delimiter[] = ".";
        char *d = delimiter;
        char f[2048];
        strcpy(f, file);
        printf("%s tests", strtok(f, d));
        rtest_is_first = 0;
        setvbuf(stdout, NULL, _IONBF, 0);
    }
    printf("\n - %s ", content);
}

bool rtest_test_true_silent(char *expr, int res, int line) {
    rassert_count++;
    if (res) {
        return true;
    }
    rprintrf(stderr, "\nERROR on line %d: %s", line, expr);
    rtest_fail_count++;
    return false;
}

bool rtest_test_true(char *expr, int res, int line) {
    rassert_count++;
    if (res) {
        fprintf(stdout, ".");
        return true;
    }
    rprintrf(stderr, "\nERROR on line %d: %s", line, expr);
    rtest_fail_count++;
    return false;
}
bool rtest_test_false_silent(char *expr, int res, int line) {
    return rtest_test_true_silent(expr, !res, line);
}
bool rtest_test_false(char *expr, int res, int line) {
    return rtest_test_true(expr, !res, line);
}
void rtest_test_skip(char *expr, int line) {
    rprintgf(stderr, "\n @skip(%s) on line %d\n", expr, line);
}
bool rtest_test_assert(char *expr, int res, int line) {
    if (rtest_test_true(expr, res, line)) {
        return true;
    }
    rtest_end("");
    exit(40);
}

#define rtest_banner(content)                                                  \
    rcurrent_banner = content;                                                 \
    rtest_test_banner(content, __FILE__);
#define rtest_true(expr) rtest_test_true(#expr, expr, __LINE__);
#define rtest_assert(expr) rtest_test_true(#expr, expr, __LINE__);
#define rassert(expr) rtest_test_assert(#expr, expr, __LINE__);
#define rtest_asserts(expr) rtest_test_true_silent(#expr, expr, __LINE__);
#define rasserts(expr) rtest_test_true_silent(#expr, expr, __LINE__);
#define rtest_false(expr)                                                      \
    rprintf(" [%s]\t%s\t\n", expr == 0 ? "OK" : "NOK", #expr);                 \
    assert_count++;                                                            \
    assert(#expr);
#define rtest_skip(expr) rtest_test_skip(#expr, __LINE__);

FILE *rtest_create_file(char *path, char *content) {
    FILE *fd = fopen(path, "wb");

    char c;
    int index = 0;

    while ((c = content[index]) != 0) {
        fputc(c, fd);
        index++;
    }
    fclose(fd);
    fd = fopen(path, "rb");
    return fd;
}

void rtest_delete_file(char *path) { unlink(path); }
#endif

char *rfcaptured = NULL;

void rfcapture(FILE *f, char *buff, size_t size) {
    rfcaptured = buff;
    setvbuf(f, rfcaptured, _IOFBF, size);
}
void rfstopcapture(FILE *f) { setvbuf(f, 0, _IOFBF, 0); }

bool _r_disable_stdout_toggle = false;

FILE *_r_original_stdout = NULL;

bool rr_enable_stdout() {
    if (_r_disable_stdout_toggle)
        return false;
    if (!_r_original_stdout) {
        stdout = fopen("/dev/null", "rb");
        return false;
    }
    if (_r_original_stdout && _r_original_stdout != stdout) {
        fclose(stdout);
    }
    stdout = _r_original_stdout;
    return true;
}
bool rr_disable_stdout() {
    if (_r_disable_stdout_toggle) {
        return false;
    }
    if (_r_original_stdout == NULL) {
        _r_original_stdout = stdout;
    }
    if (stdout == _r_original_stdout) {
        stdout = fopen("/dev/null", "rb");
        return true;
    }
    return false;
}
bool rr_toggle_stdout() {
    if (!_r_original_stdout) {
        rr_disable_stdout();
        return true;
    } else if (stdout != _r_original_stdout) {
        rr_enable_stdout();
        return true;
    } else {
        rr_disable_stdout();
        return true;
    }
}

typedef struct rprogressbar_t {
    unsigned long current_value;
    unsigned long min_value;
    unsigned long max_value;
    unsigned int length;
    bool changed;
    double percentage;
    unsigned int width;
    unsigned long draws;
    FILE *fout;
} rprogressbar_t;

rprogressbar_t *rprogressbar_new(long min_value, long max_value,
                                 unsigned int width, FILE *fout) {
    rprogressbar_t *pbar = (rprogressbar_t *)malloc(sizeof(rprogressbar_t));
    pbar->min_value = min_value;
    pbar->max_value = max_value;
    pbar->current_value = min_value;
    pbar->width = width;
    pbar->draws = 0;
    pbar->length = 0;
    pbar->changed = false;
    pbar->fout = fout ? fout : stdout;
    return pbar;
}

void rprogressbar_free(rprogressbar_t *pbar) { free(pbar); }

void rprogressbar_draw(rprogressbar_t *pbar) {
    if (!pbar->changed) {
        return;
    } else {
        pbar->changed = false;
    }
    pbar->draws++;
    char draws_text[22];
    draws_text[0] = 0;
    sprintf(draws_text, "%ld", pbar->draws);
    char *draws_textp = draws_text;
    // bool draws_text_len = strlen(draws_text);
    char bar_begin_char = ' ';
    char bar_progress_char = ' ';
    char bar_empty_char = ' ';
    char bar_end_char = ' ';
    char content[4096] = {0};
    char bar_content[1024];
    char buff[2048] = {0};
    bar_content[0] = '\r';
    bar_content[1] = bar_begin_char;
    unsigned int index = 2;
    for (unsigned long i = 0; i < pbar->length; i++) {
        if (*draws_textp) {
            bar_content[index] = *draws_textp;
            draws_textp++;
        } else {
            bar_content[index] = bar_progress_char;
        }
        index++;
    }
    char infix[] = "\033[0m";
    for (unsigned long i = 0; i < strlen(infix); i++) {
        bar_content[index] = infix[i];
        index++;
    }
    for (unsigned long i = 0; i < pbar->width - pbar->length; i++) {
        bar_content[index] = bar_empty_char;
        index++;
    }
    bar_content[index] = bar_end_char;
    bar_content[index + 1] = '\0';
    sprintf(buff, "\033[43m%s\033[0m \033[33m%.2f%%\033[0m ", bar_content,
            pbar->percentage * 100);
    strcat(content, buff);
    if (pbar->width == pbar->length) {
        strcat(content, "\r");
        for (unsigned long i = 0; i < pbar->width + 10; i++) {
            strcat(content, " ");
        }
        strcat(content, "\r");
    }
    fprintf(pbar->fout, "%s", content);
    fflush(pbar->fout);
}

bool rprogressbar_update(rprogressbar_t *pbar, unsigned long value) {
    if (value == pbar->current_value) {
        return false;
    }
    pbar->current_value = value;
    pbar->percentage = (double)pbar->current_value /
                       (double)(pbar->max_value - pbar->min_value);
    unsigned long new_length = (unsigned long)(pbar->percentage * pbar->width);
    pbar->changed = new_length != pbar->length;
    if (pbar->changed) {
        pbar->length = new_length;
        rprogressbar_draw(pbar);
        return true;
    }
    return false;
}

size_t rreadline(char *data, size_t len, bool strip_ln) {
    __attribute__((unused)) char *unused = fgets(data, len, stdin);
    size_t length = strlen(data);
    if (length && strip_ln)
        data[length - 1] = 0;
    return length;
}

void rlib_test_progressbar() {
    rtest_banner("Progress bar");
    rprogressbar_t *pbar = rprogressbar_new(0, 1000, 10, stderr);
    rprogressbar_draw(pbar);
    // No draws executed, nothing to show
    rassert(pbar->draws == 0);
    rprogressbar_update(pbar, 500);
    rassert(pbar->percentage == 0.5);
    rprogressbar_update(pbar, 500);
    rprogressbar_update(pbar, 501);
    rprogressbar_update(pbar, 502);
    // Should only have drawn one time since value did change, but percentage
    // did not
    rassert(pbar->draws == 1);
    // Changed is false because update function calls draw
    rassert(pbar->changed == false);
    rprogressbar_update(pbar, 777);
    rassert(pbar->percentage == 0.777);
    rprogressbar_update(pbar, 1000);
    rassert(pbar->percentage == 1);
}

#endif
#ifndef RTREE_H
#define RTREE_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct rtree_t {
    struct rtree_t *next;
    struct rtree_t *children;
    char c;
    void *data;
} rtree_t;

rtree_t *rtree_new() {
    rtree_t *b = (rtree_t *)rmalloc(sizeof(rtree_t));
    b->next = NULL;
    b->children = NULL;
    b->c = 0;
    b->data = NULL;
    return b;
}

rtree_t *rtree_set(rtree_t *b, char *c, void *data) {
    while (b) {
        if (b->c == 0) {
            b->c = *c;
            c++;
            if (*c == 0) {
                b->data = data;
                // printf("SET1 %c\n", b->c);
                return b;
            }
        } else if (b->c == *c) {
            c++;
            if (*c == 0) {
                b->data = data;
                return b;
            }
            if (b->children) {
                b = b->children;
            } else {
                b->children = rtree_new();
                b = b->children;
            }
        } else if (b->next) {
            b = b->next;
        } else {
            b->next = rtree_new();
            b = b->next;
            b->c = *c;
            c++;
            if (*c == 0) {
                b->data = data;
                return b;
            } else {
                b->children = rtree_new();
                b = b->children;
            }
        }
    }
    return NULL;
}

rtree_t *rtree_find(rtree_t *b, char *c) {
    while (b) {
        if (b->c == *c) {
            c++;
            if (*c == 0) {
                return b;
            }
            b = b->children;
            continue;
        }
        b = b->next;
    }
    return NULL;
}

void rtree_free(rtree_t *b) {
    if (!b)
        return;
    rtree_free(b->children);
    rtree_free(b->next);
    rfree(b);
}

void *rtree_get(rtree_t *b, char *c) {
    rtree_t *t = rtree_find(b, c);
    if (t) {
        return t->data;
    }
    return NULL;
}
#endif
#ifndef RLEXER_H
#define RLEXER_H
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#define RTOKEN_VALUE_SIZE 1024

typedef enum rtoken_type_t {
    RT_UNKNOWN = 0,
    RT_SYMBOL,
    RT_NUMBER,
    RT_STRING,
    RT_PUNCT,
    RT_OPERATOR,
    RT_EOF = 10,
    RT_BRACE_OPEN,
    RT_CURLY_BRACE_OPEN,
    RT_BRACKET_OPEN,
    RT_BRACE_CLOSE,
    RT_CURLY_BRACE_CLOSE,
    RT_BRACKET_CLOSE
} rtoken_type_t;

typedef struct rtoken_t {
    rtoken_type_t type;
    char value[RTOKEN_VALUE_SIZE];
    unsigned int line;
    unsigned int col;
} rtoken_t;

static char *_content;
static unsigned int _content_ptr;
static unsigned int _content_line;
static unsigned int _content_col;

static int isgroupingchar(char c) {
    return (c == '{' || c == '}' || c == '(' || c == ')' || c == '[' ||
            c == ']' || c == '"' || c == '\'');
}

static int isoperator(char c) {
    return (c == '+' || c == '-' || c == '/' || c == '*' || c == '=' ||
            c == '>' || c == '<' || c == '|' || c == '&');
}

static rtoken_t rtoken_new() {
    rtoken_t token;
    memset(&token, 0, sizeof(token));
    token.type = RT_UNKNOWN;
    return token;
}

rtoken_t rlex_number() {
    rtoken_t token = rtoken_new();
    token.col = _content_col;
    token.line = _content_line;
    bool first_char = true;
    int dot_count = 0;
    char c;
    while (isdigit(c = _content[_content_ptr]) ||
           (first_char && _content[_content_ptr] == '-') ||
           (dot_count == 0 && _content[_content_ptr] == '.')) {
        if (c == '.')
            dot_count++;
        first_char = false;
        char chars[] = {c, 0};
        strcat(token.value, chars);
        _content_ptr++;
        _content_col++;
    }
    token.type = RT_NUMBER;
    return token;
}

static rtoken_t rlex_symbol() {
    rtoken_t token = rtoken_new();

    token.col = _content_col;
    token.line = _content_line;
    char c;
    while (isalpha(_content[_content_ptr]) || _content[_content_ptr] == '_') {
        c = _content[_content_ptr];
        char chars[] = {c, 0};
        strcat(token.value, chars);
        _content_ptr++;
        _content_col++;
    }
    token.type = RT_SYMBOL;
    return token;
}

static rtoken_t rlex_operator() {

    rtoken_t token = rtoken_new();

    token.col = _content_col;
    token.line = _content_line;
    char c;
    bool is_first = true;
    while (isoperator(_content[_content_ptr])) {
        if (!is_first) {
            if (_content[_content_ptr - 1] == '=' &&
                _content[_content_ptr] == '-') {
                break;
            }
        }
        c = _content[_content_ptr];
        char chars[] = {c, 0};
        strcat(token.value, chars);
        _content_ptr++;
        _content_col++;
        is_first = false;
    }
    token.type = RT_OPERATOR;
    return token;
}

static rtoken_t rlex_punct() {

    rtoken_t token = rtoken_new();

    token.col = _content_col;
    token.line = _content_line;
    char c;
    bool is_first = true;
    while (ispunct(_content[_content_ptr])) {
        if (!is_first) {
            if (_content[_content_ptr] == '"') {
                break;
            }
            if (_content[_content_ptr] == '\'') {
                break;
            }
            if (isgroupingchar(_content[_content_ptr])) {
                break;
            }
            if (isoperator(_content[_content_ptr])) {
                break;
            }
        }
        c = _content[_content_ptr];
        char chars[] = {c, 0};
        strcat(token.value, chars);
        _content_ptr++;
        _content_col++;
        is_first = false;
    }
    token.type = RT_PUNCT;
    return token;
}

static rtoken_t rlex_string() {
    rtoken_t token = rtoken_new();
    char c;
    token.col = _content_col;
    token.line = _content_line;
    char str_chr = _content[_content_ptr];
    _content_ptr++;
    while (_content[_content_ptr] != str_chr) {
        c = _content[_content_ptr];
        if (c == '\\') {
            _content_ptr++;
            c = _content[_content_ptr];
            if (c == 'n') {
                c = '\n';
            } else if (c == 'r') {
                c = '\r';
            } else if (c == 't') {
                c = '\t';
            } else if (c == str_chr) {
                c = str_chr;
            }

            _content_col++;
        }
        char chars[] = {c, 0};
        strcat(token.value, chars);
        _content_ptr++;
        _content_col++;
    }
    _content_ptr++;
    token.type = RT_STRING;
    return token;
}

void rlex(char *content) {
    _content = content;
    _content_ptr = 0;
    _content_col = 1;
    _content_line = 1;
}

static void rlex_repeat_str(char *dest, char *src, unsigned int times) {
    for (size_t i = 0; i < times; i++) {
        strcat(dest, src);
    }
}

rtoken_t rtoken_create(rtoken_type_t type, char *value) {
    rtoken_t token = rtoken_new();
    token.type = type;
    token.col = _content_col;
    token.line = _content_line;
    strcpy(token.value, value);
    return token;
}

rtoken_t rlex_next() {
    while (true) {

        _content_col++;

        if (_content[_content_ptr] == 0) {
            return rtoken_create(RT_EOF, "eof");
        } else if (_content[_content_ptr] == '\n') {
            _content_line++;
            _content_col = 1;
            _content_ptr++;
        } else if (isspace(_content[_content_ptr])) {
            _content_ptr++;
        } else if (isdigit(_content[_content_ptr]) ||
                   (_content[_content_ptr] == '-' &&
                    isdigit(_content[_content_ptr + 1]))) {
            return rlex_number();
        } else if (isalpha(_content[_content_ptr]) ||
                   _content[_content_ptr] == '_') {
            return rlex_symbol();
        } else if (_content[_content_ptr] == '"' ||
                   _content[_content_ptr] == '\'') {
            return rlex_string();
        } else if (isoperator(_content[_content_ptr])) {
            return rlex_operator();
        } else if (ispunct(_content[_content_ptr])) {
            if (_content[_content_ptr] == '{') {

                _content_ptr++;
                return rtoken_create(RT_CURLY_BRACE_OPEN, "{");
            }
            if (_content[_content_ptr] == '}') {

                _content_ptr++;
                return rtoken_create(RT_CURLY_BRACE_CLOSE, "}");
            }
            if (_content[_content_ptr] == '(') {

                _content_ptr++;
                return rtoken_create(RT_BRACE_OPEN, "(");
            }
            if (_content[_content_ptr] == ')') {

                _content_ptr++;
                return rtoken_create(RT_BRACE_CLOSE, ")");
            }
            if (_content[_content_ptr] == '[') {

                _content_ptr++;
                return rtoken_create(RT_BRACKET_OPEN, "[");
            }
            if (_content[_content_ptr] == ']') {

                _content_ptr++;
                return rtoken_create(RT_BRACKET_CLOSE, "]");
            }
            return rlex_punct();
        }
    }
}

char *rlex_format(char *content) {
    rlex(content);
    char *result = (char *)malloc(strlen(content) + 4096);
    result[0] = 0;
    unsigned int tab_index = 0;
    char *tab_chars = "    ";
    unsigned int col = 0;
    rtoken_t token_previous;
    token_previous.value[0] = 0;
    token_previous.type = RT_UNKNOWN;
    while (true) {
        rtoken_t token = rlex_next();
        if (token.type == RT_EOF) {
            break;
        }

        // col = strlen(token.value);

        if (col == 0) {
            rlex_repeat_str(result, tab_chars, tab_index);
            // col = strlen(token.value);// strlen(tab_chars) * tab_index;
        }

        if (token.type == RT_STRING) {
            strcat(result, "\"");

            char string_with_slashes[strlen(token.value) * 2 + 1];
            rstraddslashes(token.value, string_with_slashes);
            strcat(result, string_with_slashes);

            strcat(result, "\"");
            // col+= strlen(token.value) + 2;
            // printf("\n");
            // printf("<<<%s>>>\n",token.value);

            memcpy(&token_previous, &token, sizeof(token));
            continue;
        }
        if (!(strcmp(token.value, "{"))) {
            if (col != 0) {
                strcat(result, "\n");
                rlex_repeat_str(result, "    ", tab_index);
            }
            strcat(result, token.value);

            tab_index++;

            strcat(result, "\n");

            col = 0;

            memcpy(&token_previous, &token, sizeof(token));
            continue;
        } else if (!(strcmp(token.value, "}"))) {
            unsigned int tab_indexed = 0;
            if (tab_index)
                tab_index--;
            strcat(result, "\n");

            rlex_repeat_str(result, tab_chars, tab_index);
            tab_indexed++;

            strcat(result, token.value);
            strcat(result, "\n");
            col = 0;

            memcpy(&token_previous, &token, sizeof(token));
            continue;
        }
        if ((token_previous.type == RT_SYMBOL && token.type == RT_NUMBER) ||
            (token_previous.type == RT_NUMBER && token.type == RT_SYMBOL) ||
            (token_previous.type == RT_PUNCT && token.type == RT_SYMBOL) ||
            (token_previous.type == RT_BRACE_CLOSE &&
             token.type == RT_SYMBOL) ||
            (token_previous.type == RT_SYMBOL && token.type == RT_SYMBOL)) {
            if (token_previous.value[0] != ',' &&
                token_previous.value[0] != '.') {
                if (token.type != RT_OPERATOR && token.value[0] != '.') {
                    strcat(result, "\n");
                    rlex_repeat_str(result, tab_chars, tab_index);
                }
            }
        }

        if (token.type == RT_OPERATOR) {
            strcat(result, " ");
        }
        if (token.type == RT_STRING) {
            strcat(result, "\"");
        }
        strcat(result, token.value);
        if (token.type == RT_STRING) {
            strcat(result, "\"");
        }

        if (token.type == RT_OPERATOR) {
            strcat(result, " ");
        }
        if (!strcmp(token.value, ",")) {
            strcat(result, " ");
        }
        col += strlen(token.value);
        memcpy(&token_previous, &token, sizeof(token));
    }
    return result;
}
#endif
#ifndef RBENCH_H
#define RBENCH_H

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

struct rbench_t;

typedef struct rbench_function_t {
#ifdef __cplusplus
    void (*call)();
#else
    void(*call);
#endif
    char name[256];
    char group[256];
    void *arg;
    void *data;
    bool first;
    bool last;
    int argc;
    unsigned long times_executed;

    nsecs_t average_execution_time;
    nsecs_t total_execution_time;
} rbench_function_t;

typedef struct rbench_t {
    unsigned int function_count;
    rbench_function_t functions[100];
    rbench_function_t *current;
    rprogressbar_t *progress_bar;
    bool show_progress;
    int winner;
    bool stdout;
    unsigned long times;
    bool silent;
    nsecs_t execution_time;
#ifdef __cplusplus
    void (*add_function)(struct rbench_t *r, const char *name,
                         const char *group, void (*)());
#else
    void (*add_function)(struct rbench_t *r, const char *name,
                         const char *group, void *);
#endif
    void (*rbench_reset)(struct rbench_t *r);
    struct rbench_t *(*execute)(struct rbench_t *r, long times);
    struct rbench_t *(*execute1)(struct rbench_t *r, long times, void *arg1);
    struct rbench_t *(*execute2)(struct rbench_t *r, long times, void *arg1,
                                 void *arg2);
    struct rbench_t *(*execute3)(struct rbench_t *r, long times, void *arg1,
                                 void *arg2, void *arg3);

} rbench_t;

FILE *_rbench_stdout = NULL;
FILE *_rbench_stdnull = NULL;

void rbench_toggle_stdout(rbench_t *r) {
    if (!r->stdout) {
        if (_rbench_stdout == NULL) {
            _rbench_stdout = stdout;
        }
        if (_rbench_stdnull == NULL) {
            _rbench_stdnull = fopen("/dev/null", "wb");
        }
        if (stdout == _rbench_stdout) {
            stdout = _rbench_stdnull;
        } else {
            stdout = _rbench_stdout;
        }
    }
}
void rbench_restore_stdout(rbench_t *r) {
    if (r->stdout)
        return;
    if (_rbench_stdout) {
        stdout = _rbench_stdout;
    }
    if (_rbench_stdnull) {
        fclose(_rbench_stdnull);
        _rbench_stdnull = NULL;
    }
}

rbench_t *rbench_new();

rbench_t *_rbench = NULL;
rbench_function_t *rbf;
rbench_t *rbench() {
    if (_rbench == NULL) {
        _rbench = rbench_new();
    }
    return _rbench;
}

typedef void *(*rbench_call)();
typedef void *(*rbench_call1)(void *);
typedef void *(*rbench_call2)(void *, void *);
typedef void *(*rbench_call3)(void *, void *, void *);

#ifdef __cplusplus
void rbench_add_function(rbench_t *rp, const char *name, const char *group,
                         void (*call)()) {
#else
void rbench_add_function(rbench_t *rp, const char *name, const char *group,
                         void *call) {
#endif
    rbench_function_t *f = &rp->functions[rp->function_count];
    rp->function_count++;
    f->average_execution_time = 0;
    f->total_execution_time = 0;
    f->times_executed = 0;
    f->call = call;
    strcpy(f->name, name);
    strcpy(f->group, group);
}

void rbench_reset_function(rbench_function_t *f) {
    f->average_execution_time = 0;
    f->times_executed = 0;
    f->total_execution_time = 0;
}

void rbench_reset(rbench_t *rp) {
    for (unsigned int i = 0; i < rp->function_count; i++) {
        rbench_reset_function(&rp->functions[i]);
    }
}
int rbench_get_winner_index(rbench_t *r) {
    int winner = 0;
    nsecs_t time = 0;
    for (unsigned int i = 0; i < r->function_count; i++) {
        if (time == 0 || r->functions[i].total_execution_time < time) {
            winner = i;
            time = r->functions[i].total_execution_time;
        }
    }
    return winner;
}
bool rbench_was_last_function(rbench_t *r) {
    for (unsigned int i = 0; i < r->function_count; i++) {
        if (i == r->function_count - 1 && r->current == &r->functions[i])
            return true;
    }
    return false;
}

rbench_function_t *rbench_execute_prepare(rbench_t *r, int findex, long times,
                                          int argc) {
    rbench_toggle_stdout(r);
    if (findex == 0) {
        r->execution_time = 0;
    }
    rbench_function_t *rf = &r->functions[findex];
    rf->argc = argc;
    rbf = rf;
    r->current = rf;
    if (r->show_progress)
        r->progress_bar = rprogressbar_new(0, times, 20, stderr);
    r->times = times;
    // printf("   %s:%s gets executed for %ld times with %d
    // arguments.\n",rf->group, rf->name, times,argc);
    rbench_reset_function(rf);

    return rf;
}
void rbench_execute_finish(rbench_t *r) {
    rbench_toggle_stdout(r);
    free(r->progress_bar);
    r->progress_bar = NULL;
    r->current->average_execution_time =
        r->current->total_execution_time / r->current->times_executed;
    ;
    // printf("   %s:%s finished executing in
    // %s\n",r->current->group,r->current->name,
    // format_time(r->current->total_execution_time));
    // rbench_show_results_function(r->current);
    if (rbench_was_last_function(r)) {
        rbench_restore_stdout(r);
        unsigned int winner_index = rbench_get_winner_index(r);
        r->winner = winner_index + 1;
        if (!r->silent)
            rprintgf(stderr, "Benchmark results:\n");
        nsecs_t total_time = 0;

        for (unsigned int i = 0; i < r->function_count; i++) {
            rbf = &r->functions[i];
            total_time += rbf->total_execution_time;
            bool is_winner = winner_index == i;
            if (is_winner) {
                if (!r->silent)
                    rprintyf(stderr, " > %s:%s:%s\n",
                             format_time(rbf->total_execution_time), rbf->group,
                             rbf->name);
            } else {
                if (!r->silent)
                    rprintbf(stderr, "   %s:%s:%s\n",
                             format_time(rbf->total_execution_time), rbf->group,
                             rbf->name);
            }
        }
        if (!r->silent)
            rprintgf(stderr, "Total execution time: %s\n",
                     format_time(total_time));
    }
    rbench_restore_stdout(r);
    rbf = NULL;
    r->current = NULL;
}
struct rbench_t *rbench_execute(rbench_t *r, long times) {

    for (unsigned int i = 0; i < r->function_count; i++) {

        rbench_function_t *f = rbench_execute_prepare(r, i, times, 0);
        rbench_call c = (rbench_call)f->call;
        nsecs_t start = nsecs();
        f->first = true;
        c();
        f->first = false;
        f->last = false;
        f->times_executed++;
        for (int j = 1; j < times; j++) {
            c();
            f->times_executed++;
            f->last = f->times_executed == r->times - 1;
            if (r->progress_bar) {
                rprogressbar_update(r->progress_bar, f->times_executed);
            }
        }
        f->total_execution_time = nsecs() - start;
        r->execution_time += f->total_execution_time;
        rbench_execute_finish(r);
    }
    return r;
}

struct rbench_t *rbench_execute1(rbench_t *r, long times, void *arg1) {

    for (unsigned int i = 0; i < r->function_count; i++) {
        rbench_function_t *f = rbench_execute_prepare(r, i, times, 1);
        rbench_call1 c = (rbench_call1)f->call;
        nsecs_t start = nsecs();
        f->first = true;
        c(arg1);
        f->first = false;
        f->last = false;
        f->times_executed++;
        for (int j = 1; j < times; j++) {
            c(arg1);
            f->times_executed++;
            f->last = f->times_executed == r->times - 1;
            if (r->progress_bar) {
                rprogressbar_update(r->progress_bar, f->times_executed);
            }
        }
        f->total_execution_time = nsecs() - start;
        r->execution_time += f->total_execution_time;
        rbench_execute_finish(r);
    }
    return r;
}

struct rbench_t *rbench_execute2(rbench_t *r, long times, void *arg1,
                                 void *arg2) {

    for (unsigned int i = 0; i < r->function_count; i++) {
        rbench_function_t *f = rbench_execute_prepare(r, i, times, 2);
        rbench_call2 c = (rbench_call2)f->call;
        nsecs_t start = nsecs();
        f->first = true;
        c(arg1, arg2);
        f->first = false;
        f->last = false;
        f->times_executed++;
        for (int j = 1; j < times; j++) {
            c(arg1, arg2);
            f->times_executed++;
            f->last = f->times_executed == r->times - 1;
            if (r->progress_bar) {
                rprogressbar_update(r->progress_bar, f->times_executed);
            }
        }
        f->total_execution_time = nsecs() - start;
        r->execution_time += f->total_execution_time;
        rbench_execute_finish(r);
    }
    return r;
}

struct rbench_t *rbench_execute3(rbench_t *r, long times, void *arg1,
                                 void *arg2, void *arg3) {

    for (unsigned int i = 0; i < r->function_count; i++) {
        rbench_function_t *f = rbench_execute_prepare(r, i, times, 3);

        rbench_call3 c = (rbench_call3)f->call;
        nsecs_t start = nsecs();
        f->first = true;
        c(arg1, arg2, arg3);
        f->first = false;
        f->last = false;
        f->times_executed++;
        for (int j = 1; j < times; j++) {
            c(arg1, arg2, arg3);
            f->times_executed++;
            f->last = f->times_executed == r->times - 1;
            if (r->progress_bar) {
                rprogressbar_update(r->progress_bar, f->times_executed);
            }
        }
        f->total_execution_time = nsecs() - start;
        rbench_execute_finish(r);
    }
    return r;
}

rbench_t *rbench_new() {

    rbench_t *r = (rbench_t *)malloc(sizeof(rbench_t));
    memset(r, 0, sizeof(rbench_t));
    r->add_function = rbench_add_function;
    r->rbench_reset = rbench_reset;
    r->execute1 = rbench_execute1;
    r->execute2 = rbench_execute2;
    r->execute3 = rbench_execute3;
    r->execute = rbench_execute;
    r->stdout = true;
    r->silent = false;
    r->winner = 0;
    r->show_progress = true;
    return r;
}
void rbench_free(rbench_t *r) { free(r); }

#endif
// END OF RLIB
#endif
