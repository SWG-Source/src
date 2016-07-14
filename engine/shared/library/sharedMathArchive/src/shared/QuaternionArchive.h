// QuaternionArchive.h
// copyright 2001 Verant Interactive
// Author: Justin Randall

#ifndef	_INCLUDED_QuaternionArchive_H
#define	_INCLUDED_QuaternionArchive_H

//-----------------------------------------------------------------------

#include "Archive/Archive.h"
#include "Archive/ByteStream.h"
#include "sharedMath/Quaternion.h"

//-----------------------------------------------------------------------

namespace Archive
{

//-----------------------------------------------------------------------
/**
	@brief get a Quaternion from a source Archive::ByteStream

	Rertrieves a quaternion's x, y, z and w values from the
	Archive::ByteStream object's current read position.

	@param source    The Archive::ByteStream object containing the
		             quaternion x, y, z and w members at the current
		             read position.
	@param target    A Quaternion object that will receive the
		             x, y z and w components from the
		             Archive::ByteStream

	@see Archive
	@see Archive::ByteStream
	@see Vector

	@author Justin Randall
*/
inline void get(ReadIterator & source, Quaternion & target)
{
	Archive::get(source, target.x);
	Archive::get(source, target.y);
	Archive::get(source, target.z);
	Archive::get(source, target.w);

#ifdef _WIN32
	if (_isnan(static_cast<double>(target.w)) || _isnan(static_cast<double>(target.x)) || _isnan(static_cast<double>(target.y)) || _isnan(static_cast<double>(target.z)))
#else
	if (std::isnan(target.w) || std::isnan(target.x) || std::isnan(target.y) || std::isnan(target.z))
#endif
	{
		target.w = 1.0f;
		target.x = 0.0f;
		target.y = 0.0f;
		target.z = 0.0f;
	}
}

//-----------------------------------------------------------------------
/**
	@brief put a Quaternion into an Archive::ByteStream

	Puts the Quaternion into the Archive::ByteStream at its current
	write position.

	@param target   The ByteStream object that will receive the data
		            at its current write position.
	@param source   A Quaternion containing the x, y, z and w
		            components that will be written to the ByteStream

	@see Archive
	@see Archive::ByteStream
	@see Quaternion

	@author Justin Randall
*/
inline void put(ByteStream & target, const Quaternion & source)
{
	Archive::put(target, source.x);
	Archive::put(target, source.y);
	Archive::put(target, source.z);
	Archive::put(target, source.w);
}

//-----------------------------------------------------------------------

}//namespace Archive

//-----------------------------------------------------------------------

#endif	// _INCLUDED_QuaternionArchive_H
