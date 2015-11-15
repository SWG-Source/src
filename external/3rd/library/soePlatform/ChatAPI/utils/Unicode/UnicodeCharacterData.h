// ======================================================================
//
// UnicodeCharData.h
// copyright (c) 2001 Sony Online Entertainment
//
// This contains Unicode 3.0 compliant Character data formats
// The data is loaded dynamically from the UnicodeData.txt definition file
//
// ======================================================================

#ifndef INCLUDED_PlatUnicodeCharData_H
#define INCLUDED_PlatUnicodeCharData_H

#if WIN32
// stl warning func not inlined
#pragma warning (disable:4710)
#pragma warning (disable:4786)
#endif

// ======================================================================

#ifdef EXTERNAL_DISTRO
namespace NAMESPACE 
{
#endif

namespace Plat_Unicode 
{
	typedef unsigned short unicode_char_t; //lint !e761 redundant typedef

	struct CharData;
	
	//-----------------------------------------------------------------
	
	struct CharData
	{
		
		enum Category
		{	
			Lu, // Letter, Uppercase 
			Ll, // Letter, Lowercase 
			Lt, // Letter, Titlecase 
			Lm, // Letter, Modifier 
			Lo, // Letter, Other 
			Mn, // Mark, Non-Spacing 
			Mc, // Mark, Spacing Combining 
			Me, // Mark, Enclosing 
			Nd, // Number, Decimal Digit 
			Nl, // Number, Letter 
			No, // Number, Other 
			Pc, // Punctuation, Connector 
			Pd, // Punctuation, Dash 
			Ps, // Punctuation, Open 
			Pe, // Punctuation, Close 
			Pi, // Punctuation, Initial quote (may behave like Ps or Pe depending on usage) 
			Pf, // Punctuation, Final quote (may behave like Ps or Pe depending on usage) 
			Po, // Punctuation, Other 
			Sm, // Symbol, Math 
			Sc, // Symbol, Currency 
			Sk, // Symbol, Modifier 
			So, // Symbol, Other 
			Zs, // Separator, Space 
			Zl, // Separator, Line 
			Zp, // Separator, Paragraph 
			Cc, // Other, Control 
			Cf, // Other, Format 
			Cs, // Other, Surrogate 
			Co, // Other, Private Use 
			Cn  // Other, Not Assigned (no characters in the file have this property) 
		};
		
		
		struct CategoryName
		{
			Category           m_category;
			const char *       m_str;
		};
		
		static const CategoryName   ms_categoryNames[];
		
		unicode_char_t        m_code;
		Category              m_category;
		
		/**
		* Note:  this Unicode system only supports upper and lower case.
		*        Titlecase is considered uppercase.  The case of a character
		*        is determined by its category (Lu,Ll,or Lt)
		*/
		
		unicode_char_t        m_reverseCase;
		
		bool                  isLowerCase () const;
		bool                  isUpperCase () const;
		
		unicode_char_t        toUpper () const;
		unicode_char_t        toLower () const;	
	};

	//-----------------------------------------------------------------
	
	/**
	* Is this character lowercase.
	*/
	inline bool CharData::isLowerCase () const
	{
		return m_category == Ll;
	}
	
	//-----------------------------------------------------------------
	
	/**
	* Returns true for upper and title case
	*/
	inline bool CharData::isUpperCase () const
	{
		return m_category == Lu || m_category == Lt; 
	}
};
#ifdef EXTERNAL_DISTRO
};
#endif
	
// ======================================================================
	
#endif
