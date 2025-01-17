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

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_library(yajl OBJECT yajl.c yajl_lex.c yajl_parser.c yajl_buf.c
          yajl_encode.c yajl_gen.c yajl_alloc.c
          yajl_tree.c
)

set(HDRS yajl_parser.h yajl_lex.h yajl_buf.h yajl_encode.h yajl_alloc.h)
set(PUB_HDRS api/yajl_parse.h api/yajl_gen.h api/yajl_common.h api/yajl_tree.h)

# useful when fixing lexer bugs.
#add_definitions(-DYAJL_LEXER_DEBUG)

# Ensure defined when building YAJL (as opposed to using it from
# another project).  Used to ensure correct function export when
# building win32 DLL.
add_definitions(-DYAJL_BUILD)

# set up some paths
set(libDir ${CMAKE_CURRENT_BINARY_DIR}/../${YAJL_DIST_NAME}/lib)
set(incDir ${CMAKE_CURRENT_BINARY_DIR}/../${YAJL_DIST_NAME}/include/yajl)
set(shareDir ${CMAKE_CURRENT_BINARY_DIR}/../${YAJL_DIST_NAME}/share/pkgconfig)

# set the output path for libraries
set(LIBRARY_OUTPUT_PATH ${libDir})

add_library(yajl-static STATIC $<TARGET_OBJECTS:yajl>)
add_library(yajl-shared SHARED $<TARGET_OBJECTS:yajl>)

#### setup shared library version number
set_target_properties(yajl PROPERTIES
                      DEFINE_SYMBOL YAJL_SHARED
                      SOVERSION ${YAJL_MAJOR}
                      VERSION ${YAJL_MAJOR}.${YAJL_MINOR}.${YAJL_MICRO})

#### build up an sdk as a post build step

# create some directories
file(MAKE_DIRECTORY ${libDir})
file(MAKE_DIRECTORY ${incDir})

# generate build-time source
#set(dollar $)
#configure_file(api/yajl_version.h.cmake ${incDir}/yajl_version.h)
#configure_file(yajl.pc.cmake ${shareDir}/yajl.pc)

include_directories(${incDir}/..)

# at build time you may specify the cmake variable LIB_SUFFIX to handle
# 64-bit systems which use 'lib64'
install(TARGETS yajl
        RUNTIME DESTINATION lib${LIB_SUFFIX}
        LIBRARY DESTINATION lib${LIB_SUFFIX}
        ARCHIVE DESTINATION lib${LIB_SUFFIX})
install(TARGETS yajl-static ARCHIVE DESTINATION lib${LIB_SUFFIX})
install(FILES ${PUB_HDRS} DESTINATION include/yajl)
install(FILES ${incDir}/yajl_version.h DESTINATION include/yajl)
install(FILES ${shareDir}/yajl.pc DESTINATION share/pkgconfig)
