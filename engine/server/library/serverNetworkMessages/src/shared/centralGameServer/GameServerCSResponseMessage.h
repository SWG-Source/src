// CSToolREquest.h
// Copyright 2004, Sony Online Entertainment Inc., all rights reserved.
// Author: Rob Hanz

#ifndef	_INCLUDED_GameServerCSResponse_H
#define	_INCLUDED_GameServerCSResponse_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "serverNetworkMessages/GameServerCSRequestMessage.h"
//-----------------------------------------------------------------------
/** A Central Server is telling the login server which cluster it is
	servicing.
*/
class GameServerCSResponseMessage: public GameNetworkMessage
{
  public:
    GameServerCSResponseMessage( const GameServerCSRequestMessage & source );
    GameServerCSResponseMessage(Archive::ReadIterator & source);
    virtual ~GameServerCSResponseMessage ();

    uint32 			getAccountId () const;
    const std::string & 	getResponse() const;
    uint32			getToolId() const;
    uint32			getLoginServerID() const;

    void			setResponse( const std::string & response );

  private:
    Archive::AutoVariable< uint32 >      m_accountId;
    Archive::AutoVariable<std::string>   m_response;
    Archive::AutoVariable< uint32 > 	 m_toolId;
    Archive::AutoVariable< uint32 >	 m_loginServerID;


    GameServerCSResponseMessage();
    GameServerCSResponseMessage(const GameServerCSResponseMessage&);
    GameServerCSResponseMessage& operator= (const GameServerCSResponseMessage&);

};

inline const std::string & GameServerCSResponseMessage::getResponse() const
{
	return m_response.get();
}

inline uint32 GameServerCSResponseMessage::getAccountId() const
{
	return m_accountId.get();
}

inline uint32 GameServerCSResponseMessage::getToolId() const
{
	return m_toolId.get();
}

inline uint32 GameServerCSResponseMessage::getLoginServerID() const
{
	return m_loginServerID.get();
}

#endif //_INCLUDED_GameServerCSResponse_H
