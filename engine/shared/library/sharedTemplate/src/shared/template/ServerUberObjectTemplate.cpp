//========================================================================
//
// ServerUberObjectTemplate.cpp
//
//IMPORTANT: Any code between //@BEGIN TFD... and //@END TFD... will be
//overwritten the next time the template definition is compiled. Do not
//make changes to code inside these blocks.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "sharedTemplate/FirstSharedTemplate.h"
#include "sharedTemplateDefinition/TemplateData.h"
#include "sharedTemplateDefinition/TemplateGlobals.h"
#include "ServerUberObjectTemplate.h"
#include "sharedDebug/DataLint.h"
#include "sharedFile/Iff.h"
#include "sharedTemplateDefinition/ObjectTemplate.h"
#include <stdio.h>



/**
 * Class constructor.
 */
ServerUberObjectTemplate::ServerUberObjectTemplate(const std::string & filename)
//@BEGIN TFD INIT
	: TpfTemplate(filename)
	,m_intListSimpleLoaded(false)
	,m_intListSimpleAppend(false)
	,m_intListWeightedListLoaded(false)
	,m_intListWeightedListAppend(false)
	,m_intListRandomRangeLoaded(false)
	,m_intListRandomRangeAppend(false)
	,m_intListDiceRollLoaded(false)
	,m_intListDiceRollAppend(false)
	,m_floatListSimpleLoaded(false)
	,m_floatListSimpleAppend(false)
	,m_floatListWeightedListLoaded(false)
	,m_floatListWeightedListAppend(false)
	,m_floatListRandomRangeLoaded(false)
	,m_floatListRandomRangeAppend(false)
	,m_enumListIndexedLoaded(false)
	,m_enumListIndexedAppend(false)
	,m_enumListWeightedListLoaded(false)
	,m_enumListWeightedListAppend(false)
	,m_stringIdListSimpleLoaded(false)
	,m_stringIdListSimpleAppend(false)
	,m_stringIdListWeightedListLoaded(false)
	,m_stringIdListWeightedListAppend(false)
	,m_stringListSimpleLoaded(false)
	,m_stringListSimpleAppend(false)
	,m_stringListWeightedListLoaded(false)
	,m_stringListWeightedListAppend(false)
	,m_triggerVolumeListLoaded(false)
	,m_triggerVolumeListAppend(false)
	,m_triggerVolumesListWeightedListLoaded(false)
	,m_triggerVolumesListWeightedListAppend(false)
	,m_boolListDerivedLoaded(false)
	,m_boolListDerivedAppend(false)
	,m_boolListSimpleLoaded(false)
	,m_boolListSimpleAppend(false)
	,m_boolListWeightedListLoaded(false)
	,m_boolListWeightedListAppend(false)
	,m_vectorListSimpleLoaded(false)
	,m_vectorListSimpleAppend(false)
	,m_filenameListSimpleLoaded(false)
	,m_filenameListSimpleAppend(false)
	,m_templateListSimpleLoaded(false)
	,m_templateListSimpleAppend(false)
	,m_structListSimpleLoaded(false)
	,m_structListSimpleAppend(false)
//@END TFD INIT
{
}	// ServerUberObjectTemplate::ServerUberObjectTemplate

/**
 * Class destructor.
 */
ServerUberObjectTemplate::~ServerUberObjectTemplate()
{
//@BEGIN TFD CLEANUP
	{
		std::vector<CompilerIntegerParam *>::iterator iter;
		for (iter = m_intListSimple.begin(); iter != m_intListSimple.end(); ++iter)
		{
			delete *iter;
			*iter = NULL;
		}
	}
	{
		std::vector<CompilerIntegerParam *>::iterator iter;
		for (iter = m_intListWeightedList.begin(); iter != m_intListWeightedList.end(); ++iter)
		{
			delete *iter;
			*iter = NULL;
		}
	}
	{
		std::vector<CompilerIntegerParam *>::iterator iter;
		for (iter = m_intListRandomRange.begin(); iter != m_intListRandomRange.end(); ++iter)
		{
			delete *iter;
			*iter = NULL;
		}
	}
	{
		std::vector<CompilerIntegerParam *>::iterator iter;
		for (iter = m_intListDiceRoll.begin(); iter != m_intListDiceRoll.end(); ++iter)
		{
			delete *iter;
			*iter = NULL;
		}
	}
	{
		std::vector<FloatParam *>::iterator iter;
		for (iter = m_floatListSimple.begin(); iter != m_floatListSimple.end(); ++iter)
		{
			delete *iter;
			*iter = NULL;
		}
	}
	{
		std::vector<FloatParam *>::iterator iter;
		for (iter = m_floatListWeightedList.begin(); iter != m_floatListWeightedList.end(); ++iter)
		{
			delete *iter;
			*iter = NULL;
		}
	}
	{
		std::vector<FloatParam *>::iterator iter;
		for (iter = m_floatListRandomRange.begin(); iter != m_floatListRandomRange.end(); ++iter)
		{
			delete *iter;
			*iter = NULL;
		}
	}
	{
		std::vector<CompilerIntegerParam *>::iterator iter;
		for (iter = m_enumListIndexed.begin(); iter != m_enumListIndexed.end(); ++iter)
		{
			delete *iter;
			*iter = NULL;
		}
	}
	{
		std::vector<CompilerIntegerParam *>::iterator iter;
		for (iter = m_enumListWeightedList.begin(); iter != m_enumListWeightedList.end(); ++iter)
		{
			delete *iter;
			*iter = NULL;
		}
	}
	{
		std::vector<StringIdParam *>::iterator iter;
		for (iter = m_stringIdListSimple.begin(); iter != m_stringIdListSimple.end(); ++iter)
		{
			delete *iter;
			*iter = NULL;
		}
	}
	{
		std::vector<StringIdParam *>::iterator iter;
		for (iter = m_stringIdListWeightedList.begin(); iter != m_stringIdListWeightedList.end(); ++iter)
		{
			delete *iter;
			*iter = NULL;
		}
	}
	{
		std::vector<StringParam *>::iterator iter;
		for (iter = m_stringListSimple.begin(); iter != m_stringListSimple.end(); ++iter)
		{
			delete *iter;
			*iter = NULL;
		}
	}
	{
		std::vector<StringParam *>::iterator iter;
		for (iter = m_stringListWeightedList.begin(); iter != m_stringListWeightedList.end(); ++iter)
		{
			delete *iter;
			*iter = NULL;
		}
	}
	{
		std::vector<TriggerVolumeParam *>::iterator iter;
		for (iter = m_triggerVolumeList.begin(); iter != m_triggerVolumeList.end(); ++iter)
		{
			delete *iter;
			*iter = NULL;
		}
	}
	{
		std::vector<TriggerVolumeParam *>::iterator iter;
		for (iter = m_triggerVolumesListWeightedList.begin(); iter != m_triggerVolumesListWeightedList.end(); ++iter)
		{
			delete *iter;
			*iter = NULL;
		}
	}
	{
		std::vector<BoolParam *>::iterator iter;
		for (iter = m_boolListDerived.begin(); iter != m_boolListDerived.end(); ++iter)
		{
			delete *iter;
			*iter = NULL;
		}
	}
	{
		std::vector<BoolParam *>::iterator iter;
		for (iter = m_boolListSimple.begin(); iter != m_boolListSimple.end(); ++iter)
		{
			delete *iter;
			*iter = NULL;
		}
	}
	{
		std::vector<BoolParam *>::iterator iter;
		for (iter = m_boolListWeightedList.begin(); iter != m_boolListWeightedList.end(); ++iter)
		{
			delete *iter;
			*iter = NULL;
		}
	}
	{
		std::vector<VectorParam *>::iterator iter;
		for (iter = m_vectorListSimple.begin(); iter != m_vectorListSimple.end(); ++iter)
		{
			delete *iter;
			*iter = NULL;
		}
	}
	{
		std::vector<StringParam *>::iterator iter;
		for (iter = m_filenameListSimple.begin(); iter != m_filenameListSimple.end(); ++iter)
		{
			delete *iter;
			*iter = NULL;
		}
	}
	{
		std::vector<StringParam *>::iterator iter;
		for (iter = m_templateListSimple.begin(); iter != m_templateListSimple.end(); ++iter)
		{
			delete *iter;
			*iter = NULL;
		}
	}
	{
		std::vector<StructParamOT *>::iterator iter;
		for (iter = m_structListSimple.begin(); iter != m_structListSimple.end(); ++iter)
		{
			delete *iter;
			*iter = NULL;
		}
	}
//@END TFD CLEANUP
}	// ServerUberObjectTemplate::~ServerUberObjectTemplate

/**
 * Static function used to register this template.
 */
void ServerUberObjectTemplate::registerMe(void)
{
	ObjectTemplateList::registerTemplate(ServerUberObjectTemplate_tag, create);
}	// ServerUberObjectTemplate::registerMe

/**
 * Creates a ServerUberObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * ServerUberObjectTemplate::create(const std::string & filename)
{
	return new ServerUberObjectTemplate(filename);
}	// ServerUberObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag ServerUberObjectTemplate::getId(void) const
{
	return ServerUberObjectTemplate_tag;
}	// ServerUberObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag ServerUberObjectTemplate::getTemplateVersion(void) const
{
	return m_templateVersion;
} // ServerUberObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag ServerUberObjectTemplate::getHighestTemplateVersion(void) const
{
	if (m_baseData == NULL)
		return m_templateVersion;
	const ServerUberObjectTemplate * base = dynamic_cast<const ServerUberObjectTemplate *>(m_baseData);
	if (base == NULL)
		return m_templateVersion;
	return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // ServerUberObjectTemplate::getHighestTemplateVersion

//@BEGIN TFD
CompilerIntegerParam * ServerUberObjectTemplate::getCompilerIntegerParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "intAtDerived") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != NULL)
					return getBaseTemplate()->getCompilerIntegerParam(name, deepCheck, index);
				return NULL;
			}
			return &m_intAtDerived;
		}
		fprintf(stderr, "trying to access single-parameter \"intAtDerived\" as an array\n");
	}
	else if (strcmp(name, "intSimple") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != NULL)
					return getBaseTemplate()->getCompilerIntegerParam(name, deepCheck, index);
				return NULL;
			}
			return &m_intSimple;
		}
		fprintf(stderr, "trying to access single-parameter \"intSimple\" as an array\n");
	}
	else if (strcmp(name, "intSimpleDeltaPositive") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != NULL)
					return getBaseTemplate()->getCompilerIntegerParam(name, deepCheck, index);
				return NULL;
			}
			return &m_intSimpleDeltaPositive;
		}
		fprintf(stderr, "trying to access single-parameter \"intSimpleDeltaPositive\" as an array\n");
	}
	else if (strcmp(name, "intSimpleDeltaNegative") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != NULL)
					return getBaseTemplate()->getCompilerIntegerParam(name, deepCheck, index);
				return NULL;
			}
			return &m_intSimpleDeltaNegative;
		}
		fprintf(stderr, "trying to access single-parameter \"intSimpleDeltaNegative\" as an array\n");
	}
	else if (strcmp(name, "intSimpleDeltaPositivePercent") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != NULL)
					return getBaseTemplate()->getCompilerIntegerParam(name, deepCheck, index);
				return NULL;
			}
			return &m_intSimpleDeltaPositivePercent;
		}
		fprintf(stderr, "trying to access single-parameter \"intSimpleDeltaPositivePercent\" as an array\n");
	}
	else if (strcmp(name, "intSimpleDeltaNegativePercent") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != NULL)
					return getBaseTemplate()->getCompilerIntegerParam(name, deepCheck, index);
				return NULL;
			}
			return &m_intSimpleDeltaNegativePercent;
		}
		fprintf(stderr, "trying to access single-parameter \"intSimpleDeltaNegativePercent\" as an array\n");
	}
	else if (strcmp(name, "intWeightedList") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != NULL)
					return getBaseTemplate()->getCompilerIntegerParam(name, deepCheck, index);
				return NULL;
			}
			return &m_intWeightedList;
		}
		fprintf(stderr, "trying to access single-parameter \"intWeightedList\" as an array\n");
	}
	else if (strcmp(name, "intWeightedListDeltaPositive") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != NULL)
					return getBaseTemplate()->getCompilerIntegerParam(name, deepCheck, index);
				return NULL;
			}
			return &m_intWeightedListDeltaPositive;
		}
		fprintf(stderr, "trying to access single-parameter \"intWeightedListDeltaPositive\" as an array\n");
	}
	else if (strcmp(name, "intWeightedListDeltaNegative") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != NULL)
					return getBaseTemplate()->getCompilerIntegerParam(name, deepCheck, index);
				return NULL;
			}
			return &m_intWeightedListDeltaNegative;
		}
		fprintf(stderr, "trying to access single-parameter \"intWeightedListDeltaNegative\" as an array\n");
	}
	else if (strcmp(name, "intWeightedListDeltaPositivePercent") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != NULL)
					return getBaseTemplate()->getCompilerIntegerParam(name, deepCheck, index);
				return NULL;
			}
			return &m_intWeightedListDeltaPositivePercent;
		}
		fprintf(stderr, "trying to access single-parameter \"intWeightedListDeltaPositivePercent\" as an array\n");
	}
	else if (strcmp(name, "intWeightedListDeltaNegativePercent") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != NULL)
					return getBaseTemplate()->getCompilerIntegerParam(name, deepCheck, index);
				return NULL;
			}
			return &m_intWeightedListDeltaNegativePercent;
		}
		fprintf(stderr, "trying to access single-parameter \"intWeightedListDeltaNegativePercent\" as an array\n");
	}
	else if (strcmp(name, "intRandomRange1") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != NULL)
					return getBaseTemplate()->getCompilerIntegerParam(name, deepCheck, index);
				return NULL;
			}
			return &m_intRandomRange1;
		}
		fprintf(stderr, "trying to access single-parameter \"intRandomRange1\" as an array\n");
	}
	else if (strcmp(name, "intRandomRange2") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != NULL)
					return getBaseTemplate()->getCompilerIntegerParam(name, deepCheck, index);
				return NULL;
			}
			return &m_intRandomRange2;
		}
		fprintf(stderr, "trying to access single-parameter \"intRandomRange2\" as an array\n");
	}
	else if (strcmp(name, "intRandomRange3") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != NULL)
					return getBaseTemplate()->getCompilerIntegerParam(name, deepCheck, index);
				return NULL;
			}
			return &m_intRandomRange3;
		}
		fprintf(stderr, "trying to access single-parameter \"intRandomRange3\" as an array\n");
	}
	else if (strcmp(name, "intRandomRange4") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != NULL)
					return getBaseTemplate()->getCompilerIntegerParam(name, deepCheck, index);
				return NULL;
			}
			return &m_intRandomRange4;
		}
		fprintf(stderr, "trying to access single-parameter \"intRandomRange4\" as an array\n");
	}
	else if (strcmp(name, "intDiceRoll1") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != NULL)
					return getBaseTemplate()->getCompilerIntegerParam(name, deepCheck, index);
				return NULL;
			}
			return &m_intDiceRoll1;
		}
		fprintf(stderr, "trying to access single-parameter \"intDiceRoll1\" as an array\n");
	}
	else if (strcmp(name, "intDiceRoll2") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != NULL)
					return getBaseTemplate()->getCompilerIntegerParam(name, deepCheck, index);
				return NULL;
			}
			return &m_intDiceRoll2;
		}
		fprintf(stderr, "trying to access single-parameter \"intDiceRoll2\" as an array\n");
	}
	else if (strcmp(name, "intListSimple") == 0)
	{
		if (index >= 0 && index < static_cast<int>(m_intListSimple.size()))
			return m_intListSimple[index];
		if (index == static_cast<int>(m_intListSimple.size()))
		{
			CompilerIntegerParam *temp = new CompilerIntegerParam();
			m_intListSimple.push_back(temp);
			return temp;
		}
		fprintf(stderr, "index for parameter \"intListSimple\" out of bounds\n");
	}
	else if (strcmp(name, "intListWeightedList") == 0)
	{
		if (index >= 0 && index < static_cast<int>(m_intListWeightedList.size()))
			return m_intListWeightedList[index];
		if (index == static_cast<int>(m_intListWeightedList.size()))
		{
			CompilerIntegerParam *temp = new CompilerIntegerParam();
			m_intListWeightedList.push_back(temp);
			return temp;
		}
		fprintf(stderr, "index for parameter \"intListWeightedList\" out of bounds\n");
	}
	else if (strcmp(name, "intListRandomRange") == 0)
	{
		if (index >= 0 && index < static_cast<int>(m_intListRandomRange.size()))
			return m_intListRandomRange[index];
		if (index == static_cast<int>(m_intListRandomRange.size()))
		{
			CompilerIntegerParam *temp = new CompilerIntegerParam();
			m_intListRandomRange.push_back(temp);
			return temp;
		}
		fprintf(stderr, "index for parameter \"intListRandomRange\" out of bounds\n");
	}
	else if (strcmp(name, "intListDiceRoll") == 0)
	{
		if (index >= 0 && index < static_cast<int>(m_intListDiceRoll.size()))
			return m_intListDiceRoll[index];
		if (index == static_cast<int>(m_intListDiceRoll.size()))
		{
			CompilerIntegerParam *temp = new CompilerIntegerParam();
			m_intListDiceRoll.push_back(temp);
			return temp;
		}
		fprintf(stderr, "index for parameter \"intListDiceRoll\" out of bounds\n");
	}
	else if (strcmp(name, "enumIndexedByEnumSingle") == 0)
	{
		if (index >= 0 && index < 3)
		{
			if (deepCheck && !isParamLoaded(name, false, index))
			{
				if (getBaseTemplate() != NULL)
					return getBaseTemplate()->getCompilerIntegerParam(name, deepCheck, index);
				return NULL;
			}
			return &m_enumIndexedByEnumSingle[index];
		}
		fprintf(stderr, "index for parameter \"enumIndexedByEnumSingle\" out of bounds\n");
	}
	else if (strcmp(name, "enumIndexedByEnumWeightedList") == 0)
	{
		if (index >= 0 && index < 3)
		{
			if (deepCheck && !isParamLoaded(name, false, index))
			{
				if (getBaseTemplate() != NULL)
					return getBaseTemplate()->getCompilerIntegerParam(name, deepCheck, index);
				return NULL;
			}
			return &m_enumIndexedByEnumWeightedList[index];
		}
		fprintf(stderr, "index for parameter \"enumIndexedByEnumWeightedList\" out of bounds\n");
	}
	else if (strcmp(name, "enumListIndexed") == 0)
	{
		if (index >= 0 && index < static_cast<int>(m_enumListIndexed.size()))
			return m_enumListIndexed[index];
		if (index == static_cast<int>(m_enumListIndexed.size()))
		{
			CompilerIntegerParam *temp = new CompilerIntegerParam();
			m_enumListIndexed.push_back(temp);
			return temp;
		}
		fprintf(stderr, "index for parameter \"enumListIndexed\" out of bounds\n");
	}
	else if (strcmp(name, "enumListWeightedList") == 0)
	{
		if (index >= 0 && index < static_cast<int>(m_enumListWeightedList.size()))
			return m_enumListWeightedList[index];
		if (index == static_cast<int>(m_enumListWeightedList.size()))
		{
			CompilerIntegerParam *temp = new CompilerIntegerParam();
			m_enumListWeightedList.push_back(temp);
			return temp;
		}
		fprintf(stderr, "index for parameter \"enumListWeightedList\" out of bounds\n");
	}
	else if (strcmp(name, "integerArray") == 0)
	{
		if (index >= 0 && index < 2)
		{
			if (deepCheck && !isParamLoaded(name, false, index))
			{
				if (getBaseTemplate() != NULL)
					return getBaseTemplate()->getCompilerIntegerParam(name, deepCheck, index);
				return NULL;
			}
			return &m_integerArray[index];
		}
		fprintf(stderr, "index for parameter \"integerArray\" out of bounds\n");
	}
	else
		return TpfTemplate::getCompilerIntegerParam(name, deepCheck, index);
	return NULL;
}	//ServerUberObjectTemplate::getCompilerIntegerParam

FloatParam * ServerUberObjectTemplate::getFloatParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "floatAtDerived") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != NULL)
					return getBaseTemplate()->getFloatParam(name, deepCheck, index);
				return NULL;
			}
			return &m_floatAtDerived;
		}
		fprintf(stderr, "trying to access single-parameter \"floatAtDerived\" as an array\n");
	}
	else if (strcmp(name, "floatSimple") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != NULL)
					return getBaseTemplate()->getFloatParam(name, deepCheck, index);
				return NULL;
			}
			return &m_floatSimple;
		}
		fprintf(stderr, "trying to access single-parameter \"floatSimple\" as an array\n");
	}
	else if (strcmp(name, "floatSimpleDeltaPositive") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != NULL)
					return getBaseTemplate()->getFloatParam(name, deepCheck, index);
				return NULL;
			}
			return &m_floatSimpleDeltaPositive;
		}
		fprintf(stderr, "trying to access single-parameter \"floatSimpleDeltaPositive\" as an array\n");
	}
	else if (strcmp(name, "floatSimpleDeltaNegative") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != NULL)
					return getBaseTemplate()->getFloatParam(name, deepCheck, index);
				return NULL;
			}
			return &m_floatSimpleDeltaNegative;
		}
		fprintf(stderr, "trying to access single-parameter \"floatSimpleDeltaNegative\" as an array\n");
	}
	else if (strcmp(name, "floatSimpleDeltaPositivePercent") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != NULL)
					return getBaseTemplate()->getFloatParam(name, deepCheck, index);
				return NULL;
			}
			return &m_floatSimpleDeltaPositivePercent;
		}
		fprintf(stderr, "trying to access single-parameter \"floatSimpleDeltaPositivePercent\" as an array\n");
	}
	else if (strcmp(name, "floatSimpleDeltaNegativePercent") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != NULL)
					return getBaseTemplate()->getFloatParam(name, deepCheck, index);
				return NULL;
			}
			return &m_floatSimpleDeltaNegativePercent;
		}
		fprintf(stderr, "trying to access single-parameter \"floatSimpleDeltaNegativePercent\" as an array\n");
	}
	else if (strcmp(name, "floatWeightedList") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != NULL)
					return getBaseTemplate()->getFloatParam(name, deepCheck, index);
				return NULL;
			}
			return &m_floatWeightedList;
		}
		fprintf(stderr, "trying to access single-parameter \"floatWeightedList\" as an array\n");
	}
	else if (strcmp(name, "floatRandomRange1") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != NULL)
					return getBaseTemplate()->getFloatParam(name, deepCheck, index);
				return NULL;
			}
			return &m_floatRandomRange1;
		}
		fprintf(stderr, "trying to access single-parameter \"floatRandomRange1\" as an array\n");
	}
	else if (strcmp(name, "floatRandomRange2") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != NULL)
					return getBaseTemplate()->getFloatParam(name, deepCheck, index);
				return NULL;
			}
			return &m_floatRandomRange2;
		}
		fprintf(stderr, "trying to access single-parameter \"floatRandomRange2\" as an array\n");
	}
	else if (strcmp(name, "floatRandomRange3") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != NULL)
					return getBaseTemplate()->getFloatParam(name, deepCheck, index);
				return NULL;
			}
			return &m_floatRandomRange3;
		}
		fprintf(stderr, "trying to access single-parameter \"floatRandomRange3\" as an array\n");
	}
	else if (strcmp(name, "floatRandomRange4") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != NULL)
					return getBaseTemplate()->getFloatParam(name, deepCheck, index);
				return NULL;
			}
			return &m_floatRandomRange4;
		}
		fprintf(stderr, "trying to access single-parameter \"floatRandomRange4\" as an array\n");
	}
	else if (strcmp(name, "floatListSimple") == 0)
	{
		if (index >= 0 && index < static_cast<int>(m_floatListSimple.size()))
			return m_floatListSimple[index];
		if (index == static_cast<int>(m_floatListSimple.size()))
		{
			FloatParam *temp = new FloatParam();
			m_floatListSimple.push_back(temp);
			return temp;
		}
		fprintf(stderr, "index for parameter \"floatListSimple\" out of bounds\n");
	}
	else if (strcmp(name, "floatListWeightedList") == 0)
	{
		if (index >= 0 && index < static_cast<int>(m_floatListWeightedList.size()))
			return m_floatListWeightedList[index];
		if (index == static_cast<int>(m_floatListWeightedList.size()))
		{
			FloatParam *temp = new FloatParam();
			m_floatListWeightedList.push_back(temp);
			return temp;
		}
		fprintf(stderr, "index for parameter \"floatListWeightedList\" out of bounds\n");
	}
	else if (strcmp(name, "floatListRandomRange") == 0)
	{
		if (index >= 0 && index < static_cast<int>(m_floatListRandomRange.size()))
			return m_floatListRandomRange[index];
		if (index == static_cast<int>(m_floatListRandomRange.size()))
		{
			FloatParam *temp = new FloatParam();
			m_floatListRandomRange.push_back(temp);
			return temp;
		}
		fprintf(stderr, "index for parameter \"floatListRandomRange\" out of bounds\n");
	}
	else if (strcmp(name, "floatArray") == 0)
	{
		if (index >= 0 && index < 2)
		{
			if (deepCheck && !isParamLoaded(name, false, index))
			{
				if (getBaseTemplate() != NULL)
					return getBaseTemplate()->getFloatParam(name, deepCheck, index);
				return NULL;
			}
			return &m_floatArray[index];
		}
		fprintf(stderr, "index for parameter \"floatArray\" out of bounds\n");
	}
	else
		return TpfTemplate::getFloatParam(name, deepCheck, index);
	return NULL;
}	//ServerUberObjectTemplate::getFloatParam

BoolParam * ServerUberObjectTemplate::getBoolParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "boolDerived") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != NULL)
					return getBaseTemplate()->getBoolParam(name, deepCheck, index);
				return NULL;
			}
			return &m_boolDerived;
		}
		fprintf(stderr, "trying to access single-parameter \"boolDerived\" as an array\n");
	}
	else if (strcmp(name, "boolSimple") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != NULL)
					return getBaseTemplate()->getBoolParam(name, deepCheck, index);
				return NULL;
			}
			return &m_boolSimple;
		}
		fprintf(stderr, "trying to access single-parameter \"boolSimple\" as an array\n");
	}
	else if (strcmp(name, "boolWeightedList") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != NULL)
					return getBaseTemplate()->getBoolParam(name, deepCheck, index);
				return NULL;
			}
			return &m_boolWeightedList;
		}
		fprintf(stderr, "trying to access single-parameter \"boolWeightedList\" as an array\n");
	}
	else if (strcmp(name, "boolListDerived") == 0)
	{
		if (index >= 0 && index < static_cast<int>(m_boolListDerived.size()))
			return m_boolListDerived[index];
		if (index == static_cast<int>(m_boolListDerived.size()))
		{
			BoolParam *temp = new BoolParam();
			m_boolListDerived.push_back(temp);
			return temp;
		}
		fprintf(stderr, "index for parameter \"boolListDerived\" out of bounds\n");
	}
	else if (strcmp(name, "boolListSimple") == 0)
	{
		if (index >= 0 && index < static_cast<int>(m_boolListSimple.size()))
			return m_boolListSimple[index];
		if (index == static_cast<int>(m_boolListSimple.size()))
		{
			BoolParam *temp = new BoolParam();
			m_boolListSimple.push_back(temp);
			return temp;
		}
		fprintf(stderr, "index for parameter \"boolListSimple\" out of bounds\n");
	}
	else if (strcmp(name, "boolListWeightedList") == 0)
	{
		if (index >= 0 && index < static_cast<int>(m_boolListWeightedList.size()))
			return m_boolListWeightedList[index];
		if (index == static_cast<int>(m_boolListWeightedList.size()))
		{
			BoolParam *temp = new BoolParam();
			m_boolListWeightedList.push_back(temp);
			return temp;
		}
		fprintf(stderr, "index for parameter \"boolListWeightedList\" out of bounds\n");
	}
	else if (strcmp(name, "boolArray") == 0)
	{
		if (index >= 0 && index < 2)
		{
			if (deepCheck && !isParamLoaded(name, false, index))
			{
				if (getBaseTemplate() != NULL)
					return getBaseTemplate()->getBoolParam(name, deepCheck, index);
				return NULL;
			}
			return &m_boolArray[index];
		}
		fprintf(stderr, "index for parameter \"boolArray\" out of bounds\n");
	}
	else
		return TpfTemplate::getBoolParam(name, deepCheck, index);
	return NULL;
}	//ServerUberObjectTemplate::getBoolParam

StringParam * ServerUberObjectTemplate::getStringParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "stringDerived") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != NULL)
					return getBaseTemplate()->getStringParam(name, deepCheck, index);
				return NULL;
			}
			return &m_stringDerived;
		}
		fprintf(stderr, "trying to access single-parameter \"stringDerived\" as an array\n");
	}
	else if (strcmp(name, "stringSimple") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != NULL)
					return getBaseTemplate()->getStringParam(name, deepCheck, index);
				return NULL;
			}
			return &m_stringSimple;
		}
		fprintf(stderr, "trying to access single-parameter \"stringSimple\" as an array\n");
	}
	else if (strcmp(name, "stringWeightedList") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != NULL)
					return getBaseTemplate()->getStringParam(name, deepCheck, index);
				return NULL;
			}
			return &m_stringWeightedList;
		}
		fprintf(stderr, "trying to access single-parameter \"stringWeightedList\" as an array\n");
	}
	else if (strcmp(name, "stringListSimple") == 0)
	{
		if (index >= 0 && index < static_cast<int>(m_stringListSimple.size()))
			return m_stringListSimple[index];
		if (index == static_cast<int>(m_stringListSimple.size()))
		{
			StringParam *temp = new StringParam();
			m_stringListSimple.push_back(temp);
			return temp;
		}
		fprintf(stderr, "index for parameter \"stringListSimple\" out of bounds\n");
	}
	else if (strcmp(name, "stringListWeightedList") == 0)
	{
		if (index >= 0 && index < static_cast<int>(m_stringListWeightedList.size()))
			return m_stringListWeightedList[index];
		if (index == static_cast<int>(m_stringListWeightedList.size()))
		{
			StringParam *temp = new StringParam();
			m_stringListWeightedList.push_back(temp);
			return temp;
		}
		fprintf(stderr, "index for parameter \"stringListWeightedList\" out of bounds\n");
	}
	else if (strcmp(name, "filenameAtDerived") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != NULL)
					return getBaseTemplate()->getStringParam(name, deepCheck, index);
				return NULL;
			}
			return &m_filenameAtDerived;
		}
		fprintf(stderr, "trying to access single-parameter \"filenameAtDerived\" as an array\n");
	}
	else if (strcmp(name, "filenameSimple") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != NULL)
					return getBaseTemplate()->getStringParam(name, deepCheck, index);
				return NULL;
			}
			return &m_filenameSimple;
		}
		fprintf(stderr, "trying to access single-parameter \"filenameSimple\" as an array\n");
	}
	else if (strcmp(name, "filenameWeightedList") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != NULL)
					return getBaseTemplate()->getStringParam(name, deepCheck, index);
				return NULL;
			}
			return &m_filenameWeightedList;
		}
		fprintf(stderr, "trying to access single-parameter \"filenameWeightedList\" as an array\n");
	}
	else if (strcmp(name, "filenameListSimple") == 0)
	{
		if (index >= 0 && index < static_cast<int>(m_filenameListSimple.size()))
			return m_filenameListSimple[index];
		if (index == static_cast<int>(m_filenameListSimple.size()))
		{
			StringParam *temp = new StringParam();
			m_filenameListSimple.push_back(temp);
			return temp;
		}
		fprintf(stderr, "index for parameter \"filenameListSimple\" out of bounds\n");
	}
	else if (strcmp(name, "templateDerived") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != NULL)
					return getBaseTemplate()->getStringParam(name, deepCheck, index);
				return NULL;
			}
			return &m_templateDerived;
		}
		fprintf(stderr, "trying to access single-parameter \"templateDerived\" as an array\n");
	}
	else if (strcmp(name, "templateSimple") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != NULL)
					return getBaseTemplate()->getStringParam(name, deepCheck, index);
				return NULL;
			}
			return &m_templateSimple;
		}
		fprintf(stderr, "trying to access single-parameter \"templateSimple\" as an array\n");
	}
	else if (strcmp(name, "templateWeightedList") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != NULL)
					return getBaseTemplate()->getStringParam(name, deepCheck, index);
				return NULL;
			}
			return &m_templateWeightedList;
		}
		fprintf(stderr, "trying to access single-parameter \"templateWeightedList\" as an array\n");
	}
	else if (strcmp(name, "templateListSimple") == 0)
	{
		if (index >= 0 && index < static_cast<int>(m_templateListSimple.size()))
			return m_templateListSimple[index];
		if (index == static_cast<int>(m_templateListSimple.size()))
		{
			StringParam *temp = new StringParam();
			m_templateListSimple.push_back(temp);
			return temp;
		}
		fprintf(stderr, "index for parameter \"templateListSimple\" out of bounds\n");
	}
	else if (strcmp(name, "stringArray") == 0)
	{
		if (index >= 0 && index < 2)
		{
			if (deepCheck && !isParamLoaded(name, false, index))
			{
				if (getBaseTemplate() != NULL)
					return getBaseTemplate()->getStringParam(name, deepCheck, index);
				return NULL;
			}
			return &m_stringArray[index];
		}
		fprintf(stderr, "index for parameter \"stringArray\" out of bounds\n");
	}
	else if (strcmp(name, "fileNameArray") == 0)
	{
		if (index >= 0 && index < 2)
		{
			if (deepCheck && !isParamLoaded(name, false, index))
			{
				if (getBaseTemplate() != NULL)
					return getBaseTemplate()->getStringParam(name, deepCheck, index);
				return NULL;
			}
			return &m_fileNameArray[index];
		}
		fprintf(stderr, "index for parameter \"fileNameArray\" out of bounds\n");
	}
	else
		return TpfTemplate::getStringParam(name, deepCheck, index);
	return NULL;
}	//ServerUberObjectTemplate::getStringParam

StringIdParam * ServerUberObjectTemplate::getStringIdParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "stringIdDerived") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != NULL)
					return getBaseTemplate()->getStringIdParam(name, deepCheck, index);
				return NULL;
			}
			return &m_stringIdDerived;
		}
		fprintf(stderr, "trying to access single-parameter \"stringIdDerived\" as an array\n");
	}
	else if (strcmp(name, "stringIdSimple") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != NULL)
					return getBaseTemplate()->getStringIdParam(name, deepCheck, index);
				return NULL;
			}
			return &m_stringIdSimple;
		}
		fprintf(stderr, "trying to access single-parameter \"stringIdSimple\" as an array\n");
	}
	else if (strcmp(name, "stringIdWeightedList") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != NULL)
					return getBaseTemplate()->getStringIdParam(name, deepCheck, index);
				return NULL;
			}
			return &m_stringIdWeightedList;
		}
		fprintf(stderr, "trying to access single-parameter \"stringIdWeightedList\" as an array\n");
	}
	else if (strcmp(name, "stringIdListSimple") == 0)
	{
		if (index >= 0 && index < static_cast<int>(m_stringIdListSimple.size()))
			return m_stringIdListSimple[index];
		if (index == static_cast<int>(m_stringIdListSimple.size()))
		{
			StringIdParam *temp = new StringIdParam();
			m_stringIdListSimple.push_back(temp);
			return temp;
		}
		fprintf(stderr, "index for parameter \"stringIdListSimple\" out of bounds\n");
	}
	else if (strcmp(name, "stringIdListWeightedList") == 0)
	{
		if (index >= 0 && index < static_cast<int>(m_stringIdListWeightedList.size()))
			return m_stringIdListWeightedList[index];
		if (index == static_cast<int>(m_stringIdListWeightedList.size()))
		{
			StringIdParam *temp = new StringIdParam();
			m_stringIdListWeightedList.push_back(temp);
			return temp;
		}
		fprintf(stderr, "index for parameter \"stringIdListWeightedList\" out of bounds\n");
	}
	else if (strcmp(name, "stringIdArray") == 0)
	{
		if (index >= 0 && index < 2)
		{
			if (deepCheck && !isParamLoaded(name, false, index))
			{
				if (getBaseTemplate() != NULL)
					return getBaseTemplate()->getStringIdParam(name, deepCheck, index);
				return NULL;
			}
			return &m_stringIdArray[index];
		}
		fprintf(stderr, "index for parameter \"stringIdArray\" out of bounds\n");
	}
	else
		return TpfTemplate::getStringIdParam(name, deepCheck, index);
	return NULL;
}	//ServerUberObjectTemplate::getStringIdParam

VectorParam * ServerUberObjectTemplate::getVectorParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "vectorAtDerived") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != NULL)
					return getBaseTemplate()->getVectorParam(name, deepCheck, index);
				return NULL;
			}
			return &m_vectorAtDerived;
		}
		fprintf(stderr, "trying to access single-parameter \"vectorAtDerived\" as an array\n");
	}
	else if (strcmp(name, "vectorSimple") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != NULL)
					return getBaseTemplate()->getVectorParam(name, deepCheck, index);
				return NULL;
			}
			return &m_vectorSimple;
		}
		fprintf(stderr, "trying to access single-parameter \"vectorSimple\" as an array\n");
	}
	else if (strcmp(name, "vectorListSimple") == 0)
	{
		if (index >= 0 && index < static_cast<int>(m_vectorListSimple.size()))
			return m_vectorListSimple[index];
		if (index == static_cast<int>(m_vectorListSimple.size()))
		{
			VectorParam *temp = new VectorParam();
			m_vectorListSimple.push_back(temp);
			return temp;
		}
		fprintf(stderr, "index for parameter \"vectorListSimple\" out of bounds\n");
	}
	else if (strcmp(name, "vectorArray") == 0)
	{
		if (index >= 0 && index < 2)
		{
			if (deepCheck && !isParamLoaded(name, false, index))
			{
				if (getBaseTemplate() != NULL)
					return getBaseTemplate()->getVectorParam(name, deepCheck, index);
				return NULL;
			}
			return &m_vectorArray[index];
		}
		fprintf(stderr, "index for parameter \"vectorArray\" out of bounds\n");
	}
	else
		return TpfTemplate::getVectorParam(name, deepCheck, index);
	return NULL;
}	//ServerUberObjectTemplate::getVectorParam

DynamicVariableParam * ServerUberObjectTemplate::getDynamicVariableParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "objvarDerived") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != NULL)
					return getBaseTemplate()->getDynamicVariableParam(name, deepCheck, index);
				return NULL;
			}
			return &m_objvarDerived;
		}
		fprintf(stderr, "trying to access single-parameter \"objvarDerived\" as an array\n");
	}
	else if (strcmp(name, "objvarSimple") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != NULL)
					return getBaseTemplate()->getDynamicVariableParam(name, deepCheck, index);
				return NULL;
			}
			return &m_objvarSimple;
		}
		fprintf(stderr, "trying to access single-parameter \"objvarSimple\" as an array\n");
	}
	else
		return TpfTemplate::getDynamicVariableParam(name, deepCheck, index);
	return NULL;
}	//ServerUberObjectTemplate::getDynamicVariableParam

StructParamOT * ServerUberObjectTemplate::getStructParamOT(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "structAtDerived") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != NULL)
					return getBaseTemplate()->getStructParamOT(name, deepCheck, index);
				return NULL;
			}
			return &m_structAtDerived;
		}
		fprintf(stderr, "trying to access single-parameter \"structAtDerived\" as an array\n");
	}
	else if (strcmp(name, "structSimple") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != NULL)
					return getBaseTemplate()->getStructParamOT(name, deepCheck, index);
				return NULL;
			}
			return &m_structSimple;
		}
		fprintf(stderr, "trying to access single-parameter \"structSimple\" as an array\n");
	}
	else if (strcmp(name, "structListSimple") == 0)
	{
		if (index >= 0 && index < static_cast<int>(m_structListSimple.size()))
			return m_structListSimple[index];
		if (index == static_cast<int>(m_structListSimple.size()))
		{
			StructParamOT *temp = new StructParamOT();
			m_structListSimple.push_back(temp);
			return temp;
		}
		fprintf(stderr, "index for parameter \"structListSimple\" out of bounds\n");
	}
	else if (strcmp(name, "structArrayEnum") == 0)
	{
		if (index >= 0 && index < 3)
		{
			if (deepCheck && !isParamLoaded(name, false, index))
			{
				if (getBaseTemplate() != NULL)
					return getBaseTemplate()->getStructParamOT(name, deepCheck, index);
				return NULL;
			}
			return &m_structArrayEnum[index];
		}
		fprintf(stderr, "index for parameter \"structArrayEnum\" out of bounds\n");
	}
	else if (strcmp(name, "structArrayInteger") == 0)
	{
		if (index >= 0 && index < 2)
		{
			if (deepCheck && !isParamLoaded(name, false, index))
			{
				if (getBaseTemplate() != NULL)
					return getBaseTemplate()->getStructParamOT(name, deepCheck, index);
				return NULL;
			}
			return &m_structArrayInteger[index];
		}
		fprintf(stderr, "index for parameter \"structArrayInteger\" out of bounds\n");
	}
	else
		return TpfTemplate::getStructParamOT(name, deepCheck, index);
	return NULL;
}	//ServerUberObjectTemplate::getStructParamOT

TriggerVolumeParam * ServerUberObjectTemplate::getTriggerVolumeParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "triggerVolumeDerived") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != NULL)
					return getBaseTemplate()->getTriggerVolumeParam(name, deepCheck, index);
				return NULL;
			}
			return &m_triggerVolumeDerived;
		}
		fprintf(stderr, "trying to access single-parameter \"triggerVolumeDerived\" as an array\n");
	}
	else if (strcmp(name, "triggerVolumeSimple") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != NULL)
					return getBaseTemplate()->getTriggerVolumeParam(name, deepCheck, index);
				return NULL;
			}
			return &m_triggerVolumeSimple;
		}
		fprintf(stderr, "trying to access single-parameter \"triggerVolumeSimple\" as an array\n");
	}
	else if (strcmp(name, "triggerVolumeWeightedList") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != NULL)
					return getBaseTemplate()->getTriggerVolumeParam(name, deepCheck, index);
				return NULL;
			}
			return &m_triggerVolumeWeightedList;
		}
		fprintf(stderr, "trying to access single-parameter \"triggerVolumeWeightedList\" as an array\n");
	}
	else if (strcmp(name, "triggerVolumeList") == 0)
	{
		if (index >= 0 && index < static_cast<int>(m_triggerVolumeList.size()))
			return m_triggerVolumeList[index];
		if (index == static_cast<int>(m_triggerVolumeList.size()))
		{
			TriggerVolumeParam *temp = new TriggerVolumeParam();
			m_triggerVolumeList.push_back(temp);
			return temp;
		}
		fprintf(stderr, "index for parameter \"triggerVolumeList\" out of bounds\n");
	}
	else if (strcmp(name, "triggerVolumesListWeightedList") == 0)
	{
		if (index >= 0 && index < static_cast<int>(m_triggerVolumesListWeightedList.size()))
			return m_triggerVolumesListWeightedList[index];
		if (index == static_cast<int>(m_triggerVolumesListWeightedList.size()))
		{
			TriggerVolumeParam *temp = new TriggerVolumeParam();
			m_triggerVolumesListWeightedList.push_back(temp);
			return temp;
		}
		fprintf(stderr, "index for parameter \"triggerVolumesListWeightedList\" out of bounds\n");
	}
	else if (strcmp(name, "triggerArray") == 0)
	{
		if (index >= 0 && index < 2)
		{
			if (deepCheck && !isParamLoaded(name, false, index))
			{
				if (getBaseTemplate() != NULL)
					return getBaseTemplate()->getTriggerVolumeParam(name, deepCheck, index);
				return NULL;
			}
			return &m_triggerArray[index];
		}
		fprintf(stderr, "index for parameter \"triggerArray\" out of bounds\n");
	}
	else
		return TpfTemplate::getTriggerVolumeParam(name, deepCheck, index);
	return NULL;
}	//ServerUberObjectTemplate::getTriggerVolumeParam

void ServerUberObjectTemplate::initStructParamOT(StructParamOT &param, const char *name)
{
	if (param.isInitialized())
		return;
	if (strcmp(name, "structAtDerived") == 0)
		param.setValue(new _Foo(""));
	else if (strcmp(name, "structSimple") == 0)
		param.setValue(new _Foo(""));
	else if (strcmp(name, "structListSimple") == 0)
		param.setValue(new _Foo(""));
	else if (strcmp(name, "structArrayEnum") == 0)
		param.setValue(new _Foo(""));
	else if (strcmp(name, "structArrayInteger") == 0)
		param.setValue(new _Foo(""));
	else
		TpfTemplate::initStructParamOT(param, name);
}	// ServerUberObjectTemplate::initStructParamOT

void ServerUberObjectTemplate::setAsEmptyList(const char *name)
{
	if (strcmp(name, "intListSimple") == 0)
	{
		m_intListSimple.clear();
		m_intListSimpleLoaded = true;
	}
	else if (strcmp(name, "intListWeightedList") == 0)
	{
		m_intListWeightedList.clear();
		m_intListWeightedListLoaded = true;
	}
	else if (strcmp(name, "intListRandomRange") == 0)
	{
		m_intListRandomRange.clear();
		m_intListRandomRangeLoaded = true;
	}
	else if (strcmp(name, "intListDiceRoll") == 0)
	{
		m_intListDiceRoll.clear();
		m_intListDiceRollLoaded = true;
	}
	else if (strcmp(name, "floatListSimple") == 0)
	{
		m_floatListSimple.clear();
		m_floatListSimpleLoaded = true;
	}
	else if (strcmp(name, "floatListWeightedList") == 0)
	{
		m_floatListWeightedList.clear();
		m_floatListWeightedListLoaded = true;
	}
	else if (strcmp(name, "floatListRandomRange") == 0)
	{
		m_floatListRandomRange.clear();
		m_floatListRandomRangeLoaded = true;
	}
	else if (strcmp(name, "enumListIndexed") == 0)
	{
		m_enumListIndexed.clear();
		m_enumListIndexedLoaded = true;
	}
	else if (strcmp(name, "enumListWeightedList") == 0)
	{
		m_enumListWeightedList.clear();
		m_enumListWeightedListLoaded = true;
	}
	else if (strcmp(name, "stringIdListSimple") == 0)
	{
		m_stringIdListSimple.clear();
		m_stringIdListSimpleLoaded = true;
	}
	else if (strcmp(name, "stringIdListWeightedList") == 0)
	{
		m_stringIdListWeightedList.clear();
		m_stringIdListWeightedListLoaded = true;
	}
	else if (strcmp(name, "stringListSimple") == 0)
	{
		m_stringListSimple.clear();
		m_stringListSimpleLoaded = true;
	}
	else if (strcmp(name, "stringListWeightedList") == 0)
	{
		m_stringListWeightedList.clear();
		m_stringListWeightedListLoaded = true;
	}
	else if (strcmp(name, "triggerVolumeList") == 0)
	{
		m_triggerVolumeList.clear();
		m_triggerVolumeListLoaded = true;
	}
	else if (strcmp(name, "triggerVolumesListWeightedList") == 0)
	{
		m_triggerVolumesListWeightedList.clear();
		m_triggerVolumesListWeightedListLoaded = true;
	}
	else if (strcmp(name, "boolListDerived") == 0)
	{
		m_boolListDerived.clear();
		m_boolListDerivedLoaded = true;
	}
	else if (strcmp(name, "boolListSimple") == 0)
	{
		m_boolListSimple.clear();
		m_boolListSimpleLoaded = true;
	}
	else if (strcmp(name, "boolListWeightedList") == 0)
	{
		m_boolListWeightedList.clear();
		m_boolListWeightedListLoaded = true;
	}
	else if (strcmp(name, "vectorListSimple") == 0)
	{
		m_vectorListSimple.clear();
		m_vectorListSimpleLoaded = true;
	}
	else if (strcmp(name, "filenameListSimple") == 0)
	{
		m_filenameListSimple.clear();
		m_filenameListSimpleLoaded = true;
	}
	else if (strcmp(name, "templateListSimple") == 0)
	{
		m_templateListSimple.clear();
		m_templateListSimpleLoaded = true;
	}
	else if (strcmp(name, "structListSimple") == 0)
	{
		m_structListSimple.clear();
		m_structListSimpleLoaded = true;
	}
	else
		TpfTemplate::setAsEmptyList(name);
}	// ServerUberObjectTemplate::setAsEmptyList

void ServerUberObjectTemplate::setAppend(const char *name)
{
	if (strcmp(name, "intListSimple") == 0)
		m_intListSimpleAppend = true;
	else if (strcmp(name, "intListWeightedList") == 0)
		m_intListWeightedListAppend = true;
	else if (strcmp(name, "intListRandomRange") == 0)
		m_intListRandomRangeAppend = true;
	else if (strcmp(name, "intListDiceRoll") == 0)
		m_intListDiceRollAppend = true;
	else if (strcmp(name, "floatListSimple") == 0)
		m_floatListSimpleAppend = true;
	else if (strcmp(name, "floatListWeightedList") == 0)
		m_floatListWeightedListAppend = true;
	else if (strcmp(name, "floatListRandomRange") == 0)
		m_floatListRandomRangeAppend = true;
	else if (strcmp(name, "enumListIndexed") == 0)
		m_enumListIndexedAppend = true;
	else if (strcmp(name, "enumListWeightedList") == 0)
		m_enumListWeightedListAppend = true;
	else if (strcmp(name, "stringIdListSimple") == 0)
		m_stringIdListSimpleAppend = true;
	else if (strcmp(name, "stringIdListWeightedList") == 0)
		m_stringIdListWeightedListAppend = true;
	else if (strcmp(name, "stringListSimple") == 0)
		m_stringListSimpleAppend = true;
	else if (strcmp(name, "stringListWeightedList") == 0)
		m_stringListWeightedListAppend = true;
	else if (strcmp(name, "triggerVolumeList") == 0)
		m_triggerVolumeListAppend = true;
	else if (strcmp(name, "triggerVolumesListWeightedList") == 0)
		m_triggerVolumesListWeightedListAppend = true;
	else if (strcmp(name, "boolListDerived") == 0)
		m_boolListDerivedAppend = true;
	else if (strcmp(name, "boolListSimple") == 0)
		m_boolListSimpleAppend = true;
	else if (strcmp(name, "boolListWeightedList") == 0)
		m_boolListWeightedListAppend = true;
	else if (strcmp(name, "vectorListSimple") == 0)
		m_vectorListSimpleAppend = true;
	else if (strcmp(name, "filenameListSimple") == 0)
		m_filenameListSimpleAppend = true;
	else if (strcmp(name, "templateListSimple") == 0)
		m_templateListSimpleAppend = true;
	else if (strcmp(name, "structListSimple") == 0)
		m_structListSimpleAppend = true;
	else
		TpfTemplate::setAppend(name);
}	// ServerUberObjectTemplate::setAppend

bool ServerUberObjectTemplate::isAppend(const char *name) const
{
	if (strcmp(name, "intListSimple") == 0)
		return m_intListSimpleAppend;
	else if (strcmp(name, "intListWeightedList") == 0)
		return m_intListWeightedListAppend;
	else if (strcmp(name, "intListRandomRange") == 0)
		return m_intListRandomRangeAppend;
	else if (strcmp(name, "intListDiceRoll") == 0)
		return m_intListDiceRollAppend;
	else if (strcmp(name, "floatListSimple") == 0)
		return m_floatListSimpleAppend;
	else if (strcmp(name, "floatListWeightedList") == 0)
		return m_floatListWeightedListAppend;
	else if (strcmp(name, "floatListRandomRange") == 0)
		return m_floatListRandomRangeAppend;
	else if (strcmp(name, "enumListIndexed") == 0)
		return m_enumListIndexedAppend;
	else if (strcmp(name, "enumListWeightedList") == 0)
		return m_enumListWeightedListAppend;
	else if (strcmp(name, "stringIdListSimple") == 0)
		return m_stringIdListSimpleAppend;
	else if (strcmp(name, "stringIdListWeightedList") == 0)
		return m_stringIdListWeightedListAppend;
	else if (strcmp(name, "stringListSimple") == 0)
		return m_stringListSimpleAppend;
	else if (strcmp(name, "stringListWeightedList") == 0)
		return m_stringListWeightedListAppend;
	else if (strcmp(name, "triggerVolumeList") == 0)
		return m_triggerVolumeListAppend;
	else if (strcmp(name, "triggerVolumesListWeightedList") == 0)
		return m_triggerVolumesListWeightedListAppend;
	else if (strcmp(name, "boolListDerived") == 0)
		return m_boolListDerivedAppend;
	else if (strcmp(name, "boolListSimple") == 0)
		return m_boolListSimpleAppend;
	else if (strcmp(name, "boolListWeightedList") == 0)
		return m_boolListWeightedListAppend;
	else if (strcmp(name, "vectorListSimple") == 0)
		return m_vectorListSimpleAppend;
	else if (strcmp(name, "filenameListSimple") == 0)
		return m_filenameListSimpleAppend;
	else if (strcmp(name, "templateListSimple") == 0)
		return m_templateListSimpleAppend;
	else if (strcmp(name, "structListSimple") == 0)
		return m_structListSimpleAppend;
	else
		return TpfTemplate::isAppend(name);
}	// ServerUberObjectTemplate::isAppend


int ServerUberObjectTemplate::getListLength(const char *name) const
{
	if (strcmp(name, "intListSimple") == 0)
	{
		return m_intListSimple.size();
	}
	else if (strcmp(name, "intListWeightedList") == 0)
	{
		return m_intListWeightedList.size();
	}
	else if (strcmp(name, "intListRandomRange") == 0)
	{
		return m_intListRandomRange.size();
	}
	else if (strcmp(name, "intListDiceRoll") == 0)
	{
		return m_intListDiceRoll.size();
	}
	else if (strcmp(name, "floatListSimple") == 0)
	{
		return m_floatListSimple.size();
	}
	else if (strcmp(name, "floatListWeightedList") == 0)
	{
		return m_floatListWeightedList.size();
	}
	else if (strcmp(name, "floatListRandomRange") == 0)
	{
		return m_floatListRandomRange.size();
	}
	else if (strcmp(name, "enumIndexedByEnumSingle") == 0)
	{
		return sizeof(m_enumIndexedByEnumSingle) / sizeof(CompilerIntegerParam);
	}
	else if (strcmp(name, "enumIndexedByEnumWeightedList") == 0)
	{
		return sizeof(m_enumIndexedByEnumWeightedList) / sizeof(CompilerIntegerParam);
	}
	else if (strcmp(name, "enumListIndexed") == 0)
	{
		return m_enumListIndexed.size();
	}
	else if (strcmp(name, "enumListWeightedList") == 0)
	{
		return m_enumListWeightedList.size();
	}
	else if (strcmp(name, "stringIdListSimple") == 0)
	{
		return m_stringIdListSimple.size();
	}
	else if (strcmp(name, "stringIdListWeightedList") == 0)
	{
		return m_stringIdListWeightedList.size();
	}
	else if (strcmp(name, "stringListSimple") == 0)
	{
		return m_stringListSimple.size();
	}
	else if (strcmp(name, "stringListWeightedList") == 0)
	{
		return m_stringListWeightedList.size();
	}
	else if (strcmp(name, "triggerVolumeList") == 0)
	{
		return m_triggerVolumeList.size();
	}
	else if (strcmp(name, "triggerVolumesListWeightedList") == 0)
	{
		return m_triggerVolumesListWeightedList.size();
	}
	else if (strcmp(name, "boolListDerived") == 0)
	{
		return m_boolListDerived.size();
	}
	else if (strcmp(name, "boolListSimple") == 0)
	{
		return m_boolListSimple.size();
	}
	else if (strcmp(name, "boolListWeightedList") == 0)
	{
		return m_boolListWeightedList.size();
	}
	else if (strcmp(name, "vectorListSimple") == 0)
	{
		return m_vectorListSimple.size();
	}
	else if (strcmp(name, "filenameListSimple") == 0)
	{
		return m_filenameListSimple.size();
	}
	else if (strcmp(name, "templateListSimple") == 0)
	{
		return m_templateListSimple.size();
	}
	else if (strcmp(name, "structListSimple") == 0)
	{
		return m_structListSimple.size();
	}
	else if (strcmp(name, "structArrayEnum") == 0)
	{
		return sizeof(m_structArrayEnum) / sizeof(StructParamOT);
	}
	else if (strcmp(name, "structArrayInteger") == 0)
	{
		return sizeof(m_structArrayInteger) / sizeof(StructParamOT);
	}
	else if (strcmp(name, "integerArray") == 0)
	{
		return sizeof(m_integerArray) / sizeof(CompilerIntegerParam);
	}
	else if (strcmp(name, "floatArray") == 0)
	{
		return sizeof(m_floatArray) / sizeof(FloatParam);
	}
	else if (strcmp(name, "stringArray") == 0)
	{
		return sizeof(m_stringArray) / sizeof(StringParam);
	}
	else if (strcmp(name, "boolArray") == 0)
	{
		return sizeof(m_boolArray) / sizeof(BoolParam);
	}
	else if (strcmp(name, "stringIdArray") == 0)
	{
		return sizeof(m_stringIdArray) / sizeof(StringIdParam);
	}
	else if (strcmp(name, "triggerArray") == 0)
	{
		return sizeof(m_triggerArray) / sizeof(TriggerVolumeParam);
	}
	else if (strcmp(name, "vectorArray") == 0)
	{
		return sizeof(m_vectorArray) / sizeof(VectorParam);
	}
	else if (strcmp(name, "fileNameArray") == 0)
	{
		return sizeof(m_fileNameArray) / sizeof(StringParam);
	}
	else
		return TpfTemplate::getListLength(name);
}	// ServerUberObjectTemplate::getListLength

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void ServerUberObjectTemplate::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	if (file.getCurrentName() != ServerUberObjectTemplate_tag)
	{
		TpfTemplate::load(file);
		return;
	}

	file.enterForm();
	m_templateVersion = file.getCurrentName();
	if (m_templateVersion == TAG(D,E,R,V))
	{
		file.enterForm();
		file.enterChunk();
		std::string baseFilename;
		file.read_string(baseFilename);
		file.exitChunk();
		const ObjectTemplate *base = ObjectTemplateList::fetch(baseFilename);
		DEBUG_WARNING(base == NULL, ("was unable to load base template %s", baseFilename.c_str()));
		if (m_baseData == base && base != NULL)
			base->releaseReference();
		else
		{
			if (m_baseData != NULL)
				m_baseData->releaseReference();
			m_baseData = base;
		}
		file.exitForm();
		m_templateVersion = file.getCurrentName();
	}
	if (getHighestTemplateVersion() != TAG(0,0,0,0))
	{
		if (DataLint::isEnabled())
			DEBUG_WARNING(true, ("template %s version out of date", file.getFileName()));
	}

	file.enterForm();

	file.enterChunk();
	int paramCount = file.read_int32();
	file.exitChunk();
	for (int i = 0; i < paramCount; ++i)
	{
		file.enterChunk();
		file.read_string(paramName, MAX_NAME_SIZE);
		if (strcmp(paramName, "intAtDerived") == 0)
			m_intAtDerived.loadFromIff(file);
		else if (strcmp(paramName, "intSimple") == 0)
			m_intSimple.loadFromIff(file);
		else if (strcmp(paramName, "intSimpleDeltaPositive") == 0)
			m_intSimpleDeltaPositive.loadFromIff(file);
		else if (strcmp(paramName, "intSimpleDeltaNegative") == 0)
			m_intSimpleDeltaNegative.loadFromIff(file);
		else if (strcmp(paramName, "intSimpleDeltaPositivePercent") == 0)
			m_intSimpleDeltaPositivePercent.loadFromIff(file);
		else if (strcmp(paramName, "intSimpleDeltaNegativePercent") == 0)
			m_intSimpleDeltaNegativePercent.loadFromIff(file);
		else if (strcmp(paramName, "intWeightedList") == 0)
			m_intWeightedList.loadFromIff(file);
		else if (strcmp(paramName, "intWeightedListDeltaPositive") == 0)
			m_intWeightedListDeltaPositive.loadFromIff(file);
		else if (strcmp(paramName, "intWeightedListDeltaNegative") == 0)
			m_intWeightedListDeltaNegative.loadFromIff(file);
		else if (strcmp(paramName, "intWeightedListDeltaPositivePercent") == 0)
			m_intWeightedListDeltaPositivePercent.loadFromIff(file);
		else if (strcmp(paramName, "intWeightedListDeltaNegativePercent") == 0)
			m_intWeightedListDeltaNegativePercent.loadFromIff(file);
		else if (strcmp(paramName, "intRandomRange1") == 0)
			m_intRandomRange1.loadFromIff(file);
		else if (strcmp(paramName, "intRandomRange2") == 0)
			m_intRandomRange2.loadFromIff(file);
		else if (strcmp(paramName, "intRandomRange3") == 0)
			m_intRandomRange3.loadFromIff(file);
		else if (strcmp(paramName, "intRandomRange4") == 0)
			m_intRandomRange4.loadFromIff(file);
		else if (strcmp(paramName, "intDiceRoll1") == 0)
			m_intDiceRoll1.loadFromIff(file);
		else if (strcmp(paramName, "intDiceRoll2") == 0)
			m_intDiceRoll2.loadFromIff(file);
		else if (strcmp(paramName, "intListSimple") == 0)
		{
			std::vector<CompilerIntegerParam *>::iterator iter;
			for (iter = m_intListSimple.begin(); iter != m_intListSimple.end(); ++iter)
			{
				delete *iter;
				*iter = NULL;
			}
			m_intListSimpleAppend = file.read_bool8();
			int listCount = file.read_int32();
			for (int j = 0; j < listCount; ++j)
			{
				CompilerIntegerParam * newData = new CompilerIntegerParam;
				newData->loadFromIff(file);
				m_intListSimple.push_back(newData);
			}
			m_intListSimpleLoaded = true;
		}
		else if (strcmp(paramName, "intListWeightedList") == 0)
		{
			std::vector<CompilerIntegerParam *>::iterator iter;
			for (iter = m_intListWeightedList.begin(); iter != m_intListWeightedList.end(); ++iter)
			{
				delete *iter;
				*iter = NULL;
			}
			m_intListWeightedListAppend = file.read_bool8();
			int listCount = file.read_int32();
			for (int j = 0; j < listCount; ++j)
			{
				CompilerIntegerParam * newData = new CompilerIntegerParam;
				newData->loadFromIff(file);
				m_intListWeightedList.push_back(newData);
			}
			m_intListWeightedListLoaded = true;
		}
		else if (strcmp(paramName, "intListRandomRange") == 0)
		{
			std::vector<CompilerIntegerParam *>::iterator iter;
			for (iter = m_intListRandomRange.begin(); iter != m_intListRandomRange.end(); ++iter)
			{
				delete *iter;
				*iter = NULL;
			}
			m_intListRandomRangeAppend = file.read_bool8();
			int listCount = file.read_int32();
			for (int j = 0; j < listCount; ++j)
			{
				CompilerIntegerParam * newData = new CompilerIntegerParam;
				newData->loadFromIff(file);
				m_intListRandomRange.push_back(newData);
			}
			m_intListRandomRangeLoaded = true;
		}
		else if (strcmp(paramName, "intListDiceRoll") == 0)
		{
			std::vector<CompilerIntegerParam *>::iterator iter;
			for (iter = m_intListDiceRoll.begin(); iter != m_intListDiceRoll.end(); ++iter)
			{
				delete *iter;
				*iter = NULL;
			}
			m_intListDiceRollAppend = file.read_bool8();
			int listCount = file.read_int32();
			for (int j = 0; j < listCount; ++j)
			{
				CompilerIntegerParam * newData = new CompilerIntegerParam;
				newData->loadFromIff(file);
				m_intListDiceRoll.push_back(newData);
			}
			m_intListDiceRollLoaded = true;
		}
		else if (strcmp(paramName, "floatAtDerived") == 0)
			m_floatAtDerived.loadFromIff(file);
		else if (strcmp(paramName, "floatSimple") == 0)
			m_floatSimple.loadFromIff(file);
		else if (strcmp(paramName, "floatSimpleDeltaPositive") == 0)
			m_floatSimpleDeltaPositive.loadFromIff(file);
		else if (strcmp(paramName, "floatSimpleDeltaNegative") == 0)
			m_floatSimpleDeltaNegative.loadFromIff(file);
		else if (strcmp(paramName, "floatSimpleDeltaPositivePercent") == 0)
			m_floatSimpleDeltaPositivePercent.loadFromIff(file);
		else if (strcmp(paramName, "floatSimpleDeltaNegativePercent") == 0)
			m_floatSimpleDeltaNegativePercent.loadFromIff(file);
		else if (strcmp(paramName, "floatWeightedList") == 0)
			m_floatWeightedList.loadFromIff(file);
		else if (strcmp(paramName, "floatRandomRange1") == 0)
			m_floatRandomRange1.loadFromIff(file);
		else if (strcmp(paramName, "floatRandomRange2") == 0)
			m_floatRandomRange2.loadFromIff(file);
		else if (strcmp(paramName, "floatRandomRange3") == 0)
			m_floatRandomRange3.loadFromIff(file);
		else if (strcmp(paramName, "floatRandomRange4") == 0)
			m_floatRandomRange4.loadFromIff(file);
		else if (strcmp(paramName, "floatListSimple") == 0)
		{
			std::vector<FloatParam *>::iterator iter;
			for (iter = m_floatListSimple.begin(); iter != m_floatListSimple.end(); ++iter)
			{
				delete *iter;
				*iter = NULL;
			}
			m_floatListSimpleAppend = file.read_bool8();
			int listCount = file.read_int32();
			for (int j = 0; j < listCount; ++j)
			{
				FloatParam * newData = new FloatParam;
				newData->loadFromIff(file);
				m_floatListSimple.push_back(newData);
			}
			m_floatListSimpleLoaded = true;
		}
		else if (strcmp(paramName, "floatListWeightedList") == 0)
		{
			std::vector<FloatParam *>::iterator iter;
			for (iter = m_floatListWeightedList.begin(); iter != m_floatListWeightedList.end(); ++iter)
			{
				delete *iter;
				*iter = NULL;
			}
			m_floatListWeightedListAppend = file.read_bool8();
			int listCount = file.read_int32();
			for (int j = 0; j < listCount; ++j)
			{
				FloatParam * newData = new FloatParam;
				newData->loadFromIff(file);
				m_floatListWeightedList.push_back(newData);
			}
			m_floatListWeightedListLoaded = true;
		}
		else if (strcmp(paramName, "floatListRandomRange") == 0)
		{
			std::vector<FloatParam *>::iterator iter;
			for (iter = m_floatListRandomRange.begin(); iter != m_floatListRandomRange.end(); ++iter)
			{
				delete *iter;
				*iter = NULL;
			}
			m_floatListRandomRangeAppend = file.read_bool8();
			int listCount = file.read_int32();
			for (int j = 0; j < listCount; ++j)
			{
				FloatParam * newData = new FloatParam;
				newData->loadFromIff(file);
				m_floatListRandomRange.push_back(newData);
			}
			m_floatListRandomRangeLoaded = true;
		}
		else if (strcmp(paramName, "enumIndexedByEnumSingle") == 0)
		{
			int listCount = file.read_int32();
			DEBUG_WARNING(listCount != 3, ("read array size of %d for an array of size 3, reading values anyway", listCount));
			int j;
			for (j = 0; j < 3 && j < listCount; ++j)
				m_enumIndexedByEnumSingle[j].loadFromIff(file);
			// if there are more params for enumIndexedByEnumSingle read and dump them
			for (; j < listCount; ++j)
			{
				CompilerIntegerParam dummy;
				dummy.loadFromIff(file);
			}
		}
		else if (strcmp(paramName, "enumIndexedByEnumWeightedList") == 0)
		{
			int listCount = file.read_int32();
			DEBUG_WARNING(listCount != 3, ("read array size of %d for an array of size 3, reading values anyway", listCount));
			int j;
			for (j = 0; j < 3 && j < listCount; ++j)
				m_enumIndexedByEnumWeightedList[j].loadFromIff(file);
			// if there are more params for enumIndexedByEnumWeightedList read and dump them
			for (; j < listCount; ++j)
			{
				CompilerIntegerParam dummy;
				dummy.loadFromIff(file);
			}
		}
		else if (strcmp(paramName, "enumListIndexed") == 0)
		{
			std::vector<CompilerIntegerParam *>::iterator iter;
			for (iter = m_enumListIndexed.begin(); iter != m_enumListIndexed.end(); ++iter)
			{
				delete *iter;
				*iter = NULL;
			}
			m_enumListIndexedAppend = file.read_bool8();
			int listCount = file.read_int32();
			for (int j = 0; j < listCount; ++j)
			{
				CompilerIntegerParam * newData = new CompilerIntegerParam;
				newData->loadFromIff(file);
				m_enumListIndexed.push_back(newData);
			}
			m_enumListIndexedLoaded = true;
		}
		else if (strcmp(paramName, "enumListWeightedList") == 0)
		{
			std::vector<CompilerIntegerParam *>::iterator iter;
			for (iter = m_enumListWeightedList.begin(); iter != m_enumListWeightedList.end(); ++iter)
			{
				delete *iter;
				*iter = NULL;
			}
			m_enumListWeightedListAppend = file.read_bool8();
			int listCount = file.read_int32();
			for (int j = 0; j < listCount; ++j)
			{
				CompilerIntegerParam * newData = new CompilerIntegerParam;
				newData->loadFromIff(file);
				m_enumListWeightedList.push_back(newData);
			}
			m_enumListWeightedListLoaded = true;
		}
		else if (strcmp(paramName, "stringIdDerived") == 0)
			m_stringIdDerived.loadFromIff(file);
		else if (strcmp(paramName, "stringIdSimple") == 0)
			m_stringIdSimple.loadFromIff(file);
		else if (strcmp(paramName, "stringIdWeightedList") == 0)
			m_stringIdWeightedList.loadFromIff(file);
		else if (strcmp(paramName, "stringIdListSimple") == 0)
		{
			std::vector<StringIdParam *>::iterator iter;
			for (iter = m_stringIdListSimple.begin(); iter != m_stringIdListSimple.end(); ++iter)
			{
				delete *iter;
				*iter = NULL;
			}
			m_stringIdListSimpleAppend = file.read_bool8();
			int listCount = file.read_int32();
			for (int j = 0; j < listCount; ++j)
			{
				StringIdParam * newData = new StringIdParam;
				newData->loadFromIff(file);
				m_stringIdListSimple.push_back(newData);
			}
			m_stringIdListSimpleLoaded = true;
		}
		else if (strcmp(paramName, "stringIdListWeightedList") == 0)
		{
			std::vector<StringIdParam *>::iterator iter;
			for (iter = m_stringIdListWeightedList.begin(); iter != m_stringIdListWeightedList.end(); ++iter)
			{
				delete *iter;
				*iter = NULL;
			}
			m_stringIdListWeightedListAppend = file.read_bool8();
			int listCount = file.read_int32();
			for (int j = 0; j < listCount; ++j)
			{
				StringIdParam * newData = new StringIdParam;
				newData->loadFromIff(file);
				m_stringIdListWeightedList.push_back(newData);
			}
			m_stringIdListWeightedListLoaded = true;
		}
		else if (strcmp(paramName, "stringDerived") == 0)
			m_stringDerived.loadFromIff(file);
		else if (strcmp(paramName, "stringSimple") == 0)
			m_stringSimple.loadFromIff(file);
		else if (strcmp(paramName, "stringWeightedList") == 0)
			m_stringWeightedList.loadFromIff(file);
		else if (strcmp(paramName, "stringListSimple") == 0)
		{
			std::vector<StringParam *>::iterator iter;
			for (iter = m_stringListSimple.begin(); iter != m_stringListSimple.end(); ++iter)
			{
				delete *iter;
				*iter = NULL;
			}
			m_stringListSimpleAppend = file.read_bool8();
			int listCount = file.read_int32();
			for (int j = 0; j < listCount; ++j)
			{
				StringParam * newData = new StringParam;
				newData->loadFromIff(file);
				m_stringListSimple.push_back(newData);
			}
			m_stringListSimpleLoaded = true;
		}
		else if (strcmp(paramName, "stringListWeightedList") == 0)
		{
			std::vector<StringParam *>::iterator iter;
			for (iter = m_stringListWeightedList.begin(); iter != m_stringListWeightedList.end(); ++iter)
			{
				delete *iter;
				*iter = NULL;
			}
			m_stringListWeightedListAppend = file.read_bool8();
			int listCount = file.read_int32();
			for (int j = 0; j < listCount; ++j)
			{
				StringParam * newData = new StringParam;
				newData->loadFromIff(file);
				m_stringListWeightedList.push_back(newData);
			}
			m_stringListWeightedListLoaded = true;
		}
		else if (strcmp(paramName, "triggerVolumeDerived") == 0)
			m_triggerVolumeDerived.loadFromIff(file);
		else if (strcmp(paramName, "triggerVolumeSimple") == 0)
			m_triggerVolumeSimple.loadFromIff(file);
		else if (strcmp(paramName, "triggerVolumeWeightedList") == 0)
			m_triggerVolumeWeightedList.loadFromIff(file);
		else if (strcmp(paramName, "triggerVolumeList") == 0)
		{
			std::vector<TriggerVolumeParam *>::iterator iter;
			for (iter = m_triggerVolumeList.begin(); iter != m_triggerVolumeList.end(); ++iter)
			{
				delete *iter;
				*iter = NULL;
			}
			m_triggerVolumeListAppend = file.read_bool8();
			int listCount = file.read_int32();
			for (int j = 0; j < listCount; ++j)
			{
				TriggerVolumeParam * newData = new TriggerVolumeParam;
				newData->loadFromIff(file);
				m_triggerVolumeList.push_back(newData);
			}
			m_triggerVolumeListLoaded = true;
		}
		else if (strcmp(paramName, "triggerVolumesListWeightedList") == 0)
		{
			std::vector<TriggerVolumeParam *>::iterator iter;
			for (iter = m_triggerVolumesListWeightedList.begin(); iter != m_triggerVolumesListWeightedList.end(); ++iter)
			{
				delete *iter;
				*iter = NULL;
			}
			m_triggerVolumesListWeightedListAppend = file.read_bool8();
			int listCount = file.read_int32();
			for (int j = 0; j < listCount; ++j)
			{
				TriggerVolumeParam * newData = new TriggerVolumeParam;
				newData->loadFromIff(file);
				m_triggerVolumesListWeightedList.push_back(newData);
			}
			m_triggerVolumesListWeightedListLoaded = true;
		}
		else if (strcmp(paramName, "boolDerived") == 0)
			m_boolDerived.loadFromIff(file);
		else if (strcmp(paramName, "boolSimple") == 0)
			m_boolSimple.loadFromIff(file);
		else if (strcmp(paramName, "boolWeightedList") == 0)
			m_boolWeightedList.loadFromIff(file);
		else if (strcmp(paramName, "boolListDerived") == 0)
		{
			std::vector<BoolParam *>::iterator iter;
			for (iter = m_boolListDerived.begin(); iter != m_boolListDerived.end(); ++iter)
			{
				delete *iter;
				*iter = NULL;
			}
			m_boolListDerivedAppend = file.read_bool8();
			int listCount = file.read_int32();
			for (int j = 0; j < listCount; ++j)
			{
				BoolParam * newData = new BoolParam;
				newData->loadFromIff(file);
				m_boolListDerived.push_back(newData);
			}
			m_boolListDerivedLoaded = true;
		}
		else if (strcmp(paramName, "boolListSimple") == 0)
		{
			std::vector<BoolParam *>::iterator iter;
			for (iter = m_boolListSimple.begin(); iter != m_boolListSimple.end(); ++iter)
			{
				delete *iter;
				*iter = NULL;
			}
			m_boolListSimpleAppend = file.read_bool8();
			int listCount = file.read_int32();
			for (int j = 0; j < listCount; ++j)
			{
				BoolParam * newData = new BoolParam;
				newData->loadFromIff(file);
				m_boolListSimple.push_back(newData);
			}
			m_boolListSimpleLoaded = true;
		}
		else if (strcmp(paramName, "boolListWeightedList") == 0)
		{
			std::vector<BoolParam *>::iterator iter;
			for (iter = m_boolListWeightedList.begin(); iter != m_boolListWeightedList.end(); ++iter)
			{
				delete *iter;
				*iter = NULL;
			}
			m_boolListWeightedListAppend = file.read_bool8();
			int listCount = file.read_int32();
			for (int j = 0; j < listCount; ++j)
			{
				BoolParam * newData = new BoolParam;
				newData->loadFromIff(file);
				m_boolListWeightedList.push_back(newData);
			}
			m_boolListWeightedListLoaded = true;
		}
		else if (strcmp(paramName, "vectorAtDerived") == 0)
			m_vectorAtDerived.loadFromIff(file);
		else if (strcmp(paramName, "vectorSimple") == 0)
			m_vectorSimple.loadFromIff(file);
		else if (strcmp(paramName, "vectorListSimple") == 0)
		{
			std::vector<VectorParam *>::iterator iter;
			for (iter = m_vectorListSimple.begin(); iter != m_vectorListSimple.end(); ++iter)
			{
				delete *iter;
				*iter = NULL;
			}
			m_vectorListSimpleAppend = file.read_bool8();
			int listCount = file.read_int32();
			for (int j = 0; j < listCount; ++j)
			{
				VectorParam * newData = new VectorParam;
				newData->loadFromIff(file);
				m_vectorListSimple.push_back(newData);
			}
			m_vectorListSimpleLoaded = true;
		}
		else if (strcmp(paramName, "filenameAtDerived") == 0)
			m_filenameAtDerived.loadFromIff(file);
		else if (strcmp(paramName, "filenameSimple") == 0)
			m_filenameSimple.loadFromIff(file);
		else if (strcmp(paramName, "filenameWeightedList") == 0)
			m_filenameWeightedList.loadFromIff(file);
		else if (strcmp(paramName, "filenameListSimple") == 0)
		{
			std::vector<StringParam *>::iterator iter;
			for (iter = m_filenameListSimple.begin(); iter != m_filenameListSimple.end(); ++iter)
			{
				delete *iter;
				*iter = NULL;
			}
			m_filenameListSimpleAppend = file.read_bool8();
			int listCount = file.read_int32();
			for (int j = 0; j < listCount; ++j)
			{
				StringParam * newData = new StringParam;
				newData->loadFromIff(file);
				m_filenameListSimple.push_back(newData);
			}
			m_filenameListSimpleLoaded = true;
		}
		else if (strcmp(paramName, "objvarDerived") == 0)
			m_objvarDerived.loadFromIff(file);
		else if (strcmp(paramName, "objvarSimple") == 0)
			m_objvarSimple.loadFromIff(file);
		else if (strcmp(paramName, "templateDerived") == 0)
			m_templateDerived.loadFromIff(file);
		else if (strcmp(paramName, "templateSimple") == 0)
			m_templateSimple.loadFromIff(file);
		else if (strcmp(paramName, "templateWeightedList") == 0)
			m_templateWeightedList.loadFromIff(file);
		else if (strcmp(paramName, "templateListSimple") == 0)
		{
			std::vector<StringParam *>::iterator iter;
			for (iter = m_templateListSimple.begin(); iter != m_templateListSimple.end(); ++iter)
			{
				delete *iter;
				*iter = NULL;
			}
			m_templateListSimpleAppend = file.read_bool8();
			int listCount = file.read_int32();
			for (int j = 0; j < listCount; ++j)
			{
				StringParam * newData = new StringParam;
				newData->loadFromIff(file);
				m_templateListSimple.push_back(newData);
			}
			m_templateListSimpleLoaded = true;
		}
		else if (strcmp(paramName, "structAtDerived") == 0)
			m_structAtDerived.loadFromIff(file);
		else if (strcmp(paramName, "structSimple") == 0)
			m_structSimple.loadFromIff(file);
		else if (strcmp(paramName, "structListSimple") == 0)
		{
			std::vector<StructParamOT *>::iterator iter;
			for (iter = m_structListSimple.begin(); iter != m_structListSimple.end(); ++iter)
			{
				delete *iter;
				*iter = NULL;
			}
			m_structListSimpleAppend = file.read_bool8();
			int listCount = file.read_int32();
			for (int j = 0; j < listCount; ++j)
			{
				StructParamOT * newData = new StructParamOT;
				newData->loadFromIff(file);
				m_structListSimple.push_back(newData);
			}
			m_structListSimpleLoaded = true;
		}
		else if (strcmp(paramName, "structArrayEnum") == 0)
		{
			int listCount = file.read_int32();
			DEBUG_WARNING(listCount != 3, ("read array size of %d for an array of size 3, reading values anyway", listCount));
			int j;
			for (j = 0; j < 3 && j < listCount; ++j)
				m_structArrayEnum[j].loadFromIff(file);
			// if there are more params for structArrayEnum read and dump them
			for (; j < listCount; ++j)
			{
				StructParamOT dummy;
				dummy.loadFromIff(file);
			}
		}
		else if (strcmp(paramName, "structArrayInteger") == 0)
		{
			int listCount = file.read_int32();
			DEBUG_WARNING(listCount != 2, ("read array size of %d for an array of size 2, reading values anyway", listCount));
			int j;
			for (j = 0; j < 2 && j < listCount; ++j)
				m_structArrayInteger[j].loadFromIff(file);
			// if there are more params for structArrayInteger read and dump them
			for (; j < listCount; ++j)
			{
				StructParamOT dummy;
				dummy.loadFromIff(file);
			}
		}
		else if (strcmp(paramName, "integerArray") == 0)
		{
			int listCount = file.read_int32();
			DEBUG_WARNING(listCount != 2, ("read array size of %d for an array of size 2, reading values anyway", listCount));
			int j;
			for (j = 0; j < 2 && j < listCount; ++j)
				m_integerArray[j].loadFromIff(file);
			// if there are more params for integerArray read and dump them
			for (; j < listCount; ++j)
			{
				CompilerIntegerParam dummy;
				dummy.loadFromIff(file);
			}
		}
		else if (strcmp(paramName, "floatArray") == 0)
		{
			int listCount = file.read_int32();
			DEBUG_WARNING(listCount != 2, ("read array size of %d for an array of size 2, reading values anyway", listCount));
			int j;
			for (j = 0; j < 2 && j < listCount; ++j)
				m_floatArray[j].loadFromIff(file);
			// if there are more params for floatArray read and dump them
			for (; j < listCount; ++j)
			{
				FloatParam dummy;
				dummy.loadFromIff(file);
			}
		}
		else if (strcmp(paramName, "stringArray") == 0)
		{
			int listCount = file.read_int32();
			DEBUG_WARNING(listCount != 2, ("read array size of %d for an array of size 2, reading values anyway", listCount));
			int j;
			for (j = 0; j < 2 && j < listCount; ++j)
				m_stringArray[j].loadFromIff(file);
			// if there are more params for stringArray read and dump them
			for (; j < listCount; ++j)
			{
				StringParam dummy;
				dummy.loadFromIff(file);
			}
		}
		else if (strcmp(paramName, "boolArray") == 0)
		{
			int listCount = file.read_int32();
			DEBUG_WARNING(listCount != 2, ("read array size of %d for an array of size 2, reading values anyway", listCount));
			int j;
			for (j = 0; j < 2 && j < listCount; ++j)
				m_boolArray[j].loadFromIff(file);
			// if there are more params for boolArray read and dump them
			for (; j < listCount; ++j)
			{
				BoolParam dummy;
				dummy.loadFromIff(file);
			}
		}
		else if (strcmp(paramName, "stringIdArray") == 0)
		{
			int listCount = file.read_int32();
			DEBUG_WARNING(listCount != 2, ("read array size of %d for an array of size 2, reading values anyway", listCount));
			int j;
			for (j = 0; j < 2 && j < listCount; ++j)
				m_stringIdArray[j].loadFromIff(file);
			// if there are more params for stringIdArray read and dump them
			for (; j < listCount; ++j)
			{
				StringIdParam dummy;
				dummy.loadFromIff(file);
			}
		}
		else if (strcmp(paramName, "triggerArray") == 0)
		{
			int listCount = file.read_int32();
			DEBUG_WARNING(listCount != 2, ("read array size of %d for an array of size 2, reading values anyway", listCount));
			int j;
			for (j = 0; j < 2 && j < listCount; ++j)
				m_triggerArray[j].loadFromIff(file);
			// if there are more params for triggerArray read and dump them
			for (; j < listCount; ++j)
			{
				TriggerVolumeParam dummy;
				dummy.loadFromIff(file);
			}
		}
		else if (strcmp(paramName, "vectorArray") == 0)
		{
			int listCount = file.read_int32();
			DEBUG_WARNING(listCount != 2, ("read array size of %d for an array of size 2, reading values anyway", listCount));
			int j;
			for (j = 0; j < 2 && j < listCount; ++j)
				m_vectorArray[j].loadFromIff(file);
			// if there are more params for vectorArray read and dump them
			for (; j < listCount; ++j)
			{
				VectorParam dummy;
				dummy.loadFromIff(file);
			}
		}
		else if (strcmp(paramName, "fileNameArray") == 0)
		{
			int listCount = file.read_int32();
			DEBUG_WARNING(listCount != 2, ("read array size of %d for an array of size 2, reading values anyway", listCount));
			int j;
			for (j = 0; j < 2 && j < listCount; ++j)
				m_fileNameArray[j].loadFromIff(file);
			// if there are more params for fileNameArray read and dump them
			for (; j < listCount; ++j)
			{
				StringParam dummy;
				dummy.loadFromIff(file);
			}
		}
		file.exitChunk(true);
	}

	file.exitForm();
	TpfTemplate::load(file);
	file.exitForm();
	return;
}	// ServerUberObjectTemplate::load

/**
 * Saves the template data to an iff file.
 *
 * @param file			file to save to
 * @param location		file type (client or server)
 */
void ServerUberObjectTemplate::save(Iff &file)
{
int count;

	file.insertForm(ServerUberObjectTemplate_tag);
	if (m_baseTemplateName.size() != 0)
	{
		file.insertForm(TAG(D,E,R,V));
		file.insertChunk(TAG(X, X, X, X));
		file.insertChunkData(m_baseTemplateName.c_str(), m_baseTemplateName.size() + 1);
		file.exitChunk();
		file.exitForm();
	}
	file.insertForm(TAG(0,0,0,0));
	file.allowNonlinearFunctions();

	int paramCount = 0;

	// save intAtDerived
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("intAtDerived");
	m_intAtDerived.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save intSimple
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("intSimple");
	m_intSimple.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save intSimpleDeltaPositive
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("intSimpleDeltaPositive");
	m_intSimpleDeltaPositive.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save intSimpleDeltaNegative
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("intSimpleDeltaNegative");
	m_intSimpleDeltaNegative.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save intSimpleDeltaPositivePercent
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("intSimpleDeltaPositivePercent");
	m_intSimpleDeltaPositivePercent.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save intSimpleDeltaNegativePercent
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("intSimpleDeltaNegativePercent");
	m_intSimpleDeltaNegativePercent.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save intWeightedList
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("intWeightedList");
	m_intWeightedList.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save intWeightedListDeltaPositive
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("intWeightedListDeltaPositive");
	m_intWeightedListDeltaPositive.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save intWeightedListDeltaNegative
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("intWeightedListDeltaNegative");
	m_intWeightedListDeltaNegative.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save intWeightedListDeltaPositivePercent
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("intWeightedListDeltaPositivePercent");
	m_intWeightedListDeltaPositivePercent.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save intWeightedListDeltaNegativePercent
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("intWeightedListDeltaNegativePercent");
	m_intWeightedListDeltaNegativePercent.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save intRandomRange1
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("intRandomRange1");
	m_intRandomRange1.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save intRandomRange2
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("intRandomRange2");
	m_intRandomRange2.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save intRandomRange3
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("intRandomRange3");
	m_intRandomRange3.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save intRandomRange4
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("intRandomRange4");
	m_intRandomRange4.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save intDiceRoll1
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("intDiceRoll1");
	m_intDiceRoll1.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save intDiceRoll2
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("intDiceRoll2");
	m_intDiceRoll2.saveToIff(file);
	file.exitChunk();
	++paramCount;
	if (!m_intListSimpleLoaded)
	{
		// mark the list as empty and extending the base list
		m_intListSimpleAppend = true;
	}
	// save intListSimple
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("intListSimple");
	file.insertChunkData(&m_intListSimpleAppend, sizeof(bool));
	count = m_intListSimple.size();
	file.insertChunkData(&count, sizeof(count));
	{for (int i = 0; i < count; ++i)
		m_intListSimple[i]->saveToIff(file);}
	file.exitChunk();
	++paramCount;
	if (!m_intListWeightedListLoaded)
	{
		// mark the list as empty and extending the base list
		m_intListWeightedListAppend = true;
	}
	// save intListWeightedList
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("intListWeightedList");
	file.insertChunkData(&m_intListWeightedListAppend, sizeof(bool));
	count = m_intListWeightedList.size();
	file.insertChunkData(&count, sizeof(count));
	{for (int i = 0; i < count; ++i)
		m_intListWeightedList[i]->saveToIff(file);}
	file.exitChunk();
	++paramCount;
	if (!m_intListRandomRangeLoaded)
	{
		// mark the list as empty and extending the base list
		m_intListRandomRangeAppend = true;
	}
	// save intListRandomRange
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("intListRandomRange");
	file.insertChunkData(&m_intListRandomRangeAppend, sizeof(bool));
	count = m_intListRandomRange.size();
	file.insertChunkData(&count, sizeof(count));
	{for (int i = 0; i < count; ++i)
		m_intListRandomRange[i]->saveToIff(file);}
	file.exitChunk();
	++paramCount;
	if (!m_intListDiceRollLoaded)
	{
		// mark the list as empty and extending the base list
		m_intListDiceRollAppend = true;
	}
	// save intListDiceRoll
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("intListDiceRoll");
	file.insertChunkData(&m_intListDiceRollAppend, sizeof(bool));
	count = m_intListDiceRoll.size();
	file.insertChunkData(&count, sizeof(count));
	{for (int i = 0; i < count; ++i)
		m_intListDiceRoll[i]->saveToIff(file);}
	file.exitChunk();
	++paramCount;
	// save floatAtDerived
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("floatAtDerived");
	m_floatAtDerived.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save floatSimple
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("floatSimple");
	m_floatSimple.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save floatSimpleDeltaPositive
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("floatSimpleDeltaPositive");
	m_floatSimpleDeltaPositive.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save floatSimpleDeltaNegative
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("floatSimpleDeltaNegative");
	m_floatSimpleDeltaNegative.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save floatSimpleDeltaPositivePercent
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("floatSimpleDeltaPositivePercent");
	m_floatSimpleDeltaPositivePercent.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save floatSimpleDeltaNegativePercent
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("floatSimpleDeltaNegativePercent");
	m_floatSimpleDeltaNegativePercent.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save floatWeightedList
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("floatWeightedList");
	m_floatWeightedList.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save floatRandomRange1
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("floatRandomRange1");
	m_floatRandomRange1.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save floatRandomRange2
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("floatRandomRange2");
	m_floatRandomRange2.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save floatRandomRange3
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("floatRandomRange3");
	m_floatRandomRange3.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save floatRandomRange4
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("floatRandomRange4");
	m_floatRandomRange4.saveToIff(file);
	file.exitChunk();
	++paramCount;
	if (!m_floatListSimpleLoaded)
	{
		// mark the list as empty and extending the base list
		m_floatListSimpleAppend = true;
	}
	// save floatListSimple
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("floatListSimple");
	file.insertChunkData(&m_floatListSimpleAppend, sizeof(bool));
	count = m_floatListSimple.size();
	file.insertChunkData(&count, sizeof(count));
	{for (int i = 0; i < count; ++i)
		m_floatListSimple[i]->saveToIff(file);}
	file.exitChunk();
	++paramCount;
	if (!m_floatListWeightedListLoaded)
	{
		// mark the list as empty and extending the base list
		m_floatListWeightedListAppend = true;
	}
	// save floatListWeightedList
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("floatListWeightedList");
	file.insertChunkData(&m_floatListWeightedListAppend, sizeof(bool));
	count = m_floatListWeightedList.size();
	file.insertChunkData(&count, sizeof(count));
	{for (int i = 0; i < count; ++i)
		m_floatListWeightedList[i]->saveToIff(file);}
	file.exitChunk();
	++paramCount;
	if (!m_floatListRandomRangeLoaded)
	{
		// mark the list as empty and extending the base list
		m_floatListRandomRangeAppend = true;
	}
	// save floatListRandomRange
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("floatListRandomRange");
	file.insertChunkData(&m_floatListRandomRangeAppend, sizeof(bool));
	count = m_floatListRandomRange.size();
	file.insertChunkData(&count, sizeof(count));
	{for (int i = 0; i < count; ++i)
		m_floatListRandomRange[i]->saveToIff(file);}
	file.exitChunk();
	++paramCount;
	// save enumIndexedByEnumSingle
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("enumIndexedByEnumSingle");
	count = 3;
	file.insertChunkData(&count, sizeof(count));
	{for (int i = 0; i < 3; ++i)
		m_enumIndexedByEnumSingle[i].saveToIff(file);}
	file.exitChunk();
	++paramCount;
	// save enumIndexedByEnumWeightedList
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("enumIndexedByEnumWeightedList");
	count = 3;
	file.insertChunkData(&count, sizeof(count));
	{for (int i = 0; i < 3; ++i)
		m_enumIndexedByEnumWeightedList[i].saveToIff(file);}
	file.exitChunk();
	++paramCount;
	if (!m_enumListIndexedLoaded)
	{
		// mark the list as empty and extending the base list
		m_enumListIndexedAppend = true;
	}
	// save enumListIndexed
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("enumListIndexed");
	file.insertChunkData(&m_enumListIndexedAppend, sizeof(bool));
	count = m_enumListIndexed.size();
	file.insertChunkData(&count, sizeof(count));
	{for (int i = 0; i < count; ++i)
		m_enumListIndexed[i]->saveToIff(file);}
	file.exitChunk();
	++paramCount;
	if (!m_enumListWeightedListLoaded)
	{
		// mark the list as empty and extending the base list
		m_enumListWeightedListAppend = true;
	}
	// save enumListWeightedList
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("enumListWeightedList");
	file.insertChunkData(&m_enumListWeightedListAppend, sizeof(bool));
	count = m_enumListWeightedList.size();
	file.insertChunkData(&count, sizeof(count));
	{for (int i = 0; i < count; ++i)
		m_enumListWeightedList[i]->saveToIff(file);}
	file.exitChunk();
	++paramCount;
	// save stringIdDerived
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("stringIdDerived");
	m_stringIdDerived.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save stringIdSimple
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("stringIdSimple");
	m_stringIdSimple.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save stringIdWeightedList
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("stringIdWeightedList");
	m_stringIdWeightedList.saveToIff(file);
	file.exitChunk();
	++paramCount;
	if (!m_stringIdListSimpleLoaded)
	{
		// mark the list as empty and extending the base list
		m_stringIdListSimpleAppend = true;
	}
	// save stringIdListSimple
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("stringIdListSimple");
	file.insertChunkData(&m_stringIdListSimpleAppend, sizeof(bool));
	count = m_stringIdListSimple.size();
	file.insertChunkData(&count, sizeof(count));
	{for (int i = 0; i < count; ++i)
		m_stringIdListSimple[i]->saveToIff(file);}
	file.exitChunk();
	++paramCount;
	if (!m_stringIdListWeightedListLoaded)
	{
		// mark the list as empty and extending the base list
		m_stringIdListWeightedListAppend = true;
	}
	// save stringIdListWeightedList
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("stringIdListWeightedList");
	file.insertChunkData(&m_stringIdListWeightedListAppend, sizeof(bool));
	count = m_stringIdListWeightedList.size();
	file.insertChunkData(&count, sizeof(count));
	{for (int i = 0; i < count; ++i)
		m_stringIdListWeightedList[i]->saveToIff(file);}
	file.exitChunk();
	++paramCount;
	// save stringDerived
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("stringDerived");
	m_stringDerived.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save stringSimple
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("stringSimple");
	m_stringSimple.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save stringWeightedList
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("stringWeightedList");
	m_stringWeightedList.saveToIff(file);
	file.exitChunk();
	++paramCount;
	if (!m_stringListSimpleLoaded)
	{
		// mark the list as empty and extending the base list
		m_stringListSimpleAppend = true;
	}
	// save stringListSimple
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("stringListSimple");
	file.insertChunkData(&m_stringListSimpleAppend, sizeof(bool));
	count = m_stringListSimple.size();
	file.insertChunkData(&count, sizeof(count));
	{for (int i = 0; i < count; ++i)
		m_stringListSimple[i]->saveToIff(file);}
	file.exitChunk();
	++paramCount;
	if (!m_stringListWeightedListLoaded)
	{
		// mark the list as empty and extending the base list
		m_stringListWeightedListAppend = true;
	}
	// save stringListWeightedList
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("stringListWeightedList");
	file.insertChunkData(&m_stringListWeightedListAppend, sizeof(bool));
	count = m_stringListWeightedList.size();
	file.insertChunkData(&count, sizeof(count));
	{for (int i = 0; i < count; ++i)
		m_stringListWeightedList[i]->saveToIff(file);}
	file.exitChunk();
	++paramCount;
	// save triggerVolumeDerived
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("triggerVolumeDerived");
	m_triggerVolumeDerived.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save triggerVolumeSimple
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("triggerVolumeSimple");
	m_triggerVolumeSimple.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save triggerVolumeWeightedList
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("triggerVolumeWeightedList");
	m_triggerVolumeWeightedList.saveToIff(file);
	file.exitChunk();
	++paramCount;
	if (!m_triggerVolumeListLoaded)
	{
		// mark the list as empty and extending the base list
		m_triggerVolumeListAppend = true;
	}
	// save triggerVolumeList
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("triggerVolumeList");
	file.insertChunkData(&m_triggerVolumeListAppend, sizeof(bool));
	count = m_triggerVolumeList.size();
	file.insertChunkData(&count, sizeof(count));
	{for (int i = 0; i < count; ++i)
		m_triggerVolumeList[i]->saveToIff(file);}
	file.exitChunk();
	++paramCount;
	if (!m_triggerVolumesListWeightedListLoaded)
	{
		// mark the list as empty and extending the base list
		m_triggerVolumesListWeightedListAppend = true;
	}
	// save triggerVolumesListWeightedList
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("triggerVolumesListWeightedList");
	file.insertChunkData(&m_triggerVolumesListWeightedListAppend, sizeof(bool));
	count = m_triggerVolumesListWeightedList.size();
	file.insertChunkData(&count, sizeof(count));
	{for (int i = 0; i < count; ++i)
		m_triggerVolumesListWeightedList[i]->saveToIff(file);}
	file.exitChunk();
	++paramCount;
	// save boolDerived
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("boolDerived");
	m_boolDerived.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save boolSimple
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("boolSimple");
	m_boolSimple.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save boolWeightedList
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("boolWeightedList");
	m_boolWeightedList.saveToIff(file);
	file.exitChunk();
	++paramCount;
	if (!m_boolListDerivedLoaded)
	{
		// mark the list as empty and extending the base list
		m_boolListDerivedAppend = true;
	}
	// save boolListDerived
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("boolListDerived");
	file.insertChunkData(&m_boolListDerivedAppend, sizeof(bool));
	count = m_boolListDerived.size();
	file.insertChunkData(&count, sizeof(count));
	{for (int i = 0; i < count; ++i)
		m_boolListDerived[i]->saveToIff(file);}
	file.exitChunk();
	++paramCount;
	if (!m_boolListSimpleLoaded)
	{
		// mark the list as empty and extending the base list
		m_boolListSimpleAppend = true;
	}
	// save boolListSimple
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("boolListSimple");
	file.insertChunkData(&m_boolListSimpleAppend, sizeof(bool));
	count = m_boolListSimple.size();
	file.insertChunkData(&count, sizeof(count));
	{for (int i = 0; i < count; ++i)
		m_boolListSimple[i]->saveToIff(file);}
	file.exitChunk();
	++paramCount;
	if (!m_boolListWeightedListLoaded)
	{
		// mark the list as empty and extending the base list
		m_boolListWeightedListAppend = true;
	}
	// save boolListWeightedList
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("boolListWeightedList");
	file.insertChunkData(&m_boolListWeightedListAppend, sizeof(bool));
	count = m_boolListWeightedList.size();
	file.insertChunkData(&count, sizeof(count));
	{for (int i = 0; i < count; ++i)
		m_boolListWeightedList[i]->saveToIff(file);}
	file.exitChunk();
	++paramCount;
	// save vectorAtDerived
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("vectorAtDerived");
	m_vectorAtDerived.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save vectorSimple
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("vectorSimple");
	m_vectorSimple.saveToIff(file);
	file.exitChunk();
	++paramCount;
	if (!m_vectorListSimpleLoaded)
	{
		// mark the list as empty and extending the base list
		m_vectorListSimpleAppend = true;
	}
	// save vectorListSimple
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("vectorListSimple");
	file.insertChunkData(&m_vectorListSimpleAppend, sizeof(bool));
	count = m_vectorListSimple.size();
	file.insertChunkData(&count, sizeof(count));
	{for (int i = 0; i < count; ++i)
		m_vectorListSimple[i]->saveToIff(file);}
	file.exitChunk();
	++paramCount;
	// save filenameAtDerived
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("filenameAtDerived");
	m_filenameAtDerived.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save filenameSimple
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("filenameSimple");
	m_filenameSimple.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save filenameWeightedList
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("filenameWeightedList");
	m_filenameWeightedList.saveToIff(file);
	file.exitChunk();
	++paramCount;
	if (!m_filenameListSimpleLoaded)
	{
		// mark the list as empty and extending the base list
		m_filenameListSimpleAppend = true;
	}
	// save filenameListSimple
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("filenameListSimple");
	file.insertChunkData(&m_filenameListSimpleAppend, sizeof(bool));
	count = m_filenameListSimple.size();
	file.insertChunkData(&count, sizeof(count));
	{for (int i = 0; i < count; ++i)
		m_filenameListSimple[i]->saveToIff(file);}
	file.exitChunk();
	++paramCount;
	// save objvarDerived
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("objvarDerived");
	m_objvarDerived.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save objvarSimple
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("objvarSimple");
	m_objvarSimple.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save templateDerived
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("templateDerived");
	m_templateDerived.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save templateSimple
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("templateSimple");
	m_templateSimple.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save templateWeightedList
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("templateWeightedList");
	m_templateWeightedList.saveToIff(file);
	file.exitChunk();
	++paramCount;
	if (!m_templateListSimpleLoaded)
	{
		// mark the list as empty and extending the base list
		m_templateListSimpleAppend = true;
	}
	// save templateListSimple
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("templateListSimple");
	file.insertChunkData(&m_templateListSimpleAppend, sizeof(bool));
	count = m_templateListSimple.size();
	file.insertChunkData(&count, sizeof(count));
	{for (int i = 0; i < count; ++i)
		m_templateListSimple[i]->saveToIff(file);}
	file.exitChunk();
	++paramCount;
	// save structAtDerived
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("structAtDerived");
	m_structAtDerived.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save structSimple
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("structSimple");
	m_structSimple.saveToIff(file);
	file.exitChunk();
	++paramCount;
	if (!m_structListSimpleLoaded)
	{
		// mark the list as empty and extending the base list
		m_structListSimpleAppend = true;
	}
	// save structListSimple
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("structListSimple");
	file.insertChunkData(&m_structListSimpleAppend, sizeof(bool));
	count = m_structListSimple.size();
	file.insertChunkData(&count, sizeof(count));
	{for (int i = 0; i < count; ++i)
		m_structListSimple[i]->saveToIff(file);}
	file.exitChunk();
	++paramCount;
	// save structArrayEnum
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("structArrayEnum");
	count = 3;
	file.insertChunkData(&count, sizeof(count));
	{for (int i = 0; i < 3; ++i)
		m_structArrayEnum[i].saveToIff(file);}
	file.exitChunk();
	++paramCount;
	// save structArrayInteger
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("structArrayInteger");
	count = 2;
	file.insertChunkData(&count, sizeof(count));
	{for (int i = 0; i < 2; ++i)
		m_structArrayInteger[i].saveToIff(file);}
	file.exitChunk();
	++paramCount;
	// save integerArray
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("integerArray");
	count = 2;
	file.insertChunkData(&count, sizeof(count));
	{for (int i = 0; i < 2; ++i)
		m_integerArray[i].saveToIff(file);}
	file.exitChunk();
	++paramCount;
	// save floatArray
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("floatArray");
	count = 2;
	file.insertChunkData(&count, sizeof(count));
	{for (int i = 0; i < 2; ++i)
		m_floatArray[i].saveToIff(file);}
	file.exitChunk();
	++paramCount;
	// save stringArray
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("stringArray");
	count = 2;
	file.insertChunkData(&count, sizeof(count));
	{for (int i = 0; i < 2; ++i)
		m_stringArray[i].saveToIff(file);}
	file.exitChunk();
	++paramCount;
	// save boolArray
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("boolArray");
	count = 2;
	file.insertChunkData(&count, sizeof(count));
	{for (int i = 0; i < 2; ++i)
		m_boolArray[i].saveToIff(file);}
	file.exitChunk();
	++paramCount;
	// save stringIdArray
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("stringIdArray");
	count = 2;
	file.insertChunkData(&count, sizeof(count));
	{for (int i = 0; i < 2; ++i)
		m_stringIdArray[i].saveToIff(file);}
	file.exitChunk();
	++paramCount;
	// save triggerArray
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("triggerArray");
	count = 2;
	file.insertChunkData(&count, sizeof(count));
	{for (int i = 0; i < 2; ++i)
		m_triggerArray[i].saveToIff(file);}
	file.exitChunk();
	++paramCount;
	// save vectorArray
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("vectorArray");
	count = 2;
	file.insertChunkData(&count, sizeof(count));
	{for (int i = 0; i < 2; ++i)
		m_vectorArray[i].saveToIff(file);}
	file.exitChunk();
	++paramCount;
	// save fileNameArray
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("fileNameArray");
	count = 2;
	file.insertChunkData(&count, sizeof(count));
	{for (int i = 0; i < 2; ++i)
		m_fileNameArray[i].saveToIff(file);}
	file.exitChunk();
	++paramCount;

	// write number of parameters
	file.goToTopOfForm();
	file.insertChunk(TAG(P, C, N, T));
	file.insertChunkData(&paramCount, sizeof(paramCount));
	file.exitChunk();

	file.exitForm(true);
	TpfTemplate::save(file);
	file.exitForm();
}	// ServerUberObjectTemplate::save


//=============================================================================
// class ServerUberObjectTemplate::_Foo

/**
 * Class constructor.
 */
ServerUberObjectTemplate::_Foo::_Foo(const std::string & filename)
	: TpfTemplate(filename)
{
}	// ServerUberObjectTemplate::_Foo::_Foo

/**
 * Class destructor.
 */
ServerUberObjectTemplate::_Foo::~_Foo()
{
}	// ServerUberObjectTemplate::_Foo::~_Foo

/**
 * Static function used to register this template.
 */
void ServerUberObjectTemplate::_Foo::registerMe(void)
{
	ObjectTemplateList::registerTemplate(_Foo_tag, create);
}	// ServerUberObjectTemplate::_Foo::registerMe

/**
 * Creates a ServerUberObjectTemplate::_Foo template.
 *
 * @return a new instance of the template
 */
ObjectTemplate * ServerUberObjectTemplate::_Foo::create(const std::string & filename)
{
	return new ServerUberObjectTemplate::_Foo(filename);
}	// ServerUberObjectTemplate::_Foo::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag ServerUberObjectTemplate::_Foo::getId(void) const
{
	return _Foo_tag;
}	// ServerUberObjectTemplate::_Foo::getId

CompilerIntegerParam * ServerUberObjectTemplate::_Foo::getCompilerIntegerParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "item1") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != NULL)
					return getBaseTemplate()->getCompilerIntegerParam(name, deepCheck, index);
				return NULL;
			}
			return &m_item1;
		}
		fprintf(stderr, "trying to access single-parameter \"item1\" as an array\n");
	}
	else
		return TpfTemplate::getCompilerIntegerParam(name, deepCheck, index);
	return NULL;
}	//ServerUberObjectTemplate::_Foo::getCompilerIntegerParam

FloatParam * ServerUberObjectTemplate::_Foo::getFloatParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "item2") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != NULL)
					return getBaseTemplate()->getFloatParam(name, deepCheck, index);
				return NULL;
			}
			return &m_item2;
		}
		fprintf(stderr, "trying to access single-parameter \"item2\" as an array\n");
	}
	else
		return TpfTemplate::getFloatParam(name, deepCheck, index);
	return NULL;
}	//ServerUberObjectTemplate::_Foo::getFloatParam

BoolParam * ServerUberObjectTemplate::_Foo::getBoolParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getBoolParam(name, deepCheck, index);
}	//ServerUberObjectTemplate::_Foo::getBoolParam

StringParam * ServerUberObjectTemplate::_Foo::getStringParam(const char *name, bool deepCheck, int index)
{
	if (strcmp(name, "item3") == 0)
	{
		if (index == 0)
		{
			if (deepCheck && !isParamLoaded(name, false, 0))
			{
				if (getBaseTemplate() != NULL)
					return getBaseTemplate()->getStringParam(name, deepCheck, index);
				return NULL;
			}
			return &m_item3;
		}
		fprintf(stderr, "trying to access single-parameter \"item3\" as an array\n");
	}
	else
		return TpfTemplate::getStringParam(name, deepCheck, index);
	return NULL;
}	//ServerUberObjectTemplate::_Foo::getStringParam

StringIdParam * ServerUberObjectTemplate::_Foo::getStringIdParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getStringIdParam(name, deepCheck, index);
}	//ServerUberObjectTemplate::_Foo::getStringIdParam

VectorParam * ServerUberObjectTemplate::_Foo::getVectorParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getVectorParam(name, deepCheck, index);
}	//ServerUberObjectTemplate::_Foo::getVectorParam

DynamicVariableParam * ServerUberObjectTemplate::_Foo::getDynamicVariableParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getDynamicVariableParam(name, deepCheck, index);
}	//ServerUberObjectTemplate::_Foo::getDynamicVariableParam

StructParamOT * ServerUberObjectTemplate::_Foo::getStructParamOT(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getStructParamOT(name, deepCheck, index);
}	//ServerUberObjectTemplate::_Foo::getStructParamOT

TriggerVolumeParam * ServerUberObjectTemplate::_Foo::getTriggerVolumeParam(const char *name, bool deepCheck, int index)
{
	return TpfTemplate::getTriggerVolumeParam(name, deepCheck, index);
}	//ServerUberObjectTemplate::_Foo::getTriggerVolumeParam

void ServerUberObjectTemplate::_Foo::initStructParamOT(StructParamOT &param, const char *name)
{
	if (param.isInitialized())
		return;
	TpfTemplate::initStructParamOT(param, name);
}	// ServerUberObjectTemplate::_Foo::initStructParamOT

void ServerUberObjectTemplate::_Foo::setAsEmptyList(const char *name)
{
	TpfTemplate::setAsEmptyList(name);
}	// ServerUberObjectTemplate::_Foo::setAsEmptyList

void ServerUberObjectTemplate::_Foo::setAppend(const char *name)
{
	TpfTemplate::setAppend(name);
}	// ServerUberObjectTemplate::_Foo::setAppend

bool ServerUberObjectTemplate::_Foo::isAppend(const char *name) const
{
	return TpfTemplate::isAppend(name);
}	// ServerUberObjectTemplate::_Foo::isAppend


int ServerUberObjectTemplate::_Foo::getListLength(const char *name) const
{
	return TpfTemplate::getListLength(name);
}	// ServerUberObjectTemplate::_Foo::getListLength

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void ServerUberObjectTemplate::_Foo::load(Iff &file)
{
static const int MAX_NAME_SIZE = 256;
char paramName[MAX_NAME_SIZE];

	file.enterForm();

	file.enterChunk();
	int paramCount = file.read_int32();
	file.exitChunk();
	for (int i = 0; i < paramCount; ++i)
	{
		file.enterChunk();
		file.read_string(paramName, MAX_NAME_SIZE);
		if (strcmp(paramName, "item1") == 0)
			m_item1.loadFromIff(file);
		else if (strcmp(paramName, "item2") == 0)
			m_item2.loadFromIff(file);
		else if (strcmp(paramName, "item3") == 0)
			m_item3.loadFromIff(file);
		file.exitChunk(true);
	}

	file.exitForm();
	UNREF(file);
}	// ServerUberObjectTemplate::_Foo::load

/**
 * Saves the template data to an iff file.
 *
 * @param file			file to save to
 * @param location		file type (client or server)
 */
void ServerUberObjectTemplate::_Foo::save(Iff &file)
{
int count;

	file.insertForm(_Foo_tag);

	int paramCount = 0;

	// save item1
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("item1");
	m_item1.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save item2
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("item2");
	m_item2.saveToIff(file);
	file.exitChunk();
	++paramCount;
	// save item3
	file.insertChunk(TAG(X, X, X, X));
	file.insertChunkString("item3");
	m_item3.saveToIff(file);
	file.exitChunk();
	++paramCount;

	// write number of parameters
	file.goToTopOfForm();
	file.insertChunk(TAG(P, C, N, T));
	file.insertChunkData(&paramCount, sizeof(paramCount));
	file.exitChunk();

	file.exitForm(true);
	UNREF(count);
}	// ServerUberObjectTemplate::_Foo::save

//@END TFD
