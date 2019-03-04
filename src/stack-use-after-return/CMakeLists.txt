add_executable(stack-use-after-return stack-use-after-return.cpp $<TARGET_OBJECTS:asan-options>)
add_test(NAME stack-use-after-return COMMAND stack-use-after-return)
