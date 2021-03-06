cmake_minimum_required(VERSION 2.8)

project(k++)

set(CMAKE_CXX_FLAGS "-std=c++0x -Wall -Wno-switch ${CMAKE_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE "-std=c++0x -Wall -Wno-switch ${CMAKE_CXX_FLAGS_RELEASE}")
set(CMAKE_CXX_FLAGS_DEBUG "-std=c++0x -Wall -Wno-switch -D_GLIBCXX_DEBUG ${CMAKE_CXX_FLAGS_DEBUG}")

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake")

find_package(FLEX)
find_package(LLVM COMPONENTS jit)
find_package(Boost COMPONENTS system program_options REQUIRED)
include(PrecompiledHeader)

set(CMAKE_EXE_LINKER_FLAGS "${LLVM_LDFLAGS} ${CMAKE_EXE_LINKER_FLAGS_DEBUG}")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${LLVM_LDFLAGS} ${CMAKE_EXE_LINKER_FLAGS_RELEASE}")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${LLVM_LDFLAGS} ${CMAKE_EXE_LINKER_FLAGS_DEBUG}")


include_directories( ${LLVM_INCLUDE_DIRS} )
link_directories( ${LLVM_LIBRARY_DIRS} )

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIR})

FLEX_TARGET(FLT lang.lex ${CMAKE_CURRENT_BINARY_DIR}/klexer.lex.cpp)
set(SRC_FILES main.cpp kparser.cpp klexer.cpp config.cpp ir_emitter.cpp)

add_executable(k++ ${SRC_FILES} ${FLEX_FLT_OUTPUTS})

target_link_libraries(k++ ${Boost_LIBRARIES})
target_link_libraries(k++ ${LLVM_LIBRARIES})

add_precompiled_header(k++ stdafx.h FORCEINCLUDE)
