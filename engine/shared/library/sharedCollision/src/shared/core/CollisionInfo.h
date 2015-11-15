//===================================================================
//
// CollisionInfo.h
// asommers 2-10-99
//
// copyright 1999, bootprint entertainment
// copyright 2002, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_CollisionInfo_H
#define INCLUDED_CollisionInfo_H

//===================================================================

#include "sharedMath/Vector.h"

class Object;

//===================================================================

class CollisionInfo
{
public:

	CollisionInfo ();
	~CollisionInfo ();

	const Object* getObject () const;
	void          setObject (const Object* object);
	const Vector& getPoint () const;
	void          setPoint (const Vector& point);
	const Vector& getNormal () const;
	void          setNormal (const Vector& normal);
	float         getTime () const;
	void          setTime (float time);

private:

	const Object* m_object;
	Vector        m_point;
	Vector        m_normal;
	float         m_time;
};

//===================================================================

inline CollisionInfo::CollisionInfo () : 
	m_object (0),
	m_point (),
	m_normal (),
	m_time (0.f)
{
}

//-------------------------------------------------------------------

inline CollisionInfo::~CollisionInfo ()
{
	m_object = 0;
}

//-------------------------------------------------------------------

inline const Object* CollisionInfo::getObject () const
{
	return m_object;
}

//-------------------------------------------------------------------

inline void CollisionInfo::setObject (const Object* const object)
{
	m_object = object;
}

//-------------------------------------------------------------------

inline const Vector& CollisionInfo::getPoint () const
{
	return m_point;
}

//-------------------------------------------------------------------

inline void CollisionInfo::setPoint (const Vector& point)
{
	m_point = point;
}

//-------------------------------------------------------------------

inline const Vector& CollisionInfo::getNormal () const
{
	return m_normal;
}

//-------------------------------------------------------------------

inline void CollisionInfo::setNormal (const Vector& normal)
{
	m_normal = normal;
}

//-------------------------------------------------------------------

inline float CollisionInfo::getTime () const
{
	return m_time;
}

//-------------------------------------------------------------------

inline void CollisionInfo::setTime (const float time)
{
	m_time = time;
}

// ======================================================================

#endif
