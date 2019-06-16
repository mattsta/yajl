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

/*
 * A header only implementation of a simple stack of bytes, used in YAJL
 * to maintain parse state.
 */

#ifndef __YAJL_BYTESTACK_H__
#define __YAJL_BYTESTACK_H__

#include "api/yajl_common.h"

typedef struct yajl_bytestack_t {
    uint8_t *stack;
    size_t size;
    size_t used;
} yajl_bytestack;

/* initialize a bytestack */
#define yajl_bs_init(obs)                                                      \
    do {                                                                       \
        (obs).stack = NULL;                                                    \
        (obs).size = 0;                                                        \
        (obs).used = 0;                                                        \
    } while (0)

/* initialize a bytestack */
#define yajl_bs_free(obs)                                                      \
    do {                                                                       \
        if ((obs).stack) {                                                     \
            YA_FREE((obs).stack);                                              \
        }                                                                      \
    } while (0)

#define yajl_bs_current(obs)                                                   \
    ({                                                                         \
        assert((obs).used > 0);                                                \
        (obs).stack[(obs).used - 1];                                           \
    })

#define yajl_bs_push(obs, byte)                                                \
    {                                                                          \
        if ((obs).size == (obs).used) {                                        \
            (obs).size *= 2;                                                   \
            (obs).stack = YA_REALLOC((void *)(obs).stack, (obs).size);         \
        }                                                                      \
                                                                               \
        (obs).stack[((obs).used)++] = (byte);                                  \
    }

/* removes the top item of the stack, returns nothing */
#define yajl_bs_pop(obs)                                                       \
    do {                                                                       \
        ((obs).used)--;                                                        \
    } while (0)

#define yajl_bs_set(obs, byte)                                                 \
    do {                                                                       \
        (obs).stack[((obs).used) - 1] = (byte);                                \
    } while (0)

#endif
