//-----------------------------------------------------------------------
// ServerUIPage.h
// copyright 2001 Verant Interactive
//-----------------------------------------------------------------------

#include "sharedNetworkMessages/ServerUserInterfaceMessages.h"

//-----------------------------------------------------------------------

class Client;
class ServerObject;
template <typename T> class Watcher;
class SuiPageData;
class SuiPageDataServer;
class Vector;

//-----------------------------------------------------------------------

class ServerUIPage
{
public:
	ServerUIPage(const std::string& pageName, ServerObject const & owner, NetworkId const & primaryControlledObject, int pageId);
	ServerUIPage(NetworkId const & primaryControlledObject, SuiPageDataServer *pageDataServer);
	~ServerUIPage();

	void                     addChildWidget        (const std::string& widgetType, const std::string& widgetName,   const std::string& parentWidgetName);
	void                     setProperty           (const std::string& widgetName, const std::string& propertyName, const Unicode::String& propertyValue);
	void                     addDataItem           (const std::string& dataSource, const std::string& dataItemName, const Unicode::String& dataItemValue);
	void                     addDataSourceContainer(const std::string& dataSourceContainer, const std::string& dataSourceContainerName, const Unicode::String& dataSourceContainerValue);
	void                     addDataSource		   (const std::string& dataSource, const std::string& dataSourceName, const Unicode::String& dataSourceValue);
	void                     subscribeToProperty   (std::string const & widget, std::string const & propertyName);
	void                     clearDataSource       (const std::string& dataSource);
	void                     clearDataSourceContainer(const std::string& dataSource);
	void                     setCallback           (const std::string& callbackFunction);
	NetworkId const &        getPrimaryControlledObject () const;
	Client*                  getClient             () const;
	ServerObject*            getOwner              () const;
	const SuiCreatePage&     getCreationMessage    () const;
	const SuiForceClosePage& getForceCloseMessage  () const;

	void                     setAssociatedObjectId (const NetworkId & id);
	void                     setAssociatedLocation (const Vector & location);
	void                     setMaxRangeFromObject (float range);

	SuiPageData const & getPageData() const;
	SuiPageData & getPageData();
	SuiPageDataServer const & getPageDataServer() const;
	SuiPageDataServer & getPageDataServer();

private:
	//disabled
	ServerUIPage();
	ServerUIPage(const ServerUIPage&);
	ServerUIPage &operator=(const ServerUIPage&);

private:
	NetworkId              m_primaryControlledObject;
	SuiPageDataServer *    m_pageDataServer;
};

//----------------------------------------------------------------------

inline SuiPageDataServer const & ServerUIPage::getPageDataServer() const
{
	return *NON_NULL(m_pageDataServer);
}

//----------------------------------------------------------------------

inline SuiPageDataServer & ServerUIPage::getPageDataServer()
{
	return *NON_NULL(m_pageDataServer);
}

//-----------------------------------------------------------------------

