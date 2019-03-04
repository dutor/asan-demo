add_executable(stack-buffer-overflow stack-buffer-overflow.cpp $<TARGET_OBJECTS:asan-options>)
add_test(NAME stack-buffer-overflow COMMAND stack-buffer-overflow)
