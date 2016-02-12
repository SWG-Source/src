//========================================================================
//
// TemplateDataIterator.cpp
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

// Includes
/////////////////////////////////////

#include "sharedTemplateDefinition/FirstSharedTemplateDefinition.h"

#include "TemplateDataIterator.h"

#include "File.h"
#include "TemplateDefinitionFile.h"

// ============================================================================
//
// TemplateDataIterator class methods
//
// ============================================================================

//-----------------------------------------------------------------------------

TemplateDataIterator::TemplateDataIterator()
: m_templateDataToIterate(nullptr),
  m_numItems(0),
  m_noMoreData(true)
{
}

//-----------------------------------------------------------------------------

TemplateDataIterator::TemplateDataIterator(const TemplateData& templateData)
: m_templateDataToIterate(nullptr),
  m_numItems(0),
  m_noMoreData(true)
{
	setTo(templateData);
}

//-----------------------------------------------------------------------------

TemplateDataIterator::~TemplateDataIterator()
{
	m_templateDataToIterate = nullptr;
}

//-----------------------------------------------------------------------------

void
TemplateDataIterator::setTo(const TemplateData &templateData)
{
	m_listOfParamLists.clear();
	m_templateDataToIterate = &templateData;

	// Loop through the TDF and all of its parents and add all of their
	// .. parameter lists into this iterator's list collection. When we're
	// .. done, the order of parameter lists will start with the base TDF's and
	// .. end at the child TDF's.
	const ParameterList* parameterListToAdd;

	while(m_templateDataToIterate != nullptr)
	{
		parameterListToAdd = &(m_templateDataToIterate->m_parameters);
		m_numItems += parameterListToAdd->size();

		// Occasionally, TDF files are stubbed and have no parameters. Be sure
		// .. not to add them as they just waste our time.
		if(!parameterListToAdd->empty())
		{
			m_listOfParamLists.push_back(parameterListToAdd);
		}

		// Get the template data from the parent TDF
		if(m_templateDataToIterate->m_fileParent != nullptr)
		{
			const TemplateDefinitionFile* parentFile = 
				m_templateDataToIterate->m_fileParent->getBaseDefinitionFile();

			if(parentFile != nullptr)
			{
				m_templateDataToIterate = parentFile->getTemplateData(parentFile->getHighestVersion());
			}
			else
			{
				m_templateDataToIterate = nullptr;
			}
		}
		else
		{
			m_templateDataToIterate = nullptr;
		}

	}

	// Initialize internal iterators
	reset();
}

//-----------------------------------------------------------------------------

bool
TemplateDataIterator::end() const
{
	return (m_noMoreData == true);
}

//-----------------------------------------------------------------------------

void
TemplateDataIterator::next()
{
	// Increment to the next parameter
	++m_currentParamPtr;

	// Have we gone to the end of our current parameter list?
	if(m_currentParamPtr == (*m_currentParamListPtr)->end())
	{
		// Go to the next list. If there are no more lists, then
		// .. we are done iterating. Otherwise, set the current
		// .. parameter to the beginning of the next list.
		++m_currentParamListPtr;

		if(m_currentParamListPtr == m_listOfParamLists.end())
		{
			m_noMoreData = true;
		}
		else
		{
			m_currentParamPtr = (*m_currentParamListPtr)->begin();
		}
	}
}

//-----------------------------------------------------------------------------

int
TemplateDataIterator::size() const
{
	return m_numItems;
}

//-----------------------------------------------------------------------------

void
TemplateDataIterator::reset()
{
	if(m_numItems > 0)
	{
		m_noMoreData = false;
	}

	m_currentParamListPtr = m_listOfParamLists.begin();
	m_currentParamPtr	  = (*m_currentParamListPtr)->begin();
}

//-----------------------------------------------------------------------------

const TemplateData::Parameter *
TemplateDataIterator::operator *() const
{
	if(this->end())
	{
		return nullptr;
	}
	else
	{
		const TemplateData::Parameter *currParam = &(*m_currentParamPtr);

		return currParam;
	}
}

