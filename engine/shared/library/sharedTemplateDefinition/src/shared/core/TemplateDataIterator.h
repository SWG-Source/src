// ============================================================================
//
//	TemplateDataIterator.h
//
//	This class iterates over a TdfFile and provides incremental access to the
//	template data contained within. An iterator is provided because TDF's
//	inherit most of their parameters, but this detail should be transparent
//	to the programmer. Once you want to create a template from a TDF, you don't
//  care where the fields come from -- you just want to know what's available.
//
//	Initialize a TemplateDataIterator to a TemplateData. Iterating on it will
//	provide access to a TemplateData Parameter.
//
//  See also TemplateData, Parameter
//
//  Used by TemplateEditor
//
//	Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef TEMPLATEDATAITERATOR_H
#define TEMPLATEDATAITERATOR_H

#include "TemplateData.h"

class TemplateDataIterator
{
public:

	// Public typedefs and enumerations
	/////////////

	typedef TemplateData::Parameter             Parameter;
	
	// Construction / destruction
	/////////////
	                    TemplateDataIterator();
	explicit            TemplateDataIterator(const TemplateData &templateData);
	                    ~TemplateDataIterator();

	// Iteration 
	/////////////
	void                setTo(const TemplateData &templateData);
	bool                end() const;
	void                next();
	void                reset();

	// Accessors
	/////////////
	int                 size() const;
	const Parameter*    operator *() const;

private:

	// Private typedefs and enumerations
	/////////////

	// Template data's store their parameters in parameter lists
	typedef TemplateData::ParameterList         ParameterList;
	typedef ParameterList::const_iterator       ParameterListItr;

	// Our parameter iterator needs to iterate over lists of parameter lists
	typedef std::list<const ParameterList *>	ParameterListList;
	typedef ParameterListList::const_iterator	ParameterListListItr;

	// Private Data
	/////////////
	const TemplateData  *m_templateDataToIterate;

	ParameterListList    m_listOfParamLists;      // A list of param lists, one from each TDF ancestor.
	ParameterListListItr m_currentParamListPtr;   // Current param list we are iterating through
	ParameterListItr     m_currentParamPtr;       // Current parameter we are iterating on

	int                  m_numItems;
	bool                 m_noMoreData;
};

#endif	// TEMPLATEDATAITERATOR_H
