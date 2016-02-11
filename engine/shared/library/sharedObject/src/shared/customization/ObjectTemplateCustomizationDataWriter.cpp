// ======================================================================
//
// ObjectTemplateCustomizationDataWriter.cpp
// Copyright 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedObject/FirstSharedObject.h"
#include "sharedObject/ObjectTemplateCustomizationDataWriter.h"

#include "sharedFoundation/TemporaryCrcString.h"
#include "sharedMath/PaletteArgb.h"
#include "sharedMath/PaletteArgbList.h"

#include <list>
#include <cstdio>
#include <string>

// ======================================================================

ObjectTemplateCustomizationDataWriter::ObjectTemplateCustomizationDataWriter(bool writeSharedOwnerVariables) :
	m_rangedIntData(new StringList()),
	m_paletteColorData(new StringList()),
	m_writeSharedOwnerVariables(writeSharedOwnerVariables)
{
}

// ----------------------------------------------------------------------

ObjectTemplateCustomizationDataWriter::~ObjectTemplateCustomizationDataWriter()
{
	delete m_paletteColorData;
	delete m_rangedIntData;
}

// ----------------------------------------------------------------------

void ObjectTemplateCustomizationDataWriter::addRangedIntCustomizationVariable(const std::string &variableName, int minValueInclusive, int defaultValue, int maxValueExclusive)
{
	//-- Check for skipping shared_owner variables.
	if (!m_writeSharedOwnerVariables)
	{
		bool const isSharedOwnerVariable = (static_cast<int>(variableName.find("shared_owner")) != static_cast<int>(std::string::npos));
		if (isSharedOwnerVariable)
		{
			// Skip it.
			return;
		}
	}

	//-- Check for special case variable for breasts.
	if (static_cast<int>(variableName.find("blend_flat_chest")) != static_cast<int>(std::string::npos))
	{
		// Found this one, adjust minimum value.
		minValueInclusive = -155;
		maxValueExclusive = 156;
	}

	//-- Write it out.
	char  conversionBuffer[64];

	std::string  buildString = "[variableName=\"";
	buildString += variableName;

	buildString += "\", minValueInclusive=";
	sprintf(conversionBuffer, "%d", minValueInclusive);
	buildString += conversionBuffer;

	buildString += ", defaultValue=";
	sprintf(conversionBuffer, "%d", defaultValue);
	buildString += conversionBuffer;

	buildString += ", maxValueExclusive=";
	sprintf(conversionBuffer, "%d", maxValueExclusive);
	buildString += conversionBuffer;

	buildString += "],";

	m_rangedIntData->push_back(buildString);
}

// ----------------------------------------------------------------------

void ObjectTemplateCustomizationDataWriter::addPaletteColorCustomizationVariable(const std::string &variableName, const std::string &paletteReferenceName, int defaultPaletteIndex)
{
	//-- Check for skipping shared_owner variables.
	if (!m_writeSharedOwnerVariables)
	{
		bool const isSharedOwnerVariable = (static_cast<int>(variableName.find("shared_owner")) != static_cast<int>(std::string::npos));
		if (isSharedOwnerVariable)
		{
			// Skip it.
			return;
		}
	}

	//-- Fetch the palette, get the maxValueExclusive for it.
	PaletteArgb const *const palette = PaletteArgbList::fetch(TemporaryCrcString(paletteReferenceName.c_str(), true));
	if (!palette)
	{
		FATAL(true, ("variable [%s] references non-existent palette [%s], variable will not be reported.", variableName.c_str(), paletteReferenceName.c_str()));
		return; //lint !e527 // unreachable // Right, this is for MSDEV.
	}

	int const paletteMaxValueExclusive = palette->getEntryCount();
	palette->release();

	//-- Build export string.
	char  conversionBuffer[64];

	std::string  buildString = "[variableName=\"";
	buildString += variableName;

	buildString += "\", palettePathName=\"";
	buildString += paletteReferenceName;

	buildString += "\", defaultPaletteIndex=";
	sprintf(conversionBuffer, "%d", defaultPaletteIndex);
	buildString += conversionBuffer;
	buildString += "], // maxValueExclusive=";

	sprintf(conversionBuffer, "%d", paletteMaxValueExclusive);
	buildString += conversionBuffer;

	m_paletteColorData->push_back(buildString);
}

// ----------------------------------------------------------------------

bool ObjectTemplateCustomizationDataWriter::writeToFile(const std::string &pathName, bool allowOverwrite)
{
	//-- Don't write anything if there's no data to write.
	bool hasData = false;

	if (!m_paletteColorData->empty())
		hasData = true;

	if (!m_rangedIntData->empty())
		hasData = true;

	if (!hasData)
	{
		DEBUG_REPORT_LOG(true, ("writeToFile(): no data: skipped writing [%s].\n", pathName.c_str()));
		return true;
	}

	//-- Handle no-overwrite case.
	if (!allowOverwrite)
	{
		FILE *const testFile = fopen(pathName.c_str(), "r");
		if (testFile != nullptr)
		{
			fclose(testFile);
			DEBUG_REPORT_LOG(true, ("writeToFile(): overwrite attempt: skipped writing [%s] because it already exists and allowOverwrite == false.\n", pathName.c_str()));
			return false;
		}
	}

	//-- open file
	FILE *const file = fopen(pathName.c_str(), "w");
	if (!file)
	{
		WARNING(true, ("failed to create file [%s].", pathName.c_str()));
		return false;
	}

	//-- sort the output data
	m_paletteColorData->sort();
	m_rangedIntData->sort();

	//-- write palette color variables
	fprintf(file, "paletteColorCustomizationVariables = [");
	{
		int i = 0;
		int const count = static_cast<int>(m_paletteColorData->size());

		const StringList::iterator endIt = m_paletteColorData->end();
		for (StringList::iterator it = m_paletteColorData->begin(); it != endIt; ++it, ++i)
		{
			if (i == (count - 1))
			{
				// -- Fixup last entry to use ] in place of final comma.
				std::string::size_type position = it->rfind(',');
				DEBUG_FATAL(static_cast<int>(position) == static_cast<int>(std::string::npos), ("bad data, where's the final comma?"));

				(*it)[position] = ']';
			}

			fprintf(file, "\n  %s", it->c_str());
		}
	}
	if (m_paletteColorData->empty())
		fprintf(file, "]");
	fprintf(file, "\n\n");

	//-- write ranged int variables
	fprintf(file, "rangedIntCustomizationVariables = [");
	{
		int i = 0;
		int const count = static_cast<int>(m_rangedIntData->size());

		const StringList::iterator endIt = m_rangedIntData->end();
		for (StringList::iterator it = m_rangedIntData->begin(); it != endIt; ++it, ++i)
		{
			if (i == (count - 1))
			{
				// -- Fixup last entry to use ] in place of final comma.
				std::string::size_type position = it->rfind(',');
				DEBUG_FATAL(static_cast<int>(position) == static_cast<int>(std::string::npos), ("bad data, where's the final comma?"));

				(*it)[position] = ']';
			}

			fprintf(file, "\n  %s", it->c_str());
		}
	}
	if (m_rangedIntData->empty())
		fprintf(file, "]");
	fprintf(file, "\n\n");

	fclose(file);

	//-- return success
	return true;
}

// ======================================================================
