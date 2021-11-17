/*
 * utf8util.hpp
 *
 * Copyright 2021 Brian Power
 */

#pragma once

#include <cstdint>
#include <string>


namespace utf8
{
	/*
	 * Is this a valid utf-8 string?
	 */
	bool is_valid_utf8(const std::string& s);

	/*
	 * @brief Get string width in characters.
	 * This can be different to number of codepoints because some Asian characters are double-wide
	 * and there are also zero-width characters (e.g. combining characters that merely modify the
	 * preceeding character).
	 */
	size_t string_width(const std::string& s);

	/*
	 * Count codepoints in utf-8 string
	 */
	size_t count_codepoints(const std::string& s);

	/*
	 * Convert utf-8 to codepoints
	 */
	std::u32string utf8_to_utf32(const std::string& s);

	/*
	 * Convert a codepoints to utf-8
	 */
	std::string utf32_to_utf8(const std::u32string& s);

	/*
	 * @brief Get a substring based on graphemes
	 *        Sequences of one or more combining characters count as part of the preceeding character.
	 *        One or more combining characters at the start of a string counts as
	 *        a character (but who knows how that will display, if at all).
	 * @param pos = character position to start with (zero-based)
	 * @param count = max characters
	 */
	std::string utf8_substring(const std::string& s, size_t pos = 0, size_t count = std::string::npos);

	/*
	 * @brief Get a substring based on codepoints
	 * @param pos = codepoint to start with
	 * @param count = max code points to return
	 */
	std::string utf8_cp_substring(const std::string& s, size_t pos = 0, size_t count = std::string::npos);
}
