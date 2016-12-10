//======================================================================
//
// DynamicVariableListNestedList.cpp
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#include "sharedFoundation/FirstSharedFoundation.h"
#include "sharedFoundation/DynamicVariableListNestedList.h"

#include "sharedFoundation/DynamicVariableList.h"

//======================================================================
	
// ======================================================================
// DynamicVariableListNestedList

DynamicVariableListNestedList::DynamicVariableListNestedList (const DynamicVariableList &dvList, const std::string &sublistName) :
		m_list(dvList),
		m_sublistName(sublistName.empty() ? sublistName : sublistName + '.')
{
}

// ----------------------------------------------------------------------

DynamicVariableListNestedList::DynamicVariableListNestedList (const DynamicVariableList &dvList) :
		m_list(dvList),
		m_sublistName()
{
}

// ----------------------------------------------------------------------

DynamicVariableListNestedList::DynamicVariableListNestedList (const DynamicVariableListNestedList &parentList, const std::string &sublistName) :
		m_list(parentList.m_list),
		m_sublistName(parentList.getContextName()+sublistName + '.')
{
	DEBUG_FATAL(sublistName.empty(),("Programmer bug:  passed empty sublist name to NestedList::NestedList.\n"));
}

// ----------------------------------------------------------------------

/**
 * The name of the list, including all parents.
 */
const std::string DynamicVariableListNestedList::getNameWithPath() const
{
	if (m_sublistName.empty())
		return std::string();
	else
		// strip off trailing '.'
		return m_sublistName.substr(0,m_sublistName.length()-1);
}

// ----------------------------------------------------------------------

/**
 * The string to append to get variables in this list.
 */
const std::string &DynamicVariableListNestedList::getContextName() const
{
	return m_sublistName;
}

// ----------------------------------------------------------------------

const std::string DynamicVariableListNestedList::getName() const
{
	if (m_sublistName.empty())
		return std::string();
	
	size_t lastDot = m_sublistName.rfind('.');
	DEBUG_FATAL(lastDot==std::string::npos,("Programmer bug: NestedList name \"%s\" was non-empty but did not a dot.\n",m_sublistName.c_str()));
	size_t nextToLastDot = m_sublistName.rfind('.',lastDot-1);
	return m_sublistName.substr(nextToLastDot+1,lastDot-nextToLastDot-1);
}

// ----------------------------------------------------------------------

int DynamicVariableListNestedList::getCount() const
{
	int count=0;
	for (const_iterator i(begin()); i!=end(); ++i)
		++count;
	return count;
}

// ----------------------------------------------------------------------

bool DynamicVariableListNestedList::empty() const
{
	return (begin() == end());
}

// ----------------------------------------------------------------------

DynamicVariableListNestedList::const_iterator DynamicVariableListNestedList::begin() const
{
	DynamicVariableList::MapType::const_iterator i=m_list.lower_bound(getContextName());
	return const_iterator(*this,i,!isInList(i));
}

// ----------------------------------------------------------------------

DynamicVariableListNestedList::const_iterator DynamicVariableListNestedList::end() const
{
	return const_iterator(*this,m_list.end(),true);
}

// ----------------------------------------------------------------------

/**
 * Returns true if the specified iterator is within the nested list
 */
bool DynamicVariableListNestedList::isInList(const DynamicVariableList::MapType::const_iterator &i) const
{
	// the first part of the item's name must == the sublist name
	// TODO:  there is a better STL function for this -- look it up

	if (i == m_list.m_map.end())
		return false;
	return (getContextName() == i->first.substr(0,getContextName().length()));
}

// ----------------------------------------------------------------------

/**
 * Returns true if we have an item matching the name, or if there is an
 * item in a nested list beginning with the name.
 */
bool DynamicVariableListNestedList::hasItem(const std::string &name) const
{
	std::string effectiveName(getContextName()+name);
	DynamicVariableList::MapType::const_iterator i=m_list.lower_bound(effectiveName);
	if (i->first == effectiveName)
		return true; // exact match
	else
	{
		// check whether it's the first part of a nested item
		effectiveName+='.';
		return (effectiveName == getNameWithPath().substr(0,effectiveName.length())); // TODO: better STL function?
	}
}

// ----------------------------------------------------------------------

bool DynamicVariableListNestedList::operator== (const DynamicVariableListNestedList &rhs) const
{
	return ((&m_list == &rhs.m_list) && (m_sublistName == rhs.m_sublistName));
}

// ----------------------------------------------------------------------

DynamicVariableListNestedList::const_iterator DynamicVariableListNestedList::getItemByPosition(int position) const
{
	DynamicVariableListNestedList::const_iterator i(begin());
	for (; (i!=end()) && (position>0); ++i)
		--position;
	return i;
}

// ----------------------------------------------------------------------

DynamicVariableListNestedList::DynamicVariableListNestedList(const DynamicVariableListNestedList &parentList, int sublistIndex) :
		m_list(parentList.m_list),
		m_sublistName()
{
	const_iterator i(parentList.getItemByPosition(sublistIndex));
	DEBUG_FATAL(i==end(),("Programmer bug:  called NestedList(NestedList&, int) with an index that was too large.\n"));
	// want this to be const, but couldn't initialize it without the iterator
	const_cast<std::string*>(&m_sublistName)->operator=(parentList.getContextName()+i.getName()+'.');
}

// ======================================================================
// DynamicVariableListNestedList item getters
// VC couldn't handle this as a templated member function, so using a
// macro instead

#define NESTED_LIST_GET_ITEM(T) \
bool DynamicVariableListNestedList::getItem(const std::string &name, T &value) const \
{ \
	return m_list.getItem(getContextName()+name,value); \
}

NESTED_LIST_GET_ITEM(int)
NESTED_LIST_GET_ITEM(std::vector<int>)
NESTED_LIST_GET_ITEM(float)
NESTED_LIST_GET_ITEM(std::vector<float>)
NESTED_LIST_GET_ITEM(Unicode::String)
NESTED_LIST_GET_ITEM(std::string)
NESTED_LIST_GET_ITEM(std::vector<Unicode::String>)
NESTED_LIST_GET_ITEM(NetworkId)
NESTED_LIST_GET_ITEM(std::vector<NetworkId>)
NESTED_LIST_GET_ITEM(DynamicVariableLocationData)
NESTED_LIST_GET_ITEM(std::vector<DynamicVariableLocationData>)
NESTED_LIST_GET_ITEM(StringId)
NESTED_LIST_GET_ITEM(std::vector<StringId>)
NESTED_LIST_GET_ITEM(Transform)
NESTED_LIST_GET_ITEM(std::vector<Transform>)
NESTED_LIST_GET_ITEM(Vector)
NESTED_LIST_GET_ITEM(std::vector<Vector>)

// ======================================================================
// DynamicVariableListNestedList::const_iterator

DynamicVariableListNestedList::const_iterator::const_iterator(const DynamicVariableListNestedList &context, const DynamicVariableList::MapType::const_iterator &position, bool atEnd) :
	m_context(context),
	m_current(position),
	m_atEnd(atEnd)
{
}

// ----------------------------------------------------------------------

/**
 * Get the type of the item currently pointed to, or LIST if the item is a nested list.
 */
DynamicVariable::DynamicVariableType DynamicVariableListNestedList::const_iterator::getType() const
{
	size_t start = m_context.getContextName().length();
	size_t dotpos = m_current->first.find('.',start);

	if (dotpos == std::string::npos)
		// simple item -- get it's type
		return m_current->second.getType();
	else
		// nested item
		return DynamicVariable::LIST;
}

// ----------------------------------------------------------------------

const std::string DynamicVariableListNestedList::const_iterator::getNameWithPath() const
{
	return m_context.getContextName() + getName();
}

// ----------------------------------------------------------------------

DynamicVariableListNestedList::const_iterator & DynamicVariableListNestedList::const_iterator::operator++()
{
	// Advance until the name changes or we leave the nested list
	std::string currentName(getName());
	for (++m_current; (m_context.isInList(m_current)) && (getName() == currentName); ++m_current)
		;

	if (!m_context.isInList(m_current))
		m_atEnd = true;

	return *this;
}

// ----------------------------------------------------------------------

bool DynamicVariableListNestedList::const_iterator::operator==(const const_iterator &rhs) const
{
	return ((m_atEnd && rhs.m_atEnd) || (m_current == rhs.m_current && m_context == rhs.m_context));
}

// ----------------------------------------------------------------------

bool DynamicVariableListNestedList::const_iterator::operator!=(const const_iterator &rhs) const
{
	return !(operator==(rhs));
}

// ----------------------------------------------------------------------

DynamicVariableListNestedList DynamicVariableListNestedList::const_iterator::getNestedList() const
{
	return DynamicVariableListNestedList(m_context,getName());
}

// ----------------------------------------------------------------------

const std::string DynamicVariableListNestedList::const_iterator::getName() const
{
	if (m_atEnd)
		return std::string();
	
	DEBUG_FATAL(!m_context.isInList(m_current),("Programmer bug:  called getName() on an iterator that was not in the NestedList.\n"));
	
	size_t start = m_context.getContextName().length();
	size_t dotpos = m_current->first.find('.',start);
	
	return m_current->first.substr(start,dotpos != std::string::npos ? dotpos-start : std::string::npos);
}

// ----------------------------------------------------------------------

int DynamicVariableListNestedList::const_iterator::getPackedPosition() const
{
	if (m_atEnd)
		return -1;
	
	DEBUG_FATAL(!m_context.isInList(m_current),("Programmer bug:  called getPackedPosition() on an iterator that was not in the NestedList.\n"));

	return m_current->second.getPosition();
}

// ======================================================================
// DynamicVariableListNestedList::const_iterator getters

#define NESTED_LIST_CONST_ITERATOR_GET_VALUE(T) \
bool DynamicVariableListNestedList::const_iterator::getValue(T &value) const \
{ \
	return m_current->second.get(value); \
}

NESTED_LIST_CONST_ITERATOR_GET_VALUE(int)
NESTED_LIST_CONST_ITERATOR_GET_VALUE(std::vector<int>)
NESTED_LIST_CONST_ITERATOR_GET_VALUE(float)
NESTED_LIST_CONST_ITERATOR_GET_VALUE(std::vector<float>)
NESTED_LIST_CONST_ITERATOR_GET_VALUE(Unicode::String)
NESTED_LIST_CONST_ITERATOR_GET_VALUE(std::string)
NESTED_LIST_CONST_ITERATOR_GET_VALUE(std::vector<Unicode::String>)
NESTED_LIST_CONST_ITERATOR_GET_VALUE(NetworkId)
NESTED_LIST_CONST_ITERATOR_GET_VALUE(std::vector<NetworkId>)
NESTED_LIST_CONST_ITERATOR_GET_VALUE(DynamicVariableLocationData)
NESTED_LIST_CONST_ITERATOR_GET_VALUE(std::vector<DynamicVariableLocationData>)
NESTED_LIST_CONST_ITERATOR_GET_VALUE(StringId)
NESTED_LIST_CONST_ITERATOR_GET_VALUE(std::vector<StringId>) 
NESTED_LIST_CONST_ITERATOR_GET_VALUE(Transform)
NESTED_LIST_CONST_ITERATOR_GET_VALUE(std::vector<Transform>) 
NESTED_LIST_CONST_ITERATOR_GET_VALUE(Vector)
NESTED_LIST_CONST_ITERATOR_GET_VALUE(std::vector<Vector>) 
//======================================================================
