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

#include "yajl_buf.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "../../datakit/src/jebuf.h"

static void yajl_buf_ensure_available(yajl_buf buf, size_t want) {
    assert(buf);

    /* first call */
    if (buf->data == NULL) {
        buf->len = YAJL_BUF_INIT_SIZE;
        buf->data = (uint8_t *)YA_CALLOC(buf->len);
    }

    size_t need = buf->len;

    while (want >= (need - buf->used)) {
#if 0
        need = jebufSizeAllocation(need << 1);
#else
        need <<= 1;
#endif
    }

    if (need != buf->len) {
        buf->data = (unsigned char *)YA_REALLOC(buf->data, need);
        buf->len = need;
    }
}

void yajl_buf_free(yajl_buf buf) {
    assert(buf);
    if (buf->data) {
        YA_FREE(buf->data);
    }
}

void yajl_buf_append(yajl_buf buf, const void *data, size_t len) {
    yajl_buf_ensure_available(buf, len);
    if (len > 0) {
        assert(data != NULL);
        memcpy(buf->data + buf->used, data, len);
        buf->used += len;
        buf->data[buf->used] = 0;
    }
}

void yajl_buf_clear(yajl_buf buf) {
    buf->used = 0;
    if (buf->data) {
        buf->data[buf->used] = 0;
    }
}

const void *yajl_buf_data(yajl_buf buf) {
    return buf->data;
}

size_t yajl_buf_len(yajl_buf buf) {
    return buf->used;
}

void yajl_buf_truncate(yajl_buf buf, size_t len) {
    assert(len <= buf->used);
    buf->used = len;
}
