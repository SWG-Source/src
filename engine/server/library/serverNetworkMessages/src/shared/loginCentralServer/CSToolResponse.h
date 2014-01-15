// CSToolResponse.h
// Copyright 2004, Sony Online Entertainment Inc., all rights reserved. 
// Author: Rob Hanz

#ifndef	_INCLUDED_CSToolResponse_H
#define	_INCLUDED_CSToolResponse_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"
class CSToolRequest;
//-----------------------------------------------------------------------
/** A Central server is responding to a CS Tool request from a LS
*/
class CSToolResponse : public GameNetworkMessage
{
  public:
    CSToolResponse ( const CSToolRequest & pRequest, const std::string &sResult );
    CSToolResponse ( uint32 accountId, const std::string & response, uint32 toolId );
    CSToolResponse (Archive::ReadIterator & source);
    virtual ~CSToolResponse ();
    
    uint32 			getAccountId () const;
    const std::string & 	getResult() const;
    uint32			getToolId() const;

  private:
    Archive::AutoVariable< uint32 >      m_accountId;
    Archive::AutoVariable<std::string>   m_result;
    Archive::AutoVariable< uint32 > 	 m_toolId;
    

    CSToolResponse();
    CSToolResponse(const CSToolResponse&);
    CSToolResponse& operator= (const CSToolResponse&);

};

inline const std::string & CSToolResponse::getResult() const
{
	return m_result.get();
}

inline uint32 CSToolResponse::getAccountId() const
{
	return m_accountId.get();
}

inline uint32 CSToolResponse::getToolId() const
{
	return m_toolId.get();
}

#endif //_INCLUDED_CSToolResponse_H
