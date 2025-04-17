#include "try.h"
#include <setjmp.h>

error *new_error(const char *message, size_t code, const char *file_name,
                 size_t line, size_t offset) {
  if (!message)
    lfatal("Can't create error with null ptr on message string");
  if (!file_name)
    lfatal("Can't create rror with null ptr on file name string");

  error *e = (error *)malloc(sizeof(error));
  if (!e)
    lfatal("Error allocation memory for error struct");

  e->message = message;
  e->file_name = file_name;
  e->line = line;
  e->offset = offset;
  e->code = code;

  return e;
}

void free_error(error *e) {
  if (e)
    free(e);
}

void trow_error(error *e) {
  if (current_catch > 0) {
    current_error = e;
    longjmp(catches[current_catch - 1], 1);
  } else {
    fprintf(stderr, "Unhandled exception: %s at %s:%zu\n", e->message,
            e->file_name, e->line);
    exit(1);
  }
}
error *get_error() { return current_error; }
