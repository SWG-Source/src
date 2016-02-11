//-----------------------------------------------------------------------
// ServerUIPage.cpp
// copyright 2001 Verant Interactive
//-----------------------------------------------------------------------

#include "serverGame/FirstServerGame.h"
#include "ServerUIPage.h"

#include "UnicodeUtils.h"
#include "serverGame/Client.h"
#include "serverGame/ServerObject.h"
#include "serverGame/SuiPageDataServer.h"
#include "sharedGame/SuiEventType.h"
#include "sharedGame/SuiPageData.h"
#include "sharedObject/NetworkIdManager.h"
#include <vector>

//-----------------------------------------------------------------------

ServerUIPage::ServerUIPage(const std::string& pageName, ServerObject const & owner, NetworkId const & primaryControlledObject, int pageId) :
m_primaryControlledObject (primaryControlledObject),
m_pageDataServer(new SuiPageDataServer(pageId, pageName, owner.getNetworkId()))
{
}

//-----------------------------------------------------------------------

ServerUIPage::ServerUIPage(NetworkId const & primaryControlledObject, SuiPageDataServer *pageDataServer) :
m_primaryControlledObject(primaryControlledObject),
m_pageDataServer(pageDataServer)
{
}

//-----------------------------------------------------------------------

ServerUIPage::~ServerUIPage()
{
	delete m_pageDataServer;
	m_pageDataServer = nullptr;
}

//-----------------------------------------------------------------------

NetworkId const & ServerUIPage::getPrimaryControlledObject () const
{
	return m_primaryControlledObject;
}

Client* ServerUIPage::getClient() const
{
	Object *const object = NetworkIdManager::getObjectById(m_primaryControlledObject);
	if(!object)
	{
		WARNING(true, ("ServerUIPage PrimaryControlledObject doesn't exist"));
		return nullptr;
	}
	ServerObject *const serverObject = object->asServerObject();
	if(!serverObject)
	{
		WARNING(true, ("ServerUIPage PrimaryControlledObject isn't a server object"));
		return nullptr;
	}
	Client * const result = serverObject->getClient();	
	if(!result)
	{
		WARNING(true, ("ServerUIPage PrimaryControlledObject has no client yet"));
		return nullptr;
	}
	return result;
}

//-----------------------------------------------------------------------

ServerObject* ServerUIPage::getOwner() const
{
	Object * const object = NetworkIdManager::getObjectById(getPageDataServer().getOwnerId());
	if (object != nullptr)
		return object->asServerObject();
	return nullptr;
}

//-----------------------------------------------------------------------

void ServerUIPage::addChildWidget(const std::string& widgetType, const std::string& widgetName, const std::string& parentWidgetName)
{
	SuiCommand command(SuiCommand::SCT_addChildWidget, parentWidgetName);
	command.initAddChildWidget(widgetType, widgetName);
	getPageData().addCommand(command);
}

//-----------------------------------------------------------------------

void ServerUIPage::setProperty(const std::string& widgetName, const std::string& propertyName, const Unicode::String& propertyValue)
{
	SuiCommand command(SuiCommand::SCT_setProperty, widgetName);
	command.initSetProperty(propertyName, propertyValue);
	getPageData().addCommand(command);
}

//-----------------------------------------------------------------------

void ServerUIPage::clearDataSource(const std::string& dataSource)
{
	SuiCommand const command(SuiCommand::SCT_clearDataSource, dataSource);
	getPageData().addCommand(command);
}

//-----------------------------------------------------------------------

void ServerUIPage::clearDataSourceContainer(const std::string& dataSourceContainer)
{
    SuiCommand const command(SuiCommand::SCT_clearDataSourceContainer, dataSourceContainer);
    getPageData().addCommand(command);
}

//-----------------------------------------------------------------------

void ServerUIPage::addDataItem(const std::string& dataSource, const std::string& dataItemName, const Unicode::String& dataItemValue)
{
	SuiCommand command(SuiCommand::SCT_addDataItem, dataSource);
	command.initAddDataItem(dataItemName, dataItemValue);
	getPageData().addCommand(command);
}

//-----------------------------------------------------------------------

void ServerUIPage::addDataSourceContainer(const std::string& dataSourceContainer, const std::string& dataSourceContainerName, const Unicode::String& dataSourceContainerValue)
{
    SuiCommand command(SuiCommand::SCT_addDataSourceContainer, dataSourceContainer);
    command.initAddDataSourceContainer(dataSourceContainerName, dataSourceContainerValue);
    getPageData().addCommand(command);
}

//-----------------------------------------------------------------------

void ServerUIPage::addDataSource(const std::string& dataSource, const std::string& dataSourceName, const Unicode::String& dataSourceValue)
{
	SuiCommand command(SuiCommand::SCT_addDataSource, dataSource);
	command.initAddDataSource(dataSourceName, dataSourceValue);
	getPageData().addCommand(command);
}

//-----------------------------------------------------------------------

void ServerUIPage::subscribeToProperty(const std::string& widgetName, const std::string& propertyName)
{
	static std::string const emptyString;
	
	getPageData().subscribeToPropertyForEvent(SuiEventType::SET_onClosedOk, emptyString, widgetName, propertyName);
	getPageData().subscribeToPropertyForEvent(SuiEventType::SET_onClosedCancel, emptyString, widgetName, propertyName);
}

//-----------------------------------------------------------------------

void ServerUIPage::setCallback(const std::string& callbackFunction)
{
	SuiCommand suscribeOkCommand(SuiCommand::SCT_subscribeToEvent, std::string());
	suscribeOkCommand.initSubscribeToEvent(SuiEventType::SET_onClosedOk, callbackFunction);

	SuiCommand suscribeCancelCommand(SuiCommand::SCT_subscribeToEvent, std::string());
	suscribeCancelCommand.initSubscribeToEvent(SuiEventType::SET_onClosedCancel, callbackFunction);

	getPageData().addCommand(suscribeOkCommand);
	getPageData().addCommand(suscribeCancelCommand);
}

//----------------------------------------------------------------------

void ServerUIPage::setAssociatedObjectId (const NetworkId & id)
{
	getPageData().setAssociatedObjectIdWithMaxRange(id, getPageData().getMaxRangeFromObject());
}

//----------------------------------------------------------------------

void  ServerUIPage::setAssociatedLocation (const Vector & location)
{
	getPageData().setAssociatedLocationWithMaxRange(location, getPageData().getMaxRangeFromObject());
}

//----------------------------------------------------------------------

void ServerUIPage::setMaxRangeFromObject (float range)
{
	getPageData().setAssociatedObjectIdWithMaxRange(getPageData().getAssociatedObjectId(), range);
}

//----------------------------------------------------------------------

SuiPageData const & ServerUIPage::getPageData() const
{
	return getPageDataServer().getPageData();
}

//----------------------------------------------------------------------

SuiPageData & ServerUIPage::getPageData()
{
	return getPageDataServer().getPageData();
}

//-----------------------------------------------------------------------

