// ======================================================================
//
// ServerSynchronizedUi.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef	INCLUDED_ServerSynchronizedUi_H
#define	INCLUDED_ServerSynchronizedUi_H


#include "Archive/AutoDeltaByteStream.h"
#include "sharedFoundation/NetworkId.h"

#include "sharedFoundation/Watcher.h"

class BaselinesMessage;
class Client;
class DeltasMessage;
class ServerObject;

class ServerSynchronizedUi
{
public:
	explicit             ServerSynchronizedUi(ServerObject& owner);
	virtual             ~ServerSynchronizedUi() = 0;

	void                 addClientObject(ServerObject & obj);
	void                 removeClientObject(const NetworkId & client);
	void                 removeAllClientObjects();

	void                 applyBaselines(Archive::ReadIterator & source);
	void                 applyDeltas(const DeltasMessage& source);
	void                 onAuthorityTransferred();
	void                 sendDeltas() const;

	int                  getNumClients () const;
	const std::vector<NetworkId> & getClients() const;

	ServerObject *       getOwner ();
	const ServerObject * getOwner () const;
	bool                 isOwnerChangingAuthority() const;

	WatchedByList        &getWatchedByList() const;

protected:
	void addToUiPackage(Archive::AutoDeltaVariableBase & source);

private:
	ServerSynchronizedUi();
	ServerSynchronizedUi(const ServerSynchronizedUi &);
	ServerSynchronizedUi& operator= (const ServerSynchronizedUi &);

	void sendBaselinesToClient(const Client& client) const;
	
	std::vector<NetworkId> *   m_clientObjects;
	ServerObject*                 m_owner;
	mutable bool                  m_ownerChangingAuthority;
	Archive::AutoDeltaByteStream  m_uiPackage;

	mutable WatchedByList     m_watchedByList;
};

//----------------------------------------------------------------------

inline WatchedByList &ServerSynchronizedUi::getWatchedByList() const
{
	return m_watchedByList;
}

// ----------------------------------------------------------------------

inline ServerObject *  ServerSynchronizedUi::getOwner ()
{
	return m_owner;
}

//----------------------------------------------------------------------

inline const ServerObject * ServerSynchronizedUi::getOwner () const
{
	return m_owner;
}

//----------------------------------------------------------------------

inline bool ServerSynchronizedUi::isOwnerChangingAuthority() const
{
	return m_ownerChangingAuthority;
}

//----------------------------------------------------------------------


#endif
