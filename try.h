#ifndef TRY_C_H
#define TRY_C_H

#include <setjmp.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

// -------------- CLCP --------------
typedef enum {
    LEVEL_FATAL = 0,
    LEVEL_WARN  = 1,
    LEVEL_DEBUG = 2,
    LEVEL_TODO  = 3,
    LEVEL_UNIMPLEMENTED = 4
} LogLevel;

static const char* const LEVEL_NAMES[] = {
    "FATAL", "WARN", "DEBUG", "TODO", "UNIMPLEMENTED"
};

static inline void lmessage(LogLevel level, const char* format, ...) {
    fprintf(stderr, "[%s] ", LEVEL_NAMES[level]);
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fprintf(stderr, "\n");
    if (level == LEVEL_FATAL || level == LEVEL_UNIMPLEMENTED) {
        exit(1);
    }
}

#define lfatal(...) lmessage(LEVEL_FATAL, __VA_ARGS__)
#define lwarn(...)  lmessage(LEVEL_WARN, __VA_ARGS__)
#define ldebug(...) lmessage(LEVEL_DEBUG, __VA_ARGS__)
#define ltodo(...)  lmessage(LEVEL_TODO, __VA_ARGS__)
#define lunimp(...) lmessage(LEVEL_UNIMPLEMENTED, "Not implemented: " __VA_ARGS__)
// -----------------------

#ifndef MAX_TRY_COUNT
#define MAX_TRY_COUNT 20
#endif

typedef struct error {
    const char *message;
    const char *file_name;
    size_t code;
    size_t line;
    size_t offset;
} error;

static jmp_buf catches[MAX_TRY_COUNT];
static size_t current_catch = 0;
static error *current_error = NULL;

error *new_error(const char *message, const size_t code, const char* file_name,size_t line, size_t offset);
void free_error(error *e);
void trow_error(error *e);
error *get_error();

#ifdef USE_ERROR_HANDLING
    #define main application_main
    int application_main(int argc, char** argv);
    #define IMPLEMENT_ERROR_HANDLING_MAIN() \
    int main(int argc, char** argv) { \
        try { \
            return application_main(argc, argv); \
        } catch { \
            error* err = get_error(); \
            fprintf(stderr, "Fatal error: %s at %s:%zu:%zu\n", \
                    err->message, err->file_name, err->line, err->offset); \
            free_error(err); \
            return 1; \
        } tryend \
    }
#else
    #define IMPLEMENT_ERROR_HANDLING_MAIN()
#endif

#define try do { \
    if (current_catch + 1 > MAX_TRY_COUNT){ \
        fprintf(stderr, "Can't add new catch, have a limit %d", MAX_TRY_COUNT); \
        exit(1); \
    } \
    current_catch++; \
    if(setjmp(catches[current_catch - 1]) == 0) {

#define catch } else { \
    error *e = get_error();

#define tryend } \
    current_catch--; \
} while (0);

#define trow(message , code) do { \
    error *e = new_error(message, code, __FILE__, __LINE__, 0); \
    trow_error(e); \
} while (0);

#define trow_with_offset(message, code, offset) do { \
    error *e = new_error(message, code, __FILE__, __LINE__, offset); \
    trow_error(e); \
} while (0)

#define error_go_top(error) trow_error(error);

#define get_code(code_var) do { \
    error *er = get_error(); \
    size_t code_var = er->code; \
} while (0);

#define log_error() do { \
 error* err = get_error(); \
 fprintf(stderr, "error: %s at %s:%zu:%zu\n", \
                err->message, err->file_name, err->line, err->offset); \
 free_error(err); \
} while (0);

#endif
