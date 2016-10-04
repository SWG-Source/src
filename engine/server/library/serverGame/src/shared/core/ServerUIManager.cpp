//-----------------------------------------------------------------------
// ServerUIManager.cpp
// copyright 2001 Verant Interactive
//-----------------------------------------------------------------------

#include "serverGame/FirstServerGame.h"
#include "ServerUIManager.h"

#include "serverGame/Client.h"
#include "serverGame/GameServer.h"
#include "serverGame/ServerMessageForwarding.h"
#include "serverGame/ServerObject.h"
#include "serverGame/ServerUIPage.h"
#include "serverGame/ServerWorld.h"
#include "serverGame/SuiPageDataServer.h"
#include "serverGame/SuiPageDataServerArchive.h"
#include "serverScript/GameScriptObject.h"
#include "serverScript/ScriptDictionary.h"
#include "serverScript/ScriptParameters.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedGame/SuiEventType.h"
#include "sharedGame/SuiPageData.h"
#include "sharedGame/SuiWidgetProperty.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "sharedNetworkMessages/ServerUserInterfaceMessages.h"
#include "sharedNetworkMessages/SuiCreatePageMessage.h"
#include "sharedNetworkMessages/SuiEventNotification.h"
#include "sharedNetworkMessages/SuiUpdatePageMessage.h"

#include "sharedFoundation/CrcConstexpr.hpp"

#include <algorithm>
#include <map>
#include <memory>

//----------------------------------------------------------------------

std::map<int, ServerUIPage*>         ServerUIManager::m_pages;
std::multimap<NetworkId, int>        ServerUIManager::m_primaryControlledObjectMap;
int                                  ServerUIManager::m_pageCounter = 1;


//-----------------------------------------------------------------------

void ServerUIManager::onPlayerAuthorityTransferred(const NetworkId& playerCreatureId)
{
	for (std::multimap<NetworkId, int>::const_iterator i = m_primaryControlledObjectMap.lower_bound(playerCreatureId); i != m_primaryControlledObjectMap.upper_bound(playerCreatureId); ++i)
	{		
		ServerUIPage * const p = getPage(i->second);
		if (p)
		{	
			GenericValueTypeMessage<std::pair<NetworkId, SuiPageDataServer> > const pageChangeAuthority("PageChangeAuthority", std::make_pair(playerCreatureId, p->getPageDataServer()));			
			ServerMessageForwarding::send(pageChangeAuthority);
		}
	}
	deleteClientPages(playerCreatureId);
}

//-----------------------------------------------------------------------

void ServerUIManager::onPlayerLogout(const NetworkId& playerCreatureId)
{
	deleteClientPages(playerCreatureId);
}

//-----------------------------------------------------------------------

void ServerUIManager::install()
{
	std::for_each(m_pages.begin(), m_pages.end(), PointerDeleterPairSecond());
	m_pages.clear();
	m_primaryControlledObjectMap.clear();
	m_pageCounter = 1;
}

//-----------------------------------------------------------------------

void ServerUIManager::remove()
{
	std::for_each(m_pages.begin(), m_pages.end(), PointerDeleterPairSecond());
	m_pages.clear();
	m_primaryControlledObjectMap.clear();
}

//-----------------------------------------------------------------------


int ServerUIManager::getNewPageId()
{
	//Try to get a unique cross-server value, so take the process ID, which should be < 255, and or it into the bottom 8 bits
	int pageCounter = m_pageCounter++;
	pageCounter <<= 8;	
	uint32 processId = GameServer::getInstance().getProcessId();
	DEBUG_WARNING(processId > 255, ("ServerUIManager: unexpectedly high GameServer process id %d, may result in SUI collisions", processId));
	processId &= 0x000000FF;
	pageCounter |= processId;
	return pageCounter;
}

/**
* @deprecated
*/

int ServerUIManager::createPage(const std::string& pageName, const ServerObject& owner, const NetworkId& primaryControlledObject, const std::string& callbackFunction)
{
	int const pageId = ServerUIManager::createPage(pageName, owner, primaryControlledObject);
	if (pageId < 0)
		return pageId;

	ServerUIPage * const serverUIPage = ServerUIManager::getPage(pageId);
	if (serverUIPage != nullptr)
	{
		serverUIPage->setCallback(callbackFunction);
		return pageId;
	}

	return -1;
}

//----------------------------------------------------------------------

int ServerUIManager::createPage(const std::string& pageName, const ServerObject& owner, const NetworkId& primaryControlledObject)
{
	int const pageId = getNewPageId();
	ServerUIPage * const p = new ServerUIPage(pageName, owner, primaryControlledObject, pageId);
	if (m_pages.count(pageId))
		IGNORE_RETURN(removePage(pageId));
	IGNORE_RETURN(m_pages.insert(std::make_pair(pageId, p)));
	IGNORE_RETURN(m_primaryControlledObjectMap.insert(std::make_pair(primaryControlledObject, pageId)));
	return pageId;
}

//-----------------------------------------------------------------------

void ServerUIManager::deleteClientPages(const NetworkId& primaryControlledObject)
{
	//if(!client)
	//	return;
	//find every page associated with this client
	for(std::multimap<NetworkId, int>::const_iterator i = m_primaryControlledObjectMap.lower_bound(primaryControlledObject); i != m_primaryControlledObjectMap.upper_bound(primaryControlledObject); ++i)
	{
		ServerUIPage* const p = getPage(i->second);
		//remove item from the pages map
		IGNORE_RETURN(m_pages.erase(i->second));
		//actually delete it
		delete p;
	}

	//remove item from client map
	IGNORE_RETURN(m_primaryControlledObjectMap.erase(primaryControlledObject));
}

//-----------------------------------------------------------------------

ServerUIPage* ServerUIManager::getPage(int pageId)
{
	std::map<int, ServerUIPage*>::const_iterator iterFind = m_pages.find(pageId);
	if (iterFind == m_pages.end())
		return nullptr;
	else
		return iterFind->second;
}

//-----------------------------------------------------------------------

bool ServerUIManager::showPage(int pageId)
{
	ServerUIPage * const page = getPage(pageId);
	if (page != nullptr)
		return showPage(*page);

	WARNING(true, ("ServerUIManager::showPage(%d) invalid page", pageId));
	return false;
}

//-----------------------------------------------------------------------

bool ServerUIManager::showPage(ServerUIPage &  page)
{
	Client * const client = page.getClient();
	if (client == nullptr)
	{
//		WARNING(true, ("ServerUIManager::showPage attempt to show page on nullptr client"));
		return false;
	}

	ServerObject const * const characterObject = client->getCharacterObject();
	if (characterObject == nullptr)
	{
		WARNING(true, ("ServerUIManager::showPage attempt to show page to client with no character object"));
		return false;
	}

	if (!characterObject->isAuthoritative())
	{
		WARNING(true, ("ServerUIManager::showPage attempt to show page to non-authoritative client"));
		return false;
	}

	if (page.getPageDataServer().isActive())
	{
		SuiPageData updatePage;
		if (page.getPageDataServer().getPageUpdate(updatePage))
		{
			SuiUpdatePageMessage const updateMsg(updatePage);
			client->send(updateMsg, true);
		}
	}
	else
	{
		SuiCreatePageMessage const createMsg(page.getPageData());
		client->send(createMsg, true);
		page.getPageDataServer().setActive();
	}

	return true;
}

//-----------------------------------------------------------------------

bool ServerUIManager::closePage(int pageId)
{
	//send close_page message
	const ServerUIPage * const page = getPage(pageId);
	if(page == nullptr)
		return false;

	Client * const client = page->getClient();
	if(client == nullptr)
		return false;

	SuiForceClosePage msg;
	msg.setClientPageId(pageId);
	client->send(msg, true);

	return ServerUIManager::removePage(pageId);
}

//----------------------------------------------------------------------

bool ServerUIManager::removePage(int pageId)
{
	//send close_page message
	const ServerUIPage * const page = getPage(pageId);
	if(page == nullptr)
		return false;

	NetworkId const & primaryControlledObject = page->getPrimaryControlledObject();

	for(std::multimap<NetworkId, int>::iterator i = m_primaryControlledObjectMap.lower_bound(primaryControlledObject); i != m_primaryControlledObjectMap.upper_bound(primaryControlledObject);)
	{
		if (i->second == pageId)
		{
			m_primaryControlledObjectMap.erase(i++);
		}
		else
		{
			++i;
		}
	}

	IGNORE_RETURN(m_pages.erase(pageId));

	delete page;

	return true;
}

//-----------------------------------------------------------------------

void ServerUIManager::receiveMessage(const MessageDispatch::MessageBase& message)
{
	Archive::ReadIterator ri = NON_NULL(dynamic_cast<const GameNetworkMessage *>(&message))->getByteStream().begin();
		
	if (message.isType(SuiEventNotification::MessageType))
	{
		SuiEventNotification const suiEventNotification(ri);

		int const pageId = suiEventNotification.getPageId();

		ServerUIPage const * const page = getPage(pageId);
		if (page == nullptr)
			return;

		Client * const client = page->getClient();
		if (client == nullptr)
		{
			removePage(pageId);
			return;
		}

		ServerObject * const characterObject = client->getCharacterObject();

		if (characterObject == nullptr)
		{
			WARNING(true, ("ServerUIManager::recieveMessage got SuiEventNotification for nullptr character object"));
			removePage(pageId);
			return;
		}

		SuiPageData const & suiPageData = page->getPageData();

		SuiPageDataServer const & suiPageDataServer = page->getPageDataServer();

		ServerObject * const ownerObject = ServerWorld::findObjectByNetworkId(suiPageDataServer.getOwnerId());

		if (ownerObject == nullptr)
		{
			WARNING(true, ("ServerUIManager::recieveMessage got SuiEventNotification for nullptr owner object"));
			removePage(pageId);
			return;
		}

		GameScriptObject * const gso = ownerObject->getScriptObject();
		if (gso == nullptr)
		{
			WARNING(true, ("ServerUIManager::recieveMessage got SuiEventNotification for owner object with no scripts"));
			removePage(pageId);
			return;
		}

		int const eventIndex = suiEventNotification.getSubscribedEventIndex();
		SuiCommand const * const command = suiPageData.findSubscribeToEventCommandByIndex(eventIndex);

		if (command == nullptr)
		{
			WARNING(true, ("ServerUIManager::recieveMessage got SuiEventNotification invalid notification index [%d]", eventIndex));
			return;
		}

		SuiCommand::WidgetPropertyVector widgetProperties;
		command->getPropertySubscriptionsForEvent(widgetProperties);

		//-- it is ok for the notification property count to be fewer than our subscription list.
		//-- this just means additional properties have been subscribed to, but the client hasn't processed
		//-- the subscriptions yet
		int const incomingPropertyCount = suiEventNotification.getPropertyCount();
		if (static_cast<int>(widgetProperties.size()) < incomingPropertyCount)
		{
			WARNING(true, ("ServerUIManager::recieveMessage got SuiEventNotification invalid property count [%d], expected [%d]", 
				static_cast<int>(widgetProperties.size()), incomingPropertyCount));
			return;
		}

		int eventType = 0;
		std::string callback;
		command->getSubscribeToEventCallback(eventType, callback);

		if (callback.empty())
		{
			WARNING(true, ("ServerUIManager::recieveMessage got SuiEventNotification invalid callback"));
			return;
		}

		std::string const & eventTypeName = SuiEventType::getNameForType(eventType);

		ScriptParams sp;
		sp.addParam(pageId, "pageId");

		if (eventType == SuiEventType::SET_onClosedOk)
			sp.addParam("Ok", "buttonPressed");
		else if (eventType == SuiEventType::SET_onClosedCancel)
			sp.addParam("Cancel", "buttonPressed");
		else
			sp.addParam("Unknown", "buttonPressed");

		sp.addParam(eventTypeName.c_str(), "eventType");
		sp.addParam(command->getTargetWidget().c_str(), "eventWidgetName");

		sp.addParam(characterObject->getNetworkId(), "player");

		{
			std::string propertyString;

			int incomingPropertyIndex = 0;
			for (SuiCommand::WidgetPropertyVector::const_iterator it = widgetProperties.begin(); it != widgetProperties.end() && incomingPropertyIndex < incomingPropertyCount; ++it, ++incomingPropertyIndex)
			{
				SuiWidgetProperty const & widgetProperty = *it;

				propertyString.clear();
				propertyString = widgetProperty.widgetName;
				propertyString.push_back('.');
				propertyString += widgetProperty.propertyName;

				sp.addParam(suiEventNotification.getProperty(incomingPropertyIndex), propertyString);
			}
		}

		ScriptDictionaryPtr sd;
		//it allocates sd, we have to clean it up later
		gso->makeScriptDictionary(sp, sd);
		if(sd.get() == nullptr)
			return;

		//call the script callback function
		gso->handleMessage(callback, sd);

		//by design, the page is considered invalid after OK or Cancel is pressed
		//so we go ahead and destroy the page, if script didn't already destroy
		//the page, otherwise, we'll have a memory leak
		if ((eventType == SuiEventType::SET_onClosedOk) || (eventType == SuiEventType::SET_onClosedCancel))
			IGNORE_RETURN(removePage(pageId));
	}
	else if(message.getType() == constcrc("PageChangeAuthority"))
	{
		//DEBUG_WARNING(true, ("ServerUIManager: received PageChangeAuthority message"));
		const GenericValueTypeMessage<std::pair<NetworkId, SuiPageDataServer> > pageChangeAuthority(ri);
		const NetworkId & primaryControlledObject = pageChangeAuthority.getValue().first;
		const SuiPageDataServer & pageDataServer  = pageChangeAuthority.getValue().second;
		const int pageId = pageDataServer.getPageData().getPageId();
		//DEBUG_WARNING(true, ("ServerUIManager: received PageChangeAuthority message, pageId = %d", pageId));
		SuiPageDataServer * const newPageDataServer = new SuiPageDataServer(pageDataServer);
		ServerUIPage * const newPage = new ServerUIPage(primaryControlledObject, newPageDataServer);
		if (m_pages.count(pageId))
			IGNORE_RETURN(removePage(pageId));
		IGNORE_RETURN(m_pages.insert(std::make_pair(pageId, newPage)));
		IGNORE_RETURN(m_primaryControlledObjectMap.insert(std::make_pair(primaryControlledObject, pageId)));
	}
}

//-----------------------------------------------------------------------

bool ServerUIManager::setPageProperty(int pageId, const std::string& widgetName, const std::string& propertyName, const Unicode::String& propertyValue)
{
	const ServerUIPage* page = getPage(pageId);
	if(!page)
		return false;
	Client* client = page->getClient();
	if(!client)
		return false;

	SuiSetProperty msg;
	msg.setClientPageId(pageId);
	msg.setWidgetName(widgetName);
	msg.setPropertyName(propertyName);
	msg.setPropertyValue(propertyValue);
	client->send(msg, true);
	return true;
}

//-----------------------------------------------------------------------
