/*
 * utf8string.hpp
 *
 * UTF-8 string class
 *
 * Copyright 2021 Brian Power
 */

#pragma once

#include <string>
#include <cstdint>
#include <iosfwd>

class utf8string : public std::string
{
public:
	/**
    *   @brief	Default constructor
    */
    utf8string();

    /**
    *   @brief	Construct from char*
    */
    utf8string(const char* str);

    /**
    *   @brief	Construct from std::string
    */
    utf8string(const std::string& str);

    /**
    *   @brief	Copy constructor
    */
    utf8string(const utf8string& str);

    /**
     * @brief	Length of string in codepoints
     */
    size_t count_codepoints() const;

    /**
     * @brief	Length of string in characters (graphemes + control chars)
     */
    size_t count_characters() const;

    /**
     * @brief	Is this a validly encoded UTF-8 string?
     */
    bool is_valid_utf8() const;

    /**
     * @brief	Get codepoints
     */
    std::u32string to_utf32() const;

    /**
     * @brief Return a UTF-8 substring based on characters (graphemes/control characters)
     * @param pos = starting character (zero-based index)
     * @param count = number of characters to return
     */
    utf8string substring(size_t pos = 0, size_t count = npos) const;

    /**
     * @brief Return a UTF-8 substring based on codepoints
     *        This may cut in the middle of a grapheme (visible character) because
     *        graphemes can be composed of multiple codepoints (e.g. base character
     *        plus combining character like a diacritic, etc.)
     * @param pos = starting codepoint (zero-based index)
     * @param count = number of codepoints to return
     */
    utf8string substring_cp(size_t pos = 0, size_t count = npos) const;
};

