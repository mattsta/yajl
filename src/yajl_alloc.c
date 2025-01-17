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

/**
 * \file yajl_alloc.h
 * default memory allocation routines for yajl which use calloc/realloc and
 * free
 */

#include "yajl_alloc.h"
#include <stdlib.h>

static void *yajl_internal_calloc(void *ctx, size_t sz) {
    (void)ctx;
    return calloc(1, sz);
}

static void *yajl_internal_realloc(void *ctx, void *previous, size_t sz) {
    (void)ctx;
    return realloc(previous, sz);
}

static void yajl_internal_free(void *ctx, void *ptr) {
    (void)ctx;
    free(ptr);
}

void yajl_set_default_alloc_funcs(yajl_alloc_funcs *yaf) {
    yaf->calloc = yajl_internal_calloc;
    yaf->free = yajl_internal_free;
    yaf->realloc = yajl_internal_realloc;
    yaf->ctx = NULL;
}

yajl_alloc_funcs yaf = {.calloc = yajl_internal_calloc,
                        .free = yajl_internal_free,
                        .realloc = yajl_internal_realloc};
