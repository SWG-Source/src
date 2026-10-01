// ======================================================================
//
// DbCheckedConversion.h
//
// Converting an integer to a narrower, or differently signed, integer type
// at the database boundary.
//
// A value read from a column, or a length handed to OCI, is converted only
// if the destination type represents it exactly. When it does not, the
// failure is logged with what was being converted and where, and false is
// returned; the caller then fails the load, encode or OCI operation. The
// value is never clamped or wrapped, because a clamped CRC, count or length
// is a different value that the rest of the server would trust.
//
// Conversions that cannot lose information (widening to a type that holds
// every value of the source) need no check and should stay plain casts.
//
// ======================================================================

#ifndef INCLUDED_DbCheckedConversion_H
#define INCLUDED_DbCheckedConversion_H

// ======================================================================

#include <limits>
#include <string>
#include <type_traits>

// ======================================================================

namespace DB
{
	namespace CheckedConversionNamespace
	{
		void reportFailure(std::string const &value, bool targetSigned, unsigned int targetBits, char const *what, char const *where);
	}

	/**
	 * True if the integer value is exactly representable in To.
	 */
	template <typename To, typename From>
	bool integerFits(From value)
	{
		static_assert(std::is_integral<To>::value && std::is_integral<From>::value, "integerFits converts integers only");
		static_assert(!std::is_same<To, bool>::value && !std::is_same<From, bool>::value, "bool is not a number; test it with a predicate");

		typedef std::numeric_limits<To> Limits;

		if constexpr (std::is_signed<From>::value && std::is_signed<To>::value)
			return value >= Limits::min() && value <= Limits::max();
		else if constexpr (!std::is_signed<From>::value && !std::is_signed<To>::value)
			return value <= Limits::max();
		else if constexpr (std::is_signed<From>::value)
			return value >= 0 && static_cast<typename std::make_unsigned<From>::type>(value) <= Limits::max();
		else
			return value <= static_cast<typename std::make_unsigned<To>::type>(Limits::max());
	}

	/**
	 * Store value in out if To represents it exactly, and return true.
	 * Otherwise leave out unchanged, log the failure (what was converted,
	 * and where: the query, object or OCI call), and return false.
	 */
	template <typename To, typename From>
	bool checkedNarrow(From value, To &out, char const *what, char const *where)
	{
		if (!integerFits<To>(value))
		{
			CheckedConversionNamespace::reportFailure(std::to_string(value), std::is_signed<To>::value, static_cast<unsigned int>(sizeof(To) * 8), what, where);
			return false;
		}
		out = static_cast<To>(value);
		return true;
	}

	/**
	 * The same, for a context that is costly to format (an object id, say):
	 * where() is called, and must return a std::string, only on failure.
	 */
	template <typename To, typename From, typename Where, typename = typename std::enable_if<std::is_invocable<Where const &>::value>::type>
	bool checkedNarrow(From value, To &out, char const *what, Where const &where)
	{
		if (!integerFits<To>(value))
		{
			std::string const context = where();
			CheckedConversionNamespace::reportFailure(std::to_string(value), std::is_signed<To>::value, static_cast<unsigned int>(sizeof(To) * 8), what, context.c_str());
			return false;
		}
		out = static_cast<To>(value);
		return true;
	}
}

// ======================================================================

#endif
