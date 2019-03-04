#include <iostream>

class Object {};

Object* alloc() {
    return new Object();
}

void dealloc(Object *obj) {
    ::free(obj);
}

int main() {
    dealloc(alloc());
    return 0;
}
