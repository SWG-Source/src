// NetworkIdManager.h
// copyright 2001 Verant Interactive
// Author: Justin Randall

#ifndef	_INCLUDED_NetworkIdManager_H
#define _INCLUDED_NetworkIdManager_H

#include "sharedFoundation/NetworkId.h"

class Object;

//-----------------------------------------------------------------------

class NetworkIdManager
{
public:
	~NetworkIdManager();

	typedef std::unordered_map<NetworkId, Object *, NetworkId::Hash> NetworkIdObjectHashMap;

	static void                      addObject      (Object & newObject);
	static Object *                  getObjectById  (const NetworkId & source);
	static void                      removeObject   (const Object & oldObject);
	static NetworkIdObjectHashMap const & getAllObjects();

	static void                      setReportObjectLeaks(bool reportObjectLeaks);

protected:
	NetworkIdManager();
	NetworkIdManager (const NetworkIdManager&);
	static NetworkIdManager & getInstance ();
	NetworkIdManager& operator= (const NetworkIdManager&);

private:
	NetworkIdObjectHashMap* const m_objectHashMap;

	static NetworkIdManager                             ms_instance;
	static bool                                         ms_reportObjectLeaks;
};

//----------------------------------------------------------------------

inline void NetworkIdManager::setReportObjectLeaks(bool reportObjectLeaks)
{
	ms_reportObjectLeaks = reportObjectLeaks;
}

#endif	// _INCLUDED_NetworkIdManager_H
