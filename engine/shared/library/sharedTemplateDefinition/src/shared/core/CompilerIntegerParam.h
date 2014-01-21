//========================================================================
//
// CompilerIntegerParam.h - Adds additional functionality to the IntegerParam 
// class for the template compiler/editor.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef _INCLUDED_CompilerIntegerParam_H
#define _INCLUDED_CompilerIntegerParam_H

#include "sharedUtility/TemplateParameter.h"

#include <string>
#include <vector>


class CompilerIntegerParam : public IntegerParam
{
public:
	         CompilerIntegerParam(void);
	virtual ~CompilerIntegerParam();

	void addSourceEnumList(std::vector<std::string> & enumList);
	const std::vector<std::string> & getSourceEnumList(void);

private:
	std::vector<std::string>     m_enumList;    // list of enum names used to 
	                                            // create this param
};


inline CompilerIntegerParam::CompilerIntegerParam(void) : IntegerParam()
{
}

inline void CompilerIntegerParam::addSourceEnumList(std::vector<std::string> & enumList)
{
	std::swap(m_enumList, enumList);
}

inline const std::vector<std::string> & CompilerIntegerParam::getSourceEnumList(void)
{
	return m_enumList;
}



#endif	// _INCLUDED_CompilerIntegerParam_H
