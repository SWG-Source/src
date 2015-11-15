// LoginClusterName.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_LoginClusterName_H
#define	_INCLUDED_LoginClusterName_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------
/** A Central Server is telling the login server which cluster it is
	servicing.
*/
class LoginClusterName : public GameNetworkMessage
{
  public:
    LoginClusterName (const std::string & newClusterName, int timeZone);
    LoginClusterName (Archive::ReadIterator & source);
    virtual ~LoginClusterName ();
    const std::string & getClusterName () const;
	int                 getTimeZone    () const;

  private:
    Archive::AutoVariable<std::string>   m_clusterName;
	Archive::AutoVariable<int>           m_timeZone;

    LoginClusterName();
    LoginClusterName(const LoginClusterName&);
    LoginClusterName& operator= (const LoginClusterName&);

};

//-----------------------------------------------------------------------

inline const std::string & LoginClusterName::getClusterName() const
{
	return m_clusterName.get();
}

//-----------------------------------------------------------------------

inline int LoginClusterName::getTimeZone() const
{
	return m_timeZone.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_LoginClusterName_H
