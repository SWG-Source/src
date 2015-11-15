// ======================================================================
//
// UnicodeBlocks.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// This contains Unicode 3.0 compliant Blocks.
//
// ======================================================================

#include "FirstUnicode.h"
#include "UnicodeBlocks.h"

#include <cstdlib>

// ======================================================================
#ifdef EXTERNAL_DISTRO
namespace NAMESPACE
{
#endif
using namespace Plat_Unicode::Blocks;
using Plat_Unicode::unicode_char_t;

//-----------------------------------------------------------------

const Plat_Unicode::Blocks::RawData Plat_Unicode::Blocks::Mapping::ms_defaultBlockData [] =
{
	{ Basic_Latin,       "Basic Latin",0x0000,0x007F },
	{ Latin_1_Supplement,"Latin-1 Supplement",0x0080,0x00FF },
	{ Latin_Extended_A,  "Latin Extended-A",0x0100,0x017F },
	{ Latin_Extended_B,  "Latin Extended-B",0x0180,0x024F },
	{ IPA_Extensions,    "IPA Extensions",0x0250,0x02AF },
	{ Spacing_Modifier_Letters, "Spacing Modifier Letters",0x02B0,0x02FF },
	{ Combining_Diacritical_Marks, "Combining Diacritical Marks",0x0300,0x036F },
	{ Greek,             "Greek",0x0370,0x03FF },
	{ Cyrillic,          "Cyrillic",0x0400,0x04FF },
	{ Armenian,          "Armenian",0x0530,0x058F },
	{ Hebrew, "Hebrew",0x0590,0x05FF },
	{ Arabic, "Arabic",0x0600,0x06FF },
	{ Syriac, "Syriac  ",0x0700,0x074F },
	{ Thaana, "Thaana",0x0780,0x07BF },
	{ Devanagari, "Devanagari",0x0900,0x097F },
	{ Bengali, "Bengali",0x0980,0x09FF },
	{ Gurmukhi, "Gurmukhi",0x0A00,0x0A7F },
	{ Gujarati, "Gujarati",0x0A80,0x0AFF },
	{ Oriya, "Oriya",0x0B00,0x0B7F },
	{ Tamil, "Tamil",0x0B80,0x0BFF },
	{ Telugu, "Telugu",0x0C00,0x0C7F },
	{ Kannada, "Kannada",0x0C80,0x0CFF },
	{ Malayalam, "Malayalam",0x0D00,0x0D7F },
	{ Sinhala, "Sinhala",0x0D80,0x0DFF },
	{ Thai, "Thai",0x0E00,0x0E7F },
	{ Lao, "Lao",0x0E80,0x0EFF },
	{ Tibetan, "Tibetan",0x0F00,0x0FFF },
	{ Myanmar, "Myanmar ",0x1000,0x109F },
	{ Georgian, "Georgian",0x10A0,0x10FF },
	{ Hangul_Jamo, "Hangul Jamo",0x1100,0x11FF },
	{ Ethiopic, "Ethiopic",0x1200,0x137F },
	{ Cherokee, "Cherokee",0x13A0,0x13FF },
	{ Unified_Canadian_Aboriginal_Syllabics, "Unified Canadian Aboriginal Syllabics",0x1400,0x167F },
	{ Ogham, "Ogham",0x1680,0x169F },
	{ Runic, "Runic",0x16A0,0x16FF },
	{ Khmer, "Khmer",0x1780,0x17FF },
	{ Mongolian, "Mongolian",0x1800,0x18AF },
	{ Latin_Extended_Additional, "Latin Extended Additional",0x1E00,0x1EFF },
	{ Greek_Extended, "Greek Extended",0x1F00,0x1FFF },
	{ General_Punctuation, "General Punctuation",0x2000,0x206F },
	{ Superscripts_and_Subscripts, "Superscripts and Subscripts",0x2070,0x209F },
	{ Currency_Symbols, "Currency Symbols",0x20A0,0x20CF },
	{ Combining_Marks_for_Symbols, "Combining Marks for Symbols",0x20D0,0x20FF },
	{ Letterlike_Symbols, "Letterlike Symbols",0x2100,0x214F },
	{ Number_Forms, "Number Forms",0x2150,0x218F },
	{ Arrows, "Arrows",0x2190,0x21FF },
	{ Mathematical_Operators, "Mathematical Operators",0x2200,0x22FF },
	{ Miscellaneous_Technical, "Miscellaneous Technical",0x2300,0x23FF },
	{ Control_Pictures, "Control Pictures",0x2400,0x243F },
	{ Optical_Character_Recognition, "Optical Character Recognition",0x2440,0x245F },
	{ Enclosed_Alphanumerics, "Enclosed Alphanumerics",0x2460,0x24FF },
	{ Box_Drawing, "Box Drawing",0x2500,0x257F },
	{ Block_Elements, "Block Elements",0x2580,0x259F },
	{ Geometric_Shapes, "Geometric Shapes",0x25A0,0x25FF },
	{ Miscellaneous_Symbols, "Miscellaneous Symbols",0x2600,0x26FF },
	{ Dingbats, "Dingbats",0x2700,0x27BF },
	{ Braille_Patterns, "Braille Patterns",0x2800,0x28FF },
	{ CJK_Radicals_Supplement, "CJK Radicals Supplement",0x2E80,0x2EFF },
	{ Kangxi_Radicals, "Kangxi Radicals",0x2F00,0x2FDF },
	{ Ideographic_Description_Characters, "Ideographic Description Characters",0x2FF0,0x2FFF },
	{ CJK_Symbols_and_Punctuation, "CJK Symbols and Punctuation",0x3000,0x303F },
	{ Hiragana, "Hiragana",0x3040,0x309F },
	{ Katakana, "Katakana",0x30A0,0x30FF },
	{ Bopomofo, "Bopomofo",0x3100,0x312F },
	{ Hangul_Compatibility_Jamo, "Hangul Compatibility Jamo",0x3130,0x318F },
	{ Kanbun, "Kanbun",0x3190,0x319F },
	{ Bopomofo_Extended, "Bopomofo Extended",0x31A0,0x31BF },
	{ Enclosed_CJK_Letters_and_Months, "Enclosed CJK Letters and Months",0x3200,0x32FF },
	{ CJK_Compatibility, "CJK Compatibility",0x3300,0x33FF },
	{ CJK_Unified_Ideographs_Extension_A, "CJK Unified Ideographs Extension A",0x3400,0x4DB5 },
	{ CJK_Unified_Ideographs, "CJK Unified Ideographs",0x4E00,0x9FFF },
	{ Yi_Syllables, "Yi Syllables",0xA000,0xA48F },
	{ Yi_Radicals, "Yi Radicals",0xA490,0xA4CF },
	{ Hangul_Syllables, "Hangul Syllables",0xAC00,0xD7A3 },
	{ High_Surrogates, "High Surrogates",0xD800,0xDB7F },
	{ High_Private_Use_Surrogates, "High Private Use Surrogates",0xDB80,0xDBFF },
	{ Low_Surrogates, "Low Surrogates",0xDC00,0xDFFF },
	{ Private_Use, "Private Use",0xE000,0xF8FF },
	{ CJK_Compatibility_Ideographs, "CJK Compatibility Ideographs",0xF900,0xFAFF },
	{ Alphabetic_Presentation_Forms, "Alphabetic Presentation Forms",0xFB00,0xFB4F },
	{ Arabic_Presentation_Forms_A, "Arabic Presentation Forms-A",0xFB50,0xFDFF },
	{ Combining_Half_Marks, "Combining Half Marks",0xFE20,0xFE2F },
	{ CJK_Compatibility_Forms, "CJK Compatibility Forms",0xFE30,0xFE4F },
	{ Small_Form_Variants, "Small Form Variants",0xFE50,0xFE6F },
	{ Arabic_Presentation_Forms_B, "Arabic Presentation Forms-B",0xFE70,0xFEFE },
	{ Specials_1, "Specials",0xFEFF,0xFEFF },
	{ Halfwidth_and_Fullwidth_Forms, "Halfwidth and Fullwidth Forms",0xFF00,0xFFEF },
	{ Specials_2, "Specials 2",0xFFF0,0xFFFD },
	{ End_Block_Ids, "End Block Ids",0xFFFE,0xFFFF }
};

//-----------------------------------------------------------------

Mapping * Mapping::ms_singleton = 0;

//-----------------------------------------------------------------

/**
* Initialize the singleton.  This must be called before mapping is used.
*/

void Mapping::initSingleton ()
{
	explicitDestroy ();
	ms_singleton = new Mapping (ms_defaultBlockData);
}
 
//-----------------------------------------------------------------

/**
* Destroys the singleton
*/

void  Mapping::explicitDestroy ()
{
	delete ms_singleton;

	ms_singleton = 0;
}

//-----------------------------------------------------------------

/**
* Construct a Mapping from an array of RawData
*/

Mapping::Mapping (const RawData dataArray []) :
m_idMap (),
m_nameMap (),
m_errorData ()
{

	size_t i = 0;
	for (; dataArray [i].m_id != End_Block_Ids; ++i)
	{
		m_idMap   [dataArray[i].m_id]   = Data (dataArray [i]);
		m_nameMap [dataArray[i].m_name] = Data (dataArray [i]);
	}

	m_errorData = Data (dataArray [i]);
}

//----------------------------------------------------------------------

/**
* Construct a complete set of code points in this block.
* @param idset should enter this function empty
*/  

const std::set<unicode_char_t> &      Data::generateFilteredIdSet (std::set<unicode_char_t> & idset) const
{
	{
		for (unicode_char_t i = m_start; i < m_end; ++i)
		{
			idset.insert (i);
		}
	}


	for (RangeGroups_t::const_iterator iter = m_rangeGroups.begin (); iter != m_rangeGroups.end (); ++iter)
	{
		const RangeGroup & rg = *iter;

		if (rg.m_isInclusive)
		{
			for (RangeGroup::Ranges_t::const_iterator rg_iter = rg.m_ranges.begin (); rg_iter != rg.m_ranges.end (); ++rg_iter)
			{
				for (unicode_char_t i = (*rg_iter).m_low; i <= (*rg_iter).m_high; ++i)
				{
					idset.insert (i);
				}
			}
		}
		else
		{
			for (RangeGroup::Ranges_t::const_iterator rg_iter = rg.m_ranges.begin (); rg_iter != rg.m_ranges.end (); ++rg_iter)
			{
				for (unicode_char_t i = (*rg_iter).m_low; i <= (*rg_iter).m_high; ++i)
				{
					idset.erase (i);
				}
			}
		}
	}

	return idset;
}

//-----------------------------------------------------------------

/**
* Add a range group with a given string representation.  Valid strings include hexadecimal numbers, dashes, and commas.
* @param inclusive indicates if this group should be additive or subtractive.  It is an error to attempt to add code points outside the Block's gross range.
*/

bool   Data::addRangeGroup (bool inclusive, const Plat_Unicode::NarrowString & str)
{
	RangeGroup rg;
	rg.m_isInclusive = inclusive;

	size_t tokenStartPos = 0;

	while (tokenStartPos != NarrowString::npos)
	{
		size_t tokenCommaPos = str.find_first_of (',', tokenStartPos);

		size_t tokenDashPos = str.find_first_of ('-', tokenStartPos);

		Range rng;

		// this is a dashed range
		if (tokenDashPos < tokenCommaPos)
		{
			rng.m_low = static_cast<unicode_char_t> (strtoul ( str.substr (tokenStartPos, tokenDashPos - tokenStartPos).c_str (), 0, 16));
			tokenStartPos = str.find_first_not_of ('-', tokenDashPos);

			if (tokenStartPos == NarrowString::npos)
				return false;

			rng.m_high = static_cast<unicode_char_t> (strtoul (str.substr (tokenStartPos, tokenCommaPos - tokenStartPos).c_str (), 0, 16));

		}

		else
		{
			rng.m_high = rng.m_low = static_cast<unicode_char_t> (strtoul ( str.substr (tokenStartPos, tokenCommaPos - tokenStartPos).c_str (), 0, 16));
		}

		rg.m_ranges.push_back (rng);

		tokenStartPos = str.find_first_not_of (',', tokenCommaPos);
	}

	m_rangeGroups.push_back (rg);
	return true;

}; 
#ifdef EXTERNAL_DISTRO
};
#endif 

// ===================================================================== },
