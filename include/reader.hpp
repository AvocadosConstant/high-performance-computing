#ifndef _READER_HPP_
#define _READER_HPP_

#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdint>

// Shamelessly lifted from TA Mayank's provided code
// Wrapper around a pointer, for reading values from byte sequence.
class Reader {
    public:
        Reader(const char *p) : ptr{p} {}
        template <typename T>
        Reader &operator>>(T &o) {
            // Assert alignment.
            assert(uintptr_t(ptr)%sizeof(T) == 0);
            o = *(T *) ptr;
            ptr += sizeof(T);
            return *this;
        }
    private:
        const char *ptr;
};

#endif
