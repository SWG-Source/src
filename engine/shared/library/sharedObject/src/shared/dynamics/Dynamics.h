// ======================================================================
//
// Dynamics.h
// copyright 1999, bootprint entertainment
// copyright 2002, sony online entertainment
//
// derived classes from dynamics should be using dynamics templates
//
// ======================================================================

#ifndef INCLUDED_Dynamics_H
#define INCLUDED_Dynamics_H

// ======================================================================

class Object;

// ======================================================================
// Abstract dynamics entity for an object

class Dynamics
{
public:

	explicit Dynamics (Object* newOwner);
	virtual ~Dynamics () = 0;

	virtual float alter(float time);

	void          setOwner (Object* newOwner);
	const Object* getOwner () const;
	Object*       getOwner ();

private:

	// disable these
	Dynamics ();
	Dynamics (const Dynamics&);
	Dynamics& operator= (const Dynamics&);
	
private:

	Object* m_owner;
};

// ======================================================================

inline void Dynamics::setOwner (Object* const owner)
{
	m_owner = owner;
}

// ----------------------------------------------------------------------

inline const Object* Dynamics::getOwner () const
{
	return m_owner;
}

// ----------------------------------------------------------------------

inline Object* Dynamics::getOwner ()
{
	return m_owner;
}

// ======================================================================

#endif
