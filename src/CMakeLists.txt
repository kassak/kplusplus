project(k++)

list(APPEND CMAKE_CXX_FLAGS "-std=c++0x ${CMAKE_CXX_FLAGS}")
list(APPEND CMAKE_CXX_FLAGS_RELEASE "-std=c++0x ${CMAKE_CXX_FLAGS_RELEASE}")
list(APPEND CMAKE_CXX_FLAGS_DEBUG "-std=c++0x ${CMAKE_CXX_FLAGS_DEBUG}")

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake")

find_package(FLEX)
include(PrecompiledHeader)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

FLEX_TARGET(FLT lang.lex ${CMAKE_CURRENT_BINARY_DIR}/klexer.lex.cpp)
set(SRC_FILES main.cpp kparser.cpp klexer.cpp)

add_executable(k++ ${SRC_FILES} ${FLEX_FLT_OUTPUTS})
add_precompiled_header(k++ stdafx.h FORCEINCLUDE)
