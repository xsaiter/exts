#include <stdio.h>
#include <stdlib.h>

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

void qsa_flush() { fflush(stdout); }

int qsa_cmp_int(const void *l, const void *r) {
  int lv = QSA_VPTR_TO_INT(l);
  int rv = QSA_VPTR_TO_INT(r);
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
  int i = 0;
  int j = strlen(s) - 1;
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
 * HEAP
 */

#define QSA_LEFT(x) (2 * (x) + 1)
#define QSA_RIGHT(x) (2 * (x) + 2)
#define QSA_PARENT(x) ((x) / 2)

qsa_heap_s *qsa_heap_make(int capacity, size_t elem_size, qsa_cmp_fn *cmp) {
  qsa_heap_s *h = qsa_malloc(sizeof(qsa_heap_s));

  h->capacity = capacity;
  h->len = 0;
  h->elem_size = elem_size;
  h->elems = qsa_malloc(h->capacity * sizeof(void *));
  for (int i = 0; i < h->capacity; ++i) {
    h->elems[i] = qsa_malloc(h->elem_size);
  }
  h->cmp = cmp;

  return h;
}

void qsa_heap_free(qsa_heap_s *h) {
  for (int i = 0; i < h->len; ++i) {
    free(h->elems[i]);
  }
  free(h);
}

inline static void swap(qsa_heap_s *h, int i, int j) {
  void *buf = h->elems[i];
  h->elems[i] = h->elems[j];
  h->elems[j] = buf;
}

static void swim(qsa_heap_s *h, int i) {
  while (i >= 0 && h->cmp(h->elems[i], h->elems[QSA_PARENT(i)]) > 0) {
    swap(h, i, QSA_PARENT(i));
    i = QSA_PARENT(i);
  }
}

static void sink(qsa_heap_s *h, int i) {
  while (QSA_LEFT(i) < h->len) {
    int l = QSA_LEFT(i);
    int r = QSA_RIGHT(i);
    int x = r;
    if (h->cmp(h->elems[l], h->elems[r]) > 0) {
      x = l;
    }
    if (h->cmp(h->elems[i], h->elems[x]) > 0) {
      break;
    }
    swap(h, i, x);
    i = x;
  }
}

void qsa_heap_add(qsa_heap_s *h, void *elem) {
  memcpy(h->elems[h->len++], elem, h->elem_size);
  swim(h, h->len - 1);
}

void *qsa_heap_top(qsa_heap_s *h) {
  assert(!qsa_heap_empty(h));
  return h->elems[0];
}

void qsa_heap_pop(qsa_heap_s *h) {
  swap(h, 0, h->len--);
  sink(h, 0);
}

bool qsa_heap_empty(qsa_heap_s *h) { return h->len == 0; }

void qsa_heap_enq(qsa_heap_s *h, void *elem) { qsa_heap_add(h, elem); }

void *qsa_heap_deq(qsa_heap_s *h) {
  void *res = qsa_heap_top(h);
  qsa_heap_pop(h);
  return res;
}

void *qsa_heap_peek(qsa_heap_s *h) { return qsa_heap_top(h); }
