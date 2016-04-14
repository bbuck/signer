#ifndef _signer_str_h
#define _signer_str_h

#include <stdlib.h>

typedef struct signer_str {
    char *str;
    size_t length;
    size_t offset;
    size_t capacity;
} signer_str;

signer_str *signer_str_from_cstr(char *cstr, size_t length);
signer_str *signer_str_make(size_t capacity);
void signer_str_destroy(signer_str *str);
signer_str *signer_str_concat(signer_str *dest, signer_str *src);
signer_str *signer_str_cstr_concat(signer_str *dest, char *src);
signer_str *signer_str_expand(signer_str *str);
void signer_str_reset(signer_str *str);

#endif
