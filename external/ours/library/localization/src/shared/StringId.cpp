//========================================================================
//
// StringId.cpp - Used to access an entry in a string table.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "FirstLocalization.h"
#include "StringId.h"

#include "LocalizationManager.h"
#include "UnicodeUtils.h"
#include <cassert>
#include <cctype>
#include <cstdlib>

//----------------------------------------------------------------------

const StringId StringId::cms_invalid;

//----------------------------------------------------------------------

StringId::StringId () :
m_table     (),
m_text      (),
m_textIndex (0)
{
}

//----------------------------------------------------------------------

StringId::StringId (const std::string & table, const std::string & text) :
m_table     (table),
m_text      (text),
m_textIndex (0)
{
}

//----------------------------------------------------------------------

StringId::StringId (const std::string & table, unsigned long textIndex) :
m_table     (table),
m_text      (),
m_textIndex (textIndex)
{
}

//----------------------------------------------------------------------

StringId::StringId       (const std::string & str) :
m_table     (),
m_text      (),
m_textIndex (0)
{
	const size_t colonpos = str.find (':');

	if (colonpos != std::string::npos)
	{
		m_table = str.substr (0, colonpos);
		if (!m_table.empty () && m_table [0] == '@')
			m_table.erase (m_table.begin ());

		std::string second = str.substr (colonpos + 1);

		if (!second.empty ())
		{
			if (isdigit (second [0]))
				m_textIndex = atoi (second.c_str ());
			else
				m_text = second;
		}
	}
	else
		m_text = str;

}

//----------------------------------------------------------------------

/**
 * Debug output, for console e.g.
 */
std::string StringId::getDebugString() const
{
	return (m_table + ":" + m_text);
}

// ----------------------------------------------------------------------

std::string StringId::getCanonicalRepresentation () const
{
	return (m_table + ":" + m_text);
}

//----------------------------------------------------------------------

/**
* @return true if this stringId is definitely invalid
*/
bool StringId::isInvalid () const
{
	return m_table.empty () || (m_text.empty () && m_textIndex == 0);
}

//----------------------------------------------------------------------

bool StringId::isValid () const
{
	return !m_table.empty () && (!m_text.empty () || m_textIndex != 0);
}

//----------------------------------------------------------------------

Unicode::String StringId::decodeString (const Unicode::String & str)
{
	if (str.empty ())
		return str;

	static Unicode::String result;
	result.clear ();

	size_t pos     = 0;
	
	//-- an encoded string can be a series of tokens, seperated by nulls
	//-- each token can be an encoded stringid ("@table:name") or a literal string

	while (pos != std::string::npos)
	{
		size_t nullpos = str.find (Unicode::unicode_char_t (0), pos);			
		
		if (pos == 0 && nullpos == std::string::npos)
		{
			if (str [0] == '@')
			{
				const StringId sid (Unicode::wideToNarrow (str));
				return sid.localize ();
			}
			else 
				return str;
		}
		
		const Unicode::String & token = nullpos == std::string::npos ? (pos == 0 ? str : str.substr (pos)) : str.substr (pos, nullpos - pos);	
		
		if (token.empty ())
			break;
		
		if (token [0] == '@')
		{
			const StringId sid (Unicode::wideToNarrow (token));
			result += sid.localize ();
		}
		else
			result += token;

		if (nullpos == std::string::npos)
			break;

		pos = nullpos + 1;
	}
	
	return result;
}

//----------------------------------------------------------------------

StringId StringId::decodeStringId (const Unicode::String & str)
{
	return StringId (Unicode::wideToNarrow (str.c_str () + 1));
}

//----------------------------------------------------------------------

void StringId::clear ()
{
	m_table.clear ();
	m_text.clear  ();
	m_textIndex = 0;
}

//----------------------------------------------------------------------

bool StringId::localize (Unicode::String & result, bool forceEnglish) const
{
	return LocalizationManager::getManager ().getLocalizedStringValue (*this, result, forceEnglish) == LocalizationManager::SVC_ok;
}

//----------------------------------------------------------------------

Unicode::String StringId::localize       (bool forceEnglish) const
{
	static Unicode::String str;
	str.clear ();
	localize (str, forceEnglish);
	return str;
}

//----------------------------------------------------------------------

bool StringId::localize (Unicode::String & result, const Unicode::NarrowString & locale, bool forceEnglish) const
{
	return LocalizationManager::getManager ( locale ).getLocalizedStringValue (*this, result, forceEnglish) == LocalizationManager::SVC_ok;
}

//----------------------------------------------------------------------

Unicode::String StringId::localize       (const Unicode::NarrowString & locale, bool forceEnglish) const
{
	static Unicode::String str;
	str.clear ();
	localize (str, locale, forceEnglish);
	return str;
}

//----------------------------------------------------------------------

/**
* Setting the text invalidates the cached text index
*/

void StringId::setTable (const std::string & table)
{
	m_table     = table;
	m_textIndex = 0;
}

//----------------------------------------------------------------------

/**
* Setting the text invalidates the cached text index
*/

void StringId::setText (const std::string & text)
{
	m_text      = text;
	m_textIndex = 0;
}

//----------------------------------------------------------------------

void StringId::setTextIndex (unsigned long textIndex) const
{
	m_textIndex = textIndex;
}

//----------------------------------------------------------------------
