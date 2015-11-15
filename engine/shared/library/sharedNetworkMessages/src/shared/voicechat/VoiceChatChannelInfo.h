// VoiceChatChannelInfo.h
// temporary message to tell a client about a channel they are allowed to join
// Steven Wyckoff

#ifndef _INCLUDED_VoiceChatChannelInfo_H
#define _INCLUDED_VoiceChatChannelInfo_H

#include "sharedFoundation/NetworkId.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

#include <string>

class VoiceChatChannelInfo : public GameNetworkMessage
{
public:
	
	static const char* const cms_name;

	explicit VoiceChatChannelInfo(std::string const & name, std::string const & displayName, std::string const & uri, 
		std::string const & password, std::string const & announceText = "", uint32 flags = CIF_None);
	explicit VoiceChatChannelInfo(Archive::ReadIterator &);
	~VoiceChatChannelInfo();

	bool getIsLeaveChannelCommand() const;
	bool getIsAutoJoin() const;
	bool getChannelExists() const;
	bool getTargetIsModerator() const;
	bool getForcedShortlist() const;
	std::string const & getName() const;
	std::string const & getDisplayName() const;
	std::string const & getUri() const;
	std::string const & getPassword() const;
	std::string const & getAnnounceText() const;

	enum ChannelInfoFlags
	{
		CIF_None = 0x00,
		CIF_LeaveChannel = 0x01,
		CIF_AutoJoin = 0x02,
		CIF_ChannelDoesNotExist = 0x04,
		CIF_TargetModerator = 0x08,
		CIF_ForcedShortlist = 0x10,
	};

private:
	VoiceChatChannelInfo & operator = (const VoiceChatChannelInfo & rhs);
	VoiceChatChannelInfo(const VoiceChatChannelInfo & source);

	Archive::AutoVariable<uint32> m_flags;
	Archive::AutoVariable<std::string> m_channelName;
	Archive::AutoVariable<std::string> m_displayName;
	Archive::AutoVariable<std::string> m_channelUri;
	Archive::AutoVariable<std::string> m_channelPassword;
	Archive::AutoVariable<std::string> m_announceText;
};

//-----------------------------------------------------------------------

inline bool VoiceChatChannelInfo::getIsLeaveChannelCommand() const
{
	return (m_flags.get() & CIF_LeaveChannel) != CIF_None;
}

//-----------------------------------------------------------------------

inline bool VoiceChatChannelInfo::getIsAutoJoin() const
{
	return (m_flags.get() & CIF_AutoJoin) != CIF_None;
}

//-----------------------------------------------------------------------

inline bool VoiceChatChannelInfo::getChannelExists() const
{
	return (m_flags.get() & CIF_ChannelDoesNotExist) == CIF_None;
}

//-----------------------------------------------------------------------

inline bool VoiceChatChannelInfo::getTargetIsModerator() const
{
	return (m_flags.get() & CIF_TargetModerator) != CIF_None;
}

//-----------------------------------------------------------------------

inline bool VoiceChatChannelInfo::getForcedShortlist() const
{
	return (m_flags.get() & CIF_ForcedShortlist) != CIF_None;
}

//-----------------------------------------------------------------------

inline const std::string & VoiceChatChannelInfo::getName() const
{
	return m_channelName.get();
}

//-----------------------------------------------------------------------

inline const std::string & VoiceChatChannelInfo::getDisplayName() const
{
	return m_displayName.get();
}

//-----------------------------------------------------------------------

inline const std::string & VoiceChatChannelInfo::getUri() const
{
	return m_channelUri.get();
}

//-----------------------------------------------------------------------

inline const std::string & VoiceChatChannelInfo::getPassword() const
{
	return m_channelPassword.get();
}

//-----------------------------------------------------------------------

inline const std::string & VoiceChatChannelInfo::getAnnounceText() const
{
	return m_announceText.get();
}

//-----------------------------------------------------------------------

#endif // _INCLUDED_VoiceChatChannelInfo_H
