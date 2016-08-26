//
// malloc.h: safe wrappers around malloc, realloc, free, strdup

#ifndef BASE_MALLOC_H
#define BASE_MALLOC_H

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus  */
void fatal(const char *fmt, ...) {
    va_list ap;
    fprintf(stderr, "FIXME: ");
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fprintf(stderr, "\n");
    exit(1);
}

// smalloc should guarantee to return a useful pointer - Halibut
// can do nothing except die when it's out of memory anyway.
void *smalloc(size_t size) {
    void *p = malloc(size);
    if (!p) {
        fatal("out of memory");
    }
    return p;
}

//srealloc should guaranteeably be able to realloc NULL
void *srealloc(void *p, size_t size) {
    void *q;
    if (p) {
        q = realloc(p, size);
    } else {
        q = malloc(size);
    }
    if (!q)
        fatal("out of memory");
    return q;
}

// sfree should guaranteeably deal gracefully with freeing NULL
void sfree(void *p) {
    if (p) {
        free(p);
    }
}

// dupstr is like strdup, but with the never-return-NULL property
// of smalloc (and also reliably defined in all environments :-)
char *dupstr(const char *s) {
    char *r = static_cast<char *>(smalloc(1+strlen(s)));
    strcpy(r,s);
    return r;
}

// snew allocates one instance of a given type, and casts the
// result so as to type-check that you're assigning it to the
// right kind of pointer. Protects against allocation bugs
// involving allocating the wrong size of thing.
#define snew(type) \
    ( (type *) smalloc (sizeof (type)) )

// snewn allocates n instances of a given type, for arrays.
#define snewn(number, type) \
    ( (type *) smalloc ((number) * sizeof (type)) )

// sresize wraps realloc so that you specify the new number of
// elements and the type of the element, with the same type-
// checking advantages. Also type-checks the input pointer.
#define sresize(array, number, type) \
    ( (void)sizeof((array)-(type *)0), \
      (type *) srealloc ((array), (number) * sizeof (type)) )

// Allocate the concatenation of N strings. Terminate arg list with NULL.
char *dupcat(const char *s1, ...) {
    int len;
    char *p, *q, *sn;
    va_list ap;

    len = strlen(s1);
    va_start(ap, s1);
    while (1) {
        sn = va_arg(ap, char *);
        if (!sn)
            break;
        len += strlen(sn);
    }
    va_end(ap);

    p = snewn(len + 1, char);
    strcpy(p, s1);
    q = p + strlen(p);

    va_start(ap, s1);
    while (1) {
        sn = va_arg(ap, char *);
        if (!sn)
            break;
        strcpy(q, sn);
        q += strlen(q);
    }
    va_end(ap);
    return p;
}
#ifdef __cplusplus
}
#endif /* __cplusplus  */
#endif /* BASE_MALLOC_H */
