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
    ::memset(buffer - 1, 0, 1024);
    dealloc(buffer);
    return 0;
}
