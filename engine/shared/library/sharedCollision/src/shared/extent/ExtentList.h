// ======================================================================
//
// ExtentList.h
// jeff grills
//
// copyright 1999 Bootprint Entertainment
//
// ======================================================================

#ifndef EXTENT_LIST_H
#define EXTENT_LIST_H

// This is an abstract factory for extents

// ======================================================================

class Extent;
class Iff;

#include "sharedFoundation/Tag.h"

// ======================================================================

class ExtentList
{
public:

	typedef Extent *(*CreateFunction)(Iff &iff);

private:

	static bool          ms_installed;

	//-- the private implementation of the list of Tag->CreateFunction bindings
	struct BindImpl;
	static BindImpl      ms_bindImpl;

public:

	static void          install(void);
	static void          remove(void);

	static void          assignBinding(Tag tag, CreateFunction createFunction);
	static void          removeBinding(Tag tag);

	static Extent *      nullFactory( Iff & iff );

	static const Extent *fetch(const Extent *extent);
	static const Extent *fetch(Iff &iff);
	static Extent *      create(Iff &iff);

	static void          release(const Extent *extent);
};

// ======================================================================

#endif
