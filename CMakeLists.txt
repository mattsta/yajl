# Copyright (c) 2007-2014, Lloyd Hilaiel <me@lloyd.io>
#
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

cmake_minimum_required(VERSION 3.0)

project(YetAnotherJSONParser C)

set(YAJL_MAJOR 2)
set(YAJL_MINOR 1)
set(YAJL_MICRO 1)

set(YAJL_DIST_NAME "yajl-${YAJL_MAJOR}.${YAJL_MINOR}.${YAJL_MICRO}")

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(CMAKE_SHARED_MODULE_CREATE_C_FLAGS
        "${CMAKE_SHARED_MODULE_CREATE_C_FLAGS} -undefined dynamic_lookup")
    cmake_policy(SET CMP0042 NEW)
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11 -pedantic -Wpointer-arith")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-format-y2k -Wstrict-prototypes")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wmissing-declarations -Wnested-externs")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wextra	-Wundef -Wwrite-strings")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wold-style-definition -Wredundant-decls")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-parameter -Wno-sign-compare ")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wmissing-prototypes")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-gnu-statement-expression")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")

add_subdirectory(src)
