#ifdef WIN32
#pragma warning (disable: 4786)
#endif

#include "stringutils.h"

#include <cctype>
#include <algorithm>

//using namespace std;
//#define transform std::transform

namespace soe
{
	void makeupper(std::string& str)
	{
		std::transform(str.begin(), str.end(), str.begin(), toupper);
	}

    std::string touppercase(const std::string& str)
    {
        std::string tmp = str;
        std::transform(tmp.begin(), tmp.end(), tmp.begin(), toupper);
        return tmp;
    }

	void makelower(std::string& str)
	{
		std::transform(str.begin(), str.end(), str.begin(), tolower);
	}

	std::string tolowercase(const std::string& str)
	{
		std::string tmp = str;
		transform(tmp.begin(), tmp.end(), tmp.begin(), tolower);
		return tmp;
	}

    bool isEqualNoCase(const std::string& left, const std::string& right)
    {
        std::string tmp1 = left;
        makeupper(tmp1);
        std::string tmp2 = right;
        makeupper(tmp2);
        return tmp1 == tmp2;
    }

	void rtrim(std::string& str)
	{
		size_t pos;
		pos = str.find_last_not_of (" "); 
		str = str.substr(0, pos + 1);
	}

	void ltrim(std::string& str)
	{
		size_t pos;
		pos = str.find_first_not_of (" ");
		if (pos != std::string::npos)
			str = str.substr(pos);
	} 

	void trim(std::string& str)
	{
		rtrim(str);
		ltrim(str);
	}

	
	// crack(dest, base, tok) takes a string and breaks it at the first instance of the token (tok)
	// if the token does not exist it is considerd to be past the end of the string.
	// dest will contain the portion before the token, base will containe the after the token, the token is discarded 
	// I want the caller to provide the dest, so we are not at fault if it leaks
	// NBM - This is code that I have developed many times before (FYI)
	void crack(std::string * dest, std::string * base, const std::string& tok)
	{
		size_t pos = base->find(tok);
		if (pos == std::string::npos)
		{
			// tok not found take the whole thing
			if (dest != nullptr) { dest->assign(*base);}
			base->clear();
			return;
		}

		if (dest != nullptr) {dest->assign(base->substr(0,pos));}
		base->erase(0,pos + tok.length());
	}

}

//*********************************************************************
//
//	Unit Tests
//
//*********************************************************************
#ifdef ENABLE_UNIT_TESTS
#include <UnitTest++.h>
#include <TestReporterStdout.h>

SUITE(stringUtilsSuite)
{
	struct stringUtilsFixture
	{
		stringUtilsFixture()
		{
			s1 = "AbcD 1&";
			s2 = "";
			s3 = "  abcd 1&  ";
		}
		~stringUtilsFixture()
		{
		}

		std::string	s1, s2, s3;
	};
 
	TEST_FIXTURE(stringUtilsFixture, MakeUpper)
	{
		soe::makeupper(s1);
		CHECK_EQUAL("ABCD 1&", s1);
		soe::makeupper(s2);
		CHECK_EQUAL("", s2);
	}

	TEST_FIXTURE(stringUtilsFixture, toppercase)
	{
		CHECK_EQUAL("ABCD 1&", soe::touppercase(s1));
		CHECK_EQUAL("", soe::touppercase(s2));
	}

	TEST_FIXTURE(stringUtilsFixture, MakeLower)
	{
		soe::makelower(s1);
		CHECK_EQUAL("abcd 1&", s1);
		soe::makelower(s2);
		CHECK_EQUAL("", s2);
	}

	TEST_FIXTURE(stringUtilsFixture, tolowercase)
	{
		CHECK_EQUAL("abcd 1&", soe::tolowercase(s1));
		CHECK_EQUAL("", soe::tolowercase(s2));
	}

	TEST(isEqualNoCase)
	{
		CHECK(soe::isEqualNoCase("aBcD", "AbCd"));
		CHECK(!(soe::isEqualNoCase("aBcD", "AbCx")));
		CHECK(!(soe::isEqualNoCase("aBcD", "AbCx")));
		CHECK(!(soe::isEqualNoCase("aBcD", "AbCdE")));
		CHECK(!(soe::isEqualNoCase("aBcD", "")));
		CHECK(!(soe::isEqualNoCase("", "AbCdE")));
	}

	TEST_FIXTURE(stringUtilsFixture, rtrim)
	{
		soe::rtrim(s3);
		CHECK_EQUAL("  abcd 1&", s3);
	}

	TEST_FIXTURE(stringUtilsFixture, ltrim)
	{
		soe::ltrim(s3);
		CHECK_EQUAL("abcd 1&  ", s3);
	}

	TEST_FIXTURE(stringUtilsFixture, trim)
	{
		soe::trim(s3);
		CHECK_EQUAL("abcd 1&", s3);
	}
}	// End of stringutils
#endif	// ENABLE_UINIT_TESTS
