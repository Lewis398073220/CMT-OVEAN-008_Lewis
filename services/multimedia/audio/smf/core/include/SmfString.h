#pragma once
#include <stdint.h>
namespace smf {
	///
	constexpr inline char Lower(char c) { return (c <= 'Z' && c >= 'A') ? (c + 'a' - 'A') : c; };
	constexpr inline char Upper(char c) { return (c <= 'z' && c >= 'a') ? (c + 'A' - 'a') : c; };

	constexpr inline uint32_t swap32(const uint32_t v) {
		return (((v) << 24) | ((v) >> 24) | ((v) >> 8 << 24 >> 8) | ((v) << 8 >> 24 << 8));
	}
	constexpr inline uint16_t swap16(const uint16_t v) {
		//return (((v) << 8) | ((v) >> 8));
		return ((((v) & 0xff) << 8) | (((v) >> 8) & 0xff));
	}
}