//======================================================================
//
// SuiPageData.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SuiPageData_H
#define INCLUDED_SuiPageData_H

//======================================================================

#include "sharedFoundation/NetworkId.h"
#include "sharedGame/SuiCommand.h"
#include "sharedMath/Vector.h"
#include <vector>

class SuiCommand;

namespace Archive
{
	class ReadIterator;
	class ByteStream;
}

//----------------------------------------------------------------------

class SuiPageData
{
public:

	typedef std::vector<SuiCommand> SuiCommandVector;

public:

	SuiPageData();
	SuiPageData(int pageId, std::string const & pageName);

	int getPageId() const;
	std::string const & getPageName() const;
	NetworkId const & getAssociatedObjectId() const;
	Vector const & getAssociatedLocation() const;
	float const getMaxRangeFromObject() const;
	SuiCommandVector const & getCommands() const;

	bool addCommand(SuiCommand const & command);
	void setAssociatedObjectIdWithMaxRange(NetworkId const & associatedObjectId, float maxRange);
	void setAssociatedLocationWithMaxRange(Vector const & associatedLocation, float maxRange);

	void subscribeToPropertyForEvent(int eventType, std::string const & eventWidgetName, std::string const & propertyWidgetName, std::string const & propertyName);

	bool subscribeToEvent(int eventType, std::string const & eventWidgetName, std::string const & callback);

	SuiCommand const * findSubscribeToEventCommand(int eventType, std::string const & eventWidgetName) const;
	SuiCommand * findSubscribeToEventCommand(int eventType, std::string const & eventWidgetName);

	SuiCommand const * findSubscribeToEventCommandByIndex(int index) const;
	SuiCommand * findSubscribeToEventCommandByIndex(int index);

	bool operator==(SuiPageData const & rhs) const;
	bool operator!=(SuiPageData const & rhs) const;
	bool getPageUpdate(int commandIndex, SuiPageData & pageData) const;

public:

	void get(Archive::ReadIterator & source);
	void put(Archive::ByteStream & target) const;

private:

	int m_pageId;
	std::string m_pageName;
	SuiCommandVector m_commands;
	NetworkId m_associatedObjectId;
	Vector m_associatedLocation;
	float m_maxRangeFromObject;
};

//----------------------------------------------------------------------

inline int SuiPageData::getPageId() const
{
	return m_pageId;
}

//----------------------------------------------------------------------

inline SuiCommand const * SuiPageData::findSubscribeToEventCommand(int eventType, std::string const & eventWidgetName) const
{
	return const_cast<SuiPageData*>(this)->findSubscribeToEventCommand(eventType, eventWidgetName);
}

//----------------------------------------------------------------------

inline SuiCommand const * SuiPageData::findSubscribeToEventCommandByIndex(int index) const
{
	return const_cast<SuiPageData*>(this)->findSubscribeToEventCommandByIndex(index);
}

//----------------------------------------------------------------------

inline bool SuiPageData::operator!=(SuiPageData const & rhs) const
{
	return !(*this==rhs);
}

//======================================================================

#endif
