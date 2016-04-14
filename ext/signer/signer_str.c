#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "signer_str.h"
#include "util.h"

signer_str *signer_str_from_cstr(char *cstr, size_t length) {
    signer_str *str = signer_str_make(length);
    check_mem(str);

    str->str = memcpy(str->str, cstr, length);
    check_mem(str->str);
    str->str[length] = '\0';
    str->length = length;

    return str;

error:
    if (str->str != NULL) {
        free(str->str);
    }

    if (str != NULL) {
        free(str);
    }

    return NULL;
}

signer_str *signer_str_make(size_t capacity) {
    signer_str *str = malloc(sizeof(signer_str));
    check_mem(str);

    str->str = malloc(sizeof(char) * (capacity + 1));
    check_mem(str->str);
    str->str[0] = '\0';
    str->str[capacity] = '\0';
    str->length = 0;
    str->capacity = capacity;
    str->offset = 0;

    return str;

error:
    if (str->str != NULL) {
        free(str->str);
    }

    if (str != NULL) {
        free(str);
    }

    return NULL;
}

void signer_str_destroy(signer_str *str) {
    signer_str_reset(str);
    if (str != NULL) {
        if (str->str != NULL) {
            free(str->str);
        }

        free(str);
    }
}

signer_str *signer_str_concat(signer_str *dest, signer_str *src) {
    if (dest == NULL || src == NULL) {
        return dest;
    }

    while (*dest->str) {
        dest->str++;
        dest->offset++;
    }
    src->str -= src->offset;
    while (*src->str) {
        if (dest->length >= dest->capacity) {
            dest = signer_str_expand(dest);
            check_mem(dest);
        }
        ++dest->length;
        ++dest->offset;
        *dest->str++ = *src->str++;
    }
    src->str -= src->length;
    src->str += src->offset;
    dest->str[0] = '\0';
    --dest->str;
    --dest->offset;

    return dest;

error:
    return NULL;
}

signer_str *signer_str_cstr_concat(signer_str *dest, char *src) {
    if (dest == NULL || src == NULL) {
        return dest;
    }

    while (*dest->str) {
        ++dest->str;
        ++dest->offset;
    }
    while (*src) {
        if (dest->length >= dest->capacity) {
            dest = signer_str_expand(dest);
            check_mem(dest);
        }
        ++dest->length;
        ++dest->offset;
        *dest->str++ = *src++;
    }
    dest->str[0] = '\0';
    --dest->str;
    --dest->offset;

    return dest;

error:
    return NULL;
}

signer_str *signer_str_expand(signer_str *src) {
    src->str -= src->offset;
    size_t new_cap = src->capacity * 2;
    char *new_buf = malloc(sizeof(char) * (new_cap + 1));
    new_buf = memcpy(new_buf, src->str, src->length);
    check_mem(new_buf);
    new_buf[new_cap] = '\0';
    if (src->str != NULL) {
        free(src->str);
    }
    src->str = new_buf;
    src->capacity = new_cap;
    new_buf = NULL;

    src->str += src->offset;

    return src;

error:
    if (new_buf != NULL) {
        free(new_buf);
    }

    signer_str_destroy(src);

    return NULL;
}

void signer_str_reset(signer_str *str) {
    str->str -= str->offset;
    str->offset = 0;
}
