add_executable(stack-use-after-scope stack-use-after-scope.cpp $<TARGET_OBJECTS:asan-options>)
add_test(NAME stack-use-after-scope COMMAND stack-use-after-scope)
