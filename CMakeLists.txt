cmake_minimum_required(VERSION 3.22)
project(COtt)

set(CMAKE_CXX_STANDARD 23)

include_directories(include)

add_library(operational_semantics_lib OBJECT
        include/operational_semantics/is_hashable.h
        include/operational_semantics/has_equality.h
        include/operational_semantics/language_semantics.h
        include/operational_semantics/small_step_semantics.h
)

add_executable(operational_semantics main.cpp
)

add_executable(uint_arithmetics examples/uint_arithmetics.cpp)
add_executable(finite_ccs examples/finite_ccs.cpp)
