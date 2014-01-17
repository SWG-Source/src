#ifdef WIN32
#pragma warning (disable : 4786 4514)
#endif

#include <string>
#include <vector>
#include "VChatAPICore.h"
#include "messages.h"
#include "common.h"
#include "Base/hash.h"

#ifdef WIN32
	#include <winsock2.h>
#elif linux
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
#endif

using namespace std;

namespace VChatSystem
{
	
	//////////////////////////////////////////////////////////////////////////////////////////////
	// VChatAPI
	//

    VChatAPI::VChatAPI(const char * hostList, unsigned connectionType, const char * failoverHostList, unsigned connectionLimit, unsigned maxMsgSize, unsigned bufferSize)
		: mConnectionType(connectionType)
    {
	    mCore = new VChatAPICore(*this, hostList, failoverHostList, connectionLimit, maxMsgSize, bufferSize);
    }

    VChatAPI::~VChatAPI() 
    {
	    delete mCore;
    }

	void VChatAPI::Process()
	{
		mCore->Process();
	}

	unsigned VChatAPI::ConnectionCount() const
	{
		return mCore->GetConnectionCount();
	}
	
	unsigned VChatAPI::GetAccount(const std::string &avatarName, 
								  const std::string &game, 
								  const std::string &server, 
								  unsigned userID,
								  unsigned avatarID,
								  void *user,
								  unsigned timeout)
	{
		unsigned trackingNumber = mCore->GetTrackingNumber();
		Message::GetAccountRequest message;		
		message.SetTrackingNumber (trackingNumber);
		message.SetAvatarName(avatarName);
		message.SetGame(game);
		message.SetServer(server);
		message.SetUserID(userID);
		message.SetAvatarID(avatarID);
	
		//	send message
		unsigned char buffer[10240];
		memset(buffer, 0, 10240);
		unsigned length = message.Write(buffer, sizeof(buffer));
		mCore->Send(buffer, length);
		mCore->PrepareCallback(MESSAGE_GET_ACCOUNT_REPLY, trackingNumber, RESULT_TIMEOUT, user, timeout);
		return trackingNumber;
	}

	unsigned VChatAPI::GetChannel(const std::string &channelName, 
								  const std::string &game, 
								  const std::string &server, 
								  const std::string &description, 
								  const std::string &password,
								  unsigned limit,
								  bool persistent,
								  void *user,
								  unsigned timeout)
	{
		unsigned trackingNumber = mCore->GetTrackingNumber();
		Message::GetChannelRequest message;		
		message.SetTrackingNumber (trackingNumber);
		message.SetChannelName(channelName);
		message.SetGame(game);
		message.SetServer(server);
		message.SetDescription(description);
		message.SetPassword(password);
		message.SetLimit(limit);
		message.SetPersistent(persistent);

		//	send message
		unsigned char buffer[10240];
		memset(buffer, 0, 10240);
		unsigned length = message.Write(buffer, sizeof(buffer));
		mCore->Send(buffer, length);
		mCore->PrepareCallback(MESSAGE_GET_CHANNEL_REQUEST, trackingNumber, RESULT_TIMEOUT, user, timeout);
		return trackingNumber;
	}

	unsigned VChatAPI::GetProximityChannel( const std::string &channelName, 
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
											unsigned distModel,
											void *user,
											unsigned timeout)
	{
		unsigned trackingNumber = mCore->GetTrackingNumber();
		Message::GetProximityChannelRequest message;		
		message.SetTrackingNumber (trackingNumber);
		message.SetChannelName(channelName);
		message.SetGame(game);
		message.SetServer(server);
		message.SetDescription(description);
		message.SetPassword(password);
		message.SetLimit(limit);
		message.SetPersistent(persistent);
		message.SetMaxRange(maxRange);
		message.SetClamping(clamping);
		message.SetRollOff(rollOff);
		message.SetMaxGain(maxGain);
		message.SetDistModel(distModel);

		//	send message
		unsigned char buffer[10240];
		memset(buffer, 0, 10240);
		unsigned length = message.Write(buffer, sizeof(buffer));
		mCore->Send(buffer, length);
		mCore->PrepareCallback(MESSAGE_GET_PROXIMITY_CHANNEL_REPLY, trackingNumber, RESULT_TIMEOUT, user, timeout);
		return trackingNumber;
	}

	unsigned VChatAPI::ChannelCommand(  const std::string &srcUserName, 
										const std::string &destUserName, 
										const std::string &destChannelAddress, 
										unsigned command,
										unsigned banTimeout,
										void *user,
										unsigned timeout)
	{
		unsigned trackingNumber = mCore->GetTrackingNumber();
		Message::ChannelCommandRequest message;		
		message.SetTrackingNumber (trackingNumber);		
		message.SetSrcUserName(srcUserName);	
		message.SetDestUserName(destUserName);	
		message.SetDestChannelAddress(destChannelAddress);	
		message.SetCommand(command);
		message.SetBanTimeout(banTimeout);

		//	send message
		unsigned char buffer[10240];
		memset(buffer, 0, 10240);
		unsigned length = message.Write(buffer, sizeof(buffer));
		mCore->Send(buffer, length);
		mCore->PrepareCallback(MESSAGE_CHANNEL_COMMAND_REPLY, trackingNumber, RESULT_TIMEOUT, user, timeout);
		return trackingNumber;
	}

	unsigned VChatAPI::DeactivateVoiceAccount(const std::string &avatarName, 
												const std::string &gameName, 
												const std::string &serverName, 
												void *user,
												unsigned timeout)
	{
		unsigned trackingNumber = mCore->GetTrackingNumber();
		Message::DeactivateVoiceAccountRequest message;				
		message.SetTrackingNumber (trackingNumber);	
		message.SetAvatarName(avatarName);
		message.SetGame(gameName);
		message.SetServer(serverName);

		//	send message
		unsigned char buffer[10240];
		memset(buffer, 0, 10240);
		unsigned length = message.Write(buffer, sizeof(buffer));
		mCore->Send(buffer, length);
		mCore->PrepareCallback(MESSAGE_DEACTIVATE_ACCOUNT_REPLY, trackingNumber, RESULT_TIMEOUT, user, timeout);
		return trackingNumber;
	}
	
	unsigned VChatAPI::ChangePassword(const std::string &channelName, 
									  const std::string &game, 
									  const std::string &server, 
									  const std::string &password,
									  void *user,
									  unsigned timeout)
	{
		unsigned trackingNumber = mCore->GetTrackingNumber();
		Message::ChangePasswordRequest message;				
		message.SetTrackingNumber (trackingNumber);	
		message.SetChannelName(channelName);
		message.SetGame(game);
		message.SetServer(server);
		message.SetPassword(password);

		//	send message
		unsigned char buffer[10240];
		memset(buffer, 0, 10240);
		unsigned length = message.Write(buffer, sizeof(buffer));
		mCore->Send(buffer, length);
		mCore->PrepareCallback(MESSAGE_CHANGE_PASSWORD_REPLY, trackingNumber, RESULT_TIMEOUT, user, timeout);
		return trackingNumber;
	}

	unsigned VChatAPI::GetAllChannels(void *user, unsigned timeout)
	{
		unsigned trackingNumber = mCore->GetTrackingNumber();
		Message::GetAllChannelsRequest message;				
		message.SetTrackingNumber (trackingNumber);	

		//	send message
		unsigned char buffer[10240];
		memset(buffer, 0, 10240);
		unsigned length = message.Write(buffer, sizeof(buffer));
		mCore->Send(buffer, length);
		mCore->PrepareCallback(MESSAGE_GET_ALL_CHANNELS_REPLY, trackingNumber, RESULT_TIMEOUT, user, timeout);
		return trackingNumber;
	}


	unsigned VChatAPI::DeleteChannel(const std::string &channelName, 
									 const std::string &game, 
									 const std::string &server, 
									 void *user,
									 unsigned timeout)
	{
		unsigned trackingNumber = mCore->GetTrackingNumber();
		Message::DeleteChannelRequest message;				
		message.SetTrackingNumber (trackingNumber);	
		message.SetChannelName(channelName);
		message.SetGame(game);
		message.SetServer(server);

		//	send message
		unsigned char buffer[10240];
		memset(buffer, 0, 10240);
		unsigned length = message.Write(buffer, sizeof(buffer));
		mCore->Send(buffer, length);
		mCore->PrepareCallback(MESSAGE_DELETE_CHANNEL_REPLY, trackingNumber, RESULT_TIMEOUT, user, timeout);
		return trackingNumber;
	}

	unsigned VChatAPI::SetUserData( unsigned userID,
									unsigned attributes,
									const std::string &email, 
									const std::string &phoneNumber, 
									const std::string & PIN, 
									const std::string & smsAccount, 
									void *user,
									unsigned timeout)
	{
		unsigned trackingNumber = mCore->GetTrackingNumber();
		Message::SetUserDataRequest message;	
		message.SetTrackingNumber (trackingNumber);	
		message.SetUserID(userID);
		message.SetAttributes(attributes);
		message.SetEmail(email);
		message.SetPhoneNumber(phoneNumber);
		message.SetPin(PIN);
		message.SetSmsAccount(smsAccount);

		//	send message
		unsigned char buffer[10240];
		memset(buffer, 0, 10240);
		unsigned length = message.Write(buffer, sizeof(buffer));
		mCore->Send(buffer, length);
		mCore->PrepareCallback(MESSAGE_SET_USER_DATA_REPLY, trackingNumber, RESULT_TIMEOUT, user, timeout);
		return trackingNumber;
	}

	unsigned VChatAPI::SetBanStatus(unsigned userID,
									unsigned banStatus, 
									void *user,
									unsigned timeout)
	{
		unsigned trackingNumber = mCore->GetTrackingNumber();
		Message::SetBanStatusRequest message;				
		message.SetTrackingNumber (trackingNumber);
		message.SetUserID(userID);
		message.SetBanStatus(banStatus);

		//	send message
		unsigned char buffer[10240];
		memset(buffer, 0, 10240);
		unsigned length = message.Write(buffer, sizeof(buffer));
		mCore->Send(buffer, length);
		mCore->PrepareCallback(MESSAGE_SET_BAN_STATUS_REPLY, trackingNumber, RESULT_TIMEOUT, user, timeout);
		return trackingNumber;
	}


	unsigned VChatAPI::GetChannelInfo(const std::string &channelName, 
									  const std::string &game, 
									  const std::string &server,
									  void *user,
									  unsigned timeout)
	{
		unsigned trackingNumber = mCore->GetTrackingNumber();
		Message::GetChannelInfoRequest message;				
		message.SetTrackingNumber (trackingNumber);
		message.SetChannelName(channelName);
		message.SetGame(game);
		message.SetServer(server);

		//	send message
		unsigned char buffer[10240];
		memset(buffer, 0, 10240);
		unsigned length = message.Write(buffer, sizeof(buffer));
		mCore->Send(buffer, length);
		mCore->PrepareCallback(MESSAGE_GET_CHANNEL_INFO_REPLY, trackingNumber, RESULT_TIMEOUT, user, timeout);
		return trackingNumber;
	}



	unsigned VChatAPI::GetChannelV2(const std::string &channelName, 
		const std::string &game, 
		const std::string &server, 
		const std::string &description, 
		const std::string &password,
		unsigned limit,
		bool persistent,
		void *user,
		unsigned timeout)
	{
		unsigned trackingNumber = mCore->GetTrackingNumber();
		Message::GetChannelRequestV2 message;		
		message.SetTrackingNumber (trackingNumber);
		message.SetChannelName(channelName);
		message.SetGame(game);
		message.SetServer(server);
		message.SetDescription(description);
		message.SetPassword(password);
		message.SetLimit(limit);
		message.SetPersistent(persistent);

		//	send message
		unsigned char buffer[10240];
		memset(buffer, 0, 10240);
		unsigned length = message.Write(buffer, sizeof(buffer));
		mCore->Send(buffer, length);
		mCore->PrepareCallback(MESSAGE_GET_CHANNEL_REPLY_V2, trackingNumber, RESULT_TIMEOUT, user, timeout);
		return trackingNumber;
	}


	unsigned VChatAPI::AddCharacterChannel(const unsigned stationID,
		const unsigned avatarID,
		const std::string &characterName,
		const std::string &worldName,
		const std::string &gameCode,
		const std::string &channelType,
		const std::string &channelDescription,
		const std::string &password,
		const std::string &channelAddress,
		const std::string &locale,
		void *user,
		unsigned timeout)
	{
		unsigned trackingNumber = mCore->GetTrackingNumber();
		Message::AddCharacterChannelRequest message;		
		message.SetTrackingNumber (trackingNumber);

		message.SetStationID(stationID);
		message.SetAvatarID(avatarID);
		message.SetCharacterName(characterName);
		message.SetWorldName(worldName);
		message.SetGameCode(gameCode);
		message.SetChannelType(channelType);
		message.SetChannelDescription(channelDescription);
		message.SetPassword(password);
		message.SetChannelAddress(channelAddress);
		message.SetLocale(locale);

		unsigned char buffer[10240];
		memset(buffer, 0, 10240);
		unsigned length = message.Write(buffer, sizeof(buffer));
		mCore->Send(buffer, length);

		mCore->PrepareCallback(MESSAGE_ADD_CHARACTER_CHANNEL_REPLY, trackingNumber, RESULT_TIMEOUT, user, timeout);
		return trackingNumber;
	}

	unsigned VChatAPI::RemoveCharacterChannel(const unsigned stationID,
		const unsigned avatarID,
		const std::string &characterName,
		const std::string &worldName,
		const std::string &gameCode,
		const std::string &channelType,
		void *user,
		unsigned timeout)	
	{
		unsigned trackingNumber = mCore->GetTrackingNumber();
		Message::RemoveCharacterChannelRequest message;		
		message.SetTrackingNumber (trackingNumber);

		message.SetStationID(stationID);
		message.SetAvatarID(avatarID);
		message.SetCharacterName(characterName);
		message.SetWorldName(worldName);
		message.SetGameCode(gameCode);
		message.SetChannelType(channelType);

		unsigned char buffer[10240];
		memset(buffer, 0, 10240);
		unsigned length = message.Write(buffer, sizeof(buffer));
		mCore->Send(buffer, length);

		mCore->PrepareCallback(MESSAGE_REMOVE_CHARACTER_CHANNEL_REPLY, trackingNumber, RESULT_TIMEOUT, user, timeout);
		return trackingNumber;
	}



	unsigned VChatAPI::GetCharacterChannel(const unsigned stationID,
		const std::string &characterName,
		const std::string &worldName,
		const std::string &gameCode,
		void *user,
		unsigned timeout)	
	{
		unsigned trackingNumber = mCore->GetTrackingNumber();
		Message::GetCharacterChannelRequest message;		
		message.SetTrackingNumber (trackingNumber);

		message.SetStationID(stationID);
		message.SetCharacterName(characterName);
		message.SetWorldName(worldName);
		message.SetGameCode(gameCode);

		unsigned char buffer[10240];
		memset(buffer, 0, 10240);
		unsigned length = message.Write(buffer, sizeof(buffer));
		mCore->Send(buffer, length);

		mCore->PrepareCallback(MESSAGE_GET_CHARACTER_CHANNEL_REPLY, trackingNumber, RESULT_TIMEOUT, user, timeout);
		return trackingNumber;
	}


}	// end VChatAPI namespace


