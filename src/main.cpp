#include <cstdio>
#include <string>
#include <cstring>
#include <vector>
#include <map>
#include <functional>


void DirectLeak() {
    new char[1024];
}


void IndirectLeak() {
    auto **pptr = new char*[1];     // 8 bytes leaked directly
    pptr[0] = new char[1024];       // 1024 bytes leaked indirectly
}


void DoubleFree() {
    auto *ptr = new char[1024];
    delete [] ptr;
    delete [] ptr;
}


void HeapUseAfterFree() {
    auto *buffer = new char[1024];
    delete [] buffer;
    fprintf(stdout, "%c\n", buffer[1023]);
}


void HeapBufferOverflow() {
    auto *buffer = new char[1024];
    ::memset(buffer, 0, 1025);
    delete [] buffer;
}


void HeapBufferUnderflow() {
    auto *buffer = new char[1024];
    ::memset(buffer - 1, 0, 1024);
    delete [] buffer;
}


void StackBufferOverflow() {
    char buffer[1024];
    fprintf(stdout, "%c\n", buffer[1024]);
}


void StackBufferUnderflow() {
    char buffer[1024];
    fprintf(stdout, "%c\n", *(buffer - 1));
}


void StackUseAfterReturn() {
    auto get = [] (size_t offset) {
        char array[1024];
        char *ptr = array + offset;
        return ptr;
    };
    auto *ptr = get(1023);
    fprintf(stdout, "%c\n", *ptr);
}


void StackUseAfterScope() {
    char *ptr = nullptr;
    {
        char array[1024];
        ptr = array + 1023;
    }
    fprintf(stderr, "%c\n", *ptr);
}


void VectorOverflow() {
    std::vector<char> v;
    v.reserve(1024);
    v.push_back(0);
    v[1] = 1;
}


void AllocDeallocMismatch() {
    ::free(new std::string(__PRETTY_FUNCTION__));
}



int main(int argc, char **argv) {
    std::map<std::string, std::function<void()>> cases = {
        {"direct-leak",             DirectLeak},
        {"indirect-leak",           IndirectLeak},
        {"double-free",             DoubleFree},
        {"heap-use-after-free",     HeapUseAfterFree},
        {"heap-buffer-overflow",    HeapBufferOverflow},
        {"heap-buffer-underflow",   HeapBufferUnderflow},
        {"stack-buffer-overflow",   StackBufferOverflow},
        {"stack-buffer-underflow",  StackBufferUnderflow},
        {"stack-use-after-return",  StackUseAfterReturn},
        {"stack-use-after-scope",   StackUseAfterScope},
        {"vector-overflow",         VectorOverflow},
        {"alloc-dealloc-mismatch",  AllocDeallocMismatch},
    };

    auto print_help = [&] () {
        fprintf(stdout, "Usage: %s <case-name>\n", argv[0]);
        fprintf(stdout, "Available cases:\n");
        for (auto &pair : cases) {
            fprintf(stdout, "\t%s\n", pair.first.c_str());
        }
    };

    if (argc != 2) {
        print_help();
        return 1;
    }

    auto iter = cases.find(argv[1]);
    if (iter == cases.end()) {
        print_help();
        return 1;
    }

    iter->second();
    return 0;
}
