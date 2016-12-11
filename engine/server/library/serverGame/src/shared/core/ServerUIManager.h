//-----------------------------------------------------------------------
// ServerUIManager.h
// copyright 2001 Verant Interactive
//-----------------------------------------------------------------------

#include "sharedMessageDispatch/Receiver.h"
#include "sharedMessageDispatch/Emitter.h"
#include "Singleton/Singleton.h"

#include "sharedFoundation/Watcher.h"

class Client;
class ServerObject;
class ServerUIPage;
class NetworkId;

//-----------------------------------------------------------------------

class ServerUIManager
{
public:
	static void install();
	static void remove();

	static void receiveMessage    (const MessageDispatch::MessageBase& message);
	static void onPlayerAuthorityTransferred(const NetworkId &playerCreatureId);
	static void onPlayerLogout    (const NetworkId &playerCreatureId);
	static int createPage         (const std::string& pageName, const ServerObject& owner, const NetworkId& primaryControlledObject);
	static ServerUIPage* getPage  (int pageId);
	static bool showPage          (int pageId);
	static bool closePage         (int pageId);

	static int getNumServerUIPages(void) {return static_cast<int>(m_pages.size());}

private:
	static int createPage         (const std::string& pageName, const ServerObject& owner, const NetworkId& primaryControlledObject, const std::string& callbackFunction);
	static bool setPageProperty   (int pageId, const std::string& widgetName, const std::string& propertyName, const Unicode::String& propertyValue);
	static void deleteClientPages (const NetworkId& primaryControlledObject);
	static bool removePage        (int pageId);
	static bool showPage          (ServerUIPage & page);

	static int  getNewPageId();

private:
	static std::map<int, ServerUIPage*>  m_pages;
	static std::multimap<NetworkId, int>  m_primaryControlledObjectMap;
	static int m_pageCounter;
};

//-----------------------------------------------------------------------
