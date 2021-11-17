/*
 * utf8util.cpp
 *
 *  Created on: 13 Nov 2021
 *      Author: brian
 */

#include <sstream>
#include <unicode/unistr.h>
#include <utf8util.h>

const char32_t REPLACEMENT_CHAR = 0xFFFD;

// is this a single-byte codepoint (0xxxxxxx)?
static inline bool _is_1_byte_cp(uint8_t b)
{
	return (b & 0x80) == 0x00;
}

// is this the start of a 2-byte codepoint (110xxxxx)?
static inline bool _is_2_byte_cp_start(uint8_t b)
{
	return (b & 0xE0) == 0xC0;
}

// is this the start of a 3-byte codepoint (1110xxxx)?
static inline bool _is_3_byte_cp_start(uint8_t b)
{
	return (b & 0xF0) == 0xE0;
}

// is this the start of a 4-byte codepoint (11110xxx)?
static inline bool _is_4_byte_cp_start(uint8_t b)
{
	return (b & 0xF8) == 0xF0;
}

// is this a continuation byte of a multi-byte codepoint (10xxxxxx)?
static inline bool _is_continuation_byte(uint8_t b)
{
	return (b & 0xC0) == 0x80;
}

// get number of bytes in codepoint at byte_pos
static size_t _get_codepoint_byte_count(const std::string& s, size_t byte_pos)
{
	if (byte_pos >= s.size())
	{
		return 0;
	}

	uint8_t b = static_cast<uint8_t>(s[byte_pos]);

	if(_is_1_byte_cp(b))
	{
		return 1;
	}
	else if (_is_2_byte_cp_start(b))
	{
		return 2;
	}
	else if (_is_3_byte_cp_start(b))
	{
		return 3;
	}
	else if (_is_4_byte_cp_start(b))
	{
		return 4;
	}
	else
	{
		// invalid character
		return 0;
	}
}

// is there a valid codepoint at the specified byte index
static bool _is_valid_codepoint_at(const std::string& s, size_t byte_pos)
{
	if (byte_pos >= s.size())
	{
		return false;
	}

	size_t byte_count = _get_codepoint_byte_count(s, byte_pos);
	if (byte_count == 0 || byte_pos + byte_count > s.size())
	{
		return false;
	}

	// check continuation bytes
	for (size_t i = byte_pos + 1; i < byte_pos + byte_count; i++)
	{
		if (!_is_continuation_byte(s[i]))
		{
			return false;
		}
	}

	return true;
}

// get the codepoint at the specified byte index (assumes validly encoded codepoint)
static char32_t _get_codepoint_at(const std::string& s, size_t byte_pos)
{
	size_t byte_count = _get_codepoint_byte_count(s, byte_pos);
	if (byte_count == 0 || byte_pos + byte_count > s.size())
	{
		// invalid
		return REPLACEMENT_CHAR;
	}

	uint8_t b = static_cast<uint8_t>(s[byte_pos]);
	char32_t codepoint = 0;

	// decode first byte
	if (byte_count == 1)
	{
		codepoint = b;
	}
	else if (byte_count == 2)
	{
		// 110xxxxx
		codepoint = (b & 0x1F);
	}
	else if (byte_count == 3)
	{
		// 1110xxxx
		codepoint = (b & 0x0F);
	}
	else
	{
		// 11110xxx
		codepoint = (b & 0x07);
	}

	// decode continuation bytes (10xxxxxx)
	for (size_t i = 1; i < byte_count; i++)
	{
		if (!_is_continuation_byte(s[byte_pos+i]))
		{
			// invalid
			return REPLACEMENT_CHAR;
		}
		codepoint <<= 6;
		codepoint += (static_cast<uint8_t>(s[byte_pos+i]) & 0x3F);
	}

	return codepoint;
}

const char32_t BACKSPACE = 0x8;
extern int mk_wcwidth(char32_t ucs);

size_t char_width(char32_t c)
{
	int width = mk_wcwidth(c);
	if (width < 0)
	{
		// wcwidth returns -1 for all control chars
		return (c == BACKSPACE ? -1 : 0);
	}
	else
	{
		return width;
	}
}

// combining chars modify the preceeding character (e.g. by adding a dicaritic)
static inline bool _is_combining_char(char32_t c)
{
	// there are no combining characters below \u0300,
	// so we avoid the expense of a call to mk_wcwidth for characters below this.
	return (c >= 0x0300 && mk_wcwidth(c) == 0);
}

/**
 * Get the byte position of the nth character (zero-based) starting from start_byte_pos
 * Ignores combining characters
 * Returns s.size() if less that n characters in string
 */
static size_t _get_nth_character_byte_pos(const std::string& s, size_t n, size_t start_byte_pos = 0)
{
	size_t byte_pos = start_byte_pos;
	size_t char_count = 0;

	while (byte_pos < s.size())
	{
		if (_is_valid_codepoint_at(s, byte_pos))
		{
			char32_t c = _get_codepoint_at(s, byte_pos);
			// combining chars don't count unless at the start of the string
			if (byte_pos == 0 || !_is_combining_char(c))
			{
				char_count++;
				if (char_count > n)
				{
					return byte_pos;
				}
			}
			byte_pos += _get_codepoint_byte_count(s, byte_pos);
		}
		else
		{
			// skip invalid byte
			byte_pos++;
		}
	}

	return s.size();
}


namespace utf8
{
	/*
	 * Is this a valid utf-8 string?
	 */
	bool is_valid_utf8(const std::string& s)
	{
		size_t byte_pos = 0;

		while (byte_pos < s.size())
		{
			if (!_is_valid_codepoint_at(s, byte_pos))
			{
				return false;
			}
			byte_pos += _get_codepoint_byte_count(s, byte_pos);
		}

		return true;
	}

	size_t string_width(const std::string& s)
	{
		size_t byte_pos = 0;
		size_t width = 0;

		while (byte_pos < s.size())
		{
			size_t cp_bytes = _get_codepoint_byte_count(s, byte_pos);
			if (cp_bytes == 0)
			{
				// skip invalid byte
				byte_pos++;
			}
			else
			{
				width += char_width(_get_codepoint_at(s, byte_pos));
				byte_pos += cp_bytes;
			}
		}

		return width;
	}

	/*
	 * Count codepoints in utf-8 string
	 */
	size_t count_codepoints(const std::string& s)
	{
		size_t byte_pos = 0;
		size_t cp_count = 0;

		while (byte_pos < s.size())
		{
			size_t cp_bytes = _get_codepoint_byte_count(s, byte_pos);
			if (cp_bytes == 0)
			{
				// skip invalid byte
				byte_pos++;
			}
			else
			{
				cp_count++;
				byte_pos += cp_bytes;
			}
		}

		return cp_count;
	}

	std::string utf8_substring(const std::string& s, size_t pos, size_t count)
	{
		size_t start_byte = _get_nth_character_byte_pos(s, pos);
		if (start_byte >= s.size())
		{
			return "";
		}

		size_t byte_count = std::string::npos;
		if (count != std::string::npos)
		{
			size_t end_byte = _get_nth_character_byte_pos(s, count, start_byte);
			byte_count = end_byte - start_byte;
		}

		return s.substr(start_byte, byte_count);
	}

	std::string utf8_cp_substring(const std::string& s, size_t pos, size_t count)
	{
		size_t byte_pos = 0; // position in bytes
		size_t cp_pos = 0; //

		// determine start byte
		while (cp_pos < pos && byte_pos < s.size())
		{
			size_t cp_bytes = _get_codepoint_byte_count(s, byte_pos);
			if (cp_bytes == 0)
			{
				// skip invalid byte
				byte_pos++;
			}
			else
			{
				byte_pos += cp_bytes;
				cp_pos++;
			}
		}

		size_t start_byte = byte_pos;
		if (start_byte >= s.size())
		{
			return "";
		}

		// determine byte_count
		size_t byte_count = 0;
		if (count == std::string::npos)
		{
			byte_count = std::string::npos;
		}
		else
		{
			size_t cp_count = 0;
			while (cp_count < count && byte_pos < s.size())
			{
				size_t cp_bytes = _get_codepoint_byte_count(s, byte_pos);
				if (cp_bytes == 0)
				{
					// skip invalid byte
					byte_pos++;
				}
				else
				{
					cp_count++;
					byte_pos += cp_bytes;
				}
			}
			byte_count = byte_pos - start_byte;
		}

		return s.substr(start_byte, byte_count);
	}

	/*
	 * Convert utf-8 to codepoints
	 */
	std::vector<char32_t> utf8_to_codepoints(const std::string& s)
	{
		std::vector<char32_t> result;
		size_t byte_pos = 0;
		while (byte_pos < s.size())
		{
			if (_is_valid_codepoint_at(s, byte_pos))
			{
				result.push_back(_get_codepoint_at(s, byte_pos));
				byte_pos += _get_codepoint_byte_count(s, byte_pos);
			}
			else
			{
				// skip invalid byte
				byte_pos++;
			}
		}

		return result;
	}

	/*
	 * Convert a codepoints to utf-8
	 */
	std::string codepoints_to_utf8(std::vector<char32_t> codepoints)
	{
		std::ostringstream oss;

		for (uint32_t codepoint : codepoints)
		{
			if (codepoint <= 0x7F)
			{
				// encode in 1 byte
				oss << static_cast<unsigned char>(codepoint);
			}
			else if (codepoint <= 0x7FF)
			{
				// encode in 2 bytes (110xxxxx 10xxxxxx)
				oss << static_cast<unsigned char>(0xC0 | (codepoint >> 6));
				oss << static_cast<unsigned char>(0x80 | (codepoint & 0x3F));
			}
			else if (codepoint <= 0xFFFF)
			{
				// encode in 3 bytes (1110xxxx 10xxxxxx 10xxxxxx)
				oss << static_cast<unsigned char>(0xE0 | (codepoint >> 12));
				oss << static_cast<unsigned char>(0x80 | ((codepoint >> 6) & 0x3F));
				oss << static_cast<unsigned char>(0x80 | (codepoint & 0x3F));
			}
			else if (codepoint <= 0x10FFFF)
			{
				// encode in 4 bytes (11110xxx 10xxxxxx 10xxxxxx 10xxxxxx)
				oss << static_cast<unsigned char>(0xF0 | (codepoint >> 18));
				oss << static_cast<unsigned char>(0x80 | ((codepoint >> 12) & 0x3F));
				oss << static_cast<unsigned char>(0x80 | ((codepoint >> 6) & 0x3F));
				oss << static_cast<unsigned char>(0x80 | (codepoint & 0x3F));
			}
		}

		return oss.str();
	}

}
