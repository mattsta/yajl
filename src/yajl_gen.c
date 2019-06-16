/*
 * Copyright (c) 2007-2014, Lloyd Hilaiel <me@lloyd.io>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "api/yajl_gen.h"
#include "dualBox.h"
#include "yajl_buf.h"
#include "yajl_encode.h"

#include <assert.h>
#include <math.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *indentString = "    ";

void yajl_gen_pretty_enable(yajl_gen g) {
    g->flags |= yajl_gen_beautify;
}

int yajl_gen_config(yajl_gen g, yajl_gen_option opt, ...) {
    int rv = 1;
    va_list ap;
    va_start(ap, opt);

    switch (opt) {
    case yajl_gen_beautify:
    case yajl_gen_validate_utf8:
    case yajl_gen_escape_solidus:
        if (va_arg(ap, int)) {
            g->flags |= opt;
        } else {
            g->flags &= ~opt;
        }

        break;
    case yajl_gen_indent_string: {
        const char *indent = va_arg(ap, const char *);
        indentString = indent;
        for (; *indent; indent++) {
            if (*indent != '\n' && *indent != '\v' && *indent != '\f' &&
                *indent != '\t' && *indent != '\r' && *indent != ' ') {
                indentString = NULL;
                rv = 0;
            }
        }

        break;
    }

    default:
        rv = 0;
    }

    va_end(ap);

    return rv;
}

void yajl_gen_deinit(yajl_gen g) {
    yajlDualStorageReset(&g->statusAtDepth);
}

void yajl_gen_free_buffer(yajl_gen g) {
    yajl_buf_free(&g->buf);
    g->buf = (yajl_buf_t){0};
}

yajl_gen yajl_gen_alloc(void) {
    yajl_gen g = NULL;

    g = (yajl_gen)YA_CALLOC(sizeof(struct yajl_gen_t));
    if (!g) {
        return NULL;
    }

    return g;
}

void yajl_gen_reset(yajl_gen g, const char *sep) {
    g->depth = 0;
    yajlDualStorageReset(&g->statusAtDepth);
    if (sep != NULL) {
        yajl_buf_append(&g->buf, sep, strlen(sep));
    }
}

void yajl_gen_free(yajl_gen g) {
    yajl_buf_free(&g->buf);
    YA_FREE(g);
}

#define INSERT_SEP                                                             \
    do {                                                                       \
        const yajl_gen_state current =                                         \
            yajlDualStorageGet(&g->statusAtDepth, g->depth);                   \
        if (current == yajl_gen_map_key || current == yajl_gen_in_array) {     \
            yajl_buf_append(&g->buf, ",", 1);                                  \
            if (g->flags & yajl_gen_beautify) {                                \
                yajl_buf_append(&g->buf, "\n", 1);                             \
            }                                                                  \
        } else if (current == yajl_gen_map_val) {                              \
            yajl_buf_append(&g->buf, ":", 1);                                  \
            if (g->flags & yajl_gen_beautify) {                                \
                yajl_buf_append(&g->buf, " ", 1);                              \
            }                                                                  \
        }                                                                      \
    } while (0)

#define INSERT_WHITESPACE                                                      \
    if ((g->flags & yajl_gen_beautify)) {                                      \
        if (yajlDualStorageGet(&g->statusAtDepth, g->depth) !=                 \
            yajl_gen_map_val) {                                                \
            unsigned int _i;                                                   \
            for (_i = 0; _i < g->depth; _i++)                                  \
                yajl_buf_append(&g->buf, indentString,                         \
                                (unsigned int)strlen(indentString));           \
        }                                                                      \
    }

#define ENSURE_NOT_KEY                                                         \
    do {                                                                       \
        const yajl_gen_state current =                                         \
            yajlDualStorageGet(&g->statusAtDepth, g->depth);                   \
        if (current == yajl_gen_map_key || current == yajl_gen_map_start) {    \
            return yajl_gen_keys_must_be_strings;                              \
        }                                                                      \
    } while (0)

/* check that we're not complete, or in error state.  in a valid state
 * to be generating */
#define ENSURE_VALID_STATE                                                     \
    do {                                                                       \
        const yajl_gen_state current =                                         \
            yajlDualStorageGet(&g->statusAtDepth, g->depth);                   \
        if (current == yajl_gen_error) {                                       \
            return yajl_gen_in_error_state;                                    \
        }                                                                      \
                                                                               \
        if (current == yajl_gen_complete) {                                    \
            return yajl_gen_generation_complete;                               \
        }                                                                      \
    } while (0)

#define INCREMENT_DEPTH                                                        \
    do {                                                                       \
        g->depth++;                                                            \
        /* Add new slot if required */                                         \
        yajlDualStorageGrow(&g->statusAtDepth, g->depth);                      \
    } while (0)

#define DECREMENT_DEPTH                                                        \
    do {                                                                       \
        g->depth--;                                                            \
    } while (0)

#define APPENDED_ATOM                                                          \
    do {                                                                       \
        yajl_gen_state *current =                                              \
            yajlDualStorageGetPtr(&g->statusAtDepth, g->depth);                \
        switch (*current) {                                                    \
        case yajl_gen_start:                                                   \
            *current = yajl_gen_complete;                                      \
            break;                                                             \
        case yajl_gen_map_start:                                               \
        case yajl_gen_map_key:                                                 \
            *current = yajl_gen_map_val;                                       \
            break;                                                             \
        case yajl_gen_array_start:                                             \
            *current = yajl_gen_in_array;                                      \
            break;                                                             \
        case yajl_gen_map_val:                                                 \
            *current = yajl_gen_map_key;                                       \
            break;                                                             \
        default:                                                               \
            break;                                                             \
        }                                                                      \
    } while (0)

#define FINAL_NEWLINE                                                          \
    do {                                                                       \
        if ((g->flags & yajl_gen_beautify) &&                                  \
            yajlDualStorageGet(&g->statusAtDepth, g->depth) ==                 \
                yajl_gen_complete) {                                           \
            yajl_buf_append(&g->buf, "\n", 1);                                 \
        }                                                                      \
    } while (0)

yajl_gen_status yajl_gen_integer(yajl_gen g, long long int number) {
    char i[32];
    ENSURE_VALID_STATE;
    ENSURE_NOT_KEY;
    INSERT_SEP;
    INSERT_WHITESPACE;
    /* TODO: replace snprintf with StrInt64ToBuf */
    const size_t wroteLen = snprintf(i, sizeof(i), "%lld", number);
    yajl_buf_append(&g->buf, i, wroteLen);
    APPENDED_ATOM;
    FINAL_NEWLINE;
    return yajl_gen_status_ok;
}

#if defined(_WIN32) || defined(WIN32)
#include <float.h>
#define isnan _isnan
#define isinf !_finite
#endif

yajl_gen_status yajl_gen_double(yajl_gen g, double number) {
    char i[32];
    ENSURE_VALID_STATE;
    ENSURE_NOT_KEY;
    if (isnan(number) || isinf(number)) {
        return yajl_gen_invalid_number;
    }

    INSERT_SEP;
    INSERT_WHITESPACE;
    const size_t wroteLen = snprintf(i, sizeof(i), "%.20g", number);
    if (strspn(i, "0123456789-") == wroteLen) {
        strcat(i, ".0");
    }

    yajl_buf_append(&g->buf, i, wroteLen);
    APPENDED_ATOM;
    FINAL_NEWLINE;
    return yajl_gen_status_ok;
}

yajl_gen_status yajl_gen_number(yajl_gen g, const void *s_, size_t l) {
    const uint8_t *s = s_;
    ENSURE_VALID_STATE;
    ENSURE_NOT_KEY;
    INSERT_SEP;
    INSERT_WHITESPACE;
    yajl_buf_append(&g->buf, s, l);
    APPENDED_ATOM;
    FINAL_NEWLINE;
    return yajl_gen_status_ok;
}

yajl_gen_status yajl_gen_string(yajl_gen g, const void *str_, size_t len) {
    const uint8_t *str = str_;

    // if validation is enabled, check that the string is valid utf8
    // XXX: This checking could be done a little faster, in the same pass as
    // the string encoding
    if (g->flags & yajl_gen_validate_utf8) {
        if (!yajl_string_validate_utf8(str, len)) {
            return yajl_gen_invalid_string;
        }
    }

    ENSURE_VALID_STATE;
    INSERT_SEP;
    INSERT_WHITESPACE;
    yajl_buf_append(&g->buf, "\"", 1);
    yajl_string_encode(&g->buf, str, len, g->flags & yajl_gen_escape_solidus);
    yajl_buf_append(&g->buf, "\"", 1);
    APPENDED_ATOM;
    FINAL_NEWLINE;
    return yajl_gen_status_ok;
}

yajl_gen_status yajl_gen_null(yajl_gen g) {
    ENSURE_VALID_STATE;
    ENSURE_NOT_KEY;
    INSERT_SEP;
    INSERT_WHITESPACE;
    yajl_buf_append(&g->buf, "null", strlen("null"));
    APPENDED_ATOM;
    FINAL_NEWLINE;
    return yajl_gen_status_ok;
}

yajl_gen_status yajl_gen_bool(yajl_gen g, int boolean) {
    const char *val = boolean ? "true" : "false";

    ENSURE_VALID_STATE;
    ENSURE_NOT_KEY;
    INSERT_SEP;
    INSERT_WHITESPACE;
    yajl_buf_append(&g->buf, val, (unsigned int)strlen(val));
    APPENDED_ATOM;
    FINAL_NEWLINE;
    return yajl_gen_status_ok;
}

yajl_gen_status yajl_gen_map_open(yajl_gen g) {
    ENSURE_VALID_STATE;
    ENSURE_NOT_KEY;
    INSERT_SEP;
    INSERT_WHITESPACE;
    INCREMENT_DEPTH;

    *yajlDualStorageGetPtr(&g->statusAtDepth, g->depth) = yajl_gen_map_start;
    yajl_buf_append(&g->buf, "{", 1);
    if ((g->flags & yajl_gen_beautify)) {
        yajl_buf_append(&g->buf, "\n", 1);
    }

    FINAL_NEWLINE;
    return yajl_gen_status_ok;
}

yajl_gen_status yajl_gen_map_close(yajl_gen g) {
    ENSURE_VALID_STATE;
    DECREMENT_DEPTH;

    if ((g->flags & yajl_gen_beautify)) {
        yajl_buf_append(&g->buf, "\n", 1);
    }

    APPENDED_ATOM;
    INSERT_WHITESPACE;
    yajl_buf_append(&g->buf, "}", 1);
    FINAL_NEWLINE;
    return yajl_gen_status_ok;
}

yajl_gen_status yajl_gen_array_open(yajl_gen g) {
    ENSURE_VALID_STATE;
    ENSURE_NOT_KEY;
    INSERT_SEP;
    INSERT_WHITESPACE;
    INCREMENT_DEPTH;
    *yajlDualStorageGetPtr(&g->statusAtDepth, g->depth) = yajl_gen_array_start;
    yajl_buf_append(&g->buf, "[", 1);
    if ((g->flags & yajl_gen_beautify)) {
        yajl_buf_append(&g->buf, "\n", 1);
    }

    FINAL_NEWLINE;
    return yajl_gen_status_ok;
}

yajl_gen_status yajl_gen_array_close(yajl_gen g) {
    ENSURE_VALID_STATE;
    DECREMENT_DEPTH;
    if ((g->flags & yajl_gen_beautify)) {
        yajl_buf_append(&g->buf, "\n", 1);
    }

    APPENDED_ATOM;
    INSERT_WHITESPACE;
    yajl_buf_append(&g->buf, "]", 1);
    FINAL_NEWLINE;
    return yajl_gen_status_ok;
}

yajl_gen_status yajl_gen_get_buf(yajl_gen g, void **buf, size_t *len) {
    *buf = g->buf.data;
    *len = yajl_buf_len(&g->buf);
    return yajl_gen_status_ok;
}

void yajl_gen_clear(yajl_gen g) {
    yajl_buf_clear(&g->buf);
}
