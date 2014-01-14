// ======================================================================
//
// UnicodeCharData.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// This contains Unicode 3.0 compliant Character data formats
// The data is loaded dynamically from the UnicodeData.txt definition file
//
// ======================================================================

#include "FirstUnicode.h"
#include "UnicodeCharacterData.h"

// ======================================================================
	
using namespace Unicode;
	
//-----------------------------------------------------------------

#define CAT_NAME(s)  { CharData::s, #s }

const CharData::CategoryName CharData::ms_categoryNames [] =
{
	
	CAT_NAME (Lu), // Letter, Uppercase 
	CAT_NAME (Ll), // Letter, Lowercase 
	CAT_NAME (Lt), // Letter, Titlecase 
	CAT_NAME (Lm), // Letter, Modifier 
	CAT_NAME (Lo), // Letter, Other 
	CAT_NAME (Mn), // Mark, Non-Spacing 
	CAT_NAME (Mc), // Mark, Spacing Combining 
	CAT_NAME (Me), // Mark, Enclosing 
	CAT_NAME (Nd), // Number, Decimal Digit 
	CAT_NAME (Nl), // Number, Letter 
	CAT_NAME (No), // Number, Other 
	CAT_NAME (Pc), // Punctuation, Connector 
	CAT_NAME (Pd), // Punctuation, Dash 
	CAT_NAME (Ps), // Punctuation, Open 
	CAT_NAME (Pe), // Punctuation, Close 
	CAT_NAME (Pi), // Punctuation, Initial quote (may behave like Ps or Pe depending on usage) 
	CAT_NAME (Pf), // Punctuation, Final quote (may behave like Ps or Pe depending on usage) 
	CAT_NAME (Po), // Punctuation, Other 
	CAT_NAME (Sm), // Symbol, Math 
	CAT_NAME (Sc), // Symbol, Currency 
	CAT_NAME (Sk), // Symbol, Modifier 
	CAT_NAME (So), // Symbol, Other 
	CAT_NAME (Zs), // Separator, Space 
	CAT_NAME (Zl), // Separator, Line 
	CAT_NAME (Zp), // Separator, Paragraph 
	CAT_NAME (Cc), // Other, Control 
	CAT_NAME (Cf), // Other, Format 
	CAT_NAME (Cs), // Other, Surrogate 
	CAT_NAME (Co), // Other, Private Use 
	CAT_NAME (Cn)  // Other, Not Assigned (no characters in the file have this property) 
};

#undef CAT_NAME

//-----------------------------------------------------------------

// ======================================================================
