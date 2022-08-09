cmake_minimum_required(VERSION 3.20)
project(eeds VERSION 0.0.1)

set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)

option(EEDS_TESTS "Build FDC Test tests" OFF)

include(FetchContent)
function(AddDependency dep url)
    FetchContent_Declare(${dep} URL ${url})
    FetchContent_GetProperties(${dep})
    if (NOT ${dep}_POPULATED)
        FetchContent_Populate(${dep})
        add_subdirectory(${${dep}_SOURCE_DIR} ${${dep}_BINARY_DIR} EXCLUDE_FROM_ALL)
    endif ()
endfunction()

if (EEDS_TESTS)
    AddDependency(googletest https://github.com/google/googletest/archive/5376968f6948923e2411081fd9372e71a59d8e77.zip)
    AddDependency(benchmark https://github.com/google/benchmark/archive/108ca84820dc932957b75e6b31faa6c3cf0aa37a.zip)
endif ()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-fsanitize=address,undefined -fno-sanitize-recover=undefined -fprofile-instr-generate -fcoverage-mapping)
    add_link_options(-fsanitize=address,undefined -fprofile-instr-generate -fcoverage-mapping)
endif ()
add_compile_options(-fno-omit-frame-pointer)
add_link_options(-fno-omit-frame-pointer)

add_compile_options(
        -Werror
        -Wall
        -Wextra
        -Wpedantic # warn if non-standard C++ is used
        -Wshadow # warn the user if a variable declaration shadows one from a parent context
        -Wnon-virtual-dtor # warn the user if a class with virtual functions has a non-virtual destructor. This helps
        -Wold-style-cast # warn for c-style casts
        -Wcast-align # warn for potential performance problem casts
        -Wunused # warn on anything being unused
        -Woverloaded-virtual # warn if you overload (not override) a virtual function
        -Wconversion # warn on type conversions that may lose data
        -Wsign-conversion # warn on sign conversions
        -Wnull-dereference # warn if a null dereference is detected
        -Wdouble-promotion # warn if float is implicit promoted to double
        -Wformat=2 # warn on security issues around functions that format output (ie printf)
        -Wimplicit-fallthrough # warn on statements that fallthrough without an explicit annotation

        -Wno-gnu-zero-variadic-macro-arguments # Disable this one, should be fine in C++20.
)
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options(
            -Wmisleading-indentation # warn if indentation implies blocks where blocks do not exist
            -Wduplicated-cond # warn if if / else chain has duplicated conditions
            -Wduplicated-branches # warn if if / else branches have duplicated code
            -Wlogical-op # warn about logical operations being used where bitwise were probably wanted
            -Wuseless-cast # warn if you perform a cast to the same type
    )
endif ()

add_library(eeds INTERFACE)
target_include_directories(eeds INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include/)

install(DIRECTORY include DESTINATION .)

function(add_fuzz_test NAME)
    add_executable(${NAME} ${ARGN})
    target_link_libraries(${NAME} PUBLIC eeds)
    target_compile_options(${NAME} PUBLIC -fsanitize=fuzzer,address,undefined -fno-sanitize-recover=undefined)
    target_link_options(${NAME} PUBLIC -fsanitize=fuzzer,address,undefined)
    target_compile_definitions(${NAME} PUBLIC EEDS_FUZZ)
endfunction()

if (EEDS_TESTS)
    file(GLOB UNITTESTS ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_*.cpp)
    add_executable(unittests ${UNITTESTS})
    target_link_libraries(unittests PUBLIC eeds gtest gtest_main gmock)
    target_compile_definitions(unittests PUBLIC EEDS_UNITTESTS)

    file(GLOB BENCHMARKS ${CMAKE_CURRENT_SOURCE_DIR}/benchmark/bm_*.cpp)
    add_executable(benchmarks ${BENCHMARKS})
    target_link_libraries(benchmarks PUBLIC eeds benchmark benchmark_main)

    add_fuzz_test(fuzz_countset fuzz/fuzz_countset.cpp)
endif ()
