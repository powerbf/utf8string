/*
 * utf8test.cpp
 *
 */

#include <utf8string.h>
#include <utf8util.h>
#include <iostream>

using namespace std;

int pass_count = 0;
int fail_count = 0;


void show_result(const std::string& description, std::string expected, std::string actual)
{
	if (expected == actual)
	{
		pass_count++;
		cout << "PASS";
	}
	else
	{
		fail_count++;
		cout << "*FAIL*";
	}
	cout << ": " << description << ":";
	cout << " expected='" << expected << "'";
	cout << " actual='" << actual << "'";
	cout << endl;
}

void show_result(const utf8string& description, size_t expected, size_t actual)
{
	show_result(description, std::to_string(expected), std::to_string(actual));
}

void show_result(const utf8string& description, bool expected, bool actual)
{
	show_result(description, std::to_string(expected), std::to_string(actual));
}



int main()
{
	// 1-byte per codepoint
	utf8string a = "English";
	show_result("'" + a + "' size", 7, a.size());
	show_result("'" + a + "' codepoints", 7, a.count_codepoints());

	// 1 or 2 bytes per codepoint
	utf8string b = "Français";
	show_result("'" + b + "' size", 9, b.size());
	show_result("'" + b + "' codepoints", 8, b.count_codepoints());

	// 3-bytes per codepoint
	utf8string c = "普通话";
	show_result("'" + c + "' size", 9, c.size());
	show_result("'" + c + "' codepoints", 3, c.count_codepoints());

	// 4-bytes per codepoint
	utf8string d = "\U0001F0A1\U0001F0A2";
	show_result("'" + d + "' size", 8, d.size());
	show_result("'" + d + "' codepoints", 2, d.count_codepoints());

	// concatenation
	utf8string e = b + c;
	show_result("'" + b + "' + '" + c + "'", "Français普通话", e);
	show_result("'" + e + "' size", 18, e.size());
	show_result("'" + e + "' codepoints", 11, e.count_codepoints());

	// decode & encode
	std::string f = b + c + d;
	vector<char32_t> codepoints = utf8::utf8_to_codepoints(f);
	std::string g = utf8::codepoints_to_utf8(codepoints);
	show_result("Re-encode", f, g);

	// substring
	std::string h = utf8::utf8_substring(f, 0, 5);
	show_result("Substring", "Franç", h);
	h = utf8::utf8_substring(f, 10, 2);
	show_result("Substring", "话🂡", h);
	h = utf8::utf8_substring(f, 10, 0);
	show_result("Substring", "", h);
	h = utf8::utf8_substring(f, 12, 100);
	show_result("Substring", "🂢", h);
	h = utf8::utf8_substring(f, 12);
	show_result("Substring", "🂢", h);
	h = utf8::utf8_substring(f, 8, std::string::npos);
	show_result("Substring", "普通话🂡🂢", h);

	show_result("is_valid_utf8", true, utf8::is_valid_utf8(f));

	// test with combining character
	utf8string hello = "Gru\u0308ße";
	show_result("Substring", "Gru\u0308", hello.substring(0, 3));
	show_result("Substring", "ß", hello.substring(3, 1));

	// test with multiple combining characters
	std::string hello2 = "Gru\u0308\u0301ße";
	show_result("Substring", "Gru\u0308\u0301", utf8::utf8_substring(hello2, 0, 3));

	// test with isolated combining character
	std::string hello3 = "\u0301Gru\u0308ße";
	show_result("Substring", "\u0301Gr", utf8::utf8_substring(hello3, 0, 3));
}
