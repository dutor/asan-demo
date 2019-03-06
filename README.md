# Introduction

AddressSanitizer, aka. ASan, is an open source tool invented by Google that dectects memory related bugs like memory leaks and corruption. ASan is based on the compiler instrumentation combined with a runtime library.


# Clone

```console
$ git clone https://github.com/dutor/asan-demo.git
```


# Build
```console
$ cd asan-demon
$ mkdir build
$ cd bulid
$ cmake ..
$ make
$ bin/asan-demo
Usage: bin/asan-demo <case-name>
Available cases:
        alloc-dealloc-mismatch
        direct-leak
        double-free
        heap-buffer-overflow
        heap-buffer-underflow
        heap-use-after-free
        indirect-leak
        stack-buffer-overflow
        stack-buffer-underflow
        stack-use-after-return
        stack-use-after-scope
        vector-overflow
$ bin/asan-demo double-free
...
```


# Tips

## How ASan Works?

Please refer to this paper, ["AddressSanitizer: a fast address sanity checker"](https://www.usenix.org/system/files/conference/atc12/atc12-final39.pdf)

## How to Enbale ASan in Your Own Project

Currently, ASan is implemented by GCC(starting from 4.8) and Clang(starting from 3.1), the newer version the better.
To enbale it, you have to compile your whole project with the following options:

  * `-fsanitize=address`, both on compiler **AND** linker.
  * `-g`, compile option to enable debuginfo.
  * `-fno-omit-frame-pointer`, compile option to make the backtrace more friendly and the stack unwinding faster.


## Don't Use With Other third-party Allocators

ASan doesn't work well with some other third-party allocators, such as tcmalloc or jemalloc.

# Case Study

There are several kinds of memory issues that AddressSanitizer is able to dectect.

## Memory Leaks

### Direct Leaks

Memory isn't released on exit, and isn't referred by other memory space(pointer) which is also not released.

Example:

```cpp
void DirectLeak() {
    new char[1024];
}
```

outputs:
```
==40799==ERROR: LeakSanitizer: detected memory leaks

Direct leak of 1024 byte(s) in 1 object(s) allocated from:
    #0 0x7f2f2844b650 in operator new[](unsigned long) (/lib64/libasan.so.5+0xf1650)
    #1 0x40a433 in DirectLeak() src/main.cpp:10
    #2 0x40e088 in std::_Function_handler<void (), void (*)()>::_M_invoke(std::_Any_data const&) /usr/include/c++/8/bits/std_function.h:297
    #3 0x40d605 in std::function<void ()>::operator()() const /usr/include/c++/8/bits/std_function.h:687
    #4 0x40b8d5 in main src/main.cpp:130
    #5 0x7f2f27e81412 in __libc_start_main ../csu/libc-start.c:308
    #6 0x40a36d in _start (build/bin/asan-demo+0x40a36d)

SUMMARY: AddressSanitizer: 1024 byte(s) leaked in 1 allocation(s).
```

### Indirect Leaks

Memory isn't released on exit, but is referred by other memory space(pointer) which is also not released.

Example:

```cpp
void IndirectLeak() {
    auto **pptr = new char*[1];     // 8 bytes leaked directly
    pptr[0] = new char[1024];       // 1024 bytes leaked indirectly
}
```

outputs:
```
==40801==ERROR: LeakSanitizer: detected memory leaks

Direct leak of 8 byte(s) in 1 object(s) allocated from:
    #0 0x7f4c3108c650 in operator new[](unsigned long) (/lib64/libasan.so.5+0xf1650)
    #1 0x40a448 in IndirectLeak() src/main.cpp:15
    #2 0x40e088 in std::_Function_handler<void (), void (*)()>::_M_invoke(std::_Any_data const&) /usr/include/c++/8/bits/std_function.h:297
    #3 0x40d605 in std::function<void ()>::operator()() const /usr/include/c++/8/bits/std_function.h:687
    #4 0x40b8d5 in main src/main.cpp:130
    #5 0x7f4c30ac2412 in __libc_start_main ../csu/libc-start.c:308
    #6 0x40a36d in _start (build/bin/asan-demo+0x40a36d)

Indirect leak of 1024 byte(s) in 1 object(s) allocated from:
    #0 0x7f4c3108c650 in operator new[](unsigned long) (/lib64/libasan.so.5+0xf1650)
    #1 0x40a456 in IndirectLeak() src/main.cpp:16
    #2 0x40e088 in std::_Function_handler<void (), void (*)()>::_M_invoke(std::_Any_data const&) /usr/include/c++/8/bits/std_function.h:297
    #3 0x40d605 in std::function<void ()>::operator()() const /usr/include/c++/8/bits/std_function.h:687
    #4 0x40b8d5 in main src/main.cpp:130
    #5 0x7f4c30ac2412 in __libc_start_main ../csu/libc-start.c:308
    #6 0x40a36d in _start (build/bin/asan-demo+0x40a36d)

SUMMARY: AddressSanitizer: 1032 byte(s) leaked in 2 allocation(s).
```

## Use After Free on Heap Allocated Memory

Accessing memory spaces that have already been freed.

Example:

```cpp
void HeapUseAfterFree() {
    auto *buffer = new char[1024];
    delete [] buffer;
    fprintf(stdout, "%c\n", buffer[1023]);
}
```

outputs:
```
==40803==ERROR: AddressSanitizer: heap-use-after-free on address 0x61900000047f at pc 0x00000040a52c bp 0x7fff9200dbf0 sp 0x7fff9200dbe0
READ of size 1 at 0x61900000047f thread T0
    #0 0x40a52b in HeapUseAfterFree() src/main.cpp:30
    #1 0x40e088 in std::_Function_handler<void (), void (*)()>::_M_invoke(std::_Any_data const&) /usr/include/c++/8/bits/std_function.h:297
    #2 0x40d605 in std::function<void ()>::operator()() const /usr/include/c++/8/bits/std_function.h:687
    #3 0x40b8d5 in main src/main.cpp:130
    #4 0x7f9a498ff412 in __libc_start_main ../csu/libc-start.c:308
    #5 0x40a36d in _start (build/bin/asan-demo+0x40a36d)

0x61900000047f is located 1023 bytes inside of 1024-byte region [0x619000000080,0x619000000480)
freed by thread T0 here:
    #0 0x7f9a49eca508 in operator delete[](void*) (/lib64/libasan.so.5+0xf2508)
    #1 0x40a4ec in HeapUseAfterFree() src/main.cpp:29
    #2 0x40e088 in std::_Function_handler<void (), void (*)()>::_M_invoke(std::_Any_data const&) /usr/include/c++/8/bits/std_function.h:297
    #3 0x40d605 in std::function<void ()>::operator()() const /usr/include/c++/8/bits/std_function.h:687
    #4 0x40b8d5 in main src/main.cpp:130
    #5 0x7f9a498ff412 in __libc_start_main ../csu/libc-start.c:308
    #6 0x40a36d in _start (build/bin/asan-demo+0x40a36d)

previously allocated by thread T0 here:
    #0 0x7f9a49ec9650 in operator new[](unsigned long) (/lib64/libasan.so.5+0xf1650)
    #1 0x40a4d5 in HeapUseAfterFree() src/main.cpp:28
    #2 0x40e088 in std::_Function_handler<void (), void (*)()>::_M_invoke(std::_Any_data const&) /usr/include/c++/8/bits/std_function.h:297
    #3 0x40d605 in std::function<void ()>::operator()() const /usr/include/c++/8/bits/std_function.h:687
    #4 0x40b8d5 in main src/main.cpp:130
    #5 0x7f9a498ff412 in __libc_start_main ../csu/libc-start.c:308
    #6 0x40a36d in _start (build/bin/asan-demo+0x40a36d)

SUMMARY: AddressSanitizer: heap-use-after-free src/main.cpp:30 in HeapUseAfterFree()
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
==40803==ABORTING
```

**NOTE**: There might be false-negative cases:

  * The freed memory has already been allocated to and used in another context. This leads to unexpected memory overwrite or data corruption.
  * The internal memory buffer of ASan is not sufficient. You could adjust this buffer to a larger size via the option `quarantine_size_mb`.

## Double Free

Free the same allocated memory space more than once.

```cpp
void DoubleFree() {
    auto *ptr = new char[1024];
    delete [] ptr;
    delete [] ptr;
}
```

outputs:
```
==40806==ERROR: AddressSanitizer: attempting double-free on 0x619000000080 in thread T0:
    #0 0x7f3e30d56508 in operator delete[](void*) (/lib64/libasan.so.5+0xf2508)
    #1 0x40a4c0 in DoubleFree() src/main.cpp:23
    #2 0x40e088 in std::_Function_handler<void (), void (*)()>::_M_invoke(std::_Any_data const&) /usr/include/c++/8/bits/std_function.h:297
    #3 0x40d605 in std::function<void ()>::operator()() const /usr/include/c++/8/bits/std_function.h:687
    #4 0x40b8d5 in main src/main.cpp:130
    #5 0x7f3e3078b412 in __libc_start_main ../csu/libc-start.c:308
    #6 0x40a36d in _start (build/bin/asan-demo+0x40a36d)

0x619000000080 is located 0 bytes inside of 1024-byte region [0x619000000080,0x619000000480)
freed by thread T0 here:
    #0 0x7f3e30d56508 in operator delete[](void*) (/lib64/libasan.so.5+0xf2508)
    #1 0x40a4ad in DoubleFree() src/main.cpp:22
    #2 0x40e088 in std::_Function_handler<void (), void (*)()>::_M_invoke(std::_Any_data const&) /usr/include/c++/8/bits/std_function.h:297
    #3 0x40d605 in std::function<void ()>::operator()() const /usr/include/c++/8/bits/std_function.h:687
    #4 0x40b8d5 in main src/main.cpp:130
    #5 0x7f3e3078b412 in __libc_start_main ../csu/libc-start.c:308
    #6 0x40a36d in _start (build/bin/asan-demo+0x40a36d)

previously allocated by thread T0 here:
    #0 0x7f3e30d55650 in operator new[](unsigned long) (/lib64/libasan.so.5+0xf1650)
    #1 0x40a496 in DoubleFree() src/main.cpp:21
    #2 0x40e088 in std::_Function_handler<void (), void (*)()>::_M_invoke(std::_Any_data const&) /usr/include/c++/8/bits/std_function.h:297
    #3 0x40d605 in std::function<void ()>::operator()() const /usr/include/c++/8/bits/std_function.h:687
    #4 0x40b8d5 in main src/main.cpp:130
    #5 0x7f3e3078b412 in __libc_start_main ../csu/libc-start.c:308
    #6 0x40a36d in _start (build/bin/asan-demo+0x40a36d)

SUMMARY: AddressSanitizer: double-free (/lib64/libasan.so.5+0xf2508) in operator delete[](void*)
==40806==ABORTING
```

**NOTE**: Often, double-free is caused by flawed logics, but it could also be due to race-conditions, e.g. concurrent assigment to the same `std::string`.

**NOTE**: This might be reported as heap-use-after-free when the same address was freed, then allocated again, then freed again, and finally accessed, in interleaved contexts.

## Out of Bound Memory Access

Overflowed or underflowed accessing on heap or stack memory spaces.

heap-buffer-overflow:

```cpp
void HeapBufferOverflow() {
    auto *buffer = new char[1024];
    ::memset(buffer, 0, 1025);
    delete [] buffer;
}
```

outputs:
```
==40807==ERROR: AddressSanitizer: heap-buffer-overflow on address 0x619000000480 at pc 0x7f3c5c5c677d bp 0x7ffc190c5da0 sp 0x7ffc190c5548
WRITE of size 1025 at 0x619000000480 thread T0
    #0 0x7f3c5c5c677c  (/lib64/libasan.so.5+0x9b77c)
    #1 0x40a59b in HeapBufferOverflow() src/main.cpp:36
    #2 0x40e088 in std::_Function_handler<void (), void (*)()>::_M_invoke(std::_Any_data const&) /usr/include/c++/8/bits/std_function.h:297
    #3 0x40d605 in std::function<void ()>::operator()() const /usr/include/c++/8/bits/std_function.h:687
    #4 0x40b8d5 in main src/main.cpp:130
    #5 0x7f3c5c052412 in __libc_start_main ../csu/libc-start.c:308
    #6 0x40a36d in _start (build/bin/asan-demo+0x40a36d)

0x619000000480 is located 0 bytes to the right of 1024-byte region [0x619000000080,0x619000000480)
allocated by thread T0 here:
    #0 0x7f3c5c61c650 in operator new[](unsigned long) (/lib64/libasan.so.5+0xf1650)
    #1 0x40a581 in HeapBufferOverflow() src/main.cpp:35
    #2 0x40e088 in std::_Function_handler<void (), void (*)()>::_M_invoke(std::_Any_data const&) /usr/include/c++/8/bits/std_function.h:297
    #3 0x40d605 in std::function<void ()>::operator()() const /usr/include/c++/8/bits/std_function.h:687
    #4 0x40b8d5 in main src/main.cpp:130
    #5 0x7f3c5c052412 in __libc_start_main ../csu/libc-start.c:308
    #6 0x40a36d in _start (build/bin/asan-demo+0x40a36d)

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
==40807==ABORTING
```

stack-buffer-underflow:

```cpp
void StackBufferUnderflow() {
    char buffer[1024];
    fprintf(stdout, "%c\n", *(buffer - 1));
}
```

outputs:
```
==40808==ERROR: AddressSanitizer: stack-buffer-underflow on address 0x7f197880681f at pc 0x00000040a7de bp 0x7fffe6021d80 sp 0x7fffe6021d70
READ of size 1 at 0x7f197880681f thread T0
    #0 0x40a7dd in StackBufferUnderflow() src/main.cpp:56
    #1 0x40e088 in std::_Function_handler<void (), void (*)()>::_M_invoke(std::_Any_data const&) /usr/include/c++/8/bits/std_function.h:297
    #2 0x40d605 in std::function<void ()>::operator()() const /usr/include/c++/8/bits/std_function.h:687
    #3 0x40b8d5 in main src/main.cpp:130
    #4 0x7f197ba08412 in __libc_start_main ../csu/libc-start.c:308
    #5 0x40a36d in _start (build/bin/asan-demo+0x40a36d)

Address 0x7f197880681f is located in stack of thread T0 at offset 31 in frame
    #0 0x40a73c in StackBufferUnderflow() src/main.cpp:54

  This frame has 1 object(s):
    [32, 1056) 'buffer' <== Memory access at offset 31 underflows this variable
HINT: this may be a false positive if your program uses some custom stack unwind mechanism or swapcontext
      (longjmp and C++ exceptions *are* supported)
SUMMARY: AddressSanitizer: stack-buffer-underflow src/main.cpp:56 in StackBufferUnderflow()
Shadow bytes around the buggy address:
  0x0fe3af0f8cb0: f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5
  0x0fe3af0f8cc0: f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5
  0x0fe3af0f8cd0: f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5
  0x0fe3af0f8ce0: f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5
  0x0fe3af0f8cf0: f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5
=>0x0fe3af0f8d00: f1 f1 f1[f1]00 00 00 00 00 00 00 00 00 00 00 00
  0x0fe3af0f8d10: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x0fe3af0f8d20: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x0fe3af0f8d30: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x0fe3af0f8d40: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x0fe3af0f8d50: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
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
==40808==ABORTING
```


## Mismatch Between Allocation and Deallocation

Such as `new` vs. `free`, `malloc` vs. `delete`, `new[]` vs. `delete`, etc.

```cpp
void AllocDeallocMismatch() {
    ::free(new std::string(__PRETTY_FUNCTION__));
}
```

outputs:
```
==40809==ERROR: AddressSanitizer: alloc-dealloc-mismatch (operator new vs free) on 0x603000000340
    #0 0x7f0b547e4480 in free (/lib64/libasan.so.5+0xef480)
    #1 0x40af0a in AllocDeallocMismatch() src/main.cpp:90
    #2 0x40e088 in std::_Function_handler<void (), void (*)()>::_M_invoke(std::_Any_data const&) /usr/include/c++/8/bits/std_function.h:297
    #3 0x40d605 in std::function<void ()>::operator()() const /usr/include/c++/8/bits/std_function.h:687
    #4 0x40b8d5 in main src/main.cpp:130
    #5 0x7f0b5421c412 in __libc_start_main ../csu/libc-start.c:308
    #6 0x40a36d in _start (build/bin/asan-demo+0x40a36d)

0x603000000340 is located 0 bytes inside of 32-byte region [0x603000000340,0x603000000360)
allocated by thread T0 here:
    #0 0x7f0b547e6470 in operator new(unsigned long) (/lib64/libasan.so.5+0xf1470)
    #1 0x40aeef in AllocDeallocMismatch() src/main.cpp:90
    #2 0x40e088 in std::_Function_handler<void (), void (*)()>::_M_invoke(std::_Any_data const&) /usr/include/c++/8/bits/std_function.h:297
    #3 0x40d605 in std::function<void ()>::operator()() const /usr/include/c++/8/bits/std_function.h:687
    #4 0x40b8d5 in main src/main.cpp:130
    #5 0x7f0b5421c412 in __libc_start_main ../csu/libc-start.c:308
    #6 0x40a36d in _start (build/bin/asan-demo+0x40a36d)

SUMMARY: AddressSanitizer: alloc-dealloc-mismatch (/lib64/libasan.so.5+0xef480) in free
==40809==HINT: if you don't care about these errors you may set ASAN_OPTIONS=alloc_dealloc_mismatch=0
==40809==ABORTING
```

## Accessing Stale Objects on Stack

stack-use-after-return:

```cpp
void StackUseAfterReturn() {
    auto get = [] (size_t offset) {
        char array[1024];
        char *ptr = array + offset;
        return ptr;
    };
    auto *ptr = get(1023);
    fprintf(stdout, "%c\n", *ptr);
}
```

outputs:
```
==40810==ERROR: AddressSanitizer: stack-use-after-return on address 0x7f31e6506c1f at pc 0x00000040aa10 bp 0x7ffc2cf9a690 sp 0x7ffc2cf9a680
READ of size 1 at 0x7f31e6506c1f thread T0
    #0 0x40aa0f in StackUseAfterReturn() src/main.cpp:67
    #1 0x40e088 in std::_Function_handler<void (), void (*)()>::_M_invoke(std::_Any_data const&) /usr/include/c++/8/bits/std_function.h:297
    #2 0x40d605 in std::function<void ()>::operator()() const /usr/include/c++/8/bits/std_function.h:687
    #3 0x40b8d5 in main src/main.cpp:130
    #4 0x7f31e9773412 in __libc_start_main ../csu/libc-start.c:308
    #5 0x40a36d in _start (build/bin/asan-demo+0x40a36d)

Address 0x7f31e6506c1f is located in stack of thread T0 at offset 1055 in frame
    #0 0x40a877 in operator() src/main.cpp:61

  This frame has 1 object(s):
    [32, 1056) 'array' <== Memory access at offset 1055 is inside this variable
HINT: this may be a false positive if your program uses some custom stack unwind mechanism or swapcontext
      (longjmp and C++ exceptions *are* supported)
SUMMARY: AddressSanitizer: stack-use-after-return src/main.cpp:67 in StackUseAfterReturn()
Shadow bytes around the buggy address:
  0x0fe6bcc98d30: f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5
  0x0fe6bcc98d40: f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5
  0x0fe6bcc98d50: f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5
  0x0fe6bcc98d60: f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5
  0x0fe6bcc98d70: f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5
=>0x0fe6bcc98d80: f5 f5 f5[f5]f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5
  0x0fe6bcc98d90: f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5
  0x0fe6bcc98da0: f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5
  0x0fe6bcc98db0: f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5
  0x0fe6bcc98dc0: f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5
  0x0fe6bcc98dd0: f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5 f5
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
==40810==ABORTING
```

stack-use-after-scope:

```cpp
void StackUseAfterScope() {
    char *ptr = nullptr;
    {
        char array[1024];
        ptr = array + 1023;
    }
    fprintf(stderr, "%c\n", *ptr);
}
```

outputs:
```
==40812==ERROR: AddressSanitizer: stack-use-after-scope on address 0x7f4045206c1f at pc 0x00000040ab87 bp 0x7ffdb2773380 sp 0x7ffdb2773370
READ of size 1 at 0x7f4045206c1f thread T0
    #0 0x40ab86 in StackUseAfterScope() src/main.cpp:77
    #1 0x40e088 in std::_Function_handler<void (), void (*)()>::_M_invoke(std::_Any_data const&) /usr/include/c++/8/bits/std_function.h:297
    #2 0x40d605 in std::function<void ()>::operator()() const /usr/include/c++/8/bits/std_function.h:687
    #3 0x40b8d5 in main src/main.cpp:130
    #4 0x7f404846a412 in __libc_start_main ../csu/libc-start.c:308
    #5 0x40a36d in _start (build/bin/asan-demo+0x40a36d)

Address 0x7f4045206c1f is located in stack of thread T0 at offset 1055 in frame
    #0 0x40aab7 in StackUseAfterScope() src/main.cpp:71

  This frame has 1 object(s):
    [32, 1056) 'array' <== Memory access at offset 1055 is inside this variable
HINT: this may be a false positive if your program uses some custom stack unwind mechanism or swapcontext
      (longjmp and C++ exceptions *are* supported)
SUMMARY: AddressSanitizer: stack-use-after-scope src/main.cpp:77 in StackUseAfterScope()
Shadow bytes around the buggy address:
  0x0fe888a38d30: f8 f8 f8 f8 f8 f8 f8 f8 f8 f8 f8 f8 f8 f8 f8 f8
  0x0fe888a38d40: f8 f8 f8 f8 f8 f8 f8 f8 f8 f8 f8 f8 f8 f8 f8 f8
  0x0fe888a38d50: f8 f8 f8 f8 f8 f8 f8 f8 f8 f8 f8 f8 f8 f8 f8 f8
  0x0fe888a38d60: f8 f8 f8 f8 f8 f8 f8 f8 f8 f8 f8 f8 f8 f8 f8 f8
  0x0fe888a38d70: f8 f8 f8 f8 f8 f8 f8 f8 f8 f8 f8 f8 f8 f8 f8 f8
=>0x0fe888a38d80: f8 f8 f8[f8]f3 f3 f3 f3 00 00 00 00 00 00 00 00
  0x0fe888a38d90: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x0fe888a38da0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x0fe888a38db0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x0fe888a38dc0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x0fe888a38dd0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
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
==40812==ABORTING
```


## Container Overflow


Example:
```cpp
void VectorOverflow() {
    std::vector<char> v;
    v.reserve(1024);
    v.push_back(0);
    v[1] = 1;
}
```

outputs:
```
==40834==ERROR: AddressSanitizer: container-overflow on address 0x619000000081 at pc 0x00000040ad96 bp 0x7fffd8a322d0 sp 0x7fffd8a322c0
WRITE of size 1 at 0x619000000081 thread T0
    #0 0x40ad95 in VectorOverflow() src/main.cpp:85
    #1 0x40e088 in std::_Function_handler<void (), void (*)()>::_M_invoke(std::_Any_data const&) /usr/include/c++/8/bits/std_function.h:297
    #2 0x40d605 in std::function<void ()>::operator()() const /usr/include/c++/8/bits/std_function.h:687
    #3 0x40b8d5 in main src/main.cpp:130
    #4 0x7f4770b00412 in __libc_start_main ../csu/libc-start.c:308
    #5 0x40a36d in _start (build/bin/asan-demo+0x40a36d)

0x619000000081 is located 1 bytes inside of 1024-byte region [0x619000000080,0x619000000480)
allocated by thread T0 here:
    #0 0x7f47710ca470 in operator new(unsigned long) (/lib64/libasan.so.5+0xf1470)
    #1 0x411bfe in __gnu_cxx::new_allocator<char>::allocate(unsigned long, void const*) /usr/include/c++/8/ext/new_allocator.h:111
    #2 0x41011e in std::allocator_traits<std::allocator<char> >::allocate(std::allocator<char>&, unsigned long) /usr/include/c++/8/bits/alloc_traits.h:436
    #3 0x40eddb in std::_Vector_base<char, std::allocator<char> >::_M_allocate(unsigned long) /usr/include/c++/8/bits/stl_vector.h:296
    #4 0x40da91 in char* std::vector<char, std::allocator<char> >::_M_allocate_and_copy<std::move_iterator<char*> >(unsigned long, std::move_iterator<char*>, std::move_iterator<char*>) /usr/include/c++/8/bits/stl_vector.h:1398
    #5 0x40cbdc in std::vector<char, std::allocator<char> >::reserve(unsigned long) /usr/include/c++/8/bits/vector.tcc:74
    #6 0x40acf6 in VectorOverflow() src/main.cpp:83
    #7 0x40e088 in std::_Function_handler<void (), void (*)()>::_M_invoke(std::_Any_data const&) /usr/include/c++/8/bits/std_function.h:297
    #8 0x40d605 in std::function<void ()>::operator()() const /usr/include/c++/8/bits/std_function.h:687
    #9 0x40b8d5 in main src/main.cpp:130
    #10 0x7f4770b00412 in __libc_start_main ../csu/libc-start.c:308
    #11 0x40a36d in _start (build/bin/asan-demo+0x40a36d)

HINT: if you don't care about these errors you may set ASAN_OPTIONS=detect_container_overflow=0.
If you suspect a false positive see also: https://github.com/google/sanitizers/wiki/AddressSanitizerContainerOverflow.
SUMMARY: AddressSanitizer: container-overflow src/main.cpp:85 in VectorOverflow()
Shadow bytes around the buggy address:
  0x0c327fff7fc0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x0c327fff7fd0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x0c327fff7fe0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x0c327fff7ff0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x0c327fff8000: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
=>0x0c327fff8010:[01]fc fc fc fc fc fc fc fc fc fc fc fc fc fc fc
  0x0c327fff8020: fc fc fc fc fc fc fc fc fc fc fc fc fc fc fc fc
  0x0c327fff8030: fc fc fc fc fc fc fc fc fc fc fc fc fc fc fc fc
  0x0c327fff8040: fc fc fc fc fc fc fc fc fc fc fc fc fc fc fc fc
  0x0c327fff8050: fc fc fc fc fc fc fc fc fc fc fc fc fc fc fc fc
  0x0c327fff8060: fc fc fc fc fc fc fc fc fc fc fc fc fc fc fc fc
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
==40834==ABORTING
```
