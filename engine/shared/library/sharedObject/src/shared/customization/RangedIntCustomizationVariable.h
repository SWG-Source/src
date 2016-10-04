// ======================================================================
//
// RangedIntCustomizationVariable.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_RangedIntCustomizationVariable_H
#define INCLUDED_RangedIntCustomizationVariable_H

// ======================================================================

#include "sharedObject/CustomizationVariable.h"

// ======================================================================

class RangedIntCustomizationVariable: public CustomizationVariable
{
public:

	virtual bool         doesVariablePersist() const;
	virtual int          getPersistedDataByteCount() const;

	virtual void         saveToByteVector(ByteVector &data) const;
	virtual bool         restoreFromByteVector(ByteVector const &data, int startIndex, int length);
	
	virtual int          getValue() const = 0;
	virtual bool         setValue(int value);

	virtual void         getRange(int &minRangeInclusive, int &maxRangeExclusive) const = 0;

	float                getNormalizedFloatValue() const;
	void                 setNormalizedFloatValue(float value);

	int                  getRangeSize() const;
	
	void setDependentVariable(const std::string &variablePathName);
	void setIsDependentVariable(bool isDependentVariable);
	bool getIsDependentVariable() const;


protected:

	RangedIntCustomizationVariable();

private:

	// disabled
	RangedIntCustomizationVariable(const RangedIntCustomizationVariable&);
	RangedIntCustomizationVariable &operator =(const RangedIntCustomizationVariable&);

	RangedIntCustomizationVariable		*m_dependentVariable;
	bool                               m_isDependentVariable;
};

// ======================================================================

#endif
