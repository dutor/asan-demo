add_executable(stack-buffer-underflow stack-buffer-underflow.cpp $<TARGET_OBJECTS:asan-options>)
add_test(NAME stack-buffer-underflow COMMAND stack-buffer-underflow)
