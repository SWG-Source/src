// CSToolREquest.h
// Copyright 2004, Sony Online Entertainment Inc., all rights reserved. 
// Author: Rob Hanz

#ifndef	_INCLUDED_CSToolRequest_H
#define	_INCLUDED_CSToolRequest_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------
/** A Central Server is telling the login server which cluster it is
	servicing.
*/
class CSToolRequest : public GameNetworkMessage
{
  public:
    CSToolRequest (uint32 accountId, uint32 accessLevel, const std::string & sCommand, const std::string & sCommandName, uint32 toolId, const std::string & sUserName );
    CSToolRequest (Archive::ReadIterator & source);
    virtual ~CSToolRequest ();
    
    uint32 			getAccountId () const;
    uint32			getAccessLevel() const;
    const std::string & 	getCommandName() const;
    const std::string & 	getCommandString() const;
    uint32			getToolId() const;
    const std::string &		getUserName() const;

  private:
    Archive::AutoVariable< uint32 >      m_accountId;
    Archive::AutoVariable<std::string>   m_command;
    Archive::AutoVariable<std::string>	 m_commandName;
    Archive::AutoVariable< uint32 >	 m_accessLevel;
    Archive::AutoVariable< uint32 > 	 m_toolId;
    Archive::AutoVariable< std::string > m_userName;
    

    CSToolRequest();
    CSToolRequest(const CSToolRequest&);
    CSToolRequest& operator= (const CSToolRequest&);

};

inline uint32 CSToolRequest::getAccountId() const
{
	return m_accountId.get();
}

inline uint32 CSToolRequest::getAccessLevel() const
{
	return m_accessLevel.get();
}

inline const std::string & CSToolRequest::getCommandString() const
{
	return m_command.get();
}

inline const std::string & CSToolRequest::getCommandName() const
{
	return m_commandName.get();
}

inline uint32 CSToolRequest::getToolId() const
{
	return m_toolId.get();
}

inline const std::string & CSToolRequest::getUserName() const
{
	return m_userName.get();
}


#endif //_INCLUDED_CSToolRequest_H
