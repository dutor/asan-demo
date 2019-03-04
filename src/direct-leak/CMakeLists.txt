add_executable(direct-leak direct-leak.cpp $<TARGET_OBJECTS:asan-options>)
add_test(NAME direct-leak COMMAND direct-leak)
