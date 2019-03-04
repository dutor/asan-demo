#include <iostream>
#include <vector>

int main() {
    std::vector<char> v;
    v.reserve(1024);
    v.push_back(0);
    v[1] = 1;
    return 0;
}
