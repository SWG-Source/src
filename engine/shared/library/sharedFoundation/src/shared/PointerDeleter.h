// ======================================================================
//
// PointerDeleter.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#ifndef POINTER_DELETER_H
#define POINTER_DELETER_H

// ======================================================================

#include <utility>

// ======================================================================
/**
 * Delete the argument.
 *
 * The argument must be a pointer type.
 */

class PointerDeleter
{
public:
	template <typename PointerType>
	void operator ()(PointerType pointer) const
		{
			delete pointer;
		}
};

// ======================================================================
/**
 * Delete the argument using array-style deletion (i.e. delete [] object).
 *
 * The argument must be a pointer type.
 */

class ArrayPointerDeleter
{
public:
	template <typename PointerType>
	void operator ()(PointerType pointer) const
		{
			delete [] pointer;
		}
};

// ======================================================================
/**
 * Delete the argument's second member variable.
 *
 * The passed in argument must be of template type std::pair.
 */

class PointerDeleterPairSecond
{
public:
	template <typename FirstType, typename SecondType>
	void operator ()(std::pair<FirstType, SecondType> &pairArgument) const
		{
			delete pairArgument.second;
		}
};

// ======================================================================
/**
 * Delete the argument's second member variable using array-style deletion.
 *
 * The passed in argument must be of template type std::pair.
 */

class ArrayPointerDeleterPairSecond
{
public:
	template <typename FirstType, typename SecondType>
	void operator ()(std::pair<FirstType, SecondType> &pairArgument) const
		{
			delete [] pairArgument.second;
		}
};

// ======================================================================

#endif
