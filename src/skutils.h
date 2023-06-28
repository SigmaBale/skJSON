#ifndef __SK_UTILS_H__
#define __SK_UTILS_H__

#define is_null(object) ((object) == NULL)
#define is_some(object) ((object) != NULL)
#define discard_const(value) (void *)(value)

/* Naive implementation of strdup function because ANSI-C doesn't have it */
char *strdup_ansi(const char *src);

#endif
