/**
 * @file
 */

#pragma once

#include <string>
#include <sstream>
#include <cstdlib>
#include <climits>
#include <algorithm>
#include <vector>
#include <stdarg.h>
#include <SDL.h>

#if __cplusplus <= 201411
#include <experimental/string_view>
namespace std {
using string_view = std::experimental::string_view;
}
#else
#include <string_view>
#endif

namespace core {
namespace string {

template<typename T, int Bits = sizeof(T) * CHAR_BIT>
inline std::string bits(T in, int newlineAfter = -1) {
	const int newlineChars = newlineAfter <= 0 ? 0 : (Bits - 1) / newlineAfter;
	char bitstr[Bits + newlineChars + 1];
	const T one = 1;
	int c = 0;
	int n = 0;
	for (int i = Bits - 1; i >= 0; --i) {
		if (newlineAfter > 0 && c == newlineAfter) {
			bitstr[n++] = '\n';
			c = 0;
		}
		bitstr[n++] = (in & (one << i)) != 0 ? '1' : '0';
		++c;
	}
	bitstr[Bits + newlineChars] = '\0';
	return std::string(bitstr);
}

inline bool isUTF8Multibyte(char c) {
	return (c & 0xc0) == 0x80;
}

inline size_t utf8LengthChar(const unsigned char c) {
	if (c < 0x80)
		return 1;
	if (c < 0xc0)
		return 0;
	if (c < 0xe0)
		return 2;
	if (c < 0xf0)
		return 3;
	if (c < 0xf8)
		return 4;
	/* 5 and 6 byte sequences are no longer valid. */
	return 0;
}

inline size_t utf8LengthInt(int c) {
	if (c <= 0x7F)
		return 1;
	if (c <= 0x07FF)
		return 2;
	if (c <= 0xFFFF)
		return 3;
	if (c <= 0x10FFFF) /* highest defined Unicode code */
		return 4;
	return 0;
}

inline size_t utf8Length(const std::string& str) {
	size_t result = 0;
	const char *string = str.c_str();

	while (string[0] != '\0') {
		const int n = utf8LengthChar((const unsigned char) *string);
		string += n;
		result++;
	}
	return result;
}

extern std::string format(const char *msg, ...)  SDL_PRINTF_VARARG_FUNC(1);

inline int toInt(const char*str) {
	return SDL_atoi(str);
}

inline int toLong(const char* str) {
	return ::atol(str);
}

inline int toInt(const std::string& str) {
	return toInt(str.c_str());
}

inline int toLong(const std::string& str) {
	return toLong(str.c_str());
}

inline bool toBool(const std::string& str) {
	return str == "1" || str == "true";
}

inline float toFloat(const std::string& str) {
	return SDL_atof(str.c_str());
}

inline void splitString(const std::string& string, std::vector<std::string>& tokens, const std::string& delimiters = " \t\r\n\f\v") {
	// Skip delimiters at beginning.
	std::string::size_type lastPos = string.find_first_not_of(delimiters, 0);
	// Find first "non-delimiter".
	std::string::size_type pos = string.find_first_of(delimiters, lastPos);

	while (std::string::npos != pos || std::string::npos != lastPos) {
		// Found a token, add it to the vector.
		tokens.push_back(string.substr(lastPos, pos - lastPos));
		// Skip delimiters.  Note the "not_of"
		lastPos = string.find_first_not_of(delimiters, pos);
		// Find next "non-delimiter"
		pos = string.find_first_of(delimiters, lastPos);
	}
}

inline std::string toLower(const std::string& string) {
	std::string convert = string;
	std::transform(convert.begin(), convert.end(), convert.begin(), (int (*)(int)) std::tolower);
	return convert;
}

inline bool startsWith(const std::string& string, const std::string& token) {
	return !string.compare(0, token.size(), token);
}

inline bool endsWith(const std::string& string, const std::string& end) {
	const std::size_t strLength = string.length();
	const std::size_t endLength = end.length();
	if (strLength >= endLength) {
		const std::size_t index = strLength - endLength;
		return string.compare(index, endLength, end) == 0;
	}
	return false;
}

inline std::string replaceAll(const std::string& str, const std::string& searchStr, const char* replaceStr, size_t replaceStrSize) {
	if (str.empty()) {
		return str;
	}
	std::string sNew = str;
	std::string::size_type loc;
	const std::string::size_type searchLength = searchStr.length();
	std::string::size_type lastPosition = 0;
	while (std::string::npos != (loc = sNew.find(searchStr, lastPosition))) {
		sNew.replace(loc, searchLength, replaceStr);
		lastPosition = loc + replaceStrSize;
	}
	return sNew;
}

inline std::string replaceAll(const std::string& str, const std::string& searchStr, const char* replaceStr) {
	return replaceAll(str, searchStr, replaceStr, strlen(replaceStr));
}

inline std::string replaceAll(const std::string& str, const std::string& searchStr, const std::string_view& replaceStr) {
	return replaceAll(str, searchStr, replaceStr.data(), replaceStr.size());
}

inline std::string replaceAll(const std::string& str, const std::string& searchStr, const std::string& replaceStr) {
	return replaceAll(str, searchStr, replaceStr.data(), replaceStr.size());
}

inline std::string_view cutAfterFirstMatch(const std::string_view str, const std::string& pattern, size_t start = 0) {
	std::string_view::size_type pos = str.find_first_of(pattern, 0);
	return str.substr(start, pos);
}

/**
 * @brief extract path with trailing /
 * @note Assumed to be normalized (no \ , only /)
 */
inline std::string_view extractPath(const std::string_view str) {
	const size_t pos = str.rfind("/");
	if (pos == std::string::npos) {
		return "";
	}
	return str.substr(0, pos + 1) ;
}

inline std::string eraseAllSpaces(const std::string& str) {
	std::string tmp = str;
	tmp.erase(std::remove(tmp.begin(), tmp.end(), ' '), tmp.end());
	return tmp;
}

inline bool contains(const std::string& str, const std::string& search) {
	return str.rfind(search) != std::string::npos;
}

inline std::string_view ltrim(const std::string_view str) {
	size_t startpos = str.find_first_not_of(" \t");
	if (std::string::npos != startpos) {
		return str.substr(startpos);
	}
	return str;
}

inline std::string_view rtrim(const std::string_view str) {
	size_t endpos = str.find_last_not_of(" \t");
	if (std::string::npos != endpos) {
		return str.substr(0, endpos + 1);
	}
	return str;
}

inline std::string_view trim(const std::string_view str) {
	return ltrim(rtrim(str));
}

inline bool iequals(const std::string& a, const std::string& b) {
	const unsigned int sz = a.size();
	if (b.size() != sz) {
		return false;
	}
	for (unsigned int i = 0u; i < sz; ++i) {
		if (tolower(a[i]) != tolower(b[i])) {
			return false;
		}
	}
	return true;
}

extern bool matches (const std::string& pattern, const std::string& text);

// pass by copy to prevent aliasing
extern std::string concat(std::string_view first, std::string_view second);

}
}
