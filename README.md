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

outputs:
```
=================================================================
==39592==ERROR: LeakSanitizer: detected memory leaks

Direct leak of 1024 byte(s) in 1 object(s) allocated from:
    #0 0x7f0a75c72650 in operator new[](unsigned long) (/lib64/libasan.so.5+0xf1650)
    #1 0x4011c3 in direct_leak() src/direct-leak/direct-leak.cpp:4
    #2 0x4011cf in main src/direct-leak/direct-leak.cpp:8
    #3 0x7f0a756a8412 in __libc_start_main ../csu/libc-start.c:308
    #4 0x4010fd in _start (build/bin/direct-leak+0x4010fd)

SUMMARY: AddressSanitizer: 1024 byte(s) leaked in 1 allocation(s).
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

outputs:
```
=================================================================
==39590==ERROR: LeakSanitizer: detected memory leaks

Direct leak of 8 byte(s) in 1 object(s) allocated from:
    #0 0x7f552ff00650 in operator new[](unsigned long) (/lib64/libasan.so.5+0xf1650)
    #1 0x4011d7 in indirect_leak() src/indirect-leak/indirect-leak.cpp:4
    #2 0x40121c in main src/indirect-leak/indirect-leak.cpp:9
    #3 0x7f552f936412 in __libc_start_main ../csu/libc-start.c:308
    #4 0x40110d in _start (build/bin/indirect-leak+0x40110d)

Indirect leak of 1024 byte(s) in 1 object(s) allocated from:
    #0 0x7f552ff00650 in operator new[](unsigned long) (/lib64/libasan.so.5+0xf1650)
    #1 0x4011e5 in indirect_leak() src/indirect-leak/indirect-leak.cpp:5
    #2 0x40121c in main src/indirect-leak/indirect-leak.cpp:9
    #3 0x7f552f936412 in __libc_start_main ../csu/libc-start.c:308
    #4 0x40110d in _start (build/bin/indirect-leak+0x40110d)

SUMMARY: AddressSanitizer: 1032 byte(s) leaked in 2 allocation(s).
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

outputs:
```
=================================================================
==39588==ERROR: AddressSanitizer: heap-use-after-free on address 0x61900000047f at pc 0x000000401264 bp 0x7fff70d5fd40 sp 0x7fff70d5fd30
WRITE of size 1 at 0x61900000047f thread T0
    #0 0x401263 in main src/heap-use-after-free/heap-use-after-free.cpp:15
    #1 0x7f0b5c121412 in __libc_start_main ../csu/libc-start.c:308
    #2 0x40111d in _start (build/bin/heap-use-after-free+0x40111d)

0x61900000047f is located 1023 bytes inside of 1024-byte region [0x619000000080,0x619000000480)
freed by thread T0 here:
    #0 0x7f0b5c6ec508 in operator delete[](void*) (/lib64/libasan.so.5+0xf2508)
    #1 0x401204 in dealloc(char*) src/heap-use-after-free/heap-use-after-free.cpp:9
    #2 0x401224 in main src/heap-use-after-free/heap-use-after-free.cpp:14
    #3 0x7f0b5c121412 in __libc_start_main ../csu/libc-start.c:308
    #4 0x40111d in _start (build/bin/heap-use-after-free+0x40111d)

previously allocated by thread T0 here:
    #0 0x7f0b5c6eb650 in operator new[](unsigned long) (/lib64/libasan.so.5+0xf1650)
    #1 0x4011e3 in alloc() src/heap-use-after-free/heap-use-after-free.cpp:5
    #2 0x401214 in main src/heap-use-after-free/heap-use-after-free.cpp:13
    #3 0x7f0b5c121412 in __libc_start_main ../csu/libc-start.c:308
    #4 0x40111d in _start (build/bin/heap-use-after-free+0x40111d)

SUMMARY: AddressSanitizer: heap-use-after-free src/heap-use-after-free/heap-use-after-free.cpp:15 in main
Shadow bytes around the buggy address:
  0x0c327fff8030: fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd
  0x0c327fff8040: fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd
  0x0c327fff8050: fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd
  0x0c327fff8060: fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd
  0x0c327fff8070: fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd
=>0x0c327fff8080: fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd[fd]
  0x0c327fff8090: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x0c327fff80a0: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x0c327fff80b0: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x0c327fff80c0: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x0c327fff80d0: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
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
==39588==ABORTING
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

outputs:
```
=================================================================
==39587==ERROR: AddressSanitizer: attempting double-free on 0x619000000080 in thread T0:
    #0 0x7eff5350d508 in operator delete[](void*) (/lib64/libasan.so.5+0xf2508)
    #1 0x401201 in main src/double-free/double-free.cpp:6
    #2 0x7eff52f42412 in __libc_start_main ../csu/libc-start.c:308
    #3 0x40110d in _start (build/bin/double-free+0x40110d)

0x619000000080 is located 0 bytes inside of 1024-byte region [0x619000000080,0x619000000480)
freed by thread T0 here:
    #0 0x7eff5350d508 in operator delete[](void*) (/lib64/libasan.so.5+0xf2508)
    #1 0x4011ee in main src/double-free/double-free.cpp:5
    #2 0x7eff52f42412 in __libc_start_main ../csu/libc-start.c:308
    #3 0x40110d in _start (build/bin/double-free+0x40110d)

previously allocated by thread T0 here:
    #0 0x7eff5350c650 in operator new[](unsigned long) (/lib64/libasan.so.5+0xf1650)
    #1 0x4011d7 in main src/double-free/double-free.cpp:4
    #2 0x7eff52f42412 in __libc_start_main ../csu/libc-start.c:308
    #3 0x40110d in _start (build/bin/double-free+0x40110d)

SUMMARY: AddressSanitizer: double-free (/lib64/libasan.so.5+0xf2508) in operator delete[](void*)
==39587==ABORTING
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

outputs:
```
=================================================================
==39586==ERROR: AddressSanitizer: heap-buffer-overflow on address 0x619000000480 at pc 0x7f51957ee77d bp 0x7ffec049d2a0 sp 0x7ffec049ca48
WRITE of size 1025 at 0x619000000480 thread T0
    #0 0x7f51957ee77c  (/lib64/libasan.so.5+0x9b77c)
    #1 0x40122e in main src/heap-buffer-overflow/heap-buffer-overflow.cpp:14
    #2 0x7f519527a412 in __libc_start_main ../csu/libc-start.c:308
    #3 0x40111d in _start (build/bin/heap-buffer-overflow+0x40111d)

0x619000000480 is located 0 bytes to the right of 1024-byte region [0x619000000080,0x619000000480)
allocated by thread T0 here:
    #0 0x7f5195844650 in operator new[](unsigned long) (/lib64/libasan.so.5+0xf1650)
    #1 0x4011e3 in alloc() src/heap-buffer-overflow/heap-buffer-overflow.cpp:5
    #2 0x401214 in main src/heap-buffer-overflow/heap-buffer-overflow.cpp:13
    #3 0x7f519527a412 in __libc_start_main ../csu/libc-start.c:308
    #4 0x40111d in _start (build/bin/heap-buffer-overflow+0x40111d)

SUMMARY: AddressSanitizer: heap-buffer-overflow (/lib64/libasan.so.5+0x9b77c)
Shadow bytes around the buggy address:
  0x0c327fff8040: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x0c327fff8050: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x0c327fff8060: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x0c327fff8070: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x0c327fff8080: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
=>0x0c327fff8090:[fa]fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x0c327fff80a0: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x0c327fff80b0: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x0c327fff80c0: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x0c327fff80d0: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x0c327fff80e0: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
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
==39586==ABORTING
```

stack-buffer-underflow:

```cpp
int main() {
    char buffer[1024];
    fprintf(stdout, "%c\n", *(buffer - 1));
    return 0;
}
```

outputs:
```
=================================================================
==39584==ERROR: AddressSanitizer: stack-buffer-underflow on address 0x7f657d00001f at pc 0x0000004012a9 bp 0x7fffc6035c00 sp 0x7fffc6035bf0
READ of size 1 at 0x7f657d00001f thread T0
    #0 0x4012a8 in main src/stack-buffer-underflow/stack-buffer-underflow.cpp:5
    #1 0x7f65803cf412 in __libc_start_main ../csu/libc-start.c:308
    #2 0x40113d in _start (build/bin/stack-buffer-underflow+0x40113d)

Address 0x7f657d00001f is located in stack of thread T0 at offset 31 in frame
    #0 0x401205 in main src/stack-buffer-underflow/stack-buffer-underflow.cpp:3

  This frame has 1 object(s):
    [32, 1056) 'buffer' <== Memory access at offset 31 underflows this variable
HINT: this may be a false positive if your program uses some custom stack unwind mechanism or swapcontext
      (longjmp and C++ exceptions *are* supported)
SUMMARY: AddressSanitizer: stack-buffer-underflow src/stack-buffer-underflow/stack-buffer-underflow.cpp:5 in main
Shadow bytes around the buggy address:
  0x0fed2f9f7fb0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x0fed2f9f7fc0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x0fed2f9f7fd0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x0fed2f9f7fe0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x0fed2f9f7ff0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
=>0x0fed2f9f8000: f1 f1 f1[f1]00 00 00 00 00 00 00 00 00 00 00 00
  0x0fed2f9f8010: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x0fed2f9f8020: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x0fed2f9f8030: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x0fed2f9f8040: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x0fed2f9f8050: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
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
==39584==ABORTING
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

outputs:
```
=================================================================
==39583==ERROR: AddressSanitizer: alloc-dealloc-mismatch (operator new vs free) on 0x602000000010
    #0 0x7f6bd86b9480 in free (/lib64/libasan.so.5+0xef480)
    #1 0x4011ed in dealloc(Object*) src/alloc-dealloc-mismatch/alloc-dealloc-mismatch.cpp:10
    #2 0x401201 in main src/alloc-dealloc-mismatch/alloc-dealloc-mismatch.cpp:14
    #3 0x7f6bd80f1412 in __libc_start_main ../csu/libc-start.c:308
    #4 0x40110d in _start (build/bin/alloc-dealloc-mismatch+0x40110d)

0x602000000010 is located 0 bytes inside of 1-byte region [0x602000000010,0x602000000011)
allocated by thread T0 here:
    #0 0x7f6bd86bb470 in operator new(unsigned long) (/lib64/libasan.so.5+0xf1470)
    #1 0x4011d3 in alloc() src/alloc-dealloc-mismatch/alloc-dealloc-mismatch.cpp:6
    #2 0x4011f9 in main src/alloc-dealloc-mismatch/alloc-dealloc-mismatch.cpp:14
    #3 0x7f6bd80f1412 in __libc_start_main ../csu/libc-start.c:308
    #4 0x40110d in _start (build/bin/alloc-dealloc-mismatch+0x40110d)

SUMMARY: AddressSanitizer: alloc-dealloc-mismatch (/lib64/libasan.so.5+0xef480) in free
==39583==HINT: if you don't care about these errors you may set ASAN_OPTIONS=alloc_dealloc_mismatch=0
==39583==ABORTING
```

## Accessing Stale Objects on Stack

stack-use-after-return:

```cpp
char* use_after_return(size_t offset) {
    char array[1024];
    char *ptr = array + offset;
    return ptr;
}

int main() {
    auto *ptr = use_after_return(1023);
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
