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
 * \file yajl_gen.h
 * Interface to YAJL's JSON generation facilities.
 */

#include "yajl_common.h"

/* Break some internal/external code separation */
#include "../yajl_buf.h"

#ifndef __YAJL_GEN_H__
#define __YAJL_GEN_H__

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif
/** generator status codes */
typedef enum {
    /** no error */
    yajl_gen_status_ok = 0,
    /** at a point where a map key is generated, a function other than
     *  yajl_gen_string was called */
    yajl_gen_keys_must_be_strings,
    /** YAJL's maximum generation depth was exceeded.  see
     *  YAJL_MAX_DEPTH */
    yajl_max_depth_exceeded,
    /** A generator function (yajl_gen_XXX) was called while in an error
     *  state */
    yajl_gen_in_error_state,
    /** A complete JSON document has been generated */
    yajl_gen_generation_complete,
    /** yajl_gen_double was passed an invalid floating point value
     *  (infinity or NaN). */
    yajl_gen_invalid_number,
    /** A print callback was passed in, so there is no internal
     * buffer to get from */
    yajl_gen_no_buf,
    /** returned from yajl_gen_string() when the yajl_gen_validate_utf8
     *  option is enabled and an invalid was passed by client code.
     */
    yajl_gen_invalid_string
} yajl_gen_status;

typedef enum __attribute__((packed)) yajl_gen_state {
    yajl_gen_start,
    yajl_gen_map_start,
    yajl_gen_map_key,
    yajl_gen_map_val,
    yajl_gen_array_start,
    yajl_gen_in_array,
    yajl_gen_complete,
    yajl_gen_error
} yajl_gen_state;

_Static_assert(sizeof(yajl_gen_state) == 1, "Not using packed state?");

typedef struct yajlGenStateStatus {
    yajl_gen_state local[8];
    yajl_gen_state *allocated;
    size_t count;
    size_t totalCountOfAllocated;
} yajlGenStateStatus;

typedef struct yajl_gen_t {
    yajl_buf_t buf;
    yajlGenStateStatus statusAtDepth;
    /* If your JSON nesting depth is higher than 72,057,594,037,927,936 you have
     * a bigger problem than running out of bits for depth storage... */
    uint64_t depth : 56;
    uint64_t flags : 8; /* flags are an instance of 'yajl_gen_option' */
} yajl_gen_t;

_Static_assert(sizeof(yajl_gen_t) == (8 * 3) + ((8 * 2) + (8 * 2)) + (8),
               "gen_t bigger than we think?");

/** an opaque handle to a generator */
typedef struct yajl_gen_t *yajl_gen;

/** configuration parameters for the parser, these may be passed to
 *  yajl_gen_config() along with option specific argument(s).  In general,
 *  all configuration parameters default to *off*. */
typedef enum yajl_gen_option {
    /** generate indented (beautiful) output */
    yajl_gen_beautify = 0x01,
    /**
     * Set an indent string which is used when yajl_gen_beautify
     * is enabled.  Maybe something like \\t or some number of
     * spaces.  The default is four spaces ' '.
     */
    yajl_gen_indent_string = 0x02,
    /**
     * Normally the generator does not validate that strings you
     * pass to it via yajl_gen_string() are valid UTF8.  Enabling
     * this option will cause it to do so.
     */
    yajl_gen_validate_utf8 = 0x08,
    /**
     * the forward solidus (slash or '/' in human) is not required to be
     * escaped in json text.  By default, YAJL will not escape it in the
     * iterest of saving bytes.  Setting this flag will cause YAJL to
     * always escape '/' in generated JSON strings.
     */
    yajl_gen_escape_solidus = 0x10
} yajl_gen_option;

/** allow the modification of generator options subsequent to handle
 *  allocation (via yajl_alloc)
 *  \returns zero in case of errors, non-zero otherwise
 */
YAJL_API int yajl_gen_config(yajl_gen g, yajl_gen_option opt, ...);

/* Maintain your own yajl_gen_t storage and free things here... */
void yajl_gen_deinit(yajl_gen g);
void yajl_gen_free_buffer(yajl_gen g);

/** allocate a generator handle
 *  \param allocFuncs an optional pointer to a structure which allows
 *                    the client to overide the memory allocation
 *                    used by yajl.  May be NULL, in which case
 *                    malloc/free/realloc will be used.
 *
 *  \returns an allocated handle on success, NULL on failure (bad params)
 */
YAJL_API yajl_gen yajl_gen_alloc(void);

/** free a generator handle */
YAJL_API void yajl_gen_free(yajl_gen handle);

YAJL_API yajl_gen_status yajl_gen_integer(yajl_gen hand, long long int number);
/** generate a floating point number.  number may not be infinity or
 *  NaN, as these have no representation in JSON.  In these cases the
 *  generator will return 'yajl_gen_invalid_number' */
YAJL_API yajl_gen_status yajl_gen_double(yajl_gen hand, double number);
YAJL_API yajl_gen_status yajl_gen_number(yajl_gen hand, const void *num,
                                         size_t len);
YAJL_API yajl_gen_status yajl_gen_string(yajl_gen hand, const void *str,
                                         size_t len);
YAJL_API yajl_gen_status yajl_gen_null(yajl_gen hand);
YAJL_API yajl_gen_status yajl_gen_bool(yajl_gen hand, int boolean);
YAJL_API yajl_gen_status yajl_gen_map_open(yajl_gen hand);
YAJL_API yajl_gen_status yajl_gen_map_close(yajl_gen hand);
YAJL_API yajl_gen_status yajl_gen_array_open(yajl_gen hand);
YAJL_API yajl_gen_status yajl_gen_array_close(yajl_gen hand);

/** access the null terminated generator buffer.  If incrementally
 *  outputing JSON, one should call yajl_gen_clear to clear the
 *  buffer.  This allows stream generation. */
YAJL_API yajl_gen_status yajl_gen_get_buf(yajl_gen hand, void **buf,
                                          size_t *len);

/** clear yajl's output buffer, but maintain all internal generation
 *  state.  This function will not "reset" the generator state, and is
 *  intended to enable incremental JSON outputing. */
YAJL_API void yajl_gen_clear(yajl_gen hand);

/** Reset the generator state.  Allows a client to generate multiple
 *  json entities in a stream. The "sep" string will be inserted to
 *  separate the previously generated entity from the current,
 *  NULL means *no separation* of entites (clients beware, generating
 *  multiple JSON numbers without a separator, for instance, will result in
 * ambiguous output)
 *
 *  Note: this call will not clear yajl's output buffer.  This
 *  may be accomplished explicitly by calling yajl_gen_clear() */
YAJL_API void yajl_gen_reset(yajl_gen hand, const char *sep);

void yajl_gen_pretty_enable(yajl_gen g);

#ifdef __cplusplus
}

#endif

#endif
