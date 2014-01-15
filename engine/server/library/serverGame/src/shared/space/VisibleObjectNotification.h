// ======================================================================
//
// VisibleObjectNotification.h
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_VisibleObjectNotification_H
#define INCLUDED_VisibleObjectNotification_H

// ======================================================================

#include "sharedObject/ObjectNotification.h"

// ======================================================================

/**
 * A Notification that alerts SpaceVisibilityManager about a visible
 * object that has moved.
 */
class VisibleObjectNotification : public ObjectNotification
{
  public:

	static VisibleObjectNotification & getInstance ();

  public:

	VisibleObjectNotification ();
	virtual ~VisibleObjectNotification ();

	virtual void removeFromWorld(Object &object) const;
	virtual bool positionAndRotationChanged(Object &object, bool dueToParentChange, const Vector &oldPosition) const;
	virtual void cellChanged(Object &object, bool dueToParentChange) const;
	
	VisibleObjectNotification (const VisibleObjectNotification&);
	VisibleObjectNotification& operator= (const VisibleObjectNotification&);

  private:

	static VisibleObjectNotification ms_instance;
};

// ======================================================================

inline VisibleObjectNotification & VisibleObjectNotification::getInstance()
{
	return ms_instance;
}

// ======================================================================

#endif
