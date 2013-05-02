project(k++)

find_package(FLEX)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

FLEX_TARGET(FLT lang.lex ${CMAKE_CURRENT_BINARY_DIR}/klexer.cpp)
set(SRC_FILES main.cpp)

add_executable(k++ ${SRC_FILES} ${FLEX_FLT_OUTPUTS})
