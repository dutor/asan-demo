#include <iostream>
#include <cstring>

char* alloc() {
    return new char[1024];
}

void dealloc(char *buffer) {
    delete [] buffer;
}

int main() {
    auto *buffer = alloc();
    ::memset(buffer, 0, 1025);
    dealloc(buffer);
    return 0;
}
