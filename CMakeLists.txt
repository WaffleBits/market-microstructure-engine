cmake_minimum_required(VERSION 3.20)

project(market_microstructure_engine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

function(enable_project_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /WX)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
endfunction()

add_library(mm_engine_native
    native/src/order_book.cpp
)
target_include_directories(mm_engine_native PUBLIC native/include)
enable_project_warnings(mm_engine_native)

add_executable(mm_engine_benchmark native/src/benchmark.cpp)
target_link_libraries(mm_engine_benchmark PRIVATE mm_engine_native)
enable_project_warnings(mm_engine_benchmark)

add_executable(mm_engine_native_tests native/tests/order_book_test.cpp)
target_link_libraries(mm_engine_native_tests PRIVATE mm_engine_native)
enable_project_warnings(mm_engine_native_tests)

enable_testing()
add_test(NAME mm_engine_native_tests COMMAND mm_engine_native_tests)
