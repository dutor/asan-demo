cmake_minimum_required(VERSION 3.0)

enable_testing()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(PROJECT_HOME ${CMAKE_CURRENT_SOURCE_DIR})
add_definitions(-DPROJECT_HOME=${PROJECT_HOME})
add_definitions(-D_GLIBCXX_SANITIZE_VECTOR)

add_compile_options(-g)
add_compile_options(-O0)
add_compile_options(-Wall)
add_compile_options(-Werror)
add_compile_options(-fsanitize=address)
add_compile_options(-fno-omit-frame-pointer)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
add_subdirectory(src)
