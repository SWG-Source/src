//======================================================================
//
// SuiPageData.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/SuiPageData.h"

#include "Archive/Archive.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedGame/SuiCommand.h"
#include "sharedGame/SuiCommandArchive.h"
#include "sharedMathArchive/VectorArchive.h"

//lint -e766 //header

//======================================================================

SuiPageData::SuiPageData() :
m_pageId(0),
m_pageName(),
m_commands(),
m_associatedObjectId(),
m_associatedLocation(Vector::maxXYZ), // "unset" value is Vector::maxXYZ
m_maxRangeFromObject(0.0f)
{
}

//----------------------------------------------------------------------

SuiPageData::SuiPageData(int pageId, std::string const & pageName) :
m_pageId(pageId),
m_pageName(pageName),
m_commands(),
m_associatedObjectId(),
m_associatedLocation(Vector::maxXYZ), // "unset" value is Vector::maxXYZ
m_maxRangeFromObject(0.0f)
{
}

//----------------------------------------------------------------------

std::string const & SuiPageData::getPageName() const
{
	return m_pageName;
}

//----------------------------------------------------------------------

NetworkId const & SuiPageData::getAssociatedObjectId() const
{
	return m_associatedObjectId;
}

//----------------------------------------------------------------------

Vector const & SuiPageData::getAssociatedLocation() const
{
	return m_associatedLocation;
}

//----------------------------------------------------------------------

float const SuiPageData::getMaxRangeFromObject() const
{
	return m_maxRangeFromObject;
}

//----------------------------------------------------------------------

SuiPageData::SuiCommandVector const & SuiPageData::getCommands() const
{
	return m_commands;
}

//----------------------------------------------------------------------

/**
* This method does not attempt to merge identical subscription commands' property subscription lists
* @todo: make the merging work
*/

bool SuiPageData::addCommand(SuiCommand const & command)
{
	if (command.getType() == SuiCommand::SCT_subscribeToEvent)
	{
		int eventType = 0;
		std::string callback;
		command.getSubscribeToEventCallback(eventType, callback);
		std::string const & targetWidget = command.getTargetWidget();

		SuiCommand const * const oldCommand = findSubscribeToEventCommand(eventType, targetWidget);

		if (oldCommand != nullptr)
		{
			WARNING(true, ("SuiPageData::addCommand attempt to add duplicate SCT_subscribeToEvent command.  Type=[%d], target=[%s]", eventType, targetWidget.c_str()));
			return false;
		}		
	}

	m_commands.push_back(command);
	return true;
}

//----------------------------------------------------------------------

void SuiPageData::setAssociatedObjectIdWithMaxRange(NetworkId const & associatedObjectId, float const maxRange)
{
	m_associatedObjectId = associatedObjectId;
	m_maxRangeFromObject = maxRange;
}

//----------------------------------------------------------------------

void SuiPageData::setAssociatedLocationWithMaxRange(Vector const & associatedLocation, float maxRange)
{
	m_associatedLocation = associatedLocation;
	m_maxRangeFromObject = maxRange;
}

//----------------------------------------------------------------------

/**
* This method call must be accompanied at some point by a call to subscribeToEvent()
*/

void SuiPageData::subscribeToPropertyForEvent(int eventType, std::string const & eventWidgetName, std::string const & propertyWidgetName, std::string const & propertyName)
{
	SuiCommand * const command = findSubscribeToEventCommand(eventType, eventWidgetName);

	if (command == nullptr)
	{
		SuiCommand newCommand(SuiCommand::SCT_subscribeToEvent, eventWidgetName);
		newCommand.initSubscribeToEvent(eventType, std::string());
		newCommand.addPropertySubscriptionToEvent(propertyWidgetName, propertyName);
		if (!addCommand(newCommand))
			WARNING(true, ("SuiPageData::subscribeToPropertyForEvent() failed to addCommand()"));
	}
	else
		command->addPropertySubscriptionToEvent(propertyWidgetName, propertyName);
}

//----------------------------------------------------------------------

bool SuiPageData::subscribeToEvent(int eventType, std::string const & eventWidgetName, std::string const & callback)
{
	SuiCommand * const command = findSubscribeToEventCommand(eventType, eventWidgetName);

	if (command == nullptr)
	{
		SuiCommand newCommand(SuiCommand::SCT_subscribeToEvent, eventWidgetName);
		newCommand.initSubscribeToEvent(eventType, callback);
		if (!addCommand(newCommand))
			WARNING(true, ("SuiPageData::subscribeToEvent() failed to addCommand()"));
		return true;
	}
	else
	{
		command->modifyEventCallback(callback);
		return false;
	}
}

//----------------------------------------------------------------------

SuiCommand * SuiPageData::findSubscribeToEventCommand(int const eventType, std::string const & eventWidgetName)
{
	for (SuiCommandVector::iterator it = m_commands.begin(); it != m_commands.end(); ++it)
	{
		SuiCommand & command = *it;
		if (command.getType() == SuiCommand::SCT_subscribeToEvent)
		{
			int existingEventType = 0;
			std::string callback;
			command.getSubscribeToEventCallback(existingEventType, callback);
			std::string const & existingTargetWidget = command.getTargetWidget();

			if (eventType == existingEventType && eventWidgetName == existingTargetWidget)
				return &command;
		}
	}

	return nullptr;
}

//----------------------------------------------------------------------

SuiCommand * SuiPageData::findSubscribeToEventCommandByIndex(int const index)
{
	int currentIndex = 0;

	for (SuiCommandVector::iterator it = m_commands.begin(); it != m_commands.end(); ++it)
	{
		SuiCommand & command = *it;
		if (command.getType() == SuiCommand::SCT_subscribeToEvent)
		{
			if (currentIndex == index)
				return &command;

			++currentIndex;
		}
	}

	return nullptr;
}

//----------------------------------------------------------------------

void SuiPageData::get(Archive::ReadIterator & source)
{
	Archive::get(source, m_pageId);
	Archive::get(source, m_pageName);
	Archive::get(source, m_commands);
	Archive::get(source, m_associatedObjectId);
	Archive::get(source, m_associatedLocation);
	Archive::get(source, m_maxRangeFromObject);
}

//----------------------------------------------------------------------

void SuiPageData::put(Archive::ByteStream & target) const
{
	Archive::put(target, m_pageId);
	Archive::put(target, m_pageName);
	Archive::put(target, m_commands);
	Archive::put(target, m_associatedObjectId);
	Archive::put(target, m_associatedLocation);
	Archive::put(target, m_maxRangeFromObject);
}

//----------------------------------------------------------------------

bool SuiPageData::operator==(SuiPageData const & rhs) const
{
	return 
		m_pageId == rhs.m_pageId &&
		m_pageName == rhs.m_pageName &&
		m_commands == rhs.m_commands &&
		m_associatedObjectId == rhs.m_associatedObjectId &&
		m_associatedLocation == rhs.m_associatedLocation &&
		m_maxRangeFromObject == rhs.m_maxRangeFromObject; //lint !e777 //testing floats
}

//----------------------------------------------------------------------

bool SuiPageData::getPageUpdate(int commandIndex, SuiPageData & pageData) const
{
	if (commandIndex < 0 || commandIndex >= static_cast<int>(m_commands.size()))
		return false;

	pageData.m_pageId = m_pageId;
	pageData.m_pageName = m_pageName;
	pageData.m_associatedObjectId = m_associatedObjectId;
	pageData.m_associatedLocation = m_associatedLocation;
	pageData.m_maxRangeFromObject = m_maxRangeFromObject;

	pageData.m_commands.clear();

	SuiCommandVector::const_iterator it = m_commands.begin();
	
	std::advance(it, commandIndex);
	
	for (; it != m_commands.end(); ++it)
	{
		SuiCommand const & command = *it;
		if (command.getType() != SuiCommand::SCT_subscribeToEvent)
			pageData.m_commands.push_back(command);
	}

	return true;
}

//======================================================================
