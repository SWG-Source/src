// ======================================================================
//
// ObjectList.h
// Portions copyright 1998 Bootprint Entertainment
// Portions copyright 2001, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_ObjectList_H
#define INCLUDED_ObjectList_H

// ======================================================================

class Object;
class Camera;

// ======================================================================
// Contain a list of pointers to objects.
//
// Objects may exist in more than one ObjectList simultaneously.

class ObjectList
{
public:

	explicit ObjectList(int initialVectorSize=0);
	~ObjectList();

	void addObject(Object *object);
	void removeObject(const Object *object);

	void removeAll(bool deleteObjects=false);

	bool isEmpty() const;
	int  getNumberOfObjects() const;

	Object       *getObject(int index);
	const Object *getObject(int index) const;

	Object       *operator [](int index);
	const Object *operator [](int index) const;

	bool find(const Object* objectToCheck, int* index=0) const;
	void removeObjectByIndex(const Object*, int index);
	
	void addToWorld();
	void removeFromWorld();

	void  prepareToAlter();
	float alter(real time);
	void  conclude();

	void setRegionOfInfluenceEnabled(bool enabled) const;
	void render(const Object *omitObject) const;

private:
	
	typedef std::vector<Object *> ObjectVector;
	bool                             m_altering;
	ObjectVector *                   m_objectVector;
	ObjectVector *                   m_alterSafeObjectVector;

private:

	// disable these routines
	ObjectList(const ObjectList &);
	ObjectList &operator =(const ObjectList &);
};

// ======================================================================

#endif
