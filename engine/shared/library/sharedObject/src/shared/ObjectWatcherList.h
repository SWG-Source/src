// ======================================================================
//
// ObjectWatcherList.h
// copyright 2002, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_ObjectWatcherList_H
#define INCLUDED_ObjectWatcherList_H

// ======================================================================

template <typename T> class Watcher;
class Object;
class Camera;

// ======================================================================
// Contain a list of watchers to objects.
//
// Objects may exist in more than one ObjectWatcherList simultaneously.

class ObjectWatcherList
{
public:

	explicit       ObjectWatcherList           (int initialVectorSize=0);
	              ~ObjectWatcherList           ();

	void           addObject                   (Object & object);
	void           removeObject                (const Object & object);
	int            removeNulls                 ();

	void           removeAll                   (bool deleteObjects=false);

	int            getNumberOfObjects          () const;

	Object *       getObject                   (int index);
	const Object * getObject                   (int index) const;

	Object       * operator []                 (int index);
	const Object * operator []                 (int index) const;

	bool           find                        (const Object & objectToCheck, int* index=0) const;
	void           removeObjectByIndex         (const Object &, int index);
	
	void           addToWorld                  ();
	void           removeFromWorld             ();

	void           prepareToAlter              ();
	void           alter                       (real time);
	void           conclude                    ();

	void           setRegionOfInfluenceEnabled (bool enabled) const;
	void           render                      (const Object *omitObject) const;

	void setSkipCellRegionOfInfluence(bool skip);

private:
	
	typedef std::vector<Watcher<Object> > ObjectVector;
	bool                                     m_altering;
	ObjectVector *                           m_objectVector;
	ObjectVector *                           m_alterSafeObjectVector;
	bool m_skipCellRegionOfInfluence;

private:

	// disable these routines
	                    ObjectWatcherList (const ObjectWatcherList &);
	ObjectWatcherList & operator =        (const ObjectWatcherList &);
};

// ======================================================================

#endif
