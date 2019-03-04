add_executable(heap-buffer-underflow heap-buffer-underflow.cpp $<TARGET_OBJECTS:asan-options>)
add_test(NAME heap-buffer-underflow COMMAND heap-buffer-underflow)
