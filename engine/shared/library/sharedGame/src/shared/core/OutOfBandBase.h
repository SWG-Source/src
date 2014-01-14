// OutOfBandBase.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_OutOfBandBase_H
#define	_INCLUDED_OutOfBandBase_H

//-----------------------------------------------------------------------

class OutOfBandBase
{
public:
	virtual ~OutOfBandBase() = 0;

	const int            getPosition  () const;
	const unsigned char  getTypeId    () const;

protected:
	OutOfBandBase(const char typeId, const int position);

private:
	OutOfBandBase & operator = (const OutOfBandBase & rhs);
	OutOfBandBase(const OutOfBandBase & source);

	int            position;
	unsigned char  typeId;
};

//-----------------------------------------------------------------------

#endif	// _INCLUDED_OutOfBandBase_H
