#ifndef QSA_COMMON_H
#define QSA_COMMON_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#define QSA_VPTR_TO_INT(p) (*(int *)(p))

#define QSA_VPTR_TO_LONG(p) (*(long *)(p))

#define QSA_FOREVER() while (1)

#define QSA_UNUSED(x) ((void)(x))

#define QSA_LOG_ERR(...) fprintf(stderr, __VA_ARGS__)

#define QSA_LOG_INFO(...) printf(__VA_ARGS__)

#define PRINT(...)                                                             \
  printf(__VA_ARGS__);                                                         \
  fflush(stdout);

#define DIE(...)                                                               \
  fprintf(stderr, __VA_ARGS__);                                                \
  perror("reason");                                                            \
  exit(EXIT_FAILURE)

void *qsa_malloc(size_t size);

void qsa_flush();

void *qsa_malloc0(size_t size);

void *qsa_calloc(size_t nmemb, size_t size);

void qsa_int_to_str(int n, char *s);

typedef int(qsa_cmp_fn)(const void *l, const void *r);

typedef bool(qsa_eq_fn)(const void *l, const void *r);

typedef void(qsa_action_fn)(void *data);

typedef unsigned int(qsa_hash_fn)(const void *arg);

int qsa_cmp_int(const void *l, const void *r);

bool qsa_eq_int(const void *l, const void *r);

typedef struct {
  int capacity;
  int len;
  size_t elem_size;
  void **elems;
  qsa_cmp_fn *cmp;
} qsa_heap_s;

qsa_heap_s *qsa_heap_make(int capacity, size_t elem_size, qsa_cmp_fn *cmp);
void qsa_heap_free(qsa_heap_s *h);
void qsa_heap_add(qsa_heap_s *h, void *elem);
void *qsa_heap_top(qsa_heap_s *h);
void qsa_heap_pop(qsa_heap_s *h);
bool qsa_heap_empty(qsa_heap_s *h);

void qsa_heap_enq(qsa_heap_s *h, void *elem);
void *qsa_heap_deq(qsa_heap_s *h);
void *qsa_heap_peek(qsa_heap_s *h);

#endif // QSA_COMMON_H
