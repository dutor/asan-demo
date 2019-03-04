#include <iostream>

int main() {
    auto *ptr = new char[1024];
    delete [] ptr;
    delete [] ptr;
    return 0;
}
