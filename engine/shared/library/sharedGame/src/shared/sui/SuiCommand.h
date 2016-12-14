//======================================================================
//
// SuiCommand.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SuiCommand_H
#define INCLUDED_SuiCommand_H

//======================================================================

class SuiWidgetProperty;

#include <vector>

namespace Archive
{
	class ReadIterator;
	class ByteStream;
}

//----------------------------------------------------------------------

class SuiCommand
{
public:
	enum Type
	{
		SCT_none,
		SCT_clearDataSource,
		SCT_addChildWidget,
		SCT_setProperty,
		SCT_addDataItem,
		SCT_subscribeToEvent,
		SCT_addDataSourceContainer,
		SCT_clearDataSourceContainer,
		SCT_addDataSource,
	};

	typedef std::vector<Unicode::String> UnicodeStringVector;
	typedef std::vector<std::string> StringVector;
	typedef std::vector<SuiWidgetProperty> WidgetPropertyVector;

public:
		
	SuiCommand();
	SuiCommand(Type type, std::string const & targetWidget);

	void initAddChildWidget(std::string const & widgetType,  std::string const & widgetName);
	void initSetProperty(std::string const & propertyName, Unicode::String const & propertyValue);
	void initAddDataItem(std::string const & dataItemName, Unicode::String const & dataItemValue);
	void initAddDataSourceContainer(std::string const & dataSourceContainerName, Unicode::String const & dataSourceContainerValue);
	void initAddDataSource(std::string const & dataSourceName, Unicode::String const & dataSourceValue);
	void initSubscribeToEvent(int eventType, std::string const & callback);

	void addPropertySubscriptionToEvent(std::string const & widgetName, std::string const & propertyName);
	void modifyEventCallback(std::string const & callback);

	void getAddChildWidget(std::string & widgetType,  std::string & widgetName) const;
	void getSetProperty(std::string & propertyName, Unicode::String & propertyValue) const;
	void getAddDataItem(std::string & dataItemName, Unicode::String & dataItemValue) const;
	void getAddDataSourceContainer(std::string & dataSourceContainerName, Unicode::String & dataSourceContainerValue) const;
	void getAddDataSource(std::string & dataSourceName, Unicode::String & dataSourceValue) const;
	void getSubscribeToEventCallback(int & eventType, std::string & callback) const;

	void getPropertySubscriptionsForEvent(WidgetPropertyVector & widgetProperties) const;

	Type getType() const;
	std::string const & getTargetWidget() const;
	bool isSubscriptionToEvent(int eventType, std::string const & widgetName);

	bool operator==(SuiCommand const & rhs) const;
	bool operator!=(SuiCommand const & rhs) const;

public:

	void get(Archive::ReadIterator & source);
	void put(Archive::ByteStream & target) const;

private:

	Type m_type;
	UnicodeStringVector m_parametersWide;
	StringVector m_parametersNarrow;
};

//----------------------------------------------------------------------

inline SuiCommand::Type SuiCommand::getType() const
{
	return m_type;
}

//----------------------------------------------------------------------

inline bool SuiCommand::isSubscriptionToEvent(int eventType, std::string const & widgetName)
{
	if (getType() == SCT_subscribeToEvent)
	{
		if (widgetName == getTargetWidget())
		{
			int myEventType = 0;
			std::string callback;
			getSubscribeToEventCallback(myEventType, callback);
			return myEventType == eventType;
		}
	}

	return false;
}

//----------------------------------------------------------------------

inline bool SuiCommand::operator!=(SuiCommand const & rhs) const
{
	return !(*this == rhs);
}

//======================================================================

#endif
