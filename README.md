# Clone

```bash
$ git clone https://github.com/dutor/asan-demo.git
```


# Build
```bash
$ cd asan-demon
$ mkdir build
$ cd bulid
$ cmake ..
$ make
```


# Run

```bash
$ ctest
```


# Case Study

There are several kinds of memory issues that AddressSanitizer could dectect.

## Memory Leaks

### Direct Leaks

Memory isn't released on exit, and isn't referred by other memory space(pointer) which is also not released.

Example:

```cpp
void direct_leak() {
    new char[1024];
}

int main() {
    direct_leak();
    return 0;
}
```

### Indirect Leaks

Memory isn't released on exit, but is referred by other memory space(pointer) which is also not released.

Example:

```cpp
void indirect_leak() {
    auto **pptr = new char*[1];     // 8 bytes leaked directly
    pptr[0] = new char[1024];       // 1024 bytes leaked indirectly
}

int main() {
    indirect_leak();
    return 0;
}
```

## Use After Free on Heap Allocated Memory

Accessing memory spaces that have already been freed.

Example:

```cpp
char* alloc() {
    return new char[1024];
}

void dealloc(char *buffer) {
    delete [] buffer;
}

int main() {
    auto *buffer = alloc();
    dealloc(buffer);
    buffer[1023] = 0;
    return 0;
}
```

**NOTE**: There might be false-negative cases:

  * The freed memory has already been allocated to and used in another context. This leads to unexpected memory overwrite or data corruption.
  * The internal memory buffer of ASAN is not sufficient. You could adjust this buffer to a larger size via the option `quarantine_size_mb`.

## Double Free

Free the same allocated memory space more than once.

```cpp
int main() {
    auto *ptr = new char[1024];
    delete [] ptr;
    delete [] ptr;
    return 0;
}
```

**NOTE**: Often, double-free is caused by flawed logics, but it could also be due to race-conditions, e.g. concurrent assigment to the same `std::string`.

**NOTE**: This might be reported as heap-use-after-free when the same address was freed, then allocated again, then freed again, and finally accessed, in interleaved contexts.

## Out of Bound Memory Access

Overflowed or underflowed accessing on heap or stack memory spaces.

heap-buffer-overflow:

```cpp
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
```

stack-buffer-underflow:

```cpp
int main() {
    char buffer[1024];
    fprintf(stdout, "%c\n", *(buffer - 1));
    return 0;
}
```


## Mismatch Between Allocation and Deallocation

Such as `new` vs. `free`, `malloc` vs. `delete`, `new[]` vs. `delete`, etc.

```cpp
class Object {};

Object* alloc() {
    return new Object();
}

void dealloc(Object *obj) {
    ::free(obj);
}

int main() {
    dealloc(alloc());
    return 0;
}
```

## Accessing Stale Objects on Stack

stack-use-after-return:

```cpp
char* use_after_scope(size_t offset) {
    char array[1024];
    char *ptr = array + offset;
    return ptr;
}

int main() {
    auto *ptr = use_after_scope(1023);
    *ptr = 1;
    return 0;
}
```

outputs:
```
=================================================================
==39536==ERROR: AddressSanitizer: stack-use-after-return on address 0x7f7caa50041f at pc 0x0000004012fd bp 0x7ffd988eb4b0 sp 0x7ffd988eb4a0
WRITE of size 1 at 0x7f7caa50041f thread T0
    #0 0x4012fc in main src/stack-use-after-return/stack-use-after-return.cpp:11
    #1 0x7f7cad964412 in __libc_start_main ../csu/libc-start.c:308
    #2 0x40111d in _start (build/bin/stack-use-after-return+0x40111d)

Address 0x7f7caa50041f is located in stack of thread T0 at offset 1055 in frame
    #0 0x4011e5 in use_after_scope(unsigned long) src/stack-use-after-return/stack-use-after-return.cpp:3

  This frame has 1 object(s):
    [32, 1056) 'array' <== Memory access at offset 1055 is inside this variable
HINT: this may be a false positive if your program uses some custom stack unwind mechanism or swapcontext
      (longjmp and C++ exceptions *are* supported)
SUMMARY: AddressSanitizer: stack-use-after-return src/stack-use-after-return/stack-use-after-return.cpp:11 in main
Shadow bytes around the buggy address:
  0x0ff015498030: f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5
  0x0ff015498040: f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5
  0x0ff015498050: f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5
  0x0ff015498060: f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5
  0x0ff015498070: f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5
=>0x0ff015498080: f5 f5 f5[f5]f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5
  0x0ff015498090: f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5
  0x0ff0154980a0: f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5
  0x0ff0154980b0: f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5
  0x0ff0154980c0: f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5
  0x0ff0154980d0: f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5
Shadow byte legend (one shadow byte represents 8 application bytes):
  Addressable:           00
  Partially addressable: 01 02 03 04 05 06 07
  Heap left redzone:       fa
  Freed heap region:       fd
  Stack left redzone:      f1
  Stack mid redzone:       f2
  Stack right redzone:     f3
  Stack after return:      f5
  Stack use after scope:   f8
  Global redzone:          f9
  Global init order:       f6
  Poisoned by user:        f7
  Container overflow:      fc
  Array cookie:            ac
  Intra object redzone:    bb
  ASan internal:           fe
  Left alloca redzone:     ca
  Right alloca redzone:    cb
==39536==ABORTING
```

stack-use-after-scope:

```cpp
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
```

outputs:
```
=================================================================
==39534==ERROR: AddressSanitizer: stack-use-after-scope on address 0x7f5020f0041f at pc 0x0000004012e5 bp 0x7ffe724a7b60 sp 0x7ffe724a7b50
READ of size 1 at 0x7f5020f0041f thread T0
    #0 0x4012e4 in use_after_scope() src/stack-use-after-scope/stack-use-after-scope.cpp:9
    #1 0x4013a4 in main src/stack-use-after-scope/stack-use-after-scope.cpp:13
    #2 0x7f502438c412 in __libc_start_main ../csu/libc-start.c:308
    #3 0x40114d in _start (build/bin/stack-use-after-scope+0x40114d)

Address 0x7f5020f0041f is located in stack of thread T0 at offset 1055 in frame
    #0 0x401215 in use_after_scope() src/stack-use-after-scope/stack-use-after-scope.cpp:3

  This frame has 1 object(s):
    [32, 1056) 'array' <== Memory access at offset 1055 is inside this variable
HINT: this may be a false positive if your program uses some custom stack unwind mechanism or swapcontext
      (longjmp and C++ exceptions *are* supported)
SUMMARY: AddressSanitizer: stack-use-after-scope src/stack-use-after-scope/stack-use-after-scope.cpp:9 in use_after_scope()
Shadow bytes around the buggy address:
  0x0fea841d8030: f8 f8 f8 f8 f8 f8 f8 f8 f8 f8 f8 f8 f8 f8 f8 f8
  0x0fea841d8040: f8 f8 f8 f8 f8 f8 f8 f8 f8 f8 f8 f8 f8 f8 f8 f8
  0x0fea841d8050: f8 f8 f8 f8 f8 f8 f8 f8 f8 f8 f8 f8 f8 f8 f8 f8
  0x0fea841d8060: f8 f8 f8 f8 f8 f8 f8 f8 f8 f8 f8 f8 f8 f8 f8 f8
  0x0fea841d8070: f8 f8 f8 f8 f8 f8 f8 f8 f8 f8 f8 f8 f8 f8 f8 f8
=>0x0fea841d8080: f8 f8 f8[f8]f3 f3 f3 f3 00 00 00 00 00 00 00 00
  0x0fea841d8090: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x0fea841d80a0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x0fea841d80b0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x0fea841d80c0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x0fea841d80d0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
Shadow byte legend (one shadow byte represents 8 application bytes):
  Addressable:           00
  Partially addressable: 01 02 03 04 05 06 07
  Heap left redzone:       fa
  Freed heap region:       fd
  Stack left redzone:      f1
  Stack mid redzone:       f2
  Stack right redzone:     f3
  Stack after return:      f5
  Stack use after scope:   f8
  Global redzone:          f9
  Global init order:       f6
  Poisoned by user:        f7
  Container overflow:      fc
  Array cookie:            ac
  Intra object redzone:    bb
  ASan internal:           fe
  Left alloca redzone:     ca
  Right alloca redzone:    cb
==39534==ABORTING
```
