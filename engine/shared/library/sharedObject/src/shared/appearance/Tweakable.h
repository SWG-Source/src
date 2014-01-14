// ======================================================================
//
// Tweakable.h
// Portions copyright 1999 Bootprint Entertainment
// Portions copyright 2001, 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_Tweakable_H
#define INCLUDED_Tweakable_H

// ======================================================================
// Simple interface for setting/getting tweak values on objects.

class Tweakable
{
public:

	Tweakable();
	virtual ~Tweakable();

	virtual float		getTweakValue ( int whichValue ) const;
	virtual void		setTweakValue ( int whichValue, float newValue );

private:

	Tweakable(const Tweakable &);
	Tweakable &operator =(const Tweakable &);
};

// ======================================================================

inline Tweakable::Tweakable()
{
}

// ======================================================================

#endif
