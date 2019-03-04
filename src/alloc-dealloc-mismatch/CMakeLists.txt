add_executable(alloc-dealloc-mismatch alloc-dealloc-mismatch.cpp $<TARGET_OBJECTS:asan-options>)
add_test(NAME alloc-dealloc-mismatch COMMAND alloc-dealloc-mismatch)
