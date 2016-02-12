#include "VChatClient.h"
#include "Base/timer.h"
#include "VChatAPI/strings.h"

VChatClient::VChatClient(const char *hostlist) 
: VChatAPI(hostlist)
, m_isDone(false)
, m_failed(false)
, m_isConnected(false)
, m_result(RESULT_FAILURE)
{
}

VChatClient::~VChatClient(void)
{
}

unsigned VChatClient::GetAccountEx(const std::string &avatarName, 
								const std::string &game, 
								const std::string &world, 
								unsigned userID,
								unsigned avatarID)
{

	Reset();
	VChatAPI::GetAccount(avatarName, game, world, userID, avatarID, nullptr);

	while(!IsDone() && !HasFailed())
	{
		Process();
		soe::Sleep(10);
	}
	return m_result;
}

void VChatClient::OnGetAccount(unsigned track, 
					 unsigned result, 
					 unsigned userID,
					 unsigned accountID, 
					 const std::string &voicePassword,
					 const std::string &encodedVoiceAccount,
					 const std::string &URI,
					 void *user)
{
	printf ("OnLogin track = %d, result = %s\n", track, ResultString[result].c_str());

	printf ("\t UserID(%d)\n\t AccountID(%d)\n\t",
		userID, 
		accountID);

	printf ("\t Password(%s)\n",
		voicePassword.c_str());
	printf ("\t EncodedName(%s)\n",
		encodedVoiceAccount.c_str());
	printf ("\t URI(%s)\n",
		URI.c_str());

	m_avatarID = accountID;

	m_track = track;
	//m_previousUser = encodedVoiceAccount;
	m_result = result;
	m_isDone = true;
}

unsigned VChatClient::DeactivateVoiceAccount(const std::string &avatarName, 
								 const std::string &game, 
								 const std::string &world)
{

	Reset();
	VChatAPI::DeactivateVoiceAccount(avatarName, game, world, nullptr);

	while(!IsDone() && !HasFailed())
	{
		Process();
		soe::Sleep(10);
	}
	return m_result;
}

void VChatClient::OnDeactivateVoiceAccount( unsigned track, 
							   unsigned result, 
							   void *user) 
{
	printf ("OnDeactivateVoiceAccount track = %d, result = %s\n", track, ResultString[result].c_str());

	m_track = track;
	m_result = result;
	m_isDone = true;
}

unsigned VChatClient::GetChannelEx( const std::string &channelName, 
									const std::string &game, 
									const std::string &server, 
									const std::string &description, 
									const std::string &password,
									unsigned limit,
									bool persistent)
{
	Reset();
	VChatAPI::GetChannel(channelName, game, server, description, password, limit, persistent, nullptr);

	while(!IsDone() && !HasFailed())
	{
		Process();
		soe::Sleep(10);
	}

	return m_result;
}

void VChatClient::OnGetChannel( unsigned track, 
							    unsigned result, 
							    const std::string &channelName, 
							    const std::string &channelURI, 
								unsigned channelID,
								void *user) 
{
	printf ("OnGetChannel track = %d, result = %s\n", track, ResultString[result].c_str());

	printf ("\t ChannelName(%s)\n\t ChannelURI(%s)\n\t ChannelID(%d)\n",
		channelName.c_str(), 
		channelURI.c_str(), 
		channelID);

	m_channelID = channelID;
	m_channelURI = channelURI;

	m_track = track;
	m_result = result;
	m_isDone = true;
}

unsigned VChatClient::GetProximityChannelEx(const std::string &channelName, 
											const std::string &game, 
											const std::string &server, 
											const std::string &description, 
											const std::string &password,
											unsigned limit,
											bool persistent,
											unsigned maxRange,
											unsigned clamping,
											float rollOff,
											float maxGain,
											unsigned distModel)
{
	Reset();
	VChatAPI::GetProximityChannel(channelName, game, server, description, password, limit, persistent, maxRange, clamping, rollOff, maxGain, distModel, nullptr);

	while(!IsDone() && !HasFailed())
	{
		Process();
		soe::Sleep(10);
	}

	return m_result;
}

void VChatClient::OnGetProximityChannel(unsigned track, 
										unsigned result, 
										const std::string &channelName, 
										const std::string &channelURI, 
										unsigned channelID,
										void *user) 
{
	printf ("OnGetProximityChannel track = %d, result = %s\n", track, ResultString[result].c_str());

	printf ("\t ChannelName(%s)\n\t ChannelURI(%s)\n\t ChannelID(%d)\n",
		channelName.c_str(), 
		channelURI.c_str(), 
		channelID);

	m_track = track;
	m_result = result;
	m_isDone = true;
}

unsigned VChatClient::ChannelCommandEx( const std::string &srcUserName, 
										const std::string &destUserName, 
										const std::string &destChannelAddress, 
										unsigned command,
										unsigned banTimeout)
{
	Reset();
	VChatAPI::ChannelCommand(srcUserName, destUserName, destChannelAddress, command, banTimeout, nullptr);

	while(!IsDone() && !HasFailed())
	{
		Process();
		soe::Sleep(10);
	}

	return m_result;
}

void VChatClient::OnChannelCommand( unsigned track, 
									unsigned result, 
									void *user) 
{
	printf ("OnChannelCommand track = %d, result = %s\n", track, ResultString[result].c_str());

	m_track = track;
	m_result = result;
	m_isDone = true;
}

unsigned VChatClient::ChangePasswordEx(const std::string &channelName, 
									   const std::string &game, 
									   const std::string &server, 
									   const std::string &password)
{
	Reset();
	VChatAPI::ChangePassword(channelName, game, server, password, nullptr);

	while(!IsDone() && !HasFailed())
	{
		Process();
		soe::Sleep(10);
	}

	return m_result;
}

void VChatClient::OnChangePassword( unsigned track, 
									unsigned result, 
									void *user)
{
	printf ("OnChangePassword track = %d, result = %s\n", track, ResultString[result].c_str());

	m_track = track;
	m_result = result;
	m_isDone = true;
}

unsigned VChatClient::GetAllChannelsEx()
{
	Reset();
	VChatAPI::GetAllChannels(nullptr);

	while(!IsDone() && !HasFailed())
	{
		Process();
		soe::Sleep(10);
	}

	return m_result;
}

void VChatClient::OnGetAllChannels( unsigned track, 
									unsigned result, 
									const ChannelVec_t & channels,
									void *user)
{
	printf ("OnGetAllChannels track = %d, result = %s\n", track, ResultString[result].c_str());

	for(unsigned i = 0; i < channels.size(); ++i)
	{
		printf ("\t%d) ChannelName(%s)\n\t ChannelType(%d)\n\t ChannelURI(%s)\n\t ChannelID(%d)\n",
			i+1,
			channels[i].m_channelName.c_str(),
			channels[i].m_channelType,
			channels[i].m_channelURI.c_str(), 
			channels[i].m_channelID);
	}

	m_track = track;
	m_result = result;
	m_isDone = true;
}

unsigned VChatClient::DeleteChannelEx(const std::string &channelName, 
									  const std::string &game, 
									  const std::string &server)
{
	Reset();
	VChatAPI::DeleteChannel(channelName, game, server, nullptr);

	while(!IsDone() && !HasFailed())
	{
		Process();
		soe::Sleep(10);
	}

	return m_result;
}

void VChatClient::OnDeleteChannel(unsigned track, 
								  unsigned result, 
								  void *user)
{
	printf ("OnDeleteChannel track = %d, result = %s\n", track, ResultString[result].c_str());

	m_track = track;
	m_result = result;
	m_isDone = true;
}


unsigned VChatClient::SetBanStatusEx(unsigned userID,
									 unsigned banStatus)
{
	Reset();
	VChatAPI::SetBanStatus(userID, banStatus, nullptr);

	while(!IsDone() && !HasFailed())
	{
		Process();
		soe::Sleep(10);
	}

	return m_result;
}

void VChatClient::OnSetBanStatus(unsigned track, 
								unsigned result, 
								void *user)
{
	printf ("OnSetBanStatus track = %d, result = %s\n", track, ResultString[result].c_str());

	m_track = track;
	m_result = result;
	m_isDone = true;
}

unsigned VChatClient::GetChannelInfoEx( const std::string &channelName, 
										const std::string &game, 
										const std::string &server)
{
	Reset();
	VChatAPI::GetChannelInfo(channelName, game, server, nullptr);

	while(!IsDone() && !HasFailed())
	{
		Process();
		soe::Sleep(10);
	}

	return m_result;
}


void VChatClient::OnGetChannelInfo( unsigned track, 
									unsigned result, 
									const std::string &channelName, 
									const std::string &channelURI, 
									unsigned channelID,
									void *user)
{
	printf ("OnGetChannelInfo track = %d, result = %s\n", track, ResultString[result].c_str());

	printf ("\t ChannelName(%s)\n\t ChannelURI(%s)\n\t ChannelID(%d)\n",
		channelName.c_str(), 
		channelURI.c_str(), 
		channelID);

	m_track = track;
	m_result = result;
	m_isDone = true;
}


unsigned VChatClient::GetChannelV2Ex(const std::string &channelName, 
								     const std::string &game, 
								     const std::string &server, 
								     const std::string &description, 
								     const std::string &password,
								     unsigned limit,
								     bool persistent)
{
	Reset();
	VChatAPI::GetChannelV2(channelName, game, server, description, password, limit, persistent, nullptr);

	while(!IsDone() && !HasFailed())
	{
		Process();
		soe::Sleep(10);
	}

	return m_result;
}

void VChatClient::OnGetChannelV2( unsigned track, 
							   unsigned result, 
							   const std::string &channelName, 
							   const std::string &channelURI, 
							   unsigned channelID,
							   unsigned isNewChannel,
							   void *user) 
{
	printf ("OnGetChannelV2 track = %d, result = %s\n", track, ResultString[result].c_str());

	printf ("\t ChannelName(%s)\n\t ChannelURI(%s)\n\t ChannelID(%d)\n\t NewChannel(%s)\n",
		channelName.c_str(), 
		channelURI.c_str(), 
		channelID,
		isNewChannel?"true":"false");

	m_channelID = channelID;
	m_channelURI = channelURI;

	m_result = result;
	m_isDone = true;
}



unsigned VChatClient::AddCharacterChannelEx(const unsigned stationID,
										const unsigned avatarID,
										const std::string &characterName,
										const std::string &worldName,
										const std::string &gameCode,
										const std::string &channelType,
										const std::string &channelDescription,
										const std::string &password,
										const std::string &channelAddress,
										const std::string &locale)
{
	Reset();
	VChatAPI::AddCharacterChannel(stationID, avatarID, characterName, worldName, 
								  gameCode, channelType, channelDescription, 
								  password, channelAddress, locale, nullptr);

	while(!IsDone() && !HasFailed())
	{
		Process();
		soe::Sleep(10);
	}

	return m_result;
}

void VChatClient::OnAddCharacterChannel(unsigned track, 
								   unsigned result, 
								   void *user) 
{
	printf ("OnAddCharacterChannel track = %d, result = %s\n", track, ResultString[result].c_str());

	m_result = result;
	m_isDone = true;
}

unsigned VChatClient::RemoveCharacterChannelEx( const unsigned stationID,
												const unsigned avatarID,
												const std::string &characterName,
												const std::string &worldName,
												const std::string &gameCode,
												const std::string &channelType)
{
	Reset();
	VChatAPI::RemoveCharacterChannel(stationID, avatarID, characterName, worldName, 
									gameCode, channelType, nullptr);

	while(!IsDone() && !HasFailed())
	{
		Process();
		soe::Sleep(10);
	}

	return m_result;
}

void VChatClient::OnRemoveCharacterChannel(unsigned track, 
									  unsigned result, 
									  void *user) 
{
	printf ("OnRemoveCharacterChannel track = %d, result = %s\n", track, ResultString[result].c_str());

	m_result = result;
	m_isDone = true;
}


unsigned VChatClient::GetCharacterChannelEx(const unsigned stationID,
										const std::string &characterName,
										const std::string &worldName,
										const std::string &gameCode)
{
	Reset();
	VChatAPI::GetCharacterChannel(stationID, characterName, worldName, gameCode, nullptr);

	while(!IsDone() && !HasFailed())
	{
		Process();
		soe::Sleep(10);
	}

	return m_result;
}

void VChatClient::OnGetCharacterChannel(unsigned track,
								   unsigned result,
								   const CharacterChannelVec_t &characterChannels,
								   void * user)
{
	printf ("OnGetCharacterChannel track = %d, result = %s numChannels=%d\n", track, ResultString[result].c_str(), characterChannels.size());

	for (unsigned i = 0; i < characterChannels.size(); ++i)
	{
		PrintChannel(characterChannels[i]);
	}

	m_result = result;
	m_isDone = true;
}


void VChatClient::PrintChannel(const CharacterChannel &characterChannel)
{
	printf("Character Channel \n\t  m_stationID(%d) \n\t m_avatarID(%d) \n\t m_characterName(%s) \n\t m_worldName(%s) \n", characterChannel.m_stationID, characterChannel.m_avatarID, characterChannel.m_characterName.c_str(), characterChannel.m_worldName.c_str());
	printf("\t m_gameCode(%s) \n\t m_channelType(%s) \n\t m_channelDescription(%s) \n", characterChannel.m_gameCode.c_str(), characterChannel.m_channelType.c_str(), characterChannel.m_channelDescription.c_str());
	printf("\t m_channelAddress(%s) \n\t m_locale(%s) \n\t m_password(%s)\n", characterChannel.m_channelAddress.c_str(), characterChannel.m_locale.c_str(), characterChannel.m_password.c_str());
}

