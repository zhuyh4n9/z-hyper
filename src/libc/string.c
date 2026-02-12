#include <string.h>
#include <stdint.h>
// TODO: Optimize these functions
void *memcpy(void *dest, const void *src, size_t n)
{
    uint64_t steps = n >> 3;
    uint64_t *d64 = (uint64_t *)dest;
    const uint64_t *s64 = (const uint64_t *)src;

    for (uint64_t i = 0; i < steps; i++) {
        d64[i] = s64[i];
    }
    size_t remaining = n & 0x7;
    unsigned char *d8 = (unsigned char *)(d64 + steps);
    const unsigned char *s8 = (const unsigned char *)(s64 + steps);
    for (size_t i = 0; i < remaining; i++) {
        d8[i] = s8[i];
    }
    return dest;
}

// TODO: Optimize these functions
void *memset(void *s, int c, size_t n)
{
    uint64_t steps = n >> 3;
    uint64_t byte_c = (unsigned char)c;
    uint64_t word_c = (byte_c << 56) | (byte_c << 48) | (byte_c << 40) | (byte_c << 32) |
                      (byte_c << 24) | (byte_c << 16) | (byte_c << 8) | byte_c;
    uint64_t *p64 = (uint64_t *)s;

    for (uint64_t i = 0; i < steps; i++) {
        p64[i] = word_c;
    }
    size_t remaining = n & 0x7;
    unsigned char *p8 = (unsigned char *)(p64 + steps);
    for (size_t i = 0; i < remaining; i++) {
        p8[i] = (unsigned char)c;
    }
    return s;

}

int strcmp(const char *s1, const char *s2)
{
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char *)s1 - *(unsigned char *)s2;
}

int strncmp(const char *s1, const char *s2, size_t n)
{
    for (size_t i = 0; i < n; i++) {
        if (s1[i] != s2[i]) {
            return (unsigned char)s1[i] - (unsigned char)s2[i];
        }
        if (s1[i] == '\0') {
            return 0;
        }
    }
    return 0;
}

size_t strlen(const char *s)
{
    size_t len = 0;
    while (s[len] != '\0') {
        len++;
    }
    return len;
}

char *strcpy(char *dest, const char *src)
{
    char *d = dest;
    while ((*d++ = *src++) != '\0') {
        ;
    }
    return dest;
}

char *strncpy(char *dest, const char *src, size_t n)
{
    char *d = dest;
    size_t i;
    for (i = 0; i < n && src[i] != '\0'; i++) {
        d[i] = src[i];
    }
    for (; i < n; i++) {
        d[i] = '\0';
    }
    return dest;
}

char *strcat(char *dest, const char *src)
{
    char *d = dest;
    while (*d != '\0') {
        d++;
    }
    while ((*d++ = *src++) != '\0') {
        ;
    }
    return dest;
}

char *strncat(char *dest, const char *src, size_t n)
{
    char *d = dest;
    while (*d != '\0') {
        d++;
    }
    size_t i;
    for (i = 0; i < n && src[i] != '\0'; i++) {
        d[i] = src[i];
    }
    d[i] = '\0';
    return dest;
}

char *strstr(const char *haystack, const char *needle)
{
    if (*needle == '\0') {
        return (char *)haystack;
    }
    for (; *haystack != '\0'; haystack++) {
        if (*haystack == *needle) {
            const char *h = haystack;
            const char *n = needle;
            while (*h != '\0' && *n != '\0' && *h == *n) {
                h++;
                n++;
            }
            if (*n == '\0') {
                return (char *)haystack;
            }
        }
    }
    return NULL;
}