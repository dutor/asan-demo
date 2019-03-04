add_executable(indirect-leak indirect-leak.cpp $<TARGET_OBJECTS:asan-options>)
add_test(NAME indirect-leak COMMAND indirect-leak)
