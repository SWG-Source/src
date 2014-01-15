// ======================================================================
//
// DbBufferString.h
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_DbBufferString_H
#define INCLUDED_DbBufferString_H

// ======================================================================

#include "sharedDatabaseInterface/DbBindableBase.h"
#include "sharedDatabaseInterface/DbBindableString.h"
#include "sharedDatabaseInterface/DbBindableUnicode.h"

// ======================================================================

class NullEncodedUnicodeString;
class NullEncodedStandardString;

// ======================================================================

namespace DB
{
/**
 * A nullable string class similar to DB::BindableString, but uses std::string
 * internally.  Unlike DB::BindableString, it does not allocate the maximum
 * size for the buffer if the data is shorter.  This class will save memory
 * over DB::BindableString if used in a buffer that stores a lot of rows.
 * However, it cannot be bound directly in a query.  It must be copied
 * to a varray or a DB::BindableString first.
 */
	class BufferString : public Bindable
	{
	  public:
		BufferString(size_t maxSize);
		template<int S>
			BufferString(const BindableString<S> & rhs)
			{
				rhs.getValue(m_value);
				indicator=m_value.size();
			}
			
		template<int S>
			BufferString(const BindableUnicode<S> & rhs)
			{
				m_value = Unicode::wideToUTF8(rhs.getValue());
				indicator=m_value.size();
			}

	  public:
		void getValue(std::string & value) const;
		void getValue(Unicode::String & value) const;
		void getValue(NullEncodedUnicodeString & value) const;
		void getValue(NullEncodedStandardString & value) const;
		Unicode::String getValue() const;
		const std::string & getValueASCII() const;
		BufferString & operator=(const std::string & value);
		BufferString & operator=(const Unicode::String & value);
		BufferString & operator=(const NullEncodedUnicodeString & value);
		BufferString & operator=(const NullEncodedStandardString & value);
		void setValue(const std::string & value);
		void setValue(const Unicode::String & value);

		virtual std::string outputValue() const;
					
	  private:
		std::string m_value;
		size_t m_maxSize;
	};

	// ======================================================================

} // namespace

// ======================================================================


#endif
