#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"

void *qsa_malloc(size_t size) {
  void *p = malloc(size);
  if (!p) {
    DIE("xmalloc");
  }
  return p;
}

void *qsa_malloc0(size_t size) {
  void *p = calloc(1, size);
  if (!p) {
    DIE("xmalloc0");
  }
  return p;
}

void *qsa_calloc(size_t nmemb, size_t size) {
  void *p = calloc(nmemb, size);
  if (!p) {
    DIE("xcalloc");
  }
  return p;
}

void qsa_flush(void) { fflush(stdout); }

int qsa_cmp_int(const void *l, const void *r) {
  int lv = CONST_VOID_PTR_TO_INT(l);
  int rv = CONST_VOID_PTR_TO_INT(r);
  if (lv > rv) {
    return 1;
  }
  if (lv < rv) {
    return -1;
  }
  return 0;
}

bool qsa_eq_int(const void *l, const void *r) { return qsa_cmp_int(l, r) == 0; }

static void mirror_str(char *s) {
  size_t i = 0;
  size_t j = strlen(s) - 1;
  while (i < j) {
    char c = s[i];
    s[i] = s[j];
    s[j] = c;
    ++i;
    --j;
  }
}

void int_to_str(int n, char *s) {
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
  mirror_str(s);
}

/*
 * PQ
 */

#define PQ_LEFT(x) (2 * (x))
#define PQ_RIGHT(x) (2 * (x) + 1)
#define PQ_PARENT(x) ((x) / 2)

pq_s *pq_create(size_t capacity, size_t elem_size, qsa_cmp_fn *cmp) {
  pq_s *q = qsa_malloc(sizeof(pq_s));
  q->capacity = capacity;
  q->size = 0;
  q->elem_size = elem_size;
  q->cmp = cmp;
  q->elems = qsa_malloc(capacity * sizeof(void *));
  for (size_t i = 0; i < capacity; ++i) {
    q->elems[i] = qsa_malloc(elem_size);
  }
  return q;
}

void pq_free(pq_s *q) {
  for (size_t i = 0; i < q->size; ++i) {
    free(q->elems[i]);
  }
  free(q);
}

static bool pq_eq(pq_s *q, size_t i, size_t j) {
  return q->cmp(q->elems[i], q->elems[j]);
}

static void swap(pq_s *q, size_t i, size_t j) {
  void *t = q->elems[i];
  q->elems[i] = q->elems[j];
  q->elems[j] = t;
}

static void up(pq_s *q, size_t i) {
  while (i > 1) {
    size_t p = PQ_PARENT(i);
    if (!pq_eq(q, p, i)) {
      break;
    }
    swap(q, i, p);
    i = p;
  }
}

static void down(pq_s *q, size_t i) {
  while (true) {
    size_t l = PQ_LEFT(i);
    if (l > q->size) {
      break;
    }
    size_t r = PQ_RIGHT(i);
    if (l < q->size && pq_eq(q, l, r)) {
      l++;
    }
    if (!pq_eq(q, i, l)) {
      break;
    }
    swap(q, i, l);
    i = l;
  }
}

static void pq_add(pq_s *q, void *elem) {
  q->size++;
  memcpy(q->elems[q->size], elem, q->elem_size);
  up(q, q->size);
}

static void *pq_top(pq_s *q) {
  assert(!pq_empty(q));
  return q->elems[1];
}

static void pq_pop(pq_s *q) {
  swap(q, 1, q->size--);
  down(q, 1);
}

bool pq_empty(pq_s *q) { return q->size == 0; }

void pq_enq(pq_s *q, void *elem) { pq_add(q, elem); }

void *pq_deq(pq_s *q) {
  void *res = pq_top(q);
  pq_pop(q);
  return res;
}

void *pq_peek(pq_s *q) { return pq_top(q); }
