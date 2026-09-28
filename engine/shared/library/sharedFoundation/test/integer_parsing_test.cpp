// Regression test: integer text is parsed at its destination's width, with
// the same result on every ABI.
//
// glibc atoi()/strtol() clamp out-of-range text to the range of long, which
// is 32 bits on ILP32 and 64 bits on LP64, so the same text used to give
// different values on the two builds (and garbage silently became 0). The
// test prints the result of every parser this branch uses, for a table of
// valid, out-of-range and malformed text, and integer-parsing-test.sh
// compares the output with integer_parsing_expected.txt. The expected output
// is the same for -m32 and -m64.

#include "sharedFoundation/FirstSharedFoundation.h"
#include "sharedCommandParser/CommandParser.h"
#include "sharedFoundation/DynamicVariable.h"
#include "sharedFoundation/FixedWidthParse.h"
#include "UnicodeUtils.h"

#include <cstdio>
#include <ctime>
#include <string>
#include <vector>

namespace
{
	char const * const cs_inputs[] =
	{
		"0", "-1", "+5", "-0", "2147483647", "2147483648", "-2147483648", "-2147483649",
		"4294967295", "4294967296", "4294967297", "18446744073709551616",
		" 12 ", "\t7\n", "12abc", "49m", "0x10", "1 2", "", " ", "abc", "+", "-"
	};

	Unicode::String wide(std::string const &text)
	{
		Unicode::String result;
		for (std::string::const_iterator i = text.begin(); i != text.end(); ++i)
			result.push_back(static_cast<unsigned char>(*i));
		return result;
	}

	std::string quoted(std::string const &text)
	{
		std::string result("\"");
		for (std::string::const_iterator i = text.begin(); i != text.end(); ++i)
		{
			if (*i == '\t') result += "\\t";
			else if (*i == '\n') result += "\\n";
			else if (*i == '\0') result += "\\0";
			else result += *i;
		}
		return result + "\"";
	}

	void dynamicInt(std::string const &label, Unicode::String const &text)
	{
		DynamicVariable v;
		v.load(0, DynamicVariable::INT, text);
		int value = 12345;
		bool const ok = v.get(value);
		std::printf("DynamicVariable INT       %-24s %s %d\n", label.c_str(), ok ? "ok " : "rej", ok ? value : 0);
	}

	void dynamicIntArray(std::string const &label, Unicode::String const &text)
	{
		DynamicVariable v;
		v.load(0, DynamicVariable::INT_ARRAY, text);
		std::vector<int> values;
		bool const ok = v.get(values);
		std::printf("DynamicVariable INT_ARRAY %-24s %s [", label.c_str(), ok ? "ok " : "rej");
		for (std::vector<int>::const_iterator i = values.begin(); i != values.end(); ++i)
			std::printf("%d,", *i);
		std::printf("]\n");
	}
}

int main()
{
	for (size_t i = 0; i < sizeof(cs_inputs) / sizeof(cs_inputs[0]); ++i)
	{
		std::string const text(cs_inputs[i]);
		Unicode::String const w = wide(text);
		int32_t i32 = 111, lead = 555;
		uint32_t u32 = 222, bits = 333;
		int32 argInt = 444;
		uint32 argUint = 666;
		bool const a = FixedWidthParse::parseInt32(text.c_str(), 10, i32);
		bool const b = FixedWidthParse::parseUint32(text.c_str(), 10, u32);
		bool const c = FixedWidthParse::parseBits32(text.c_str(), 10, bits);
		bool const d = FixedWidthParse::parseLeadingInt32(text.c_str(), 10, lead);
		bool const e = CommandParser::parseArg(w, argInt);
		bool const f = CommandParser::parseArg(w, argUint);
		std::printf("%-24s int32 %s %d | uint32 %s %u | bits32 %s %u | leading %s %d | parseArg int32 %s %d uint32 %s %u | Unicode::toInt %d\n",
			quoted(text).c_str(), a ? "ok " : "rej", i32, b ? "ok " : "rej", u32, c ? "ok " : "rej", bits, d ? "ok " : "rej", lead,
			e ? "ok " : "rej", argInt, f ? "ok " : "rej", argUint, Unicode::toInt(w));
	}

	// a non-ASCII code unit that narrows to '1', and an embedded nul
	{
		Unicode::String dotless;
		dotless.push_back(0x0131);
		int32 value = 5;
		bool const ok = CommandParser::parseArg(dotless, value);
		std::printf("parseArg U+0131             %s %d\n", ok ? "ok " : "rej", value);
		Unicode::String nul = wide("12");
		nul.push_back(0);
		nul.push_back('9');
		value = 5;
		bool const ok2 = CommandParser::parseArg(nul, value);
		std::printf("parseArg \"12\\09\"             %s %d\n", ok2 ? "ok " : "rej", value);
	}

	// console date and time arguments
	{
		char const * const args[] = { "cmd", "2026", "9", "27", "13", "5", "0" };
		CommandParser::StringVector_t argv;
		for (size_t i = 0; i < 7; ++i)
			argv.push_back(wide(args[i]));
		struct tm t = tm();
		bool ok = CommandParser::parseDateTimeArgs(argv, 1, t);
		std::printf("parseDateTimeArgs 2026-9-27 13:05:00  %s year=%d mon=%d mday=%d %d:%d:%d\n", ok ? "ok " : "rej", t.tm_year, t.tm_mon, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec);
		argv[1] = wide("-2147483648");
		ok = CommandParser::parseDateTimeArgs(argv, 1, t);
		std::printf("parseDateTimeArgs year INT_MIN        %s\n", ok ? "ok " : "rej");
		argv[1] = wide("4294969322");
		ok = CommandParser::parseDateTimeArgs(argv, 1, t);
		std::printf("parseDateTimeArgs year 2^32 + 2026    %s\n", ok ? "ok " : "rej");
		ok = CommandParser::parseDateTimeArgs(argv, 2, t);
		std::printf("parseDateTimeArgs too few arguments   %s\n", ok ? "ok " : "rej");
	}

	// objvar text, as script and the database store it
	dynamicInt("\"42\"", wide("42"));
	dynamicInt("\"-2147483648\"", wide("-2147483648"));
	dynamicInt("\"2147483648\"", wide("2147483648"));
	dynamicInt("\"4294967297\"", wide("4294967297"));
	dynamicInt("\"12abc\"", wide("12abc"));
	dynamicIntArray("\"\"", wide(""));
	dynamicIntArray("\"1:2:-3:\"", wide("1:2:-3:"));
	dynamicIntArray("\"1:2147483648:\"", wide("1:2147483648:"));
	dynamicIntArray("\"1:2\" (no final :)", wide("1:2"));
	{
		Unicode::String nul = wide("1");
		nul.push_back(0);
		nul += wide("garbage:");
		dynamicIntArray("\"1\\0garbage:\"", nul);
		Unicode::String dotless;
		dotless.push_back(0x0131);
		dotless.push_back(':');
		dynamicIntArray("\"\\u0131:\"", dotless);
	}
	{
		std::vector<int> in;
		in.push_back(7);
		in.push_back(-8);
		in.push_back(2147483647);
		DynamicVariable packed(in);
		std::vector<int> out;
		bool const ok = packed.get(out);
		std::printf("DynamicVariable INT_ARRAY %-24s %s [", "pack/unpack round trip", ok ? "ok " : "rej");
		for (std::vector<int>::const_iterator i = out.begin(); i != out.end(); ++i)
			std::printf("%d,", *i);
		std::printf("]\n");
	}
	return 0;
}
