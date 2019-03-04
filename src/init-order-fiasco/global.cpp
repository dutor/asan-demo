int __attribute__((noinline)) set_global() {
    return 123;
}
int global = set_global();
int read_global() {
    return global;
}
