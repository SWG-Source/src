// ======================================================================
//
// Attributes.cpp
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "swgSharedUtility/FirstSwgSharedUtility.h"
#include "swgSharedUtility/Attributes.h"

#include "StringId.h"
#include "UnicodeUtils.h"
#include "swgSharedUtility/Attributes.def"

//----------------------------------------------------------------------

namespace
{
	const std::string cs_attributeNames[] =
		{
			"health",
			"strength",
			"constitution",
			"action",
			"quickness",
			"stamina",
			"mind",
			"focus",
			"willpower"
		};

	const int cs_attributeNameCount = static_cast<int>(sizeof(cs_attributeNames)/sizeof(cs_attributeNames[0]));

	StringId cs_attributeStringIds     [cs_attributeNameCount];
	StringId cs_attributeDescStringIds [cs_attributeNameCount];

	//----------------------------------------------------------------------

	bool s_installed = false;

	void install ()
	{
		if (s_installed)
			return;

		s_installed = true;

		static const std::string tableName = "att_n";
		static const std::string tableDesc = "att_d";

		for (int i = 0; i < cs_attributeNameCount; ++i)
		{
			cs_attributeStringIds [i]     = StringId (tableName, Unicode::toLower (cs_attributeNames [i]));
			cs_attributeDescStringIds [i] = StringId (tableDesc, Unicode::toLower (cs_attributeNames [i]));
		}
	}
}

// ======================================================================
/** 
 * Retrieve the display name of a state.
 *
 * This function handles out-of-range state values.
 *
 * @param state  the state for which a display name is desired.
 *
 * @return  the display name of a state.
 */

const std::string & Attributes::getAttributeName (int attrib)
{
	DEBUG_FATAL(cs_attributeNameCount != Attributes::NumberOfAttributes, ("Attribute name table in Attributes.cpp needs to be updated."));
	if ((attrib < 0) || (attrib >= cs_attributeNameCount))
	{
		static const std::string empty;
		return empty;
	}
	else
		return cs_attributeNames[attrib];
}

//----------------------------------------------------------------------

const StringId & Attributes::getAttributeStringId (int attribute)
{
	if (!s_installed)
		install ();

	if (attribute < 0 || attribute >= cs_attributeNameCount)
	{
		static StringId nullStringId;
		return nullStringId;
	}
	else
		return cs_attributeStringIds [attribute];
}

//----------------------------------------------------------------------

const StringId &    Attributes::getAttributeDescStringId (int attribute)
{
	if (!s_installed)
		install ();

	if (attribute < 0 || attribute >= cs_attributeNameCount)
	{
		static StringId nullStringId;
		return nullStringId;
	}
	else
		return cs_attributeDescStringIds [attribute];
}


//----------------------------------------------------------------------
