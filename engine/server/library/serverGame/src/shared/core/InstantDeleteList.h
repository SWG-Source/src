// ======================================================================
//
// InstantDeleteList.h
// copyright (c) 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_InstantDeleteList_H
#define INCLUDED_InstantDeleteList_H

// ======================================================================

class ObjectTemplate;

// ======================================================================

/**
 * List of object templates that can be deleted immediately.
 */
class InstantDeleteList
{
  public:
	static void install();
	static void remove();

	static bool isOnList(const ObjectTemplate &theTemplate);

  private:
	typedef std::set<uint32> ListType; 
	static ListType *ms_theList;
};

// ======================================================================

#endif
