/*
 * utf8string.cpp
 *
 * UTF-8 string class
 *
 * Copyright 2021 Brian Power
 */

#include <utf8string.h>
#include <utf8util.h>


utf8string::utf8string()
{
}

utf8string::utf8string(const char* str)
    : std::string(str ? str : "")
{
}

utf8string::utf8string(const std::string& str)
    : std::string(str)
{
}

utf8string::utf8string(const utf8string& str)
    : std::string(str)
{
}

size_t utf8string::count_codepoints() const
{
	return utf8::count_codepoints(*this);
}

bool utf8string::is_valid_utf8() const
{
	return utf8::is_valid_utf8(*this);
}

std::u32string utf8string::to_utf32() const
{
	return utf8::utf8_to_utf32(*this);
}

utf8string utf8string::substring(size_t pos, size_t count) const
{
	return utf8::utf8_substring(*this, pos, count);
}

utf8string utf8string::substring_cp(size_t pos, size_t count) const
{
	return utf8::utf8_cp_substring(*this, pos, count);
}

