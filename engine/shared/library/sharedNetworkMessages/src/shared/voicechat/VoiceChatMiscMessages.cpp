

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "VoiceChatMiscMessages.h"


const char* const VoiceChatAddClientToChannel::cms_name = "VoiceChatAddClientToChannel";

VoiceChatAddClientToChannel::VoiceChatAddClientToChannel(NetworkId const & clientId, std::string const & clientName, std::string const & roomName, bool forceShortlist)
: GameNetworkMessage(cms_name),
  m_clientId(clientId),
  m_clientName(clientName),
  m_channelName(roomName),
  m_forceShortlist(forceShortlist)
{
	addVariable(m_clientId);
	addVariable(m_clientName);
	addVariable(m_channelName);
	addVariable(m_forceShortlist);
}

VoiceChatAddClientToChannel::VoiceChatAddClientToChannel(Archive::ReadIterator &source)
: GameNetworkMessage(cms_name),
  m_clientId(),
  m_clientName(),
  m_channelName(),
  m_forceShortlist(false)
{
	addVariable(m_clientId);
	addVariable(m_clientName);
	addVariable(m_channelName);
	addVariable(m_forceShortlist);
	unpack(source);
}

VoiceChatAddClientToChannel::~VoiceChatAddClientToChannel()
{

}


//-----------------------------------------------------------------------

const char* const VoiceChatRemoveClientFromChannel::cms_name = "VoiceChatRemoveClientFromChannel";

VoiceChatRemoveClientFromChannel::VoiceChatRemoveClientFromChannel(NetworkId const & clientId, std::string const & clientName, std::string const & roomName)
: GameNetworkMessage(cms_name),
m_clientId(clientId),
m_clientName(clientName),
m_channelName(roomName)
{
	addVariable(m_clientId);
	addVariable(m_clientName);
	addVariable(m_channelName);
}

VoiceChatRemoveClientFromChannel::VoiceChatRemoveClientFromChannel(Archive::ReadIterator &source)
: GameNetworkMessage(cms_name),
m_clientId(),
m_clientName(),
m_channelName()
{
	addVariable(m_clientId);
	addVariable(m_clientName);
	addVariable(m_channelName);
	unpack(source);
}

VoiceChatRemoveClientFromChannel::~VoiceChatRemoveClientFromChannel()
{
}



//-----------------------------------------------------------------------

const char* const VoiceChatGetChannel::cms_name = "VoiceChatGetChannel";

VoiceChatGetChannel::VoiceChatGetChannel(std::string const & _roomName, std::string const & _password, bool _public, bool _persistant, uint32 _limit)
: GameNetworkMessage(cms_name),
  m_roomName(_roomName),
  m_password(_password),
  m_limit(_limit),
  m_isPublic(_public),
  m_persistant(_persistant)
{
	addVariable(m_roomName);
	addVariable(m_password);
	addVariable(m_limit);
	addVariable(m_isPublic);
	addVariable(m_persistant);
}

//-----------------------------------------------------------------------

VoiceChatGetChannel::VoiceChatGetChannel(Archive::ReadIterator & source)
: GameNetworkMessage(cms_name),
  m_roomName(),
  m_password(),
  m_limit(100),
  m_persistant(false)
{
	addVariable(m_roomName);
	addVariable(m_password);
	addVariable(m_limit);
	addVariable(m_isPublic);
	addVariable(m_persistant);
	unpack(source);
}

//-----------------------------------------------------------------------

VoiceChatGetChannel::~VoiceChatGetChannel()
{
}

//-----------------------------------------------------------------------

//-----------------------------------------------------------------------

const char* const VoiceChatOnGetChannel::cms_name = "VoiceChatOnGetChannel";

VoiceChatOnGetChannel::VoiceChatOnGetChannel(std::string const & _roomName, std::string const & _uri, std::string const & _password, bool _public, bool _persistant, bool _success)
: GameNetworkMessage(cms_name),
  m_roomName(_roomName),
  m_uri(_uri),
  m_password(_password),
  m_isPublic(_public),
  m_persistant(_persistant),
  m_success(_success)
{
	addVariable(m_roomName);
	addVariable(m_uri);
	addVariable(m_password);
	addVariable(m_isPublic);
	addVariable(m_persistant);
	addVariable(m_success);
}

//-----------------------------------------------------------------------

VoiceChatOnGetChannel::VoiceChatOnGetChannel(Archive::ReadIterator & source)
: GameNetworkMessage(cms_name),
  m_roomName(),
  m_uri(),
  m_password(),
  m_isPublic(false),
  m_persistant(false),
  m_success(false)
{
	addVariable(m_roomName);
	addVariable(m_uri);
	addVariable(m_password);
	addVariable(m_isPublic);
	addVariable(m_persistant);
	addVariable(m_success);
	unpack(source);
}

//-----------------------------------------------------------------------

VoiceChatOnGetChannel::~VoiceChatOnGetChannel()
{
}

//-----------------------------------------------------------------------
//-----------------------------------------------------------------------
//-----------------------------------------------------------------------

const char* const VoiceChatDeleteChannel::cms_name = "VoiceChatDeleteChannel";

VoiceChatDeleteChannel::VoiceChatDeleteChannel(std::string const & _roomName)
: GameNetworkMessage(cms_name),
m_roomName(_roomName)
{
	addVariable(m_roomName);
}

VoiceChatDeleteChannel::VoiceChatDeleteChannel(Archive::ReadIterator &source)
: GameNetworkMessage(cms_name),
m_roomName()
{
	addVariable(m_roomName);
	unpack(source);
}

VoiceChatDeleteChannel::~VoiceChatDeleteChannel()
{
}

//-----------------------------------------------------------------------
//-----------------------------------------------------------------------
//-----------------------------------------------------------------------
const char* const VoiceChatChannelCommand::cms_name = "VoiceChatChannelCommand";

VoiceChatChannelCommand::VoiceChatChannelCommand(std::string const & _srcUserName, std::string const & _destUserName,
	std::string const & _channelName, uint32 _command, uint32 _banTimeout)
: GameNetworkMessage(cms_name),
m_srcUserName(_srcUserName),
m_destUserName(_destUserName),
m_channelName(_channelName),
m_command(_command),
m_banTimeout(_banTimeout)
{
	addVariable(m_srcUserName);
	addVariable(m_destUserName);
	addVariable(m_channelName);
	addVariable(m_command);
	addVariable(m_banTimeout);
}

VoiceChatChannelCommand::VoiceChatChannelCommand(Archive::ReadIterator &source)
: GameNetworkMessage(cms_name),
  m_srcUserName(),
  m_destUserName(),
  m_channelName(),
  m_command(0),
  m_banTimeout(0)
{
	addVariable(m_srcUserName);
	addVariable(m_destUserName);
	addVariable(m_channelName);
	addVariable(m_command);
	addVariable(m_banTimeout);
	unpack(source);
}

VoiceChatChannelCommand::~VoiceChatChannelCommand()
{
}

//-----------------------------------------------------------------------
//-----------------------------------------------------------------------
//-----------------------------------------------------------------------
const char* const VoiceChatOnChannelCommand::cms_name = "VoiceChatOnChannelCommand";

VoiceChatOnChannelCommand::VoiceChatOnChannelCommand(std::string const & _srcUserName, std::string const & _destUserName,
		std::string const & _channelName, uint32 _command, uint32 _banTimeout, uint32 _result)
: GameNetworkMessage(cms_name),
 m_srcUserName(_srcUserName),
 m_destUserName(_destUserName),
 m_channelName(_channelName),
 m_command(_command),
 m_banTimeout(_banTimeout),
 m_result(_result)
{
	addVariable(m_srcUserName);
	addVariable(m_destUserName);
	addVariable(m_channelName);
	addVariable(m_command);
	addVariable(m_banTimeout);
	addVariable(m_result);
}

VoiceChatOnChannelCommand::VoiceChatOnChannelCommand(Archive::ReadIterator &source)
: GameNetworkMessage(cms_name),
m_srcUserName(),
m_destUserName(),
m_channelName(),
m_command(0),
m_banTimeout(0),
m_result(0)
{
	addVariable(m_srcUserName);
	addVariable(m_destUserName);
	addVariable(m_channelName);
	addVariable(m_command);
	addVariable(m_banTimeout);
	addVariable(m_result);
	unpack(source);
}

VoiceChatOnChannelCommand::~VoiceChatOnChannelCommand()
{
}

//-----------------------------------------------------------------------
//-----------------------------------------------------------------------
//-----------------------------------------------------------------------

const char* const VoiceChatRequestPersonalChannel::cms_name = "VoiceChatRequestPersonalChannel";

VoiceChatRequestPersonalChannel::VoiceChatRequestPersonalChannel (NetworkId const & owner, bool createIt)
: GameNetworkMessage(cms_name),
m_owner(owner),
m_pleaseCreateIt(createIt)
{
	addVariable(m_owner);
	addVariable(m_pleaseCreateIt);
}

VoiceChatRequestPersonalChannel::VoiceChatRequestPersonalChannel (Archive::ReadIterator &source)
: GameNetworkMessage(cms_name),
m_owner(),
m_pleaseCreateIt(false)
{
	addVariable(m_owner);
	addVariable(m_pleaseCreateIt);
	unpack(source);
}

VoiceChatRequestPersonalChannel::~VoiceChatRequestPersonalChannel()
{
}

//-----------------------------------------------------------------------
//-----------------------------------------------------------------------
//-----------------------------------------------------------------------

const char* const  VoiceChatInvite::cms_name = "VoiceChatInvite";

VoiceChatInvite::VoiceChatInvite (NetworkId const & requester, std::string channelName, NetworkId const & inviteeId, std::string const & inviteeName)
: GameNetworkMessage(cms_name),
m_requester(requester),
m_channelName(channelName),
m_inviteeId(inviteeId),
m_inviteeName(inviteeName),
m_channelUri()
{
	addVariable(m_requester);
	addVariable(m_channelName);
	addVariable(m_inviteeId);
	addVariable(m_inviteeName);
	addVariable(m_channelUri);
}

VoiceChatInvite::VoiceChatInvite (NetworkId const & sourceId, NetworkId const & targetId, std::string const & channelName, std::string const & uri)
: GameNetworkMessage(cms_name),
m_requester(sourceId),
m_channelName(channelName),
m_inviteeId(targetId),
m_inviteeName(),
m_channelUri(uri)
{
	addVariable(m_requester);
	addVariable(m_channelName);
	addVariable(m_inviteeId);
	addVariable(m_inviteeName);
	addVariable(m_channelUri);
}


VoiceChatInvite::VoiceChatInvite (Archive::ReadIterator &source)
: GameNetworkMessage(cms_name),
m_requester(),
m_channelName(),
m_inviteeId(),
m_inviteeName(),
m_channelUri()
{
	addVariable(m_requester);
	addVariable(m_channelName);
	addVariable(m_inviteeId);
	addVariable(m_inviteeName);
	addVariable(m_channelUri);
	unpack(source);
}

VoiceChatInvite::~VoiceChatInvite()
{
}


//-----------------------------------------------------------------------
//-----------------------------------------------------------------------
//-----------------------------------------------------------------------

const char* const  VoiceChatKick::cms_name = "VoiceChatKick";

VoiceChatKick::VoiceChatKick (NetworkId const & requester, std::string channelName, NetworkId const & kickeeId, std::string const & kickeeName)
: GameNetworkMessage(cms_name),
m_requester(requester),
m_channelName(channelName),
m_kickeeId(kickeeId),
m_kickeeName(kickeeName)
{
	addVariable(m_requester);
	addVariable(m_channelName);
	addVariable(m_kickeeId);
	addVariable(m_kickeeName);
}

VoiceChatKick::VoiceChatKick (NetworkId const & sourceId, NetworkId const & targetId, std::string channelName)
: GameNetworkMessage(cms_name),
m_requester(sourceId),
m_channelName(channelName),
m_kickeeId(targetId),
m_kickeeName()
{
	addVariable(m_requester);
	addVariable(m_channelName);
	addVariable(m_kickeeId);
	addVariable(m_kickeeName);
}

VoiceChatKick::VoiceChatKick (Archive::ReadIterator &source)
: GameNetworkMessage(cms_name),
m_requester(),
m_channelName(),
m_kickeeId(),
m_kickeeName()
{
	addVariable(m_requester);
	addVariable(m_channelName);
	addVariable(m_kickeeId);
	addVariable(m_kickeeName);
	unpack(source);
}

VoiceChatKick::~VoiceChatKick()
{
}

//-----------------------------------------------------------------------
//-----------------------------------------------------------------------
//-----------------------------------------------------------------------

const char* const  VoiceChatStatus::cms_name = "VoiceChatStatus";

VoiceChatStatus::VoiceChatStatus (const uint32 statusCode)
: GameNetworkMessage(cms_name),
  m_status(statusCode)
{
	addVariable(m_status);
}

VoiceChatStatus::VoiceChatStatus (Archive::ReadIterator &source)
: GameNetworkMessage(cms_name),
  m_status(SC_VoiceStatusUnknown)
{
	addVariable(m_status);
	unpack(source);
}
VoiceChatStatus::~VoiceChatStatus()
{

}

//-----------------------------------------------------------------------
//-----------------------------------------------------------------------
//-----------------------------------------------------------------------

const char* const  VoiceChatRequestChannelInfo::cms_name = "VoiceChatRequestChannelInfo";

VoiceChatRequestChannelInfo::VoiceChatRequestChannelInfo (NetworkId const & requester, std::string const & channelName)
: GameNetworkMessage(cms_name),
m_requester(requester),
m_channelName(channelName)
{
	addVariable(m_requester);
	addVariable(m_channelName);
}

VoiceChatRequestChannelInfo::VoiceChatRequestChannelInfo (Archive::ReadIterator &source)
: GameNetworkMessage(cms_name),
m_requester(),
m_channelName()
{
	addVariable(m_requester);
	addVariable(m_channelName);
	unpack(source);
}

VoiceChatRequestChannelInfo::~VoiceChatRequestChannelInfo()
{
}
