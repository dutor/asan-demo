#include <iostream>

int main() {
    char buffer[1024];
    fprintf(stdout, "%c\n", *(buffer - 1));
    return 0;
}
