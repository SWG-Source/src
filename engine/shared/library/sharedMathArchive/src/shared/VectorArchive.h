// VectorArchive.h
// copyright 2001 Verant Interactive
// Author: Justin Randall

#ifndef	_INCLUDED_VectorArchive_H
#define	_INCLUDED_VectorArchive_H

//-----------------------------------------------------------------------

#include "Archive/Archive.h"
#include "Archive/ByteStream.h"
#include "sharedMath/Vector.h"

//-----------------------------------------------------------------------

namespace Archive
{
void get(const ByteStream & source, Vector & target);
void put(ByteStream & target, const Vector & source);

//-----------------------------------------------------------------------
/**
	@brief get a Vector from an Archive::ByteStream object

	This get's the vector data as 3 floats: x, y, and z

	@param source    The Archive::ByteStream object containing the Vector
	                 data at it's current read position.
	@param target    A Vector object that will have it's values set from
	                 the Archive::ByteStream data

	@see Archive
	@see Archive::ByteStream
	@see Vector

	@author Justin Randall
*/
inline void get(ReadIterator & source, Vector & target)
{
	Archive::get(source, target.x);
	Archive::get(source, target.y);
	Archive::get(source, target.z);
}

//-----------------------------------------------------------------------
/**
	@brief put a Vector into an Archive::ByteStream object

	This puts the Vector object's data into the Archive::ByteStream
	at it's current write position as x, y and z respectively.

	@param target    The Archive::ByteStream object receiving the
	                 Vector object's values at its current write
	                 position.
	@param source    A Vector object containing the data to be written
	                 to the Archive::ByteStream

	@see Archive
	@see Archive::ByteStream
	@see Vector

	@author Justin Randall
*/
inline void put(ByteStream & target, const Vector & source)
{
	Archive::put(target, source.x);
	Archive::put(target, source.y);
	Archive::put(target, source.z);
}

//-----------------------------------------------------------------------

/**
	@brief get a Vector from an Archive::ByteStream object

	This get's the vector data as 3 floats: x, y, and z

	@param source    The Archive::ByteStream object containing the Vector
	                 data at it's current read position.
	@param target    A Vector object that will have it's values set from
	                 the Archive::ByteStream data

	@see Archive
	@see Archive::ByteStream
	@see Vector
*/
inline void get(ReadIterator & source, Vector * target)
{
	Archive::get(source, target->x);
	Archive::get(source, target->y);
	Archive::get(source, target->z);
}

//-----------------------------------------------------------------------

/**
	@brief put a Vector into an Archive::ByteStream object

	This puts the Vector object's data into the Archive::ByteStream
	at it's current write position as x, y and z respectively.

	@param target    The Archive::ByteStream object receiving the
	                 Vector object's values at its current write
	                 position.
	@param source    A Vector object containing the data to be written
	                 to the Archive::ByteStream

	@see Archive
	@see Archive::ByteStream
	@see Vector
*/
inline void put(ByteStream & target, const Vector * source)
{
	Archive::put(target, source->x);
	Archive::put(target, source->y);
	Archive::put(target, source->z);
}

//-----------------------------------------------------------------------

}//namespace Archive

#endif	// _INCLUDED_VectorArchive_H
