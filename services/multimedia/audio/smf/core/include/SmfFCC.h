#pragma once
#include "SmfString.h"
//#define fcc4(c0,c1,c2,c3) (((unsigned)c0)|((unsigned)c1<<8)|((unsigned)c2<<16)|((unsigned)c3<<24))
//#define fcc4x(c) fcc4(c[0],c[1],c[2],c[3])
namespace smf {
	constexpr inline unsigned fcc(const char* s, int c=0) {
		unsigned v = *s;
		if (!c || c > 4)c = 4;
		for (int i = 1; i < c; i++) {
			if (s[i]) {
				v |= (unsigned)s[i] << (8 * i);
			}
			else {
				break;
			}
		}
		return v;
	}
	constexpr inline unsigned fcclow(const char* s, int c=0) {
		unsigned v = *s;
		if (!c || c > 4)c = 4;
		for (int i = 1; i < c; i++) {
			if (s[i]) {
				v |= (unsigned)Lower(s[i]) << (8 * i);
			}
			else {
				break;
			}
		}
		return v;
	}

	constexpr inline unsigned long long fcc64(const char* s, int c=0) {
		unsigned long long v = *s;
		if (!c || c > 8)c = 8;
		for (int i = 1; i < c; i++) {
			if (s[i]) {
				v |= (unsigned long long)s[i] << (8 * i);
			}
			else {
				break;
			}
		}
		return v;
	}
	constexpr inline unsigned long long fcc64low(const char* s, int c=0) {
		unsigned long long v = *s;
		if (!c || c > 8)c = 8;
		for (int i = 1; i < c; i++) {
			if (s[i]) {
				v |= (unsigned long long)Lower(s[i]) << (8 * i);
			}
			else {
				break;
			}
		}
		return v;
	}
}