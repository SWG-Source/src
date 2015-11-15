// VoiceChatOnGetAccount.h
// sent to the client when the login info is ready for them
// Steven Wyckoff

#ifndef _INCLUDED_VoiceChatOnGetAccount_H
#define _INCLUDED_VoiceChatOnGetAccount_H

#include "sharedFoundation/NetworkId.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

#include <string>

class VoiceChatOnGetAccount : public GameNetworkMessage
{
public:
	
	enum GetAccountResult
	{
		GAR_SUCCESS = 0,
		GAR_FAILURE = 1,
	};

	static const char* const cms_name;

	explicit VoiceChatOnGetAccount(unsigned resultCode, std::string const & loginName, std::string const & password, std::string const & connectionServer);
	explicit VoiceChatOnGetAccount(Archive::ReadIterator &);
	~VoiceChatOnGetAccount();

	unsigned getResult() const;
	std::string const & getName() const;
	std::string const & getPassword() const;
	std::string const & getConnectionServerAddress() const;

private:
	VoiceChatOnGetAccount & operator = (const VoiceChatOnGetAccount & rhs);
	VoiceChatOnGetAccount(const VoiceChatOnGetAccount & source);

	Archive::AutoVariable<unsigned> m_result;
	Archive::AutoVariable<std::string> m_loginName;
	Archive::AutoVariable<std::string> m_password;
	Archive::AutoVariable<std::string> m_connectionServerAddress;
};

//-----------------------------------------------------------------------

inline unsigned VoiceChatOnGetAccount::getResult() const
{
	return m_result.get();
}

//-----------------------------------------------------------------------

inline const std::string & VoiceChatOnGetAccount::getName() const
{
	return m_loginName.get();
}

//-----------------------------------------------------------------------

inline const std::string & VoiceChatOnGetAccount::getPassword() const
{
	return m_password.get();
}

//-----------------------------------------------------------------------

inline const std::string & VoiceChatOnGetAccount::getConnectionServerAddress() const
{
	return m_connectionServerAddress.get();
}

//-----------------------------------------------------------------------

#endif // _INCLUDED_VoiceChatOnGetAccount_H
