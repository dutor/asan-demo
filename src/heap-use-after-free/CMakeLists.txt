add_executable(heap-use-after-free heap-use-after-free.cpp $<TARGET_OBJECTS:asan-options>)
add_test(NAME heap-use-after-free COMMAND heap-use-after-free)
