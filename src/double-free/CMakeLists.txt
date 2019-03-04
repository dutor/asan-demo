add_executable(double-free double-free.cpp $<TARGET_OBJECTS:asan-options>)
add_test(NAME double-free COMMAND double-free)
