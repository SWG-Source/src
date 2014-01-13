// ======================================================================
//
// LoginClusterStatusEx.h
// copyright (c) 2005 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_LoginClusterStatusEx_H
#define INCLUDED_LoginClusterStatusEx_H

// ======================================================================

#include "sharedNetworkMessages/GameNetworkMessage.h"

/**
 * Sent From:  LoginServer
 * Sent To:    Client
 * Action:     Provides extended cluster data (branch; changelist info)
*/
class LoginClusterStatusEx : public GameNetworkMessage
{
public:

	static const char * const MessageType;

	struct ClusterData
	{
		uint32      m_clusterId;
		std::string m_branch;
		std::string m_networkVersion;
		uint32      m_version;
		uint32      m_reserved1;
		uint32      m_reserved2;
		uint32      m_reserved3;
		uint32      m_reserved4;
	};
	
public:
	const std::vector<ClusterData> &getData() const;
	
public:
	LoginClusterStatusEx(const std::vector<ClusterData> &data);
	LoginClusterStatusEx(Archive::ReadIterator & source);
	~LoginClusterStatusEx();
	
private:
	Archive::AutoArray<ClusterData> m_data;
	
private: //disable
	LoginClusterStatusEx();
	LoginClusterStatusEx(const LoginClusterStatusEx&);
	LoginClusterStatusEx& operator= (const LoginClusterStatusEx&);
};

// ======================================================================

namespace Archive
{
	inline void get(ReadIterator & source, LoginClusterStatusEx::ClusterData &c)
	{
		get( source, c.m_clusterId );
		get( source, c.m_branch );
		get( source, c.m_networkVersion );
		get( source, c.m_version );
		get( source, c.m_reserved1 );
		get( source, c.m_reserved2 );
		get( source, c.m_reserved3 );
		get( source, c.m_reserved4 );
	}

	inline void put(ByteStream & target, const LoginClusterStatusEx::ClusterData &c)
	{
		put( target, c.m_clusterId );
		put( target, c.m_branch );
		put( target, c.m_networkVersion );
		put( target, c.m_version );
		put( target, c.m_reserved1 );
		put( target, c.m_reserved2 );
		put( target, c.m_reserved3 );
		put( target, c.m_reserved4 );
	}
}

// ======================================================================

inline const std::vector<LoginClusterStatusEx::ClusterData> &LoginClusterStatusEx::getData() const
{
	return m_data.get();
}

// ======================================================================

#endif
