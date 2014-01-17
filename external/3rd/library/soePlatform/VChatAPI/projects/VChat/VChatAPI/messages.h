#ifndef _VCHAT_SERVER_API_MESSAGES_H_
#define _VCHAT_SERVER_API_MESSAGES_H_

#include <string>
#include <vector>

#include "clientSerialize.h"

#ifndef _VCHAT_API_API_HEADERS_
#define _VCHAT_API_API_HEADERS_
	#ifndef API_NAMESPACE
	#define API_NAMESPACE VChatSystem
	#include "Api/api.h"
	#undef API_NAMESPACE
	#else
	#include "Api/api.h"
	#endif
#endif

namespace VChatSystem
{

    enum
    {
		//	internal message
		MESSAGE_RETRY_NOTIFICATIONS		= 1,		
        //  Requests
		MESSAGE_GET_ACCOUNT_REQUEST = 0x1100,
		MESSAGE_GET_CHANNEL_REQUEST,
		MESSAGE_GET_PROXIMITY_CHANNEL_REQUEST,
		MESSAGE_CHANNEL_COMMAND_REQUEST,
		MESSAGE_DEACTIVATE_ACCOUNT_REQUEST,
		MESSAGE_CHANGE_PASSWORD_REQUEST,
		MESSAGE_GET_ALL_CHANNELS_REQUEST,
		MESSAGE_DELETE_CHANNEL_REQUEST,
		MESSAGE_SET_USER_DATA_REQUEST,
		MESSAGE_SET_BAN_STATUS_REQUEST,
		MESSAGE_GET_CHANNEL_INFO_REQUEST,
		MESSAGE_GET_CHANNEL_REQUEST_V2,
		MESSAGE_ADD_CHARACTER_CHANNEL,
		MESSAGE_REMOVE_CHARACTER_CHANNEL,
		MESSAGE_GET_CHARACTER_CHANNEL,
		MESSAGE_UPDATE_CHARACTER_CHANNEL,
		        
        //  Replies
		MESSAGE_GET_ACCOUNT_REPLY	    = 0x8100,
		MESSAGE_GET_CHANNEL_REPLY,
		MESSAGE_GET_PROXIMITY_CHANNEL_REPLY,
		MESSAGE_CHANNEL_COMMAND_REPLY,
		MESSAGE_DEACTIVATE_ACCOUNT_REPLY,
		MESSAGE_CHANGE_PASSWORD_REPLY,
		MESSAGE_GET_ALL_CHANNELS_REPLY,
		MESSAGE_DELETE_CHANNEL_REPLY,
		MESSAGE_SET_USER_DATA_REPLY,
		MESSAGE_SET_BAN_STATUS_REPLY,
		MESSAGE_GET_CHANNEL_INFO_REPLY,
		MESSAGE_GET_CHANNEL_REPLY_V2,
		MESSAGE_ADD_CHARACTER_CHANNEL_REPLY,
		MESSAGE_REMOVE_CHARACTER_CHANNEL_REPLY,
		MESSAGE_GET_CHARACTER_CHANNEL_REPLY,
		MESSAGE_UPDATE_CHARACTER_CHANNEL_REPLY,
    };


	namespace Message
	{

////////////////////////////////////////////////////////////////////////////////
	
	////////////////////////////////////////
	//  Login
	DefineMessageBegin(GetAccountRequest, Tracked, MESSAGE_GET_ACCOUNT_REQUEST)
		DefineMessageMember(AvatarName, std::string)
		DefineMessageMember(Game, std::string)
		DefineMessageMember(Server, std::string)
		DefineMessageMember(UserID, unsigned)
		DefineMessageMember(AvatarID, unsigned)	
	DefineMessageEnd

	DefineMessageBegin(GetAccountReply, TrackedReply, MESSAGE_GET_ACCOUNT_REPLY)
		DefineMessageMember(Result, unsigned)	
		DefineMessageMember(UserID, unsigned)
		DefineMessageMember(AccountID, unsigned)
		DefineMessageMember(VoicePassword, std::string)
		DefineMessageMember(EncodedAccountName, std::string)
		DefineMessageMember(URI, std::string)
	DefineMessageEnd

	
	////////////////////////////////////////
	//  GetChannel
	DefineMessageBegin(GetChannelRequest, Tracked, MESSAGE_GET_CHANNEL_REQUEST)	
		DefineMessageMember(ChannelName, std::string)		
		DefineMessageMember(Game, std::string)		
		DefineMessageMember(Server, std::string)		
		DefineMessageMember(Description, std::string)		
		DefineMessageMember(Password, std::string)	
		DefineMessageMember(Limit, unsigned)	
		DefineMessageMember(Persistent, unsigned)	
	DefineMessageEnd

	DefineMessageBegin(GetChannelReply, TrackedReply, MESSAGE_GET_CHANNEL_REPLY)	
		DefineMessageMember(Result, unsigned)		
		DefineMessageMember(ChannelName, std::string)	
		DefineMessageMember(ChannelURI, std::string)	
		DefineMessageMember(ChannelID, unsigned)
	DefineMessageEnd

	///////////////////////////////////////
	// GetProximityChannel
	DefineMessageBegin(GetProximityChannelRequest, Tracked, MESSAGE_GET_PROXIMITY_CHANNEL_REQUEST)	
		DefineMessageMember(ChannelName, std::string)		
		DefineMessageMember(Game, std::string)		
		DefineMessageMember(Server, std::string)		
		DefineMessageMember(Description, std::string)		
		DefineMessageMember(Password, std::string)	
		DefineMessageMember(Limit, unsigned)	
		DefineMessageMember(Persistent, unsigned)	
		DefineMessageMember(MaxRange, unsigned)	
		DefineMessageMember(Clamping, unsigned)	
		DefineMessageMember(RollOff, float)	
		DefineMessageMember(MaxGain, float)	
		DefineMessageMember(DistModel, unsigned)	
	DefineMessageEnd

	DefineMessageBegin(GetProximityChannelReply, TrackedReply, MESSAGE_GET_PROXIMITY_CHANNEL_REPLY)	
		DefineMessageMember(Result, unsigned)	
		DefineMessageMember(ChannelName, std::string)		
		DefineMessageMember(ChannelURI, std::string)	
		DefineMessageMember(ChannelID, unsigned)
	DefineMessageEnd


	///////////////////////////////////////
	// ChannelCommand
	DefineMessageBegin(ChannelCommandRequest, Tracked, MESSAGE_CHANNEL_COMMAND_REQUEST)	
		DefineMessageMember(SrcUserName, std::string)		
		DefineMessageMember(DestUserName, std::string)		
		DefineMessageMember(DestChannelAddress, std::string)		
		DefineMessageMember(Command, unsigned)			
		DefineMessageMember(BanTimeout, unsigned)		
	DefineMessageEnd

	DefineMessageBegin(ChannelCommandReply, TrackedReply, MESSAGE_CHANNEL_COMMAND_REPLY)
		DefineMessageMember(Result, unsigned)	
	DefineMessageEnd

	///////////////////////////////////////
	// DeactivateVoiceAccount
	DefineMessageBegin(DeactivateVoiceAccountRequest, Tracked, MESSAGE_DEACTIVATE_ACCOUNT_REQUEST)
		DefineMessageMember(AvatarName, std::string)
		DefineMessageMember(Game, std::string)
		DefineMessageMember(Server, std::string)
	DefineMessageEnd

	DefineMessageBegin(DeactivateVoiceAccountReply, TrackedReply, MESSAGE_DEACTIVATE_ACCOUNT_REPLY)
		DefineMessageMember(Result, unsigned)	
	DefineMessageEnd

	///////////////////////////////////////
	// ChangePassword
	DefineMessageBegin(ChangePasswordRequest, Tracked, MESSAGE_CHANGE_PASSWORD_REQUEST)
		DefineMessageMember(ChannelName, std::string)		
		DefineMessageMember(Game, std::string)		
		DefineMessageMember(Server, std::string)		
		DefineMessageMember(Password, std::string)	
	DefineMessageEnd

	DefineMessageBegin(ChangePasswordReply, TrackedReply, MESSAGE_CHANGE_PASSWORD_REPLY)
		DefineMessageMember(Result, unsigned)	
	DefineMessageEnd



	////////////////////////////////////////////////////////////////////////////////
	//	GetAllChannels
	DefineMessageBegin(GetAllChannelsRequest, Tracked, MESSAGE_GET_ALL_CHANNELS_REQUEST)	
	DefineMessageEnd

	DefineMessageBegin(GetAllChannelsReply, TrackedReply, MESSAGE_GET_ALL_CHANNELS_REPLY)
		DefineMessageMember(Result, unsigned)	
		DefineMessageMember(ChannelsVec, ChannelVec_t)	
	DefineMessageEnd


	///////////////////////////////////////
	// DeleteChannel
	DefineMessageBegin(DeleteChannelRequest, Tracked, MESSAGE_DELETE_CHANNEL_REQUEST)
		DefineMessageMember(ChannelName, std::string)		
		DefineMessageMember(Game, std::string)		
		DefineMessageMember(Server, std::string)	
	DefineMessageEnd

	DefineMessageBegin(DeleteChannelReply, TrackedReply, MESSAGE_DELETE_CHANNEL_REPLY)
		DefineMessageMember(Result, unsigned)	
	DefineMessageEnd


	///////////////////////////////////////
	// SetUserData
	DefineMessageBegin(SetUserDataRequest, Tracked, MESSAGE_SET_USER_DATA_REQUEST)
		DefineMessageMember(UserID, unsigned)		
		DefineMessageMember(Attributes, unsigned)		
		DefineMessageMember(Email, std::string)			
		DefineMessageMember(PhoneNumber, std::string)			
		DefineMessageMember(Pin, std::string)			
		DefineMessageMember(SmsAccount, std::string)
	DefineMessageEnd

	DefineMessageBegin(SetUserDataReply, TrackedReply, MESSAGE_SET_USER_DATA_REPLY)
	DefineMessageMember(Result, unsigned)	
	DefineMessageEnd


	///////////////////////////////////////
	// SetBanStatus
	DefineMessageBegin(SetBanStatusRequest, Tracked, MESSAGE_SET_BAN_STATUS_REQUEST)
		DefineMessageMember(UserID, unsigned)		
		DefineMessageMember(BanStatus, unsigned)	
	DefineMessageEnd

	DefineMessageBegin(SetBanStatusReply, TrackedReply, MESSAGE_SET_BAN_STATUS_REPLY)
		DefineMessageMember(Result, unsigned)	
	DefineMessageEnd


	////////////////////////////////////////
	//  GetChannelInfo
	DefineMessageBegin(GetChannelInfoRequest, Tracked, MESSAGE_GET_CHANNEL_INFO_REQUEST)	
		DefineMessageMember(ChannelName, std::string)		
		DefineMessageMember(Game, std::string)		
		DefineMessageMember(Server, std::string)	
	DefineMessageEnd

	DefineMessageBegin(GetChannelInfoReply, TrackedReply, MESSAGE_GET_CHANNEL_INFO_REPLY)	
		DefineMessageMember(Result, unsigned)		
		DefineMessageMember(ChannelName, std::string)	
		DefineMessageMember(ChannelURI, std::string)	
		DefineMessageMember(ChannelID, unsigned)
	DefineMessageEnd

	////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////
	//  GetChannel
	DefineMessageBegin(GetChannelRequestV2, Tracked, MESSAGE_GET_CHANNEL_REQUEST_V2)	
		DefineMessageMember(ChannelName, std::string)		
		DefineMessageMember(Game, std::string)		
		DefineMessageMember(Server, std::string)		
		DefineMessageMember(Description, std::string)		
		DefineMessageMember(Password, std::string)	
		DefineMessageMember(Limit, unsigned)	
		DefineMessageMember(Persistent, unsigned)	
	DefineMessageEnd

	DefineMessageBegin(GetChannelReplyV2, TrackedReply, MESSAGE_GET_CHANNEL_REPLY_V2)	
		DefineMessageMember(Result, unsigned)		
		DefineMessageMember(ChannelName, std::string)	
		DefineMessageMember(ChannelURI, std::string)	
		DefineMessageMember(ChannelID, unsigned)
		DefineMessageMember(IsNewChannel, unsigned)	
	DefineMessageEnd


	////////////////////////////////////////
	//  AddCharacterChannel
	DefineMessageBegin(AddCharacterChannelRequest, Tracked, MESSAGE_ADD_CHARACTER_CHANNEL)
		DefineMessageMember( StationID,unsigned )
		DefineMessageMember( AvatarID, unsigned )
		DefineMessageMember( CharacterName, std::string )
		DefineMessageMember( WorldName, std::string )
		DefineMessageMember( GameCode, std::string )
		DefineMessageMember( ChannelType, std::string )
		DefineMessageMember( ChannelDescription, std::string )
		DefineMessageMember( Password, std::string)
		DefineMessageMember( ChannelAddress, std::string )
		DefineMessageMember( Locale, std::string )
	DefineMessageEnd

	DefineMessageBegin(AddCharacterChannelReply, TrackedReply, MESSAGE_ADD_CHARACTER_CHANNEL_REPLY)
		DefineMessageMember(Result, unsigned)	
	DefineMessageEnd

	////////////////////////////////////////
	//  RemoveCharacterChannel
	DefineMessageBegin(RemoveCharacterChannelRequest, Tracked, MESSAGE_REMOVE_CHARACTER_CHANNEL)
		DefineMessageMember( StationID,unsigned )
		DefineMessageMember( AvatarID, unsigned )
		DefineMessageMember( CharacterName, std::string )
		DefineMessageMember( WorldName, std::string )
		DefineMessageMember( GameCode, std::string )
		DefineMessageMember( ChannelType, std::string )
	DefineMessageEnd

	DefineMessageBegin(RemoveCharacterChannelReply, TrackedReply, MESSAGE_REMOVE_CHARACTER_CHANNEL_REPLY)
		DefineMessageMember(Result, unsigned)	
	DefineMessageEnd


	////////////////////////////////////////
	//  GetCharacterChannel
	DefineMessageBegin(GetCharacterChannelRequest, Tracked, MESSAGE_GET_CHARACTER_CHANNEL)
		DefineMessageMember( StationID,unsigned )
		DefineMessageMember( CharacterName, std::string )
		DefineMessageMember( WorldName, std::string )
		DefineMessageMember( GameCode, std::string )
	DefineMessageEnd

	DefineMessageBegin(GetCharacterChannelReply, TrackedReply, MESSAGE_GET_CHARACTER_CHANNEL_REPLY)
		DefineMessageMember(Result, unsigned)
		DefineMessageMember(ChannelList, CharacterChannelVec_t)
	DefineMessageEnd

	////////////////////////////////////////
	//  UpdateCharacterChannel
	DefineMessageBegin(UpdateCharacterChannelRequest, Tracked, MESSAGE_UPDATE_CHARACTER_CHANNEL)
		DefineMessageMember( StationID,unsigned )
		DefineMessageMember( AvatarID, unsigned )
		DefineMessageMember( CharacterName, std::string )
		DefineMessageMember( WorldName, std::string )
		DefineMessageMember( GameCode, std::string )
		DefineMessageMember( ChannelType, std::string )
		DefineMessageMember( ChannelDescription, std::string )
		DefineMessageMember( ChannelID, unsigned )
		DefineMessageMember( Password, std::string)
		DefineMessageMember( ChannelURI, std::string )
		DefineMessageMember( Locale, std::string )
	DefineMessageEnd

	DefineMessageBegin(UpdateCharacterChannelReply, TrackedReply, MESSAGE_UPDATE_CHARACTER_CHANNEL_REPLY)
		DefineMessageMember(Result, unsigned)	
	DefineMessageEnd

	/*	,
	,
	,
	MESSAGE_UPDATE_CHARACTER_CHANNEL,*/



	////////////////////////////////////////////////////////////////////////////////

    }  // namepsace Message

}	//	namespace API_NAMESPACE


#endif	//_VCHAT_SERVER_API_MESSAGES_H_


