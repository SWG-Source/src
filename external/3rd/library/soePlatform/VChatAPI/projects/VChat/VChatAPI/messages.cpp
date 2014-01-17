#ifdef WIN32
#pragma warning (disable : 4786 4514)
#endif

#include "messages.h"

using namespace std;

namespace VChatSystem
{

	namespace Message
	{
		////////////////////////////////////////
		//  Login
		ImplementMessageBegin(GetAccountRequest, Tracked)
			ImplementMessageMemberEx(AvatarName, std::string(), MAX_AVATAR_LEN)
			ImplementMessageMemberEx(Game, std::string(), MAX_GAME_LEN)
			ImplementMessageMemberEx(Server, std::string(), MAX_WORLD_LEN)
			ImplementMessageMember(UserID, 0)
			ImplementMessageMember(AvatarID, 0)	
		ImplementMessageEnd

		ImplementMessageBegin(GetAccountReply, TrackedReply)
			ImplementMessageMember(Result, 0)	
			ImplementMessageMember(UserID, 0)
			ImplementMessageMember(AccountID, 0)
			ImplementMessageMemberEx(VoicePassword, std::string(), MAX_PASSWORD_LEN)
			ImplementMessageMemberEx(EncodedAccountName, std::string(), MAX_FULL_NAME_LEN)
			ImplementMessageMemberEx(URI, std::string(), MAX_URI_LEN)
		ImplementMessageEnd


			////////////////////////////////////////
			//  GetChannel
		ImplementMessageBegin(GetChannelRequest, Tracked)	
			ImplementMessageMemberEx(ChannelName, std::string(), MAX_CHANNEL_LEN)		
			ImplementMessageMemberEx(Game, std::string(), MAX_GAME_LEN)		
			ImplementMessageMemberEx(Server, std::string(), MAX_WORLD_LEN)		
			ImplementMessageMemberEx(Description, std::string(), MAX_DESCRIPTION_LEN)		
			ImplementMessageMemberEx(Password, std::string(), MAX_PASSWORD_LEN)	
			ImplementMessageMember(Limit, 0)	
			ImplementMessageMember(Persistent, 0)	
		ImplementMessageEnd

		ImplementMessageBegin(GetChannelReply, TrackedReply)	
			ImplementMessageMember(Result, 0)		
			ImplementMessageMemberEx(ChannelName, std::string(), MAX_FULL_NAME_LEN)	
			ImplementMessageMemberEx(ChannelURI, std::string(), MAX_FULL_NAME_LEN)	
			ImplementMessageMember(ChannelID, 0)	
		ImplementMessageEnd

			///////////////////////////////////////
			// GetProximityChannel
		ImplementMessageBegin(GetProximityChannelRequest, Tracked)	
			ImplementMessageMemberEx(ChannelName, std::string(), MAX_CHANNEL_LEN)		
			ImplementMessageMemberEx(Game, std::string(), MAX_GAME_LEN)		
			ImplementMessageMemberEx(Server, std::string(), MAX_WORLD_LEN)		
			ImplementMessageMemberEx(Description, std::string(), MAX_DESCRIPTION_LEN)		
			ImplementMessageMemberEx(Password, std::string(), MAX_PASSWORD_LEN)	
			ImplementMessageMember(Limit, 0)	
			ImplementMessageMember(Persistent, 0)	
			ImplementMessageMember(MaxRange, 0)	
			ImplementMessageMember(Clamping, 0)	
			ImplementMessageMember(RollOff, 0)	
			ImplementMessageMember(MaxGain, 0)	
			ImplementMessageMember(DistModel, 0)	
		ImplementMessageEnd

		ImplementMessageBegin(GetProximityChannelReply, TrackedReply)	
			ImplementMessageMember(Result, 0)	
			ImplementMessageMemberEx(ChannelName, std::string(), MAX_FULL_NAME_LEN)	
			ImplementMessageMemberEx(ChannelURI, std::string(), MAX_FULL_NAME_LEN)	
			ImplementMessageMember(ChannelID, 0)	
		ImplementMessageEnd


		///////////////////////////////////////
		// ChannelCommand
		ImplementMessageBegin(ChannelCommandRequest, Tracked)	
			ImplementMessageMemberEx(SrcUserName, std::string(), MAX_FULL_NAME_LEN)		
			ImplementMessageMemberEx(DestUserName, std::string(), MAX_FULL_NAME_LEN)		
			ImplementMessageMemberEx(DestChannelAddress, std::string(), MAX_FULL_NAME_LEN)		
			ImplementMessageMember(Command, 0)	
			ImplementMessageMember(BanTimeout, 0)	
		ImplementMessageEnd

		ImplementMessageBegin(ChannelCommandReply, TrackedReply)
			ImplementMessageMember(Result, 0)	
		ImplementMessageEnd

			///////////////////////////////////////
			// DeactivateVoiceAccount
		ImplementMessageBegin(DeactivateVoiceAccountRequest, Tracked)
			ImplementMessageMemberEx(AvatarName, std::string(), MAX_AVATAR_LEN)
			ImplementMessageMemberEx(Game, std::string(), MAX_GAME_LEN)
			ImplementMessageMemberEx(Server, std::string(), MAX_WORLD_LEN)
		ImplementMessageEnd

		ImplementMessageBegin(DeactivateVoiceAccountReply, TrackedReply)
			ImplementMessageMember(Result, 0)	
		ImplementMessageEnd

		///////////////////////////////////////
		// ChangePassword
		ImplementMessageBegin(ChangePasswordRequest, Tracked)
			ImplementMessageMemberEx(ChannelName, std::string(), MAX_CHANNEL_LEN)		
			ImplementMessageMemberEx(Game, std::string(), MAX_GAME_LEN)		
			ImplementMessageMemberEx(Server, std::string(), MAX_WORLD_LEN)		
			ImplementMessageMemberEx(Password, std::string(), MAX_PASSWORD_LEN)	
		ImplementMessageEnd

		ImplementMessageBegin(ChangePasswordReply, TrackedReply)
			ImplementMessageMember(Result, 0)	
		ImplementMessageEnd
		////////////////////////////////////////
	
		////////////////////////////////////////////////////////////////////////////////
		//	GetAllChannels
		ImplementMessageBegin(GetAllChannelsRequest, Tracked)	
		ImplementMessageEnd

		////////////////////////////////////////////////////////////////////////////////
		//	GetAllChannelsReply
		ImplementMessageBegin(GetAllChannelsReply, TrackedReply)
			ImplementMessageMember(Result, 0)
			ImplementMessageMemberEx(ChannelsVec, ChannelVec_t(), 4096)
		ImplementMessageEnd	


		///////////////////////////////////////
		// DeleteChannelRequest
		ImplementMessageBegin(DeleteChannelRequest, Tracked)
			ImplementMessageMemberEx(ChannelName, std::string(), MAX_CHANNEL_LEN)		
			ImplementMessageMemberEx(Game, std::string(), MAX_GAME_LEN)		
			ImplementMessageMemberEx(Server, std::string(), MAX_WORLD_LEN)	
		ImplementMessageEnd

		ImplementMessageBegin(DeleteChannelReply, TrackedReply)
			ImplementMessageMember(Result, 0)	
		ImplementMessageEnd

		///////////////////////////////////////
		// SetUserData
		ImplementMessageBegin(SetUserDataRequest, Tracked)
			ImplementMessageMember(UserID, 0)		
			ImplementMessageMember(Attributes, 0)		
			ImplementMessageMemberEx(Email, std::string(), MAX_EMAIL_LEN)			
			ImplementMessageMemberEx(PhoneNumber, std::string(), MAX_PHONE_NUMBER_LEN)			
			ImplementMessageMemberEx(Pin, std::string(), MAX_PIN_LEN)			
			ImplementMessageMemberEx(SmsAccount, std::string(), MAX_SMS_ACCT_LEN)
		ImplementMessageEnd

		ImplementMessageBegin(SetUserDataReply, TrackedReply)
			ImplementMessageMember(Result, 0)	
		ImplementMessageEnd

		///////////////////////////////////////
		// SetBanStatus
		ImplementMessageBegin(SetBanStatusRequest, Tracked)
			ImplementMessageMember(UserID, 0)		
			ImplementMessageMember(BanStatus, 0)	
		ImplementMessageEnd

		ImplementMessageBegin(SetBanStatusReply, TrackedReply)
			ImplementMessageMember(Result, 0)	
		ImplementMessageEnd


		////////////////////////////////////////
		//  GetChannel
		ImplementMessageBegin(GetChannelInfoRequest, Tracked)	
			ImplementMessageMemberEx(ChannelName, std::string(), MAX_CHANNEL_LEN)		
			ImplementMessageMemberEx(Game, std::string(), MAX_GAME_LEN)		
			ImplementMessageMemberEx(Server, std::string(), MAX_WORLD_LEN)	
		ImplementMessageEnd

		ImplementMessageBegin(GetChannelInfoReply, TrackedReply)	
			ImplementMessageMember(Result, 0)		
			ImplementMessageMemberEx(ChannelName, std::string(), MAX_FULL_NAME_LEN)	
			ImplementMessageMemberEx(ChannelURI, std::string(), MAX_FULL_NAME_LEN)	
			ImplementMessageMember(ChannelID, 0)	
		ImplementMessageEnd

		////////////////////////////////////////
		//  GetChannel
		ImplementMessageBegin(GetChannelRequestV2, Tracked)	
			ImplementMessageMemberEx(ChannelName, std::string(), MAX_CHANNEL_LEN)		
			ImplementMessageMemberEx(Game, std::string(), MAX_GAME_LEN)		
			ImplementMessageMemberEx(Server, std::string(), MAX_WORLD_LEN)		
			ImplementMessageMemberEx(Description, std::string(), MAX_DESCRIPTION_LEN)		
			ImplementMessageMemberEx(Password, std::string(), MAX_PASSWORD_LEN)	
			ImplementMessageMember(Limit, 0)	
			ImplementMessageMember(Persistent, 0)	
		ImplementMessageEnd

		ImplementMessageBegin(GetChannelReplyV2, TrackedReply)	
			ImplementMessageMember(Result, 0)		
			ImplementMessageMemberEx(ChannelName, std::string(), MAX_FULL_NAME_LEN)	
			ImplementMessageMemberEx(ChannelURI, std::string(), MAX_FULL_NAME_LEN)	
			ImplementMessageMember(ChannelID, 0)	
			ImplementMessageMember(IsNewChannel, 0)	
		ImplementMessageEnd

		////////////////////////////////////////
		//  AddCharacterChannel
		ImplementMessageBegin(AddCharacterChannelRequest, Tracked)
			ImplementMessageMember( StationID,0 )
			ImplementMessageMember( AvatarID, 0 )
			ImplementMessageMemberEx( CharacterName, std::string(),MAX_FULL_NAME_LEN )
			ImplementMessageMemberEx( WorldName, std::string(), MAX_WORLD_LEN )
			ImplementMessageMemberEx( GameCode, std::string(), MAX_GAME_LEN )
			ImplementMessageMemberEx( ChannelType, std::string(),MAX_FULL_NAME_LEN )
			ImplementMessageMemberEx( ChannelDescription, std::string(), MAX_DESCRIPTION_LEN )
			ImplementMessageMemberEx( Password, std::string(), MAX_PASSWORD_LEN)
			ImplementMessageMemberEx( ChannelAddress, std::string(),MAX_CHANNEL_LEN )
			ImplementMessageMemberEx( Locale, std::string(),MAX_FULL_NAME_LEN )
		ImplementMessageEnd

		ImplementMessageBegin(AddCharacterChannelReply, TrackedReply)
			ImplementMessageMember(Result, 0)	
		ImplementMessageEnd

		////////////////////////////////////////
		//  RemoveCharacterChannel
		ImplementMessageBegin(RemoveCharacterChannelRequest, Tracked)
			ImplementMessageMember( StationID,0 )
			ImplementMessageMember( AvatarID, 0 )
			ImplementMessageMemberEx( CharacterName, std::string(),MAX_FULL_NAME_LEN )
			ImplementMessageMemberEx( WorldName, std::string(), MAX_WORLD_LEN )
			ImplementMessageMemberEx( GameCode, std::string(), MAX_GAME_LEN )
			ImplementMessageMemberEx( ChannelType, std::string(),MAX_FULL_NAME_LEN )
		ImplementMessageEnd

		ImplementMessageBegin(RemoveCharacterChannelReply, TrackedReply)
			ImplementMessageMember(Result, 0)	
		ImplementMessageEnd


		////////////////////////////////////////
		//  GetCharacterChannel
		ImplementMessageBegin(GetCharacterChannelRequest, Tracked)
			ImplementMessageMember( StationID,0 )
			ImplementMessageMemberEx( CharacterName, std::string(),MAX_FULL_NAME_LEN )
			ImplementMessageMemberEx( WorldName, std::string(), MAX_WORLD_LEN )
			ImplementMessageMemberEx( GameCode, std::string(), MAX_GAME_LEN )
		ImplementMessageEnd

		ImplementMessageBegin(GetCharacterChannelReply, TrackedReply)
			ImplementMessageMember(Result, 0)	
			ImplementMessageMemberEx(ChannelList, CharacterChannelVec_t(), MAX_NAME_PAIR_LEN)
		ImplementMessageEnd

		////////////////////////////////////////
		//  UpdateCharacterChannel
		ImplementMessageBegin(UpdateCharacterChannelRequest, Tracked)
			ImplementMessageMember( StationID,0 )
			ImplementMessageMember( AvatarID, 0 )
			ImplementMessageMemberEx( CharacterName, std::string(),MAX_FULL_NAME_LEN )
			ImplementMessageMemberEx( WorldName, std::string(), MAX_WORLD_LEN )
			ImplementMessageMemberEx( GameCode, std::string(), MAX_GAME_LEN )
			ImplementMessageMemberEx( ChannelType, std::string(),MAX_FULL_NAME_LEN )
			ImplementMessageMemberEx( ChannelDescription, std::string(), MAX_DESCRIPTION_LEN )
			ImplementMessageMember( ChannelID, 0 )
			ImplementMessageMemberEx( Password, std::string(), MAX_PASSWORD_LEN)
			ImplementMessageMemberEx( ChannelURI, std::string(),MAX_FULL_NAME_LEN )
			ImplementMessageMemberEx( Locale, std::string(),MAX_FULL_NAME_LEN )
		ImplementMessageEnd

		ImplementMessageBegin(UpdateCharacterChannelReply, TrackedReply)
			ImplementMessageMember(Result, 0)	
		ImplementMessageEnd

   }

}

