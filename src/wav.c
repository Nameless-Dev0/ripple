#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <ctype.h>
#include "wav.h"

/* Helper to convert raw bytes (src) into an ASCII string (dest) */
static inline void bytes_to_ascii(const char *src, char *dest) {

static inline void bytes_to_ascii(const char *src, char *dest) {
    if (!src || !dest) return;

    for (size_t i = 0; i < 4; i++) {
        unsigned char c = (unsigned char)src[i];
        dest[i] = isprint(c) ? (char)c : '.'; /* '.' is a placeholder for non-printable characters */
    }
    dest[4] = '\0';
}

