

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "VoiceChatOnGetAccount.h"

const char* const VoiceChatOnGetAccount::cms_name = "VoiceChatOnGetAccount";

VoiceChatOnGetAccount::VoiceChatOnGetAccount(unsigned resultCode, std::string const & loginName, std::string const & password, std::string const & connectionServerAddress)
: GameNetworkMessage(cms_name),
  m_result(resultCode),
  m_loginName(loginName),
  m_password(password),
  m_connectionServerAddress(connectionServerAddress)
{
	addVariable(m_result);
	addVariable(m_loginName);
	addVariable(m_password);
	addVariable(m_connectionServerAddress);
}

VoiceChatOnGetAccount::VoiceChatOnGetAccount(Archive::ReadIterator & source)
: GameNetworkMessage(cms_name),
	m_result(GAR_FAILURE),
	m_loginName(),
	m_password(),
	m_connectionServerAddress()
{
	addVariable(m_result);
	addVariable(m_loginName);
	addVariable(m_password);
	addVariable(m_connectionServerAddress);
	unpack(source);
}

VoiceChatOnGetAccount::~VoiceChatOnGetAccount()
{

}
