// CSToolREquest.h
// Copyright 2004, Sony Online Entertainment Inc., all rights reserved. 
// Author: Rob Hanz

#ifndef	_INCLUDED_CSDBNetMessages_H
#define	_INCLUDED_CSDBNetMessages_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------
/** A Central Server is telling the login server which cluster it is
	servicing.
*/
class CSGetCharactersRequestMessage : public GameNetworkMessage
{
  public:
    CSGetCharactersRequestMessage (uint32 accountId, uint32 accessLevel, uint32 targetId, uint32 toolId, uint32 loginServer );
    CSGetCharactersRequestMessage (Archive::ReadIterator & source);
    virtual ~CSGetCharactersRequestMessage();
    
    uint32 			getAccountId () const;
    uint32			getTargetAccountId() const;
    uint32 			getAccessLevel() const;
    uint32			getToolId() const;
    uint32			getLoginServerId() const;

  private:
    Archive::AutoVariable< uint32 >      m_accountId;
    Archive::AutoVariable< uint32 >      m_targetAccount;
    Archive::AutoVariable< uint32 >	 m_accessLevel;
    Archive::AutoVariable< uint32 > 	 m_toolId;
    Archive::AutoVariable< uint32 >	 m_loginServerId;
    

    CSGetCharactersRequestMessage();
    CSGetCharactersRequestMessage(const CSGetCharactersRequestMessage&);
    CSGetCharactersRequestMessage & operator= (const CSGetCharactersRequestMessage&);

};

inline uint32 CSGetCharactersRequestMessage::getAccountId() const
{
	return m_accountId.get();
}

inline uint32 CSGetCharactersRequestMessage::getTargetAccountId() const
{
	return m_targetAccount.get();
}

inline uint32 CSGetCharactersRequestMessage::getAccessLevel() const
{
	return m_accessLevel.get();
}

inline uint32 CSGetCharactersRequestMessage::getToolId() const
{
	return m_toolId.get();
}

inline uint32 CSGetCharactersRequestMessage::getLoginServerId() const
{
	return m_loginServerId.get();
}

class GetCharactersForAccountCSReplyMsg : public GameNetworkMessage
{
  public:
    GetCharactersForAccountCSReplyMsg (uint32 accountId, uint32 accessLevel, uint32 targetId, uint32 toolId, uint32 loginServer, const std::string & response );
    GetCharactersForAccountCSReplyMsg (Archive::ReadIterator & source);
    virtual ~GetCharactersForAccountCSReplyMsg();
    
    uint32 			getAccountId () const;
    uint32			getTargetAccountId() const;
    uint32 			getAccessLevel() const;
    uint32			getToolId() const;
    uint32			getLoginServerId() const;
    const std::string	&		getResponse() const;
    // data go here.

  private:
    Archive::AutoVariable< uint32 >      m_accountId;
    Archive::AutoVariable< uint32 >      m_targetAccount;
    Archive::AutoVariable< uint32 >	 m_accessLevel;
    Archive::AutoVariable< uint32 > 	 m_toolId;
    Archive::AutoVariable< uint32 >	 m_loginServerId;
    Archive::AutoVariable< std::string > m_responseString;
    

    GetCharactersForAccountCSReplyMsg();
    GetCharactersForAccountCSReplyMsg(const GetCharactersForAccountCSReplyMsg&);
    GetCharactersForAccountCSReplyMsg & operator= (const GetCharactersForAccountCSReplyMsg&);

};

inline uint32 GetCharactersForAccountCSReplyMsg::getAccountId() const
{
	return m_accountId.get();
}

inline uint32 GetCharactersForAccountCSReplyMsg::getTargetAccountId() const
{
	return m_targetAccount.get();
}

inline uint32 GetCharactersForAccountCSReplyMsg::getAccessLevel() const
{
	return m_accessLevel.get();
}

inline uint32 GetCharactersForAccountCSReplyMsg::getToolId() const
{
	return m_toolId.get();
}

inline uint32 GetCharactersForAccountCSReplyMsg::getLoginServerId() const
{
	return m_loginServerId.get();
}

const inline std::string& GetCharactersForAccountCSReplyMsg::getResponse() const
{
	return m_responseString.get();
}

class CSGetDeletedItemsRequestMessage : public GameNetworkMessage
{
public:
    CSGetDeletedItemsRequestMessage (uint32 accountId, uint32 accessLevel, uint32 targetId, uint32 toolId, uint32 loginServer );
    CSGetDeletedItemsRequestMessage (Archive::ReadIterator & source);
    virtual ~CSGetDeletedItemsRequestMessage();

    uint32 			getAccountId () const;
    uint32			getTargetAccountId() const;
    uint32 			getAccessLevel() const;
    uint32			getToolId() const;
    uint32			getLoginServerId() const;

  private:
    Archive::AutoVariable< uint32 >      m_accountId;
    Archive::AutoVariable< uint32 >      m_targetAccount;
    Archive::AutoVariable< uint32 >	 m_accessLevel;
    Archive::AutoVariable< uint32 > 	 m_toolId;
    Archive::AutoVariable< uint32 >	 m_loginServerId;
    
    CSGetDeletedItemsRequestMessage();
    CSGetDeletedItemsRequestMessage(const CSGetDeletedItemsRequestMessage&);
    CSGetDeletedItemsRequestMessage & operator= (const CSGetDeletedItemsRequestMessage&);
        
};
inline uint32 CSGetDeletedItemsRequestMessage::getAccountId() const
{
	return m_accountId.get();
}

inline uint32 CSGetDeletedItemsRequestMessage::getTargetAccountId() const
{
	return m_targetAccount.get();
}

inline uint32 CSGetDeletedItemsRequestMessage::getAccessLevel() const
{
	return m_accessLevel.get();
}

inline uint32 CSGetDeletedItemsRequestMessage::getToolId() const
{
	return m_toolId.get();
}

inline uint32 CSGetDeletedItemsRequestMessage::getLoginServerId() const
{
	return m_loginServerId.get();
}

class GetDeletedItemsReplyMessage : public GameNetworkMessage
{
  public:
    GetDeletedItemsReplyMessage (uint32 accountId, uint32 accessLevel, const NetworkId &characterId, uint32 toolId, uint32 loginServer, const std::string & response );
    GetDeletedItemsReplyMessage (Archive::ReadIterator & source);
    virtual ~GetDeletedItemsReplyMessage();
    
    uint32   			getAccountId () const;
    const NetworkId &		getCharacterId() const;
    uint32 			getAccessLevel() const;
    uint32			getToolId() const;
    uint32			getLoginServerId() const;
    const std::string	&		getResponse() const;
    // data go here.

  private:
    Archive::AutoVariable< uint32 >      m_accountId;
    Archive::AutoVariable< NetworkId >   m_characterId;
    Archive::AutoVariable< uint32 >	 m_accessLevel;
    Archive::AutoVariable< uint32 > 	 m_toolId;
    Archive::AutoVariable< uint32 >	 m_loginServerId;
    Archive::AutoVariable< std::string > m_responseString;
    

    GetDeletedItemsReplyMessage();
    GetDeletedItemsReplyMessage(const GetDeletedItemsReplyMessage&);
    GetDeletedItemsReplyMessage & operator= (const GetDeletedItemsReplyMessage&);

};

inline uint32 GetDeletedItemsReplyMessage::getAccountId() const
{
	return m_accountId.get();
}

inline const NetworkId & GetDeletedItemsReplyMessage::getCharacterId() const
{
	return m_characterId.get();
}

inline uint32 GetDeletedItemsReplyMessage::getAccessLevel() const
{
	return m_accessLevel.get();
}

inline uint32 GetDeletedItemsReplyMessage::getToolId() const
{
	return m_toolId.get();
}

inline uint32 GetDeletedItemsReplyMessage::getLoginServerId() const
{
	return m_loginServerId.get();
}

const inline std::string& GetDeletedItemsReplyMessage::getResponse() const
{
	return m_responseString.get();
}

class GetCharacterIdReplyMessage : public GameNetworkMessage
{
  public:
    GetCharacterIdReplyMessage (uint32 accountId, const NetworkId &characterId, uint32 toolId, uint32 loginServer, const std::string & characterName);
    GetCharacterIdReplyMessage (Archive::ReadIterator & source);
    virtual ~GetCharacterIdReplyMessage();
    
    uint32   			getAccountId () const;
    const NetworkId &		getCharacterId() const;
    uint32			getToolId() const;
    uint32			getLoginServerId() const;
    const std::string	&	getCharacterName() const;
    // data go here.

  private:
    Archive::AutoVariable< uint32 >      m_accountId;
    Archive::AutoVariable< NetworkId >   m_characterId;
    Archive::AutoVariable< uint32 > 	 m_toolId;
    Archive::AutoVariable< uint32 >	 m_loginServerId;
    Archive::AutoVariable< std::string > m_characterName;
    

    GetCharacterIdReplyMessage();
    GetCharacterIdReplyMessage(const GetCharacterIdReplyMessage&);
    GetCharacterIdReplyMessage & operator= (const GetCharacterIdReplyMessage&);

};

inline uint32 GetCharacterIdReplyMessage::getAccountId() const
{
	return m_accountId.get();
}

inline const NetworkId & GetCharacterIdReplyMessage::getCharacterId() const
{
	return m_characterId.get();
}

inline const std::string & GetCharacterIdReplyMessage::getCharacterName() const
{
	return m_characterName.get();
}

inline uint32 GetCharacterIdReplyMessage::getToolId() const
{
	return m_toolId.get();
}

inline uint32 GetCharacterIdReplyMessage::getLoginServerId() const
{
	return m_loginServerId.get();
}

class DBCSRequestMessage : public GameNetworkMessage
{
public:
	DBCSRequestMessage( uint32 accountId, uint32 accessLevel, uint32 toolId, uint32 loginServer, const std::string & command, const std::string & commandline );
	
	DBCSRequestMessage( Archive::ReadIterator & source );
	virtual ~DBCSRequestMessage();
	
	uint32			getAccountId() const;
	uint32			getAccessLevel() const;
	uint32			getToolId() const;
	uint32			getLoginServerId() const;
	const std::string &		getCommand() const;
	const std::string &		getCommandLine() const;
	
	
private:
	Archive::AutoVariable< uint32 >		m_accountId;
	Archive::AutoVariable< uint32 >		m_accessLevel;
	Archive::AutoVariable< uint32 >		m_toolId;
	Archive::AutoVariable< uint32 >		m_loginServer;
	Archive::AutoVariable< std::string >	m_command;
	Archive::AutoVariable< std::string >	m_commandLine;
	
	DBCSRequestMessage();
	DBCSRequestMessage( const DBCSRequestMessage & );
	DBCSRequestMessage & operator = ( const DBCSRequestMessage& );
};

inline uint32 DBCSRequestMessage::getAccountId() const
{
	return m_accountId.get();
}

inline uint32 DBCSRequestMessage::getAccessLevel() const
{
	return m_accessLevel.get();
}

inline uint32 DBCSRequestMessage::getToolId() const
{
	return m_toolId.get();
}

inline uint32 DBCSRequestMessage::getLoginServerId() const
{
	return m_loginServer.get();
}

inline const std::string& DBCSRequestMessage::getCommand() const
{
	return m_command.get();
}

inline const std::string& DBCSRequestMessage::getCommandLine() const
{
	return m_commandLine.get();
}

#endif //_INCLUDED_CSDBNetMessages_H
//
