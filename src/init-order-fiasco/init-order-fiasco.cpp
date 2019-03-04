#include <iostream>

int read_global();

int my_global = read_global();

int main() {
    fprintf(stdout, "%d\n", my_global);
    return 0;
}
