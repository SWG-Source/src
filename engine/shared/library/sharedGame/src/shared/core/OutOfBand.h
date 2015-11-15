// OutOfBand.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_OutOfBand_H
#define	_INCLUDED_OutOfBand_H

//-----------------------------------------------------------------------

#include "sharedGame/OutOfBandBase.h"

//-----------------------------------------------------------------------

template<typename ValueType>
class OutOfBand : public OutOfBandBase
{
public:
	OutOfBand(const char typeId, const int position, ValueType * object);
	~OutOfBand();

	const ValueType *  getObject  () const;
	ValueType *        getObject  ();

private:
	OutOfBand & operator = (const OutOfBand & rhs);
	OutOfBand(const OutOfBand & source);

private:
	ValueType * const  object;
};

//-----------------------------------------------------------------------

template<typename ValueType>
OutOfBand<ValueType>::OutOfBand(const char t, const int i, ValueType * o) :
OutOfBandBase(t, i),
object(o)
{
}

//-----------------------------------------------------------------------

template<typename ValueType>
OutOfBand<ValueType>::~OutOfBand()
{
}

//-----------------------------------------------------------------------

template<typename ValueType>
const ValueType * OutOfBand<ValueType>::getObject() const
{
	return object;
}

//-----------------------------------------------------------------------

template<typename ValueType>
ValueType * OutOfBand<ValueType>::getObject()
{
	return object;
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_OutOfBand_H
