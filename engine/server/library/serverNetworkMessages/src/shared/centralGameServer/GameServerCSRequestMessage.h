// CSToolREquest.h
// Copyright 2004, Sony Online Entertainment Inc., all rights reserved. 
// Author: Rob Hanz

#ifndef	_INCLUDED_GameServerCSRequest_H
#define	_INCLUDED_GameServerCSRequest_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"

class CSToolRequest;

//-----------------------------------------------------------------------
/** A Central Server is telling the login server which cluster it is
	servicing.
*/
class GameServerCSRequestMessage : public GameNetworkMessage
{
  public:
    GameServerCSRequestMessage ( uint32 accountId, 
    				 const std::string & command,
				 uint32 toolId,
				 uint32 accessLevel, 
				 uint32 loginServerID,
				 const std::string & userName,
				 const std::string & commandName );
    GameServerCSRequestMessage (Archive::ReadIterator & source);
    virtual ~GameServerCSRequestMessage ();
    
    uint32 			getAccountId () const;
    const std::string & 	getCommandString() const;
    uint32			getToolId() const;
    uint32			getAccessLevel() const;
    uint32			getLoginServerID() const;
    const std::string& 		getUserName() const;
    const std::string& 		getCommandName() const;
    
  private:
    Archive::AutoVariable< uint32 >      m_accountId;
    Archive::AutoVariable<std::string>   m_command;
    Archive::AutoVariable< uint32 > 	 m_toolId;
    Archive::AutoVariable< uint32 >	 m_accessLevel;
    Archive::AutoVariable< uint32 >	 m_loginServerID;
    Archive::AutoVariable< std::string > m_userName;
    Archive::AutoVariable< std::string > m_commandName;
    

    GameServerCSRequestMessage();
    GameServerCSRequestMessage(const GameServerCSRequestMessage&);
    GameServerCSRequestMessage& operator= (const GameServerCSRequestMessage&);

};

inline uint32 GameServerCSRequestMessage::getAccountId() const
{
	return m_accountId.get();
}

inline const std::string & GameServerCSRequestMessage::getCommandString() const
{
	return m_command.get();
}

inline uint32 GameServerCSRequestMessage::getAccessLevel() const
{
	return m_accessLevel.get();
}

inline uint32 GameServerCSRequestMessage::getToolId() const
{
	return m_toolId.get();
}

inline uint32 GameServerCSRequestMessage::getLoginServerID() const
{
	return m_loginServerID.get();
}

inline const std::string & GameServerCSRequestMessage::getUserName() const
{
	return m_userName.get();
}

inline const std::string & GameServerCSRequestMessage::getCommandName() const
{
	return m_commandName.get();
}
#endif //_INCLUDED_GameServerCSRequest_H
