#ifndef	__STRINGUTILS_H__
#define	__STRINGUTILS_H__

#include <string>

#define LOWER_CASE_STRING_COMPARISON_OPERATOR(__oper_type__)														\
	bool operator __oper_type__(const lowerCaseString & rhs) const { return mString __oper_type__ rhs.mString; }

#define UPPER_CASE_STRING_COMPARISON_OPERATOR(__oper_type__)														\
	bool operator __oper_type__(const upperCaseString & rhs) const { return mString __oper_type__ rhs.mString; }

namespace soe
{
	void makeupper(std::string& str);
    std::string touppercase(const std::string& str);
	void makelower(std::string& str);
	std::string tolowercase(const std::string& str);
    bool isEqualNoCase(const std::string& left, const std::string& right);
	void rtrim(std::string& str);
	void ltrim(std::string& str);
	void trim(std::string& str);
	void crack(std::string * dest, std::string * base, const std::string& tok);
	void inline crack(std::string& dest, std::string& base, const std::string& tok) {crack(&dest, &base, tok);}
    void inline crack(std::string& base, const std::string& tok) {crack(nullptr, &base, tok);}

	class lowerCaseString
	{
	public:
		lowerCaseString() {}
		lowerCaseString(const std::string & src) : mString(src) { makelower(mString); }
		lowerCaseString(const char * src) : mString(src ? src : "") { makelower(mString); }
		operator std::string() const { return mString; }

		LOWER_CASE_STRING_COMPARISON_OPERATOR(<)
		LOWER_CASE_STRING_COMPARISON_OPERATOR(<=)
		LOWER_CASE_STRING_COMPARISON_OPERATOR(==)
		LOWER_CASE_STRING_COMPARISON_OPERATOR(>=)
		LOWER_CASE_STRING_COMPARISON_OPERATOR(>)

	private:
		std::string	mString;
	};

	class upperCaseString
	{
	public:
		upperCaseString() {}
		upperCaseString(const std::string & src) : mString(src) { makeupper(mString); }
		upperCaseString(const char * src) : mString(src ? src : "") { makeupper(mString); }
		operator std::string() const { return mString; }

		UPPER_CASE_STRING_COMPARISON_OPERATOR(<)
		UPPER_CASE_STRING_COMPARISON_OPERATOR(<=)
		UPPER_CASE_STRING_COMPARISON_OPERATOR(==)
		UPPER_CASE_STRING_COMPARISON_OPERATOR(>=)
		UPPER_CASE_STRING_COMPARISON_OPERATOR(>)

	private:
		std::string	mString;
	};
}

#endif

