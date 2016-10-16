// ======================================================================
//
// DBBindableString.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_DBBindableString_H
#define INCLUDED_DBBindableString_H

// ======================================================================

#include "sharedDatabaseInterface/DbBindableBase.h"

#include "UnicodeUtils.h"

// ======================================================================

namespace DB
{
	/**
	 * A large instrument resembling a cello.
	 */
	class BindableStringBase : public Bindable
	{
	public:

		~BindableStringBase();

		void *getBuffer();
		int getS();

		virtual std::string outputValue() const;
		
	protected:

		BindableStringBase(char *buffer, int s);
		BindableStringBase(char *buffer, int s, const BindableStringBase &rhs);
		void operator=(const BindableStringBase &rhs);
		
	private:

		BindableStringBase();
		BindableStringBase(const BindableStringBase &rhs); // disable
		
	private:

		char *m_buffer;
		int   m_s;
	};

	inline BindableStringBase::BindableStringBase(char *buffer, int s)	:
		m_buffer(buffer),
		m_s(s)
	{
	}

	/**
	 * This takes the place of a copy contructor.
	 * It gets buffer and s from the derived class, instead of from rhs.
	 */
	inline BindableStringBase::BindableStringBase(char *buffer, int s, const BindableStringBase &rhs) :
		Bindable(rhs),
		m_buffer(buffer),
		m_s(s)
	{
		DEBUG_FATAL(m_s!=rhs.m_s,("Cannot use BindableString copy constructor on different size strings.\n")); // compiler shouldn't allow this, but just to make sure...
	}

	inline BindableStringBase::~BindableStringBase()
	{
		m_buffer = 0;
	}

	inline void *BindableStringBase::getBuffer()
	{
		return m_buffer;
	}

	inline int BindableStringBase::getS()
	{
		return m_s;
	}

	inline void BindableStringBase::operator=(const BindableStringBase &rhs)
	{
		//does not copy m_buffer or m_s
		DEBUG_FATAL(m_s!=rhs.m_s,("Cannot use BindableString::operator= on different size strings.\n")); // compiler shouldn't allow this, but just to make sure...
		Bindable::operator=(rhs);
	}

// ======================================================================
/** Template that allows creating a class representing a C++ char[] array
	bound to a VARCHAR in the database.

	BindableString is a template so that the size can be specified when it
	is instantiated.  The template parameter S represents the size of the
	array.

	Question:  Why is this a template?  Couldn't S be a parameter to the
	constructor?

	Short answer:  Because of the way ODBC works, the size must be known at
	compile time

	Long answer:  We want to be able to use these things in arrays that are
	bound.  The functions for binding an array do not allow an element of the
	array to be a pointer to another location.  Therefore, to work with ODBC,
	the buffer for the string must be part of the class, not a pointer.

	TODO:  Currently this template represents 8-bit characters.  Make it support
	Unicode.
	TODO:  We have both setValue() functions and operator='s.  Should just pick
	one and use it.
 */
	template<int S>
	class BindableString : public BindableStringBase
	{
	  public:
		BindableString();
		explicit BindableString(const Unicode::String &_value);
		explicit BindableString(const std::string &_value);
		BindableString(const BindableString<S> &rhs);

/** Copy the string to another string
 */
		char *getValue(char *buffer, size_t bufsize) const;
/** Copy the string to a Unicode String class
 */
		Unicode::String getValue() const;
/** Copy the string to an 8-bit String class
 */
		std::string getValueASCII() const;

/** Allocate a new string and copy the buffer to it.  The caller takes responsibility
	for delete[]ing it later.  (If the string is nullptr, this returns a nullptr pointer.)
 */
		char *makeCopyOfValue() const;

/** Set the value of the bindable string from another (nullptr-terminated) string

	If the source buffer is too big, it	FATAL's.  Another alternative would be to have
	it truncate the string.  Right now, it's better to FATAL to avoid hard-to-find bugs
	caused by the database fields being too small.  However, might want to revist this
	later.
 */
		void setValue(const char *buffer);

/** Set the value of the bindable string from a String class.  See also setValue(const char*).
 */
		void setValue(const Unicode::String &buffer);

/**
 * Set the value of the bindable string from a std::string.
 */
		void setValue(const std::string &buffer);

/** Same as setValue
 */
		BindableString &operator=(const Unicode::String &buffer);
		BindableString &operator=(const std::string &buffer);

// Alternate forms of getValue, useful for auto-generated code (because of type conversion issues)
		void getValue(Unicode::String &buffer) const;
		void getValue(std::string &buffer) const;

		int getS() const;

	  private:

		char   m_value[S+1]; // column of size S, plus one byte for a trailing nullptr
	};

// ======================================================================
// Template functions

	template<int S>
	BindableString<S>::BindableString()
	: BindableStringBase(m_value, S)
	{
	}

	template<int S>
	BindableString<S>::BindableString(const Unicode::String &value)
	: BindableStringBase(m_value, S)
	{
		setValue(value);
	}

	template<int S>
	BindableString<S>::BindableString(const std::string &value)
	: BindableStringBase(m_value, S)
	{
		setValue(value);
	}

	template<int S>
	BindableString<S>::BindableString(const BindableString<S> &rhs)
	: BindableStringBase(m_value, S, rhs)
	{
		strncpy(m_value,rhs.m_value,S+1);
	}
 
	template<int S>
	char *BindableString<S>::getValue(char *buffer, size_t bufsize) const
	{
		if (isNull())
		{
			return strncpy(buffer,"\0",1); // in the database, a zero-length string and a nullptr aren't really
			// the same thing, but this is the closest we can do here.
		}
		return strncpy(buffer,m_value,(bufsize>(S+1))?(S+1):bufsize);
	}

	template<int S>
	Unicode::String BindableString<S>::getValue() const
	{
		if (isNull())
		{
			return Unicode::String();
		}
		else
		{
		//-- Justin Randall [4/11/2001 5:56:56 PM] --
			// @todo : value is currently a char, should be an short or Unicode::String::value_type or wchar_t
			return Unicode::String(Unicode::narrowToWide(m_value));
		}
	}

	template<int S>
	std::string BindableString<S>::getValueASCII() const
	{
		if (isNull())
		{
			return std::string();
		}
		else
		{
			return std::string(m_value);
		}
	}

	template<int S>
	void BindableString<S>::setValue(const char *buffer)
	{
		if (buffer==0)
		{
			setNull();
			return;
		}

		size_t bufsize = strlen(buffer);

		if (bufsize >= S) {
			WARNING(true, ("Attmpted to insert %s which is too long. Truncating.", buffer));
			indicator = S;
		} else {
			indicator = bufsize;
		}

		memcpy(m_value, buffer, indicator);
                m_value[indicator] = '\0';
	}

	template<int S>
	void BindableString<S>::setValue(const Unicode::String &buffer)
	{
                size_t bufsize = buffer.size();

                if (bufsize >= S) {
                        WARNING(true, ("Attmpted to insert %s which is too long. Truncating.", buffer.c_str()));
                        indicator = S;
			memcpy(m_value, Unicode::wideToNarrow(buffer).c_str(), indicator-1);
                } else {
                        indicator = bufsize;
                }

        	memcpy(m_value, Unicode::wideToNarrow(buffer).c_str(), indicator);
		m_value[indicator] = '\0';
	}

	template<int S>
	void BindableString<S>::setValue(const std::string &buffer)
	{
                size_t bufsize = buffer.size();

                if (bufsize >= S) {
                        WARNING(true, ("Attmpted to insert %s which is too long. Truncating.", buffer.c_str()));
                        indicator = S;
                } else {
                        indicator = bufsize;
			memcpy(m_value, buffer.c_str(), indicator);
                }

		memcpy(m_value, buffer.c_str(), indicator);
                m_value[indicator] = '\0';
	}

	template<int S>
	BindableString<S> &BindableString<S>::operator=(const Unicode::String &buffer)
	{
		setValue(buffer);
		return *this;
	}

	template<int S>
	BindableString<S> &BindableString<S>::operator=(const std::string &buffer)
	{
		setValue(buffer.c_str());
		return *this;
	}

	template<int S>
	char *BindableString<S>::makeCopyOfValue() const
	{
		if (isNull()) return 0;

		char *target=new char[indicator+1];
		strncpy(target,m_value,indicator+1);
		return target;
	}

	template<int S>
	void BindableString<S>::getValue(Unicode::String &buffer) const
	{
		buffer=getValue();
	}

	template<int S>
	void BindableString<S>::getValue(std::string &buffer) const
	{
		buffer=getValueASCII();
	}

	template<int S>
	int BindableString<S>::getS() const
	{
		return S;
	}

// ======================================================================
}

#endif
