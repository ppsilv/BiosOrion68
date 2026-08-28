#ifndef __STRING_H__
#define __STRING_H__

#include <inttypes.h>
#include <stddef.h>

extern void *memcpy (void *__restrict __dest, const void *__restrict __src, size_t __n) __attribute__((nonnull (1, 2)));
extern void *memmove (void *__dest, const void *__src, size_t __n)                                 __attribute__((nonnull (1, 2)));
extern void *memccpy (void *__restrict __dest, const void *__restrict __src,  int __c, size_t __n) __attribute__((nonnull (1, 2)));
extern void *memset (void *__s, int __c, size_t __n) __attribute__((nonnull (1)));
extern int memcmp (const void *__s1, const void *__s2, size_t __n);
extern void *memchr (void *__s, int __c, size_t __n) __attribute__((nonnull (1)));
extern void *memmem (const void *__haystack, size_t __haystacklen, const void *__needle, size_t __needlelen) __attribute__((nonnull (1)));


extern char *strcpy (char *__restrict __dest, const char *__restrict __src) __attribute__((nonnull (1, 2)));
extern char *strncpy (char *__restrict __dest, const char *__restrict __src, size_t __n) __attribute__((nonnull (1, 2)));
extern char *strcat (char *__restrict __dest, const char *__restrict __src)  __attribute__((nonnull (1, 2)));
extern char *strncat (char *__restrict __dest, const char *__restrict __src, size_t __n) __attribute__((nonnull (1, 2)));
extern int strcmp (const char *__s1, const char *__s2) __attribute__((nonnull (1, 2)));
extern int strncmp (const char *__s1, const char *__s2, size_t __n) __attribute__((nonnull (1, 2)));
extern size_t strlen (const char *__s) __attribute__((nonnull (1)));
extern size_t strnlen (const char *__string, size_t __maxlen) __attribute__((nonnull (1)));
extern size_t strcspn(const char *s, const char *reject)  __attribute__((nonnull (1, 2)));
extern char *strdup (const char *__s) __attribute__((nonnull (1)));
extern char *strndup (const char *__string, size_t __n) __attribute__((nonnull (1)));
extern char *strchr (char *__s, int __c) __attribute__((nonnull (1)));
extern char *strstr (const char *__haystack, const char *__needle) __attribute__((nonnull (1, 2)));
extern char *strtok (char *__restrict __s, const char *__restrict __delim) __attribute__((nonnull (1, 2)));

extern char *strerror (int __errnum);


void* malloc(size_t size);
void* calloc(size_t n, size_t size);
void free(void* ptr);

#endif

