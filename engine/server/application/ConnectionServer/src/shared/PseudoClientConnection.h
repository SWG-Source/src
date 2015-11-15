// PseudoClientConnection.h
// copyright 2001 Verant Interactive

#ifndef	_PseudoClientConnection_H
#define	_PseudoClientConnection_H

#include "serverNetworkMessages/TransferCharacterData.h"

namespace Archive
{
	class ByteStream;
}

class GameConnection;

//-----------------------------------------------------------------------

class PseudoClientConnection 
{
public:
	PseudoClientConnection(const TransferCharacterData & transferDataFromCentralServer, unsigned int stationId);
	virtual ~PseudoClientConnection();

	void                             controlAssumed             ();
	void                             onBankLoaded               ();
	void                             onPackedHousesLoaded       ();

	static PseudoClientConnection *  getPseudoClientConnection  (const NetworkId & characterId);
	static PseudoClientConnection *  getPseudoClientConnection  (unsigned int stationId);
	static void                      install                    ();
	void                             requestGameServerForLogin  () const;
	static bool                      tryToDeliverMessageTo      (unsigned int stationId, const Archive::ByteStream & msg);
	static bool                      tryToDeliverMessageTo      (const NetworkId & characterId, const Archive::ByteStream & msg);
	static void                      gameConnectionClosed       (const GameConnection *);
	
	const TransferCharacterData &    getTransferCharacterData   () const;

protected:
	friend class CentralConnection;
	static void  destroyAllPseudoClientConnectionInstances  ();
	
private:
	PseudoClientConnection();
	PseudoClientConnection(const PseudoClientConnection&);
	PseudoClientConnection& operator=(const PseudoClientConnection&);
	
	void  receiveMessage        (const Archive::ByteStream &);
	void  gameConnectionClosed  ();
	
	TransferCharacterData  m_transferCharacterData;
	GameConnection *       m_gameConnection;
	unsigned int           m_trackStationId;
};

//-----------------------------------------------------------------------
#endif	// _PseudoClientConnection_H

