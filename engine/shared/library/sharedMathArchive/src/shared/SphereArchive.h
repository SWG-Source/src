// SphereArchive.h
// copyright 2004 Sony Online Entertainment

#ifndef	_INCLUDED_SphereArchive_H
#define	_INCLUDED_SphereArchive_H

//-----------------------------------------------------------------------

#include "Archive/Archive.h"
#include "Archive/ByteStream.h"
#include "sharedMath/Sphere.h"
#include "sharedMathArchive/VectorArchive.h"

//-----------------------------------------------------------------------
namespace Archive
{

inline void get(ReadIterator & source, Sphere & target)
{
	Vector v;
	float f;
	Archive::get(source, v);
	Archive::get(source, f);
	target.setCenter(v);
	target.setRadius(f);
}

//-----------------------------------------------------------------------

inline void put(ByteStream & target, const Sphere & source)
{
	Archive::put(target, source.getCenter());
	Archive::put(target, static_cast<float>(source.getRadius()));
}

}//namespace Archive

//-----------------------------------------------------------------------

#endif	// _INCLUDED_SphereArchive_H
