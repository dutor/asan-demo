add_executable(init-order-fiasco global.cpp init-order-fiasco.cpp $<TARGET_OBJECTS:asan-options>)
add_test(NAME init-order-fiasco COMMAND init-order-fiasco)
