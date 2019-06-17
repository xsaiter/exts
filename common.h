#ifndef EXTS_COMMON_H
#define EXTS_COMMON_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#define CONST_VOID_PTR_TO_INT(p) (*(const int *)(p))
#define CONST_VOID_PTR_TO_LONG(p) (*(const long *)(p))

#define FOREVER() while (1)
#define UNUSED(x) ((void)(x))

#define LOG_ERR(...) fprintf(stderr, __VA_ARGS__)
#define LOG_INFO(...) printf(__VA_ARGS__)

#define PRINT(...)                                                             \
  printf(__VA_ARGS__);                                                         \
  fflush(stdout);

#define DIE(...)                                                               \
  fprintf(stderr, __VA_ARGS__);                                                \
  perror("reason");                                                            \
  exit(EXIT_FAILURE)

void *qsa_malloc(size_t size);
void *qsa_malloc0(size_t size);
void *qsa_calloc(size_t nmemb, size_t size);

typedef int(qsa_cmp_fn)(const void *l, const void *r);
typedef bool(qsa_eq_fn)(const void *l, const void *r);
typedef void(qsa_action_fn)(void *data);
typedef unsigned int(qsa_hash_fn)(const void *arg);

int qsa_cmp_int(const void *l, const void *r);
bool qsa_eq_int(const void *l, const void *r);

void qsa_flush(void);

typedef struct {
  size_t capacity;
  size_t size;
  size_t elem_size;
  void **elems;
  qsa_cmp_fn *cmp;
} pq_s;

pq_s *pq_create(size_t capacity, size_t elem_size, qsa_cmp_fn *cmp);
void pq_free(pq_s *h);
bool pq_empty(pq_s *h);
void pq_enq(pq_s *h, void *elem);
void *pq_deq(pq_s *h);
void *pq_peek(pq_s *h);

#endif // EXTS_COMMON_H
