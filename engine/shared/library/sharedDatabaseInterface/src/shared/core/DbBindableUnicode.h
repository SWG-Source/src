// ======================================================================
//
// DBBindableUnicode.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_DBBindableUnicode_H
#define INCLUDED_DBBindableUnicode_H

// ======================================================================

#include "sharedDatabaseInterface/DbBindableBase.h"

#include "UnicodeUtils.h"

// ======================================================================

namespace DB
{
	/**
	 * A large instrument resembling a cello.
	 */
	class BindableUnicodeBase : public Bindable
	{
	public:

		~BindableUnicodeBase();

		void *getBuffer();
		int getS();

		virtual std::string outputValue() const;

	protected:

		BindableUnicodeBase(char *buffer, int s);
		BindableUnicodeBase(char *buffer, int s, const BindableUnicodeBase &rhs);
		void operator=(const BindableUnicodeBase &rhs);
		
	private:

		BindableUnicodeBase();
		BindableUnicodeBase(const BindableUnicodeBase &rhs); // disable
		
	private:

		char *m_buffer;
		int   m_s;
	};

	inline BindableUnicodeBase::BindableUnicodeBase(char *buffer, int s)	:
		m_buffer(buffer),
		m_s(s)
	{
	}

	/**
	 * This takes the place of a copy contructor.
	 * It gets buffer and s from the derived class, instead of from rhs.
	 */
	inline BindableUnicodeBase::BindableUnicodeBase(char *buffer, int s, const BindableUnicodeBase &rhs) :
		Bindable(rhs),
		m_buffer(buffer),
		m_s(s)
	{
		DEBUG_FATAL(m_s!=rhs.m_s,("Cannot use BindableUnicode copy constructor on different size strings.\n")); // compiler shouldn't allow this, but just to make sure...
	}

	inline BindableUnicodeBase::~BindableUnicodeBase()
	{
		m_buffer = 0;
	}

	inline void *BindableUnicodeBase::getBuffer()
	{
		return m_buffer;
	}

	inline int BindableUnicodeBase::getS()
	{
		return m_s;
	}

	inline void BindableUnicodeBase::operator=(const BindableUnicodeBase &rhs)
	{
		//does not copy m_buffer or m_s
		DEBUG_FATAL(m_s!=rhs.m_s,("Cannot use BindableUnicode::operator= on different size strings.\n")); // compiler shouldn't allow this, but just to make sure...
		Bindable::operator=(rhs);
	}

// ======================================================================
/** Template that allows creating a class representing a C++ char[] array
	bound to a VARCHAR in the database.

	BindableUnicode is a template so that the size can be specified when it
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
	class BindableUnicode : public BindableUnicodeBase
	{
	  public:
		BindableUnicode();
		explicit BindableUnicode(const Unicode::String &_value);
		BindableUnicode(const BindableUnicode<S> &rhs);

/** Copy the string to another string
 */
		//char *getValue(char *buffer, size_t bufsize) const;
/** Copy the string to a Unicode String class
 */
		Unicode::String getValue() const;

/** Set the value of the bindable string from a String class.  See also setValue(const char*).
 */
		void setValue(const Unicode::String &buffer);

/** Same as setValue
 */
		BindableUnicode &operator=(const Unicode::String &buffer);

// Alternate forms of getValue, useful for auto-generated code (because of type conversion issues)
		void getValue(Unicode::String &buffer) const;

		int getS() const;

	  private:

		char   m_value[S+1]; // column of size S, plus one byte for a trailing nullptr
	};

// ======================================================================
// Template functions

	template<int S>
	BindableUnicode<S>::BindableUnicode()
	: BindableUnicodeBase(m_value, S)
	{
	}

	template<int S>
	BindableUnicode<S>::BindableUnicode(const Unicode::String &value)
	: BindableUnicodeBase(m_value, S)
	{
		setValue(value);
	}

	template<int S>
	BindableUnicode<S>::BindableUnicode(const BindableUnicode<S> &rhs)
	: BindableUnicodeBase(m_value, S, rhs)
	{
		strncpy(m_value,rhs.m_value,S+1);
	}
 
	template<int S>
	Unicode::String BindableUnicode<S>::getValue() const
	{
		if (isNull())
		{
			return Unicode::String();
		}
		else
		{
			Unicode::String str;
			std::basic_string<char> cstr (m_value, m_value + indicator);
			str = Unicode::utf8ToWide(cstr);
			return str;

		//-- Justin Randall [4/11/2001 5:56:56 PM] --
			// @todo : value is currently a char, should be an short or Unicode::String::value_type or wchar_t
			//return Unicode::String(Unicode::narrowToWide(m_value));
		}
	}

	template<int S>
	void BindableUnicode<S>::setValue(const Unicode::String &buffer)
	{
                std::string str;
                str = Unicode::wideToUTF8(buffer, str);

                size_t bufsize = str.size();

                if (bufsize >= S) {
                        WARNING(true, ("Attmpted to insert %s which is too long. Truncating.", buffer.c_str()));
                        indicator = S;
                } else {
                        indicator = bufsize;
                }	

		memcpy(m_value, str.c_str(), indicator);	
		m_value[indicator] = '\0';
	}

	template<int S>
	BindableUnicode<S> &BindableUnicode<S>::operator=(const Unicode::String &buffer)
	{
		setValue(buffer);
		return *this;
	}

	template<int S>
	void BindableUnicode<S>::getValue(Unicode::String &buffer) const
	{
		buffer.assign(getValue());
	}

	template<int S>
	int BindableUnicode<S>::getS() const
	{
		return S;
	}

// ======================================================================
}

#endif
