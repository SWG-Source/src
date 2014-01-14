//======================================================================
//
// SuiCommand.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/SuiCommand.h"

#include "Archive/Archive.h"
#include "sharedGame/SuiWidgetProperty.h"
#include "sharedGame/SuiWidgetProperty.h"
#include "unicodeArchive/UnicodeArchive.h"

//======================================================================

namespace
{
	size_t const parameterNarrowSizes[] = 
	{
		0, // SCT_none
		1, // SCT_clearDataSource
		3, // SCT_addChildWidget
		2, // SCT_setProperty
		2, // SCT_addDataItem
		3, // SCT_subscribeToEvent // possibly more pairs of parameters
		2, // SCT_addDataSourceContainer
		1,  // SCT_clearDataSourceContainer
		2  // SCT_addDataSource
	};
	
	size_t const parameterWideSizes[] = 
	{
		0, // SCT_none
		0, // SCT_clearDataSource
		0, // SCT_addChildWidget
		1, // SCT_setProperty
		1, // SCT_addDataItem
		0, // SCT_subscribeToEvent // possibly more pairs of parameters
		1, // SCT_addDataSourceContainer
		0,  // SCT_clearDataSourceContainer
		1, // SCT_addDataSource
	};
	
	int const numParameterSizes = sizeof(parameterWideSizes) / sizeof(parameterWideSizes[0]);
}

//----------------------------------------------------------------------

SuiCommand::SuiCommand() :
m_type(SCT_none),
m_parametersWide(),
m_parametersNarrow()
{
}

//----------------------------------------------------------------------

SuiCommand::SuiCommand(Type type, std::string const & targetWidget) :
m_type(type),
m_parametersWide(),
m_parametersNarrow()
{
	int const typeIndex = static_cast<int>(type);
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, typeIndex, numParameterSizes);

	if (parameterNarrowSizes[typeIndex] > 0)
		m_parametersNarrow.reserve(parameterNarrowSizes[static_cast<int>(typeIndex)]);

	if (parameterWideSizes[typeIndex] > 0)
		m_parametersWide.reserve(parameterWideSizes[static_cast<int>(typeIndex)]);

	m_parametersNarrow.push_back(targetWidget);
}

//----------------------------------------------------------------------

void SuiCommand::initAddChildWidget(std::string const & widgetType,  std::string const & widgetName)
{
	DEBUG_FATAL(m_type != SCT_addChildWidget, ("SuiCommand invalid command type")); 

	m_parametersNarrow.push_back(widgetType);
	m_parametersNarrow.push_back(widgetName);
}

//----------------------------------------------------------------------

void SuiCommand::initSetProperty(std::string const & propertyName, Unicode::String const & propertyValue)
{
	DEBUG_FATAL(m_type != SCT_setProperty, ("SuiCommand invalid command type"));

	m_parametersNarrow.push_back(propertyName);
	m_parametersWide.push_back(propertyValue);
}

//----------------------------------------------------------------------

void SuiCommand::initAddDataItem(std::string const & dataItemName, Unicode::String const & dataItemValue)
{
	DEBUG_FATAL(m_type != SCT_addDataItem, ("SuiCommand invalid command type"));

	m_parametersNarrow.push_back(dataItemName);
	m_parametersWide.push_back(dataItemValue);
}

//----------------------------------------------------------------------

void SuiCommand::initAddDataSourceContainer(std::string const & dataSourceContainerName, Unicode::String const & dataSourceContainerValue)
{
    DEBUG_FATAL(m_type != SCT_addDataSourceContainer, ("SuiCommand invalid command type"));

    m_parametersNarrow.push_back(dataSourceContainerName);
    m_parametersWide.push_back(dataSourceContainerValue);
}

//----------------------------------------------------------------------

void SuiCommand::initAddDataSource(std::string const & dataSourceName, Unicode::String const & dataSourceValue)
{
	DEBUG_FATAL(m_type != SCT_addDataSource, ("SuiCommand invalid command type"));

	m_parametersNarrow.push_back(dataSourceName);
	m_parametersWide.push_back(dataSourceValue);
}


//----------------------------------------------------------------------

/**
* @param callback this param is not used on the client
*/
void SuiCommand::initSubscribeToEvent(int eventType, std::string const & callback)
{
	DEBUG_FATAL(m_type != SCT_subscribeToEvent, ("SuiCommand invalid command type"));

	static std::string typeString;
	typeString.clear();
	typeString.push_back(static_cast<char>(eventType));

	m_parametersNarrow.push_back(typeString);
	m_parametersNarrow.push_back(callback);
}

//----------------------------------------------------------------------

void SuiCommand::modifyEventCallback(std::string const & callback)
{
	if (m_parametersNarrow.size() < 3)
	{
		WARNING(true, ("SuiCommand::modifyEventCallback on uninitialized callback for widget [%s]", getTargetWidget().c_str()));
		return;
	}

	m_parametersNarrow[2] = callback;
}

//----------------------------------------------------------------------

void SuiCommand::addPropertySubscriptionToEvent(std::string const & widgetName, std::string const & propertyName)
{
	DEBUG_FATAL(m_type != SCT_subscribeToEvent, ("SuiCommand invalid command type"));

	m_parametersNarrow.reserve(m_parametersNarrow.size() + 2);
	m_parametersNarrow.push_back(widgetName);
	m_parametersNarrow.push_back(propertyName);
}

//----------------------------------------------------------------------

void SuiCommand::getAddChildWidget(std::string & widgetType,  std::string & widgetName) const
{
	DEBUG_FATAL(m_type != SCT_addChildWidget, ("SuiCommand invalid command type"));

	if (m_parametersNarrow.size() < 3)
		WARNING(true, ("SuiCommand::getAddChildWidget() invalid number of narrow parameters [%d] for type [%d]", static_cast<int>(m_parametersNarrow.size()), static_cast<int>(m_type)));
	else
	{
		widgetType = m_parametersNarrow[1];
		widgetName = m_parametersNarrow[2];
	}
}

//----------------------------------------------------------------------

void SuiCommand::getSetProperty(std::string & propertyName, Unicode::String & propertyValue) const
{
	DEBUG_FATAL(m_type != SCT_setProperty, ("SuiCommand invalid command type"));

	if (m_parametersNarrow.size() < 2)
		WARNING(true, ("SuiCommand::getSetProperty() invalid number of narrow parameters [%d] for type [%d]", static_cast<int>(m_parametersNarrow.size()), static_cast<int>(m_type)));
	else if (m_parametersWide.size() < 1)
		WARNING(true, ("SuiCommand::getSetProperty() invalid number of wide parameters [%d] for type [%d]", static_cast<int>(m_parametersWide.size()), static_cast<int>(m_type)));
	else
	{
		propertyName = m_parametersNarrow[1];
		propertyValue = m_parametersWide[0];
	}
}

//----------------------------------------------------------------------

void SuiCommand::getAddDataItem(std::string & dataItemName, Unicode::String & dataItemValue) const
{
	DEBUG_FATAL(m_type != SCT_addDataItem, ("SuiCommand invalid command type"));

	if (m_parametersNarrow.size() < 2)
		WARNING(true, ("SuiCommand::getAddDataItem() invalid number of narrow parameters [%d] for type [%d]", static_cast<int>(m_parametersNarrow.size()), static_cast<int>(m_type)));
	else if (m_parametersWide.size() < 1)
		WARNING(true, ("SuiCommand::getAddDataItem() invalid number of wide parameters [%d] for type [%d]", static_cast<int>(m_parametersWide.size()), static_cast<int>(m_type)));
	else
	{
		dataItemName = m_parametersNarrow[1];
		dataItemValue = m_parametersWide[0];
	}
}

//----------------------------------------------------------------------

void SuiCommand::getAddDataSourceContainer(std::string & dataSourceContainerName, Unicode::String & dataSourceContainerValue) const
{
	DEBUG_FATAL(m_type != SCT_addDataSourceContainer, ("SuiCommand invalid command type"));

	if (m_parametersNarrow.size() < 2)
		WARNING(true, ("SuiCommand::getAddDataSourceContainer() invalid number of narrow parameters [%d] for type [%d]", static_cast<int>(m_parametersNarrow.size()), static_cast<int>(m_type)));
	else if (m_parametersWide.size() < 1)
		WARNING(true, ("SuiCommand::getAddDataSourceContainer() invalid number of wide parameters [%d] for type [%d]", static_cast<int>(m_parametersWide.size()), static_cast<int>(m_type)));
	else
	{
		dataSourceContainerName = m_parametersNarrow[1];
		dataSourceContainerValue = m_parametersWide[0];
	}
}

//----------------------------------------------------------------------

void SuiCommand::getAddDataSource(std::string & dataSourceName, Unicode::String & dataSourceValue) const
{
	DEBUG_FATAL(m_type != SCT_addDataSource, ("SuiCommand invalid command type"));

	if (m_parametersNarrow.size() < 2)
		WARNING(true, ("SuiCommand::getAddDataSource() invalid number of narrow parameters [%d] for type [%d]", static_cast<int>(m_parametersNarrow.size()), static_cast<int>(m_type)));
	else if (m_parametersWide.size() < 1)
		WARNING(true, ("SuiCommand::getAddDataSource() invalid number of wide parameters [%d] for type [%d]", static_cast<int>(m_parametersWide.size()), static_cast<int>(m_type)));
	else
	{
		dataSourceName = m_parametersNarrow[1];
		dataSourceValue = m_parametersWide[0];
	}
}

//----------------------------------------------------------------------

/**
* @param callback this param is not used on the client
*/

void SuiCommand::getSubscribeToEventCallback(int & eventType, std::string & callback) const
{
	DEBUG_FATAL(m_type != SCT_subscribeToEvent, ("SuiCommand invalid command type"));

	if (m_parametersNarrow.size() < 3)
		WARNING(true, ("SuiCommand::getSubscribeToEventCallback() invalid number of narrow parameters [%d] for type [%d]", static_cast<int>(m_parametersNarrow.size()), static_cast<int>(m_type)));
	else
	{
		if (m_parametersNarrow[1].empty())
			WARNING(true, ("SuiCommand empty event type for subscription"));
		else
			eventType = m_parametersNarrow[1][0];

		callback = m_parametersNarrow[2];
	}
}

//----------------------------------------------------------------------

void SuiCommand::getPropertySubscriptionsForEvent(WidgetPropertyVector & widgetProperties) const
{
	DEBUG_FATAL(m_type != SCT_subscribeToEvent, ("SuiCommand invalid command type"));

	int const paramCount = static_cast<int>(m_parametersNarrow.size());

	if (m_parametersNarrow.size() < 3)
		WARNING(true, ("SuiCommand::getPropertySubscriptionsForEvent() invalid number of narrow parameters [%d] for type [%d]", static_cast<int>(m_parametersNarrow.size()), static_cast<int>(m_type)));
	else 
	{
		widgetProperties.reserve(static_cast<size_t>((paramCount - 3) / 2));
		for (int i = 3; i < paramCount; )
		{
			SuiWidgetProperty prop;
			prop.widgetName = m_parametersNarrow[static_cast<size_t>(i++)];
			prop.propertyName = m_parametersNarrow[static_cast<size_t>(i++)];
			widgetProperties.push_back(prop);
		}
	}
}

//----------------------------------------------------------------------

std::string const & SuiCommand::getTargetWidget() const
{
	if (m_parametersNarrow.size() == 0)
		WARNING(true, ("SuiCommand::getTargetWidget() invalid number of parameters [%d] for type [%d]", static_cast<int>(m_parametersNarrow.size()), static_cast<int>(m_type)));
	
	return m_parametersNarrow[0];
}

//----------------------------------------------------------------------

void SuiCommand::get(Archive::ReadIterator & source)
{
	uint8 type;
	Archive::get(source, type);
	m_type = static_cast<SuiCommand::Type>(type);
	Archive::get(source, m_parametersWide);
	Archive::get(source, m_parametersNarrow);
}

//----------------------------------------------------------------------

void SuiCommand::put(Archive::ByteStream & target) const
{
	Archive::put(target, static_cast<uint8>(m_type));
	Archive::put(target, m_parametersWide);
	Archive::put(target, m_parametersNarrow);
}

//----------------------------------------------------------------------

bool SuiCommand::operator==(SuiCommand const & rhs) const
{
	return 
		m_type == rhs.m_type &&
		m_parametersWide == rhs.m_parametersWide &&
		m_parametersNarrow == rhs.m_parametersNarrow;
}

//======================================================================
