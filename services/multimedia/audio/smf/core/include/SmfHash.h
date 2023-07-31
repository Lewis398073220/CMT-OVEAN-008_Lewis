#pragma once
#include "SmfString.h"
namespace smf {
    const unsigned _FNV_offset_basis = 2166136261U;//0x811c9dc5
    const unsigned _FNV_prime = 16777619U;//0x01000193
    ///
    constexpr inline unsigned Hash(unsigned v, char s) {
        return (v ^ (unsigned)Lower(s)) * _FNV_prime;
    }
    ///
    constexpr inline unsigned Hash(unsigned v, const char* s) {
        return *s ? Hash(Hash(v, *s), s + 1) : v;
    }
    constexpr inline unsigned Hash(const char* s) {
        return s ? Hash(_FNV_offset_basis, s) : 0;
    }
    ///
    constexpr inline unsigned Hash(unsigned v, const char* s, char end) {
        return (*s && *s != end) ? Hash(Hash(v, *s), s + 1, end) : v;
    }
    constexpr inline unsigned Hash(const char* s, char end) {
        return s ? Hash(_FNV_offset_basis, s, end) : 0;
    }
    ///
    constexpr inline unsigned Hash(unsigned v, const char* s, int max) {
        return (*s && max) ? Hash(Hash(v, *s), s + 1, max - 1) : v;
    }
    constexpr inline unsigned Hash(const char* s, int max) {
        return (s && max) ? Hash(_FNV_offset_basis, s, max) : 0;
    }
    ///
    constexpr inline unsigned Hash(unsigned v, const char* s, char end, int max) {
        return (*s && max && *s != end) ? Hash(Hash(v, *s), s + 1, end, max - 1) : v;
    }
    constexpr inline unsigned Hash(const char* s, char end, int max) {
        return (s && max) ? Hash(_FNV_offset_basis, s, end, max) : 0;
    }

    constexpr inline unsigned HashX(const char*s) {
        unsigned v = _FNV_offset_basis;
        while (*s) {
            v = Hash(v, *s++);
        }
        return v;
    }
}

