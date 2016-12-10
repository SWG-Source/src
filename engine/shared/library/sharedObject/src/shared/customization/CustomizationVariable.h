// ======================================================================
//
// CustomizationVariable.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_CustomizationVariable_H
#define INCLUDED_CustomizationVariable_H

// ======================================================================

class CustomizationData;
class CustomizationVariable;
class ObjectTemplateCustomizationDataWriter;

// ======================================================================

class CustomizationVariable
{
friend class CustomizationData;

public:

	typedef std::vector<byte>  ByteVector;
	
public:

	virtual                 ~CustomizationVariable();

	virtual bool             doesVariablePersist() const = 0;
	virtual int              getPersistedDataByteCount() const = 0;

	/**
	 * Append state of this variable to data buffer for subsequent restoration.
	 *
	 * @param data  a vector of bytes where the state of this variable will be stored.
	 */
	virtual void  saveToByteVector(ByteVector &data) const = 0;

	/**
	 * Restore the state of the variable using the data and offset provided.
	 *
	 * @param data        a vector of bytes where the state of this variable was previously stored.
	 * @param startIndex  a 0-based index into the data buffer indicating where this variable's state data begins.
	 * @param length      length of the data recorded for this variable.
	 *
	 * @return            true if data restoration occurred successfully; false if some error occured.
	 */
	virtual bool  restoreFromByteVector(ByteVector const &data, int startIndex, int length) = 0;

	const CustomizationData *getOwner() const;
	CustomizationData*		 getOwner();

	virtual void             writeObjectTemplateExportString(const std::string &variablePathName, ObjectTemplateCustomizationDataWriter &writer) const = 0;

#ifdef _DEBUG
	virtual std::string      debugToString() const = 0;
#endif

	virtual void             alter(CustomizationData &owner);

protected:

	CustomizationVariable();

	void  signalVariableModified() const;

private:

	void setOwner(CustomizationData *owner);

	// disabled
	CustomizationVariable(const CustomizationVariable&);
	CustomizationVariable &operator =(const CustomizationVariable&);

private:

	CustomizationData *m_owner;

};

// ======================================================================

inline const CustomizationData *CustomizationVariable::getOwner() const
{
	return m_owner;
}

// ======================================================================

inline CustomizationData *CustomizationVariable::getOwner()
{
	return m_owner;
}

// ======================================================================

inline void CustomizationVariable::setOwner(CustomizationData *owner)
{
	m_owner = owner;
}

// ======================================================================

#endif
