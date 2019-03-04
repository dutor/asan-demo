#include <iostream>

void use_after_scope() {
    char *ptr = nullptr;
    {
        char array[1024];
        ptr = array + 1023;
    }
    fprintf(stderr, "%d\n", *ptr);
}

int main() {
    use_after_scope();
    return 0;
}
