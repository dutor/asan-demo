#include <iostream>


char* alloc() {
    return new char[1024];
}

void dealloc(char *buffer) {
    delete [] buffer;
}

int main() {
    auto *buffer = alloc();
    dealloc(buffer);
    buffer[1023] = 0;
    return 0;
}
