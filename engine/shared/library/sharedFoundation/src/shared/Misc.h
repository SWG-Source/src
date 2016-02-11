// ======================================================================
//
// Misc.h
// jeff grills
//
// copyright 1998 Bootprint Entertainment
//
// ======================================================================

#ifndef MISC_H
#define MISC_H

// ======================================================================
/**
 * Return the square of a number.
 *
 * This routine is templated, and implemented in terms of the * operator.
 *
 * @return The square of its argument
 */
template<class T>
inline T sqr(
	const T &t  // The number to square
	)
{
	return (t * t);
}

// ----------------------------------------------------------------------
/**
 * Make sure value is between two values.
 * 
 * This routine is templated, and implemented in terms of the > and > operators.
 * 
 * @param minT  Minimum variable
 * @param t  Variable to clamp against min and max
 * @param maxT  Minimum variable
 * @return If t is less than min, return min; if t is greater than max, return max
 * otherwise, return t
 */

template<class T>inline const T clamp(const T &minT, const T &t, const T &maxT)
{
	DEBUG_FATAL(maxT < minT, ("clamp error: max value is less than min value"));
	return (t < minT) ? minT : ((t > maxT) ? maxT : t);
}

// ----------------------------------------------------------------------
/**
 * returns whether a value is within a given range, including the range end points.
 * 
 * This routine will fatal in debug builds if max is less than min.
 * 
 * @return True if value falls within the range min .. max, including min and max,
 * false otherwise.
 */

template<class T>inline bool WithinRangeInclusiveInclusive(const T &rangeMin, const T &value, const T &rangeMax)
{
	DEBUG_FATAL(rangeMax < rangeMin, ("range error: max value is less than min value"));
	return (value >= rangeMin) && (value <= rangeMax);
}

// ----------------------------------------------------------------------
/**
 * returns whether a value is within a given range, excluding the range end points.
 * 
 * This routine will fatal in debug builds if max is less than min.
 * 
 * @return True if value falls within the range min .. max, not including min and max,
 * false otherwise.
 */

template<class T>inline bool WithinRangeExclusiveExclusive(const T &rangeMin, const T &value, const T &rangeMax)
{
	DEBUG_FATAL(rangeMax < rangeMin, ("range error: max value is less than min value"));
	return (value > rangeMin) && (value < rangeMax);
}

// ----------------------------------------------------------------------
/**
 * returns true if a value falls within the range (base - epsilon) .. (base + epsilon),
 * including range endpoints.
 * 
 * This routine will fatal during debug builds if epsilon is less than zero.
 * 
 * @return true if value is within the range (base - epsilon) .. (base + epsilon), including
 * the range endpoints.  Otherwise returns false.
 */

template<class T>inline bool WithinEpsilonInclusive(const T &base, const T &value, const T &epsilon)
{
	DEBUG_FATAL(epsilon < static_cast<T>(0), ("error: epsilon is less than zero"));
	return (value >= base - epsilon) && (value <= base + epsilon);
}

// ----------------------------------------------------------------------
/**
 * returns true if a value falls within the range (base - epsilon) .. (base + epsilon),
 * excluding range endpoints.
 * 
 * This routine will fatal during debug builds if epsilon is less than zero.
 * 
 * @return true if value is within the range (base - epsilon) .. (base + epsilon), including
 * the range endpoints.  Otherwise returns false.
 */

template<class T>inline bool WithinEpsilonExclusive(const T &base, const T &value, const T &epsilon)
{
	DEBUG_FATAL(epsilon < static_cast<T>(0), ("error: epsilon is less than zero"));
	return (value > base - epsilon) && (value < base + epsilon);
}

// ----------------------------------------------------------------------
/**
 * Zero out an object.
 * 
 * This routine is templated.
 * 
 * This routine will byte-zero fill the object which is passed to it.
 * 
 * @param t  The object to zero out
 */

template<class T>inline void Zero(T &t)
{
	memset(&t, 0, sizeof(t));
}

// ----------------------------------------------------------------------
/**
 * Duplicate a string.
 * 
 * This routine allocates memory for a copy of its argument, copies
 * the source string into the new memory, and then returns a pointer to
 * the new memory.
 * 
 * This routine will return nullptr if called with nullptr.
 * 
 * @param source  The string to copy
 * @return A pointer to a copy of the source argument
 */

inline char *DuplicateString(const char *source)
{
	if (!source)
		return nullptr;

	const uint length = strlen(source)+1;
	char *result = NON_NULL (new char[length]);
	memcpy(result, source, length);
	return result;
}

// ----------------------------------------------------------------------
/**
 * Duplicate a string.
 * 
 * This routine allocates memory for a copy of its argument, copies
 * the source string into the new memory, and then returns a pointer to
 * the new memory.
 * 
 * This routine will return nullptr if called with nullptr.
 * 
 * @param source  The string to copy
 * @return A pointer to a copy of the source argument
 */

inline char *DuplicateStringWithToLower(const char *source)
{
	if (!source)
		return nullptr;

	const uint length = strlen(source)+1;
	char *result = NON_NULL (new char[length]);

	for (uint i = 0; i < length; ++i)
		result[i] = static_cast<char>(tolower(source[i]));

	return result;
}

// ----------------------------------------------------------------------
/**
 * memset with a integer length.
 * 
 * This routine will
 * 
 * @param data  Location to fill at
 * @param value  Vvalue to fill with
 * @param length  Number of bytes to fill
 */

inline void imemset(void *data, int value, int length)
{
	DEBUG_FATAL(!data, ("nullptr data arg"));
	memset(data, value, static_cast<uint>(length));
}

// ----------------------------------------------------------------------
/**
 * memcpy with a integer length.
 * 
 * This routine will
 * 
 * @param destination  Location to copy to
 * @param source  Location to copy from
 * @param length  Number of bytes to copy
 */

inline void imemcpy(void *destination, const void *source, int length)
{
	DEBUG_FATAL(!destination, ("nullptr destination arg"));
	DEBUG_FATAL(!source, ("nullptr source arg"));
	memcpy(destination, source, static_cast<uint>(length));
}

// ----------------------------------------------------------------------
/**
 * memmove with a integer length.
 * 
 * This routine will
 * 
 * @param destination  Location to copy to
 * @param source  Location to copy from
 * @param length  Number of bytes to copy
 */

inline void *memmove(void *destination, const void *source, int length)
{
	DEBUG_FATAL(!destination, ("nullptr destination arg"));
	DEBUG_FATAL(!source, ("nullptr source arg"));
	return memmove(destination, source, static_cast<uint>(length));
}

// ----------------------------------------------------------------------
/**
 * Returns the length of the string as an int.
 * 
 * @param string  String to compute the length
 * @return The length of the string as an int
 */

inline int istrlen(const char *string)
{
	DEBUG_FATAL(!string, ("nullptr string arg"));
	return static_cast<int>(strlen(string));
}

// ----------------------------------------------------------------------
/**
 * Determine if a number is a power of two.
 * 
 * This routine runs in O(1) time.
 * 
 * @param value  Value to see if it is a power of two
 * @return True if the value is a power of two, otherwise false
 */

template<class T> inline bool IsPowerOfTwo(T value)
{
	return ((value - 1) & (value)) == 0;
}

// ----------------------------------------------------------------------
/**
 * Returns the number of bits set in an integral type.
 * 
 * Type T must provide integral-type bitwise operations and semantics.
 * 
 * @param value  Value to count the number of bits
 */

template<class T> inline int GetBitCount(T value)
{
	int count = 0;
	while (value)
	{
		value &= (value-1);
		++count;
	}

	return count;
}

// ----------------------------------------------------------------------
/**
 * Returns the index of the first bit set in an integral type.
 * 
 * Type T must provide integral-type bitwise operations and semantics.
 * 
 * @param value  Value to find the first set bit
 * @return Returns the zero-based index of the first bit set in an
 * integral type, with the least significant byte being index 0.
 * 
 * Returns -1 if no bits are set in the value.
 */

template<class T>inline int GetFirstBitSet(T value)
{
	if (!value)
		return -1;

	int count;
	for (count = 0; !(value & static_cast<T>(0x01)); ++count, value >>= 1)
		{}

	return count;
}

// ----------------------------------------------------------------------
/**
 * Calculate the factorial of the specified value.
 * 
 * This routine takes O(n) time to compute.
 * 
 * @param number  The number to compute the factorial of
 * @return The factorial of the specified value
 */

inline int factorial(int number)
{
	int i, result;

	for (result = 1, i = 2; i <= number; ++i)
		result *= i;

	return result;
}

// ----------------------------------------------------------------------
/**
 * Calculate the number of combinations of i elements taken from a set of n elements.
 * 
 * This routine computes: n! / (i! * (n-i)!).
 * 
 * @param n  Number of elements in the set
 * @param i  Number of elements to pick from the set
 * @return The number of distinct combinations
 */

inline int choose(int n, int i)
{
	return factorial(n) / (factorial(i) * factorial(n - i));
}

// ----------------------------------------------------------------------
/**
 * Linear interpolate from start to end along t, where t is from 0..1.
 * 
 * t must be from 0..1
 * 
 * @return A value from start to end
 */

template<class T> const T linearInterpolate (const T& start, const T& end, const real t)
{
	return static_cast<T>((end - start) * t) + start;
}

// ----------------------------------------------------------------------
/**
 * A non-linear, smooth interpolation from start to end along t, where t is from 0..1.
 * 
 * (-2 * (end - start) * t^3) + (3 * (end - start) * t^2) + start
 *
 * t must be from 0..1
 * 
 * @return A value from start to end
 */

template<class T> const T cubicInterpolate(const T& start, const T& end, const real t)
{
	real const tSquared = sqr(t);
	real const tCubed = tSquared * t;
	T const & diff = end - start;
	return static_cast<T>((diff * (tCubed * static_cast<real>(-2))) + (diff * (tSquared * static_cast<real>(3))) + start);
}

// ----------------------------------------------------------------------
/**
 * A lint-friendly boolean compare.
 * 
 * @return A value from start to end
 */

inline bool boolEqual(bool const lhs, bool const rhs)
{
	return (lhs && rhs) || (!lhs && !rhs);
}

// ======================================================================

#endif
