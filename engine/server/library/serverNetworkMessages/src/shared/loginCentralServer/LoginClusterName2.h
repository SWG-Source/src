// LoginClusterName2.h
// Copyright 2005, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall, JMatzen

#ifndef	_INCLUDED_LoginClusterName2_H
#define	_INCLUDED_LoginClusterName2_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------
/** A Central Server is telling the login server which cluster it is
	servicing.
*/
class LoginClusterName2 : public GameNetworkMessage
{
  public:
    LoginClusterName2 (const std::string & newClusterName, int timeZone);
    LoginClusterName2 (Archive::ReadIterator & source);
    virtual ~LoginClusterName2 ();
    const std::string & getClusterName () const;
	int                 getTimeZone    () const;
	int                 getChangelist()   const;
	const std::string & getBranch() const;
	const std::string & getNetworkVersion() const;

  private:
	Archive::AutoVariable<std::string>   m_clusterName;
	Archive::AutoVariable<int>           m_timeZone;
	Archive::AutoVariable<std::string>   m_branch;
	Archive::AutoVariable<int>           m_changelist;
	Archive::AutoVariable<std::string>   m_networkVersion;
	

    LoginClusterName2();
    LoginClusterName2(const LoginClusterName2&);
    LoginClusterName2& operator= (const LoginClusterName2&);

};

//-----------------------------------------------------------------------

inline const std::string & LoginClusterName2::getClusterName() const
{
	return m_clusterName.get();
}

//-----------------------------------------------------------------------

inline int LoginClusterName2::getTimeZone() const
{
	return m_timeZone.get();
}

//-----------------------------------------------------------------------

inline int LoginClusterName2::getChangelist() const
{
	return m_changelist.get();
}

//-----------------------------------------------------------------------

inline const std::string &LoginClusterName2::getBranch() const
{
	return m_branch.get();
}

//-----------------------------------------------------------------------

inline const std::string &LoginClusterName2::getNetworkVersion() const
{
	return m_networkVersion.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_LoginClusterName2_H
