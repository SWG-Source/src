// ======================================================================
//
// ObjectTemplateCustomizationDataWriter.h
// Copyright 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#ifndef INCLUDED_ObjectTemplateCustomizationDataWriter_H
#define INCLUDED_ObjectTemplateCustomizationDataWriter_H

// ======================================================================

// -TRF- replace this with whatever flag is needed to include for internal-only
// builds.  For now it is DEBUG-enabled only.

#ifdef _DEBUG
#define USE_OBJ_TEMPLATE_CUSTOM_VAR_WRITER  1
#else
#define USE_OBJ_TEMPLATE_CUSTOM_VAR_WRITER  0
#endif

// ======================================================================

class ObjectTemplateCustomizationDataWriter
{
public:

	ObjectTemplateCustomizationDataWriter(bool writeSharedOwnerVariables);
	~ObjectTemplateCustomizationDataWriter();

	void  addRangedIntCustomizationVariable(const std::string &variableName, int minValueInclusive, int defaultValue, int maxValueExclusive);
	void  addPaletteColorCustomizationVariable(const std::string &variableName, const std::string &paletteReferenceName, int defaultPaletteIndex);

	bool  writeToFile(const std::string &pathName, bool allowOverwrite);

private:

	typedef std::list<std::string>  StringList;

private:

	// Disabled.
	ObjectTemplateCustomizationDataWriter(const ObjectTemplateCustomizationDataWriter&);
	ObjectTemplateCustomizationDataWriter &operator =(const ObjectTemplateCustomizationDataWriter&);

private:

	StringList *m_rangedIntData;
	StringList *m_paletteColorData;

	bool        m_writeSharedOwnerVariables;

};

// ======================================================================

#endif
