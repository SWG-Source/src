// GameConnectionServerMessages.h
// copyright 2001 Verant Interactive


#ifndef	_GameConnectionServerMessages_H
#define	_GameConnectionServerMessages_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedMath/Vector.h"
#include "sharedMathArchive/VectorArchive.h"
#include "sharedFoundation/NetworkId.h"

//-----------------------------------------------------------------------
/**
* A game server is connecting to the user server and is sending intial info about itself.
*/

class NewGameServer : public GameNetworkMessage
{
  public:
    NewGameServer (const std::string & sceneName, uint32 serverId);
    NewGameServer (Archive::ReadIterator & source);
    virtual ~NewGameServer ();

    uint32                          getServerId() const;
    std::string                     getSceneName() const;
        
  private:

    Archive::AutoVariable<std::string> m_sceneName;
    Archive::AutoVariable<uint32>       m_serverId;
    
    NewGameServer();
    NewGameServer(const NewGameServer&);
    NewGameServer& operator= (const NewGameServer&);

};
//-----------------------------------------------------------------------

inline uint32 NewGameServer::getServerId() const
{
    return m_serverId.get();
}
//-----------------------------------------------------------------------

inline std::string NewGameServer::getSceneName() const
{
    return m_sceneName.get();
}

// ======================================================================

/**
 * The TransferControl message is send from GameServer to Connection server when
 * a controlled object has its authority transfered to another server.
 */
class TransferControlMessage : public GameNetworkMessage
{
public:
	TransferControlMessage(NetworkId oid, uint32 newGameServerId, bool skipLoadScreen, const std::vector<NetworkId> &observedObjects);
	TransferControlMessage(Archive::ReadIterator & source);
	~TransferControlMessage();

	NetworkId getNetworkId() const;
	uint32 getGameServerId() const;
	bool getSkipLoadScreen() const;
	const std::vector<NetworkId> & getObservedObjects() const;

private:
	Archive::AutoVariable<NetworkId> m_oid;
	Archive::AutoVariable<uint32> m_gameServerId;
	Archive::AutoVariable<bool> m_skipLoadScreen;
 	Archive::AutoArray<NetworkId> m_observedObjects;
    
	TransferControlMessage();
	TransferControlMessage(const TransferControlMessage&);
	TransferControlMessage & operator= (const TransferControlMessage&);
};

//-----------------------------------------------------------------------

inline NetworkId TransferControlMessage::getNetworkId() const
{
	return m_oid.get();
}
//-----------------------------------------------------------------------

inline uint32 TransferControlMessage::getGameServerId() const
{
	return m_gameServerId.get();
}

//-----------------------------------------------------------------------

inline bool TransferControlMessage::getSkipLoadScreen() const
{
	return m_skipLoadScreen.get();
}

//-----------------------------------------------------------------------

inline const std::vector<NetworkId> &TransferControlMessage::getObservedObjects() const
{
	return m_observedObjects.get();
}

//-----------------------------------------------------------------------
/**
 * The Control Assumed message is sent from Game Server to Connection server in response
 * to a NewClient message to let the connection server that it has assumed authoritative
 * control of that object.
 */

class ControlAssumed : public GameNetworkMessage
{
  public:
    explicit ControlAssumed(NetworkId oid, const std::string & newSceneName, const bool skipLoadScreen,
                            const Vector &startPos, const float startYaw, const std::string & templateName,
                            const int64 timeSeconds);
    ControlAssumed(Archive::ReadIterator & source);
    virtual ~ControlAssumed();

    NetworkId                        getNetworkId      () const;
    const std::string &              getSceneName      () const;
	const bool                       getSkipLoadScreen () const;
    const Vector &                   getStartPosition  () const;
    const float                      getStartYaw       () const;
    const std::string &              getTemplateName   () const;
	const int64                      getTimeSeconds    () const;
    
  private:
    Archive::AutoVariable<NetworkId>    m_oid;
    Archive::AutoVariable<std::string>  m_sceneName;
	Archive::AutoVariable<bool>         m_skipLoadScreen;
    Archive::AutoVariable<Vector>       m_startPosition;
	Archive::AutoVariable<float>        m_startYaw;
    Archive::AutoVariable<std::string>  m_templateName;
	Archive::AutoVariable<int64>        m_timeSeconds;
    

    ControlAssumed();
    ControlAssumed(const ControlAssumed &);
    ControlAssumed & operator= (const ControlAssumed &);
};

//-----------------------------------------------------------------------

inline NetworkId ControlAssumed::getNetworkId() const
{
    return m_oid.get();
}
//-----------------------------------------------------------------------

inline const std::string & ControlAssumed::getSceneName() const
{
    return m_sceneName.get();
}

//-----------------------------------------------------------------------

inline const bool ControlAssumed::getSkipLoadScreen() const
{
	return m_skipLoadScreen.get();
}

//-----------------------------------------------------------------------

inline const Vector & ControlAssumed::getStartPosition() const
{
    return m_startPosition.get();
}

//-----------------------------------------------------------------------

inline const float ControlAssumed::getStartYaw() const
{
    return m_startYaw.get();
}

//-----------------------------------------------------------------------

inline const std::string & ControlAssumed::getTemplateName() const
{
    return m_templateName.get();
}

//-----------------------------------------------------------------------

inline const int64 ControlAssumed::getTimeSeconds() const
{
	return m_timeSeconds.get();
}

//-----------------------------------------------------------------------
/**
 * The Drop Client message is sent from Connection Server to the Game Server
 * when a particular client needs to be dropped from the game server
 * 
 */
class DropClient : public GameNetworkMessage
{
  public:
    explicit DropClient (NetworkId oid);
    DropClient (Archive::ReadIterator & source);
    virtual ~DropClient ();

    void 				   setImmediate(bool newValue);
    
    NetworkId                              getNetworkId() const;
    bool				   getImmediate() const;

  private:
    Archive::AutoVariable<NetworkId>       m_oid;
    Archive::AutoVariable<bool>		   m_immediate;

    DropClient();
    DropClient(const DropClient &);
    DropClient& operator = (const DropClient &);
};

//-----------------------------------------------------------------------

inline void DropClient::setImmediate(bool newValue)
{
    m_immediate.set(newValue);
}

inline bool DropClient::getImmediate() const
{
    return m_immediate.get();
}

inline NetworkId DropClient::getNetworkId() const
{
    return m_oid.get();
}

//-----------------------------------------------------------------------
/**
 * The KickPlayer message is sent from Game Server to Connection Server
 * in order to forcibly disconnect a client.
 * 
 */
class KickPlayer : public GameNetworkMessage
{
  public:
    KickPlayer (const NetworkId& oid, const std::string & reason);
    KickPlayer (Archive::ReadIterator & source);
    virtual ~KickPlayer ();

    const NetworkId &                     getNetworkId() const;
	const std::string &                   getReason() const;
	
  private:
    Archive::AutoVariable<NetworkId>       m_oid;
	Archive::AutoVariable<std::string>     m_reason;

    KickPlayer();
    KickPlayer(const KickPlayer &);
    KickPlayer& operator = (const KickPlayer &);
};

//-----------------------------------------------------------------------

inline const NetworkId& KickPlayer::getNetworkId() const
{
    return m_oid.get();
}

//-----------------------------------------------------------------------

inline const std::string& KickPlayer::getReason() const
{
	return m_reason.get();
}

//-----------------------------------------------------------------------

class GameClientMessage : public GameNetworkMessage
{
public:
	
	GameClientMessage (const std::vector<NetworkId> & distributionList, bool reliable, const GameNetworkMessage& msg);
	GameClientMessage (const std::vector<NetworkId> & distributionList, bool reliable, Archive::ReadIterator & msg);
    GameClientMessage (Archive::ReadIterator & source);
    ~GameClientMessage();
	
	const std::vector<NetworkId> &   getDistributionList  () const;
    bool                       getReliable          () const;
    const Archive::ByteStream& getByteStream        () const;
	// @todo : this is a hack. Implement bytestreams with iterators!!!!!!
	void                       pack(Archive::ByteStream & target) const; // special handling for multiple reads    
	
private:
	
    Archive::AutoArray<NetworkId>              distributionList;
    Archive::AutoVariable<bool>                reliable;
    Archive::AutoVariable<Archive::ByteStream> byteStream;
    
    GameClientMessage();
    GameClientMessage(const GameClientMessage&);
    const GameClientMessage& operator= (GameClientMessage&);
};

//-----------------------------------------------------------------------
inline const std::vector<NetworkId> & GameClientMessage::getDistributionList() const
{
    return distributionList.get();
}

//-----------------------------------------------------------------------
inline bool GameClientMessage::getReliable() const
{
    return reliable.get();
}
//-----------------------------------------------------------------------

inline const Archive::ByteStream & GameClientMessage::getByteStream() const
{
    return byteStream.get();
}

//-----------------------------------------------------------------------

#endif	// _GameConnectionServerMessages_H
