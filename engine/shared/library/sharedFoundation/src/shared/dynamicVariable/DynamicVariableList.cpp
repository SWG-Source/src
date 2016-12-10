//========================================================================
//
// DynamicVariableList.cpp
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "sharedFoundation/FirstSharedFoundation.h"
#include "sharedFoundation/DynamicVariableList.h"
#include "sharedFoundation/DynamicVariableListNestedList.h"

// ======================================================================

namespace DynamicVariableNamespace
{
	static const int s_numPositions = 20;
	static const int s_positionMasks[20] = {1,2,4,8,16,32,64,128,256,512,1024,2048,4096,8192,16384,32768,65536,131072,262144,524288};
	static const int s_allPositionsSet = 1048575;
	static const size_t s_maxPackedStringLength = 50;
	static const int s_initialRevision = -1;
}
using namespace DynamicVariableNamespace;

// ======================================================================

DynamicVariableList::DynamicVariableList() :
		m_map(),
		m_freePositions(s_allPositionsSet),
		m_revision(s_initialRevision)
{
}

// ----------------------------------------------------------------------

DynamicVariableList::~DynamicVariableList()
{
}

// ----------------------------------------------------------------------

bool DynamicVariableList::removeItemByName(const std::string &name)
{
	if (name == ".")
	{
		for(MapType::const_iterator it = begin(); it != end(); ++it)
		{
			if (it->second.getPosition() != -1)
				releasePosition(it->second.getPosition());
		}
		m_map.clear();
		updateRevision();
		return true;
	}

	MapType::const_iterator i=lower_bound(name);
	if (i==end())
		return false;
	if (i->first == name)
	{
		if (i->second.getPosition() != -1)
			releasePosition(i->second.getPosition());
		m_map.erase(i);  // exact match
		updateRevision();
		return true;
	}
	else
	{
		// check whether it's the first part of a nested item
		if ((name+'.') == i->first.substr(0,name.length()+1)) // TODO: better STL function?
		{
			NestedList sublist(*this,name);
			while (sublist.isInList(i))
			{
				MapType::const_iterator next(i);
				++next;
				if (i->second.getPosition() != -1)
					releasePosition(i->second.getPosition());
				m_map.erase(i);
				i = next;
			}
			updateRevision();
			return true;
		}
		else
			return false; // no matching sublist
	}
}

// ----------------------------------------------------------------------

bool DynamicVariableList::hasItem(const std::string &name) const
{
	MapType::const_iterator i=m_map.lower_bound(name);
	if (i==m_map.end())
		return false;
	if (i->first == name)
		return true; // exact match
	else
	{
		// check whether it's the first part of a nested item
		return ((name+'.') == i->first.substr(0,name.length()+1)); // TODO: better STL function?
	}
}

// ---------------------------------------------------------------------------

/*void DynamicVariableList::debugPrint(std::string &output) const
{
	for (MapType::const_iterator i=m_map.begin(); i!=m_map.end(); ++i)
		i->second.debugPrint(output);
}*/

// ----------------------------------------------------------------------

void DynamicVariableList::addToPackage(Archive::AutoDeltaByteStream & bs, Archive::AutoDeltaByteStream & bs_np)
{
	bs.addVariable(m_freePositions);
	bs.addVariable(m_map);
	bs_np.addVariable(m_revision);
}

// ----------------------------------------------------------------------

DynamicVariableList::MapType::const_iterator DynamicVariableList::lower_bound(const std::string &name) const
{
	return m_map.lower_bound(name);
}

// ----------------------------------------------------------------------

DynamicVariableList::MapType::const_iterator DynamicVariableList::begin() const
{
	return m_map.begin();
}

// ----------------------------------------------------------------------

DynamicVariableList::MapType::const_iterator DynamicVariableList::end() const
{
	return m_map.end();
}

// ----------------------------------------------------------------------

DynamicVariable::DynamicVariableType DynamicVariableList::getType(const std::string &name) const
{
	MapType::const_iterator i=m_map.find(name);
	if (i!=m_map.end())
		return i->second.getType();
	else
		// for compatibility with the old interface, no exact match == (possibly empty) nested list
		return DynamicVariable::LIST;
}

// ----------------------------------------------------------------------

DynamicVariableList::MapType::const_iterator DynamicVariableList::find(const std::string &name) const
{
	return m_map.find(name);
}

// ----------------------------------------------------------------------

void DynamicVariableList::copy(const std::string &destObjvarName, const DynamicVariableList &sourceList, const std::string &sourceObjvarName)
{
	if (sourceList.getType(sourceObjvarName)==DynamicVariable::LIST)
	{
		std::string oldPrefix(sourceObjvarName+'.');
		std::string newPrefix(destObjvarName+'.');
		for (MapType::const_iterator i=sourceList.lower_bound(oldPrefix); (i!=sourceList.end()) && (i->first.substr(0,oldPrefix.length()) == oldPrefix); ++i)
		{
			setItem(newPrefix + i->first.substr(oldPrefix.length(),static_cast<size_t>(std::string::npos)),i->second); // static_cast needed for Windows
		}
	}
	else
	{
		MapType::const_iterator sourceObjvar(sourceList.m_map.find(sourceObjvarName));
		if (sourceObjvar != sourceList.m_map.end())
			setItem(destObjvarName,sourceObjvar->second);
	}
}

// ======================================================================
// DynamicVariableList::const_iterator

/*DynamicVariableList::const_iterator::const_iterator(const MapType::const_iterator &source) :
		MapType::const_iterator (source)
{
}

// ----------------------------------------------------------------------

std::string DynamicVariableList::const_iterator::getName() const
{
	size_t dotpos = getNameWithPath().rfind('.');
	if (dotpos == std::string::npos)
		return getNameWithPath();
	return getNameWithPath().substr(dotpos+1,std::string::npos);
}

// ----------------------------------------------------------------------

const std::string &DynamicVariableList::const_iterator::getNameWithPath() const
{
	return operator*().first;
}*/

// ======================================================================
// DynamicVariableList getters
// Using a macro because VC couldn't handle it when I tried a template member function

#define GET_ITEM(T) \
bool DynamicVariableList::getItem(const std::string &name,  T &value) const \
{ \
	MapType::const_iterator i=m_map.find(name); \
	if (i==m_map.end()) \
		return false; \
	else \
		return (i->second.get(value)); \
}

GET_ITEM(int)
GET_ITEM(std::vector<int>)
GET_ITEM(float)
GET_ITEM(std::vector<float>)
GET_ITEM(Unicode::String)
GET_ITEM(std::string)
GET_ITEM(std::vector<Unicode::String>)
GET_ITEM(NetworkId)
GET_ITEM(std::vector<NetworkId>)
GET_ITEM(DynamicVariableLocationData)
GET_ITEM(std::vector<DynamicVariableLocationData>)
GET_ITEM(StringId)
GET_ITEM(std::vector<StringId>)
GET_ITEM(Transform)
GET_ITEM(std::vector<Transform>)
GET_ITEM(Vector)
GET_ITEM(std::vector<Vector>)

// ======================================================================
// DynamicVariableList setters
// Using a macro because VC couldn't handle it when I tried a template member function

#define SET_ITEM(T) \
bool DynamicVariableList::setItem(const std::string &name, const T &value) \
{ \
	setItem(name,DynamicVariable(value)); \
	return true; \
}

SET_ITEM(int)
SET_ITEM(std::vector<int>)
SET_ITEM(float)
SET_ITEM(std::vector<float>)
SET_ITEM(Unicode::String)
SET_ITEM(std::string)
SET_ITEM(std::vector<Unicode::String>)
SET_ITEM(NetworkId)
SET_ITEM(std::vector<NetworkId>)
SET_ITEM(DynamicVariableLocationData)
SET_ITEM(std::vector<DynamicVariableLocationData>)
SET_ITEM(StringId)
SET_ITEM(std::vector<StringId>)
SET_ITEM(Transform)
SET_ITEM(std::vector<Transform>)
SET_ITEM(Vector)
SET_ITEM(std::vector<Vector>)

bool DynamicVariableList::setItem(const std::string &name, DynamicVariable value) 
{
	if (name.empty())
	{
		WARNING_STRICT_FATAL(true,("Attepted to set an object variable with no name"));
		return false;
	}

	size_t length = value.getUTF8Length();
	if (length > DynamicVariable::MAX_DYNAMIC_VARIABLE_UTF8_LENGTH) 
	{
		WARNING_STRICT_FATAL(true, ("DynamicVariableList tried to set objvar %s to too long a value", name.c_str()));
		return false;
	}
	
	bool canUsePosition = true;
	if ((name.length() > s_maxPackedStringLength) || (length > s_maxPackedStringLength))
		canUsePosition = false; // objvar can't use a packed position if it is too long
	
	MapType::const_iterator i=m_map.find(name);
	if (i==m_map.end())
	{
		if (canUsePosition)
			value.setPosition(getFreePosition());
		else
			value.setPosition(-1);

		updateRevision();
	}
	else
	{
		if (value != i->second) 
			updateRevision();

		if ((i->second.getPosition() != -1) && !canUsePosition)
		{
			// old value was using a packed position, but the new value can't.  
			releasePosition(i->second.getPosition());
			value.setPosition(-1);
			m_map.erase(name); // send the delta as an erase and an add, so that the DB handles it correctly

			updateRevision();
		}
		else
		{
			// use the same packed position, if any
			value.setPosition(i->second.getPosition());
		}
	}
	
	m_map.set(name,value);
	return true;
} 

// ----------------------------------------------------------------------

int DynamicVariableList::getFreePosition()
{
	if (m_freePositions.get() == 0)
		return -1;

	for (int i=0; i< s_numPositions; ++i)
	{
//		DEBUG_REPORT_LOG(true,("i=%i, m_freePositions=%i, positionMasks[i]=%i\n",i, m_freePositions.get(),positionMasks[i]));
		if ((m_freePositions.get() & s_positionMasks[i]) != 0)
		{
//			DEBUG_REPORT_LOG(true,("Picking position %i\n",i));
			m_freePositions=m_freePositions.get() - s_positionMasks[i];
			return i;
		}
	}
	return -1;
}

// ----------------------------------------------------------------------

void DynamicVariableList::releasePosition(int positionId)
{
	DEBUG_FATAL(positionId < 0 || positionId >= s_numPositions, ("Position number out of range."));
	DEBUG_FATAL(((m_freePositions.get() & s_positionMasks[positionId]) != 0),("Attempted to release a position that was not in use"));

	m_freePositions= m_freePositions.get() + s_positionMasks[positionId];
}

// ----------------------------------------------------------------------

size_t DynamicVariableList::size() const
{
	return m_map.size();
}

// ----------------------------------------------------------------------

int DynamicVariableList::getRevision() const
{
	return m_revision.get();
}

// ----------------------------------------------------------------------

void DynamicVariableList::updateRevision()
{
	m_revision.set(m_revision.get() + 1);
}



// ======================================================================
