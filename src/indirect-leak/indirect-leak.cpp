#include <iostream>

void indirect_leak() {
    auto **pptr = new char*[1];     // 8 bytes leaked directly
    pptr[0] = new char[1024];       // 1024 bytes leaked indirectly
}

int main() {
    indirect_leak();
    return 0;
}
