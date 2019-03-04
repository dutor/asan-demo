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
#include <iostream>

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
#include <iostream>

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
#include <iostream>

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
#include <iostream>

int main() {
    auto *ptr = new char[1024];
    delete [] ptr;
    delete [] ptr;
    return 0;
}
```

**NOTE**: Often, double-free is caused by wrong logics, but it could also be due to race-conditions, e.g. concurrent assigment to the same `std::string`.

**NOTE**: This might be reported as heap-use-after-free when the same address was freed, then allocated again, then freed again, and finally accessed, in interleaved contexts.

## Out of Bound Memory Access

Overflowed or underflowed accessing on heap or stack memory spaces.

heap-buffer-overflow:

```cpp
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
    ::memset(buffer, 0, 1025);
    dealloc(buffer);
    return 0;
}
```

stack-buffer-underflow:

```cpp
#include <iostream>

int main() {
    char buffer[1024];
    fprintf(stdout, "%c\n", *(buffer - 1));
    return 0;
}
```


## Mismatch Between Allocation and Deallocation

Such as `new` vs. `free`, `malloc` vs. `delete`, `new[]` vs. `delete`, etc.

```cpp
#include <iostream>

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
#include <iostream>

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

stack-use-after-scope:

```cpp
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
```
