// ======================================================================
//
// DbBindableInt32.h
//
// A column holding a signed 32-bit integer: the SQL "int" and integral
// "number" columns of the SWG schema.
//
// The column is bound to the database as exactly four bytes on every
// platform. Its C++ value is int32_t, and it exchanges values only as
// int32_t: setting it from any other arithmetic type does not compile, so a
// caller that holds a wider, narrower or unsigned value has to say how that
// value becomes an int32_t (see DbCheckedConversion.h), instead of the
// conversion being chosen silently by the width of long.
//
// A default-constructed column is NULL, and stays NULL until a value is set;
// callers test isNull() before getValue().
//
// An unsigned 32-bit column (station id, CRC, Tag) is a DB::BindableUint32,
// and a 64-bit column is a DB::BindableInt64.
//
// ======================================================================

#ifndef INCLUDED_DbBindableInt32_H
#define INCLUDED_DbBindableInt32_H

// ======================================================================

#include "sharedDatabaseInterface/DbBindableBase.h"

#include <cstdint>
#include <string>
#include <type_traits>

// ======================================================================

namespace DB
{
	class BindableInt32 : public Bindable
	{
		// Every arithmetic type other than int32_t is rejected.
		template <typename T>
		using Rejected = typename std::enable_if<std::is_arithmetic<T>::value>::type;

	  public:
		BindableInt32();
		explicit BindableInt32(int32_t value);
		template <typename T, typename = Rejected<T> > explicit BindableInt32(T) = delete;

		int32_t getValue() const;
		void getValue(int32_t &buffer) const;
		template <typename T, typename = Rejected<T> > void getValue(T &) const = delete;

		void setValue(int32_t value);
		template <typename T, typename = Rejected<T> > void setValue(T) = delete;
		BindableInt32 &operator=(int32_t value);
		template <typename T, typename = Rejected<T> > BindableInt32 &operator=(T) = delete;

		void *getBuffer();

		virtual std::string outputValue() const;

	  protected:
		virtual void clearValue();

	  private:
		int32_t m_value;
	};
}

// ======================================================================

// The value of a NULL column is not a number the column holds. It starts
// as -999, the sentinel BindableLong used, so that code which reads a NULL
// column by mistake sees the same value it always has, never 0.
inline DB::BindableInt32::BindableInt32() :
	Bindable(),
	m_value(-999)
{
}

// ----------------------------------------------------------------------

inline DB::BindableInt32::BindableInt32(int32_t value) :
	Bindable(sizeof(m_value)),
	m_value(value)
{
}

// ----------------------------------------------------------------------

inline int32_t DB::BindableInt32::getValue() const
{
	return m_value;
}

// ----------------------------------------------------------------------

inline void DB::BindableInt32::getValue(int32_t &buffer) const
{
	buffer = m_value;
}

// ----------------------------------------------------------------------

inline void DB::BindableInt32::setValue(int32_t value)
{
	indicator = sizeof(m_value);
	m_value = value;
}

// ----------------------------------------------------------------------

inline DB::BindableInt32 &DB::BindableInt32::operator=(int32_t value)
{
	setValue(value);
	return *this;
}

// ----------------------------------------------------------------------

inline void DB::BindableInt32::clearValue()
{
	m_value = -999; // the value of a default-constructed (NULL) column
}

// ----------------------------------------------------------------------

inline void *DB::BindableInt32::getBuffer()
{
	return &m_value;
}

// ----------------------------------------------------------------------

inline std::string DB::BindableInt32::outputValue() const
{
	return std::to_string(m_value);
}

// ======================================================================

#endif
