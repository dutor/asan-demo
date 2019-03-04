add_executable(vector-overflow vector-overflow.cpp $<TARGET_OBJECTS:asan-options>)
add_test(NAME vector-overflow COMMAND vector-overflow)
