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
 * default memory allocation routines for yajl which use malloc/realloc and
 * free
 */

#ifndef __YAJL_ALLOC_H__
#define __YAJL_ALLOC_H__

#include "api/yajl_common.h"

/* Global allocation functions so we don't tote them around inside structs */
extern yajl_alloc_funcs yaf;

#define YA_CALLOC(sz) yaf.calloc(NULL, sz)
#define YA_FREE(ptr) yaf.free(NULL, ptr)
#define YA_REALLOC(ptr, sz) yaf.realloc(NULL, ptr, sz)

void yajl_set_default_alloc_funcs(yajl_alloc_funcs *yaf);

#endif
