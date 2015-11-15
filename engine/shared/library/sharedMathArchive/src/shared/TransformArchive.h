// TransformArchive.h
// copyright 2001 Verant Interactive
// Author: Justin Randall

#ifndef	_INCLUDED_TransformArchive_H
#define	_INCLUDED_TransformArchive_H

//-----------------------------------------------------------------------

#include "Archive/Archive.h"
#include "Archive/ByteStream.h"
#include "sharedMathArchive/QuaternionArchive.h"
#include "sharedMath/Transform.h"
#include "sharedMathArchive/VectorArchive.h"

//-----------------------------------------------------------------------
namespace Archive
{

//-----------------------------------------------------------------------
/**
	@brief Get a Transform  from a ByteStream object

	Reads a Quaternion and a Vector from the source
	ByteStream object at it's current read position, then
	sets the Transform data using the Quaternion and Vector
	values.

	@param source   The ByteStream object containing the Quaternion
		            and Vector data at it's current read position.
	@param target   A Transform object that will receive values from
		            the Quaternion and Vector.

	@see Archive
	@see Archive::ByteStream
	@see Transform
	@see Quaternion
	@see Vector
	@see Archive::get(const ByteStream & source, Quaternion & target)
	@see Archive::get(const ByteStream & source, Vector & target)

	@author Justin Randall
*/
inline void get(ReadIterator & source, Transform & target)
{
	Quaternion q;
	Vector v;
	Archive::get(source, q);
	Archive::get(source, v);
	q.getTransform(&target);
#ifdef _DEBUG
	target.validate();
#endif
	target.setPosition_p(v);
}

//-----------------------------------------------------------------------
/**
	@brief Put a Transform into a ByteStream object

	Extracts a Quaternion and Vector from the supplied Transform, then
	puts the Quaternion followed by the Vector into the ByteStream
	at its current write position.

	@param target    The ByteStream that will receive the Quaternion
		             and Vector derived from the Transform. The 
		             Quaternion and Vector data is committed at the
		             ByteStream obect's current write position.
	@param source    A transform that will be used to extract 
		             Quaternion and Vector data, which is then
		             put into the target ByteStream object.

	@see Archive
	@see Archive::ByteStream
	@see Transform
	@see Quaternion 
	@see Vector
	@see Archive::put(ByteStream & target, const Vector & source)
	@see Archive::put(ByteStream & target, const Quaternion & source)

	@author Justin Randall
*/
inline void put(ByteStream & target, const Transform & source)
{
#ifdef _DEBUG
	source.validate();
#endif

	Quaternion q(source);

	Vector v = source.getPosition_p();
	Archive::put(target, q);
	Archive::put(target, v);
}

//-----------------------------------------------------------------------

}//namespace Archive

//-----------------------------------------------------------------------

#endif	// _INCLUDED_TransformArchive_H
