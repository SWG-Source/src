// ======================================================================
//
// LocalizedString.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_LocalizedString_H
#define INCLUDED_LocalizedString_H

#if WIN32
// stl warning func not inlined
#pragma warning (disable:4710)
// unref inline func removed
#pragma warning (disable:4514)
// symbol name too long
#pragma warning (disable:4786)
#endif

#include "Unicode.h"

// ======================================================================

/**
* LocalizedString is a class that wraps a Unicode string, a string id,
* and a timestamp indicating the last modification to this string on disk.
* The string id is an index into localization map, in this case the
* LocalizedStringTable.
*/

class LocalizedStringTableRW;
class AbstractFile;

class LocalizedString
{
public:
	friend class                LocalizedStringTableRW;

	// TODO: make these typedefs platform dependent

	typedef unsigned long       id_type;
	typedef unsigned long       crc_type;

								LocalizedString(id_type id, crc_type sourceCrc, Unicode::String const & str);
								LocalizedString(id_type id, Unicode::String const & str); // GENERATE Crc FROM UNICODE.

	                            LocalizedString (const LocalizedString & rhs);
	LocalizedString &           operator=       (const LocalizedString & rhs);
	                            LocalizedString ();
	                           ~LocalizedString ();
	bool                        operator==      (const LocalizedString & rhs);



	id_type                     getId           () const;
	crc_type                    getCrc() const;
	const Unicode::String &     getString       () const;

	size_t                      getNumLines     () const;
	const Unicode::String       getStringLine   (size_t num) const;

	static LocalizedString *    load_0000       (AbstractFile & fl);
	static LocalizedString * load_0001(AbstractFile & fl);


	static crc_type const nullCrc;
	void buildCrc();

	// rls - The source crc is used with translated string pairs.  The sourceCrc 
	// is the Crc of the string that was used to generate the translatd text.
	void setSourceCrc(crc_type const crc);
	crc_type getSourceCrc() const;

protected:

	Unicode::String             m_str;
	id_type                     m_id;

	crc_type m_crc;
	crc_type m_sourceCrc;

	//-- not stored in file
	size_t                      m_numLines;
	size_t *                    m_lineStarts;

protected:

	void                        resetLineCounts ();

private:

};

//-----------------------------------------------------------------

/**
* Get the unique identifier (withing a table) for this LocalizedString.
*/
inline LocalizedString::id_type LocalizedString::getId () const
{
	return m_id;
}

//-----------------------------------------------------------------

/**
* Get the last-modified timestamp.
*/

inline LocalizedString::crc_type LocalizedString::getCrc() const
{
	return m_crc;
}

//-----------------------------------------------------------------

/**
* Get the actual string value.
*/

inline const Unicode::String & LocalizedString::getString () const
{
	return m_str;
}

//-----------------------------------------------------------------

/**
* Get the (cached) number of lines in the string.
*/

inline size_t LocalizedString::getNumLines () const
{
	return m_numLines;
}

//-----------------------------------------------------------------

/**
* Returns a single line of the string.  If num is greater or equal than the number of
* available lines, the entire string is returned.
*/
inline const Unicode::String LocalizedString::getStringLine (size_t num) const
{
	if (m_numLines < 2 || num >= m_numLines)
		return m_str;

	if (num == m_numLines - 1)
		return m_str.substr (m_lineStarts [num]);

	return m_str.substr (m_lineStarts [num], (m_lineStarts [num+1] - m_lineStarts [num] - 1));
}


//-----------------------------------------------------------------

/**
* Set the source crc.
*/

inline void LocalizedString::setSourceCrc(LocalizedString::crc_type const crc)
{
	m_sourceCrc = crc;
}

//-----------------------------------------------------------------

/**
* Get the source crc.
*/

inline LocalizedString::crc_type LocalizedString::getSourceCrc() const
{
	return m_sourceCrc;
}


// ======================================================================

#endif
