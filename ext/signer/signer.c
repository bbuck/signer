#include <ruby.h>
#include <openssl/hmac.h>
#include <openssl/evp.h>

#include "stack.h"
#include "signer_str.h"
#include "util.h"

const int DEFAULT_BUFF_SIZE = 2048;

VALUE cSigner = Qnil;

void Init_signer();
VALUE Signer_method_join(VALUE self, VALUE strs);
VALUE Signer_method_dump(VALUE self, VALUE hash);
VALUE signer_sorted_keys(VALUE hash);
VALUE Signer_method_sign(VALUE self, VALUE data, VALUE key);

void ns_pop(signer_str *ns);

typedef struct signer_iter_node {
    VALUE keys;
    VALUE hash;
    int pos;
    int max;
} signer_iter_node;

void Init_signer() {
    cSigner = rb_define_class("Signer", rb_cObject);
    rb_define_singleton_method(cSigner, "join", Signer_method_join, 1);
    rb_define_singleton_method(cSigner, "dump", Signer_method_dump, 1);
    rb_define_singleton_method(cSigner, "sign", Signer_method_sign, 2);
}

VALUE Signer_method_join(VALUE self, VALUE strs) {
    signer_str *result = signer_str_make(DEFAULT_BUFF_SIZE);
    if (result == NULL) {
        rb_raise(rb_eRuntimeError, "Unable to allocate result");

        return Qnil;
    }

    int str_count = RARRAY_LEN(strs);
    for (int i = 0; i < str_count; ++i) {
        VALUE str = rb_ary_entry(strs, i);
        char *cstr = StringValueCStr(str);
        result = signer_str_cstr_concat(result, cstr);
        if (result == NULL) {
            rb_raise(rb_eRuntimeError, "Failed to concatenate strings");
            goto error;
        }
    }

    signer_str_reset(result);
    VALUE ret_str = rb_str_new(result->str, result->length);
    signer_str_destroy(result);
    result = NULL;

    return ret_str;

error:
    if (result != NULL) {
        signer_str_destroy(result);
    }

    return Qnil;
}

VALUE Signer_method_dump(VALUE self, VALUE hash) {
    signer_str *result = signer_str_make(DEFAULT_BUFF_SIZE);
    if (result == NULL) {
        rb_raise(rb_eRuntimeError, "Failed to allocate result string");
        goto error;
    }

    const VALUE TO_S = rb_intern("to_s");

    signer_str *ns = signer_str_make(100);
    if (RB_TYPE_P(hash, T_HASH)) {
        VALUE keys = signer_sorted_keys(hash);
        signer_iter_node node = {
            .hash = hash,
            .keys = keys,
            .pos = 0,
            .max = RARRAY_LEN(keys)
        };
        signer_stack *stk = signer_stack_make();
        stk = signer_stack_push(stk, &node);

        while (!signer_stack_empty(stk)) {
            signer_iter_node *cur_node = (signer_iter_node *)stk->value;
            if (cur_node->pos >= cur_node->max) {
                stk = signer_stack_pop(stk);
                ns_pop(ns);

                continue;
            }
            VALUE key = rb_ary_entry(cur_node->keys, cur_node->pos);
            VALUE value = rb_hash_aref(cur_node->hash, key);
            ++cur_node->pos;

            key = rb_funcall(key, TO_S, 0);
            switch (TYPE(value)) {
            case T_HASH:
                if (ns->length > 0) {
                    ns = signer_str_cstr_concat(ns, ".");
                }
                ns = signer_str_cstr_concat(ns, StringValueCStr(key));
                if (ns == NULL) {
                    rb_raise(rb_eRuntimeError, "Unable to append namesapce");
                }
                keys = signer_sorted_keys(value);
                node = (signer_iter_node) {
                    .hash = value,
                    .keys = keys,
                    .pos = 0,
                    .max = RARRAY_LEN(keys)
                };
                stk = signer_stack_push(stk, &node);
                break;
            default:
                if (ns->length > 0) {
                    result = signer_str_concat(result, ns);
                    result = signer_str_cstr_concat(result, ".");
                }
                result = signer_str_cstr_concat(result, StringValueCStr(key));

                result = signer_str_cstr_concat(result, ": ");
                value = rb_funcall(value, TO_S, 0);
                result = signer_str_cstr_concat(result, StringValueCStr(value));
                result = signer_str_cstr_concat(result, "\n");

                if (result == NULL) {
                    rb_raise(rb_eRuntimeError, "Failed to join keys");
                    goto error;
                }
                break;
            }
        }

        signer_stack_destroy(stk);
        stk = NULL;
    }

    signer_str_destroy(ns);
    ns = NULL;

    signer_str_reset(result);
    VALUE ret_str = rb_str_new(result->str, result->length);
    signer_str_destroy(result);
    result = NULL;

    return ret_str;

error:
    signer_str_destroy(result);
    signer_str_destroy(ns);

    return Qnil;
}

VALUE signer_sorted_keys(VALUE hash) {
    VALUE keys = rb_funcall(hash, rb_intern("keys"), 0);
    rb_ary_sort_bang(keys);

    return keys;
}

void ns_pop(signer_str *ns) {
    if (ns->length > 0) {
        ns->str -= ns->offset;
        ns->str += ns->length;

        size_t removed = 0;
        while (*ns->str != '.' && removed < ns->length) {
            ++removed;
            --ns->str;
        }
        ns->str[0] = '\0';
        size_t new_length = ns->length - removed;
        ns->str -= new_length;
        ns->offset = 0;
        ns->length = new_length;
    }
}

void bin_to_strhex(unsigned char *bin, unsigned int binsz, char **result);

VALUE Signer_method_sign(VALUE self, VALUE data, VALUE key) {
    VALUE data_to_sign = Signer_method_dump(self, data);
    int expected = EVP_MD_size(EVP_sha256());
    unsigned int result_len = 0;
    unsigned char *result = malloc(sizeof(unsigned char) * expected);
    check_mem(result);
    HMAC(EVP_sha256(), StringValueCStr(key), RSTRING_LEN(key),
         StringValueCStr(data_to_sign), RSTRING_LEN(data_to_sign),
         result, &result_len);

    char buff[10] = { '\0' };
    if (result_len > 0) {
        // signer_str *hex_digest = signer_str_make(result_len * 2);
        // for (unsigned int i = 0; i < result_len; ++i) {
        //     sprintf(buff, "%02x", result[i]);
        //     printf("iteration %d: %s\n", i + 1, buff);
        //     hex_digest = signer_str_cstr_concat(hex_digest, buff);
        // }
        char *hex_digest;
        bin_to_strhex(result, result_len, &hex_digest);
        VALUE sig = rb_str_new2(hex_digest);
        free(result);
        // signer_str_destroy(hex_digest);
        free(hex_digest);

        return sig;
    }

    rb_raise(rb_eRuntimeError, "Failed to sign data");

error:
    if (result != NULL) {
        free(result);
    }

    return Qnil;
}

// pulled from stackoverflow answer http://stackoverflow.com/a/17147874/445322
// was written by Yannuth
void bin_to_strhex(unsigned char *bin, unsigned int binsz, char **result) {
    char hex_str[] = "0123456789abcdef";
    unsigned int i;

    *result = (char *)malloc(binsz * 2 + 1);
    (*result)[binsz * 2] = 0;

    if (!binsz) {
        return;
    }

    for (i = 0; i < binsz; i++) {
        (*result)[i * 2 + 0] = hex_str[(bin[i] >> 4) & 0x0F];
        (*result)[i * 2 + 1] = hex_str[(bin[i]) & 0x0F];
    }
}
