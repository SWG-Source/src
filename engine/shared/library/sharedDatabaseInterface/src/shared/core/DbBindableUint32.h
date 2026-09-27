// ======================================================================
//
// DbBindableUint32.h
//
// A column holding an unsigned 32-bit value: a station id, a CRC, a Tag.
//
// SWG databases store these as the signed 32-bit integer with the same
// bits, the form 32-bit servers have always written and that SQL, PL/SQL
// and external tools compare against (e.g. -1890265955 for 2404701341).
// The column owns that encoding: it holds a 4-byte signed integer, bound to
// the database at exactly that width on every platform, and exchanges
// values with C++ only as uint32_t. Setting it from any other type does not
// compile, so a caller cannot choose a different representation by casting.
//
// ======================================================================

#ifndef INCLUDED_DbBindableUint32_H
#define INCLUDED_DbBindableUint32_H

// ======================================================================

#include "sharedDatabaseInterface/DbBindableBase.h"

#include <cstdint>
#include <string>
#include <type_traits>

// ======================================================================

namespace DB
{
	class BindableUint32 : public Bindable
	{
		// Every arithmetic type other than uint32_t is rejected.
		template <typename T>
		using Rejected = typename std::enable_if<std::is_arithmetic<T>::value>::type;

	  public:
		BindableUint32();
		explicit BindableUint32(uint32_t value);
		template <typename T, typename = Rejected<T> > explicit BindableUint32(T) = delete;

		uint32_t getValue() const;
		void getValue(uint32_t &buffer) const;

		void setValue(uint32_t value);
		template <typename T, typename = Rejected<T> > void setValue(T) = delete;
		BindableUint32 &operator=(uint32_t value);
		template <typename T, typename = Rejected<T> > BindableUint32 &operator=(T) = delete;

		// The stored form, for the database layer.
		int32_t getDatabaseValue() const;
		void *getBuffer();

		virtual std::string outputValue() const;

	  private:
		int32_t m_value;
	};
}

// ======================================================================

inline DB::BindableUint32::BindableUint32() :
	Bindable(),
	m_value(0)
{
}

// ----------------------------------------------------------------------

inline DB::BindableUint32::BindableUint32(uint32_t value) :
	Bindable(sizeof(m_value)),
	m_value(static_cast<int32_t>(value)) // modulo 2^32 (GCC, Clang; C++20)
{
}

// ----------------------------------------------------------------------

inline uint32_t DB::BindableUint32::getValue() const
{
	return static_cast<uint32_t>(m_value);
}

// ----------------------------------------------------------------------

inline void DB::BindableUint32::getValue(uint32_t &buffer) const
{
	buffer = getValue();
}

// ----------------------------------------------------------------------

inline void DB::BindableUint32::setValue(uint32_t value)
{
	indicator = sizeof(m_value);
	m_value = static_cast<int32_t>(value); // modulo 2^32 (GCC, Clang; C++20)
}

// ----------------------------------------------------------------------

inline DB::BindableUint32 &DB::BindableUint32::operator=(uint32_t value)
{
	setValue(value);
	return *this;
}

// ----------------------------------------------------------------------

inline int32_t DB::BindableUint32::getDatabaseValue() const
{
	return m_value;
}

// ----------------------------------------------------------------------

inline void *DB::BindableUint32::getBuffer()
{
	return &m_value;
}

// ----------------------------------------------------------------------

inline std::string DB::BindableUint32::outputValue() const
{
	return std::to_string(getValue());
}

// ======================================================================

#endif
