add_subdirectory(asan-options)
add_subdirectory(init-order-fiasco)

add_executable(asan-demo main.cpp $<TARGET_OBJECTS:asan-options>)
