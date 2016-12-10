//======================================================================
//
// DynamicVariableListNestedList.h
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_DynamicVariableListNestedList_H
#define INCLUDED_DynamicVariableListNestedList_H

#include "Archive/AutoDeltaMap.h"
#include "sharedFoundation/DynamicVariable.h"
#include <string>

class DynamicVariableList;

//======================================================================
/**
* A bookmark to a nested list.
*
* DynamicVariableList is a flat map of name -> value, but it can be organized into logical nested lists.  Nested lists are
* identified using ".".  For example, if a variable name is "deed.build_time", it is considered to be in a logial nested
* list called "deed".
*
* The class NestedList represents a logical nested list, i.e. context in which operations can be performed on the DynamicVariableList.
*/
class DynamicVariableListNestedList
{
public:

	typedef Archive::AutoDeltaMap<std::string, DynamicVariable> MapType;

	/**
	* An iterator for a nested list.
	*
	* NestedList::const_iterator traverses things at the level of the NestedList.  If the NestedList contains sublists,
	* const_iterator will hit each sublist, but will not recurse into the items within the list.  For example, suppose
	* the DynamicVariableList contains this:
	*     deed.location.x  40
	*     deed.location.y  50
	*     deed.build_time  60
	* Suppose a NestedList is created referring to "deed".  Calling getName() on a const_iterator on that NestedList
	* as it traverses the nested list will return "location" and "build_time".
	*/

	class const_iterator
	{
	public:
		const_iterator (const DynamicVariableListNestedList &context, const MapType::const_iterator &position, bool atEnd);

		const std::string                    getName          () const;
		const std::string                    getNameWithPath  () const;
		DynamicVariableListNestedList        getNestedList    () const;
		DynamicVariable::DynamicVariableType getType          () const;
		int                                  getPackedPosition() const;
		const_iterator &                     operator++       ();
		bool                                 operator==       (const const_iterator &rhs) const;
		bool                                 operator!=       (const const_iterator &rhs) const;

		bool getValue (int & value) const;
		bool getValue (std::vector<int> & value) const;
		bool getValue (float & value) const;
		bool getValue (std::vector<float> & value) const;
		bool getValue (Unicode::String & value) const;
		bool getValue (std::string & value) const;
		bool getValue (std::vector<Unicode::String> & value) const;
		bool getValue (NetworkId & value) const;
		bool getValue (std::vector<NetworkId> & value) const;
		bool getValue (DynamicVariableLocationData & value) const;
		bool getValue (std::vector<DynamicVariableLocationData> & value) const;
		bool getValue (StringId &value) const;
		bool getValue (std::vector<StringId> &value) const;
		bool getValue (Transform &value) const;
		bool getValue (std::vector<Transform> &value) const;
		bool getValue (Vector &value) const;
		bool getValue (std::vector<Vector> &value) const;

	private:
		const DynamicVariableListNestedList &   m_context;
		MapType::const_iterator                 m_current;
		bool                                    m_atEnd;

	private:
		const_iterator &operator=(const const_iterator &rhs); //disable
	};

public:
	DynamicVariableListNestedList (const DynamicVariableList &dvList, const std::string &sublistName);
	DynamicVariableListNestedList (const DynamicVariableList &dvList);
	DynamicVariableListNestedList (const DynamicVariableListNestedList &parentList, const std::string &sublistName);
	DynamicVariableListNestedList (const DynamicVariableListNestedList &parentList, int sublistIndex);
	
	const std::string   getNameWithPath   () const;
	const std::string   getName           () const;
	const std::string & getContextName    () const;
	bool                isInList          (const MapType::const_iterator &i) const;
	int                 getCount          () const;
	bool                empty             () const;
	bool                hasItem           (const std::string &name) const;
	bool                operator==        (const DynamicVariableListNestedList &rhs) const;
	const_iterator      begin             () const;
	const_iterator      end               () const;
	const_iterator      getItemByPosition (int position) const;
	
	bool getItem (const std::string &name,int & value) const;
	bool getItem (const std::string &name,std::vector<int> & value) const;
	bool getItem (const std::string &name,float & value) const;
	bool getItem (const std::string &name,std::vector<float> & value) const;
	bool getItem (const std::string &name,Unicode::String & value) const;
	bool getItem (const std::string &name,std::string & value) const;
	bool getItem (const std::string &name,std::vector<Unicode::String> & value) const;
	bool getItem (const std::string &name,NetworkId & value) const;
	bool getItem (const std::string &name,std::vector<NetworkId> & value) const;
	bool getItem (const std::string &name,DynamicVariableLocationData & value) const;
	bool getItem (const std::string &name,std::vector<DynamicVariableLocationData> & value) const;
	bool getItem (const std::string &name,StringId &value) const;
	bool getItem (const std::string &name,std::vector<StringId> &value) const;
	bool getItem (const std::string &name,Transform &value) const;
	bool getItem (const std::string &name,std::vector<Transform> &value) const;
	bool getItem (const std::string &name,Vector &value) const;
	bool getItem (const std::string &name,std::vector<Vector> &value) const;
	
private:
	const DynamicVariableList & m_list;
	const std::string           m_sublistName;
	
private:
	DynamicVariableListNestedList &operator=(const DynamicVariableListNestedList &rhs); //disable
};

//======================================================================

#endif
