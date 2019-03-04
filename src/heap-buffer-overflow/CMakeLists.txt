add_executable(heap-buffer-overflow heap-buffer-overflow.cpp $<TARGET_OBJECTS:asan-options>)
add_test(NAME heap-buffer-overflow COMMAND heap-buffer-overflow)
