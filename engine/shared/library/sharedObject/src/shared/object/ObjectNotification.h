// ======================================================================
//
// ObjectNotifcation.h
// Copyright 2002 Sony Online Entertainment
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_ObjectNotifcation_H
#define INCLUDED_ObjectNotifcation_H

// ======================================================================

class Object;
class Vector;

// ======================================================================

class ObjectNotification
{
public:

	virtual int getPriority() const;

	virtual void addToWorld(Object &object) const;
	virtual void removeFromWorld(Object &object) const;

	virtual bool positionChanged(Object &object, bool dueToParentChange, const Vector &oldPosition) const;
	virtual void rotationChanged(Object &object, bool dueToParentChange) const;
	virtual bool positionAndRotationChanged(Object &object, bool dueToParentChange, const Vector &oldPosition) const;
	virtual void cellChanged(Object &object, bool dueToParentChange) const;
	virtual void extentChanged(Object &object) const;
	virtual void pobFixupComplete(Object &object) const;

private:

	int positionChanged(Object &object) const;
	int positionAndRotationChanged(Object &object) const;

	int addToWorld(const Object &object) const;
	int removeFromWorld(const Object &object) const;

	int positionChanged(const Object &object, const Vector &oldPosition) const;
	int rotationChanged(const Object &object) const;
	int positionAndRotationChanged(const Object &object, const Vector &oldPosition) const;

	int cellChanged(const Object &object) const;

protected:

	ObjectNotification();
	virtual ~ObjectNotification();

private:

	ObjectNotification(const ObjectNotification &);
	ObjectNotification &operator =(const ObjectNotification &);
};

// ======================================================================

#endif
