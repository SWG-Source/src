//========================================================================
//
// CompilerDynamicVariableParamData.h - Adds additional functionality to the IntegerParam 
// class for the template compiler/editor.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef _INCLUDED_CompilerDynamicVariableParamData_H
#define _INCLUDED_CompilerDynamicVariableParamData_H

#include "sharedUtility/TemplateParameter.h"


class CompilerDynamicVariableParamData : public DynamicVariableParamData
{
public:
	         CompilerDynamicVariableParamData(const std::string &name, DynamicVariableParamData::DataType type);
	virtual ~CompilerDynamicVariableParamData();

private:
	CompilerDynamicVariableParamData(const CompilerDynamicVariableParamData &);
	CompilerDynamicVariableParamData & operator =(const CompilerDynamicVariableParamData &);
};


#endif	// _INCLUDED_CompilerDynamicVariableParamData_H
