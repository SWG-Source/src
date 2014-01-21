//========================================================================
//
// CompilerDynamicVariableParamData.cpp - 
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "sharedTemplateDefinition/FirstSharedTemplateDefinition.h"
#include "sharedTemplateDefinition/CompilerDynamicVariableParamData.h"

#include "sharedTemplateDefinition/CompilerIntegerParam.h"


/**
 * Class constructor.
 *
 * @param name		variable name
 * @param type		variable type
 */
CompilerDynamicVariableParamData::CompilerDynamicVariableParamData(
	const std::string &name, DynamicVariableParamData::DataType type) :
	DynamicVariableParamData(name, type)
{
	if (type == DynamicVariableParamData::INTEGER)
	{
		delete m_data.iparam;
		m_data.iparam = new CompilerIntegerParam();
	}
}	// CompilerDynamicVariableParamData::CompilerDynamicVariableParamData

/**
 * Class destructor.
 */
CompilerDynamicVariableParamData::~CompilerDynamicVariableParamData()
{
}	// CompilerDynamicVariableParamData::~CompilerDynamicVariableParamData

