#include <iostream>

void direct_leak() {
    new char[1024];
}

int main() {
    direct_leak();
    return 0;
}
