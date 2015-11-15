#ifdef WIN32
#pragma warning (disable : 4786 4514)
#endif

#include <stdio.h>
#include <time.h>
#include "VChatAPICore.h"
#include "messages.h"
#include "Base/hash.h"

using namespace std;
//using namespace API_NAMESPACE::Message;


namespace VChatSystem
{
    
    VChatAPICore::VChatAPICore(VChatAPI & client, const char * hostList, const char * failoverHostList, 
                    unsigned connectionLimit, unsigned maxMsgSize, unsigned bufferSize) 
		: CommonAPI(hostList, failoverHostList, connectionLimit, maxMsgSize, bufferSize)
        , mClient(client)
    {
		// requests
		RegisterTrackedMessage(MESSAGE_GET_ACCOUNT_REQUEST);
		RegisterTrackedMessage(MESSAGE_GET_CHANNEL_REQUEST);
		RegisterTrackedMessage(MESSAGE_GET_PROXIMITY_CHANNEL_REQUEST);
		RegisterTrackedMessage(MESSAGE_CHANNEL_COMMAND_REQUEST);
		RegisterTrackedMessage(MESSAGE_DEACTIVATE_ACCOUNT_REQUEST);
		RegisterTrackedMessage(MESSAGE_CHANGE_PASSWORD_REQUEST);
		RegisterTrackedMessage(MESSAGE_GET_ALL_CHANNELS_REQUEST);
		RegisterTrackedMessage(MESSAGE_DELETE_CHANNEL_REQUEST);
		RegisterTrackedMessage(MESSAGE_SET_USER_DATA_REQUEST);
		RegisterTrackedMessage(MESSAGE_SET_BAN_STATUS_REQUEST);
		RegisterTrackedMessage(MESSAGE_GET_CHANNEL_INFO_REQUEST);
		RegisterTrackedMessage(MESSAGE_GET_CHANNEL_REQUEST_V2);
		RegisterTrackedMessage(MESSAGE_ADD_CHARACTER_CHANNEL);
		RegisterTrackedMessage(MESSAGE_REMOVE_CHARACTER_CHANNEL);
		RegisterTrackedMessage(MESSAGE_GET_CHARACTER_CHANNEL);
		RegisterTrackedMessage(MESSAGE_UPDATE_CHARACTER_CHANNEL);

		// replies
		RegisterTrackedMessage(MESSAGE_GET_ACCOUNT_REPLY);
		RegisterTrackedMessage(MESSAGE_GET_CHANNEL_REPLY);
		RegisterTrackedMessage(MESSAGE_GET_PROXIMITY_CHANNEL_REPLY);
		RegisterTrackedMessage(MESSAGE_CHANNEL_COMMAND_REPLY);
		RegisterTrackedMessage(MESSAGE_DEACTIVATE_ACCOUNT_REPLY);
		RegisterTrackedMessage(MESSAGE_CHANGE_PASSWORD_REPLY);
		RegisterTrackedMessage(MESSAGE_GET_ALL_CHANNELS_REPLY);
		RegisterTrackedMessage(MESSAGE_DELETE_CHANNEL_REPLY);
		RegisterTrackedMessage(MESSAGE_SET_USER_DATA_REPLY);   
		RegisterTrackedMessage(MESSAGE_SET_BAN_STATUS_REPLY);   
		RegisterTrackedMessage(MESSAGE_GET_CHANNEL_INFO_REPLY);   
		RegisterTrackedMessage(MESSAGE_GET_CHANNEL_REPLY_V2);     
		RegisterTrackedMessage(MESSAGE_ADD_CHARACTER_CHANNEL_REPLY);     
		RegisterTrackedMessage(MESSAGE_REMOVE_CHARACTER_CHANNEL_REPLY);     
		RegisterTrackedMessage(MESSAGE_GET_CHARACTER_CHANNEL_REPLY);     
		RegisterTrackedMessage(MESSAGE_UPDATE_CHARACTER_CHANNEL_REPLY);     
		
    }

    VChatAPICore::~VChatAPICore()
    {
    }

	const char * VChatAPICore::GetVersion()
	{
		return "2008.04.25";
	}

	HashValue_t VChatAPICore::CreateHashValue(const string & name)
	{
		return HashUserName(name.c_str());
	}

	unsigned VChatAPICore::CheckMessageId(const unsigned char * data, unsigned dataLen)
	{
		API_NAMESPACE::Message::Basic message;
		message.Read(data, dataLen);
		return message.GetMsgId();
	}
	
	unsigned VChatAPICore::CheckTrackingNumber(const unsigned char * data, unsigned dataLen)
	{
		API_NAMESPACE::Message::Tracked message;
		message.Read(data, dataLen);
		return message.GetTrackingNumber();
	}

	unsigned VChatAPICore::FormatStatusMsg(unsigned char * buffer, unsigned size)
	{
		API_NAMESPACE::Message::Connect message;
		message.SetVersion(GetVersion());
		return message.Write(buffer, size);
	}

	bool VChatAPICore::CheckConnectReply(const unsigned char * data, unsigned dataLen)
	{
		API_NAMESPACE::Message::ConnectReply message;
		return (message.Read(data, dataLen) && 
				message.GetMsgId() == MESSAGE_CONNECT_REPLY && 
				message.GetResult() == RESULT_SUCCESS);
	}

	void VChatAPICore::OnConnectionOpened(const char * address)
	{
		mClient.OnConnectionOpened(address);
	}

	void VChatAPICore::OnConnectionFailed(const char * address)
	{
		mClient.OnConnectionFailed(address);
	}

	void VChatAPICore::OnConnectionClosed(const char * address, const char * reason)
	{
		mClient.OnConnectionClosed(address, reason);
	}

	void VChatAPICore::OnConnectionShutdownNotified(const char * address, unsigned outstandingRequests)
	{
		mClient.OnConnectionShutdownNotified(address, outstandingRequests);
	}
 
	/////////////////////////////////////////
	// request callbacks for successful communication
	/////////////////////////////////////////

	void VChatAPICore::Callback(const unsigned char * data, unsigned dataLen, unsigned messageId, void * userData)
	{
		switch (messageId)
		{			
			case MESSAGE_GET_ACCOUNT_REPLY:
			{
				Message::GetAccountReply message;
				if(message.Read(data, dataLen))
					mClient.OnGetAccount(message.GetTrackingNumber(), message.GetResult(), message.GetUserID(), message.GetAccountID(), message.GetVoicePassword(), message.GetEncodedAccountName(), message.GetURI(), userData);
				break;
			}
			case MESSAGE_GET_CHANNEL_REPLY:
			{
				Message::GetChannelReply message;
				if(message.Read(data, dataLen))
					mClient.OnGetChannel(message.GetTrackingNumber(), message.GetResult(), message.GetChannelName(), message.GetChannelURI(), message.GetChannelID(), userData);
				break;
			}
			case MESSAGE_GET_PROXIMITY_CHANNEL_REPLY:
			{
				Message::GetProximityChannelReply message;
				if(message.Read(data, dataLen))
					mClient.OnGetProximityChannel(message.GetTrackingNumber(), message.GetResult(), message.GetChannelName(), message.GetChannelURI(), message.GetChannelID(), userData);
				break;
			}
			case MESSAGE_CHANNEL_COMMAND_REPLY:
			{
				Message::ChannelCommandReply message;
				if(message.Read(data, dataLen))
					mClient.OnChannelCommand(message.GetTrackingNumber(), message.GetResult(), userData);
				break;
			}
			case MESSAGE_DEACTIVATE_ACCOUNT_REPLY:
			{
				Message::DeactivateVoiceAccountReply message;
				if(message.Read(data, dataLen))
					mClient.OnDeactivateVoiceAccount(message.GetTrackingNumber(), message.GetResult(), userData);
				break;
			}
			case MESSAGE_CHANGE_PASSWORD_REPLY:
			{
				Message::ChangePasswordReply message;
				if(message.Read(data, dataLen))
					mClient.OnChangePassword(message.GetTrackingNumber(), message.GetResult(), userData);
				break;
			}
			case MESSAGE_GET_ALL_CHANNELS_REPLY:
			{
				Message::GetAllChannelsReply message;
				if(message.Read(data, dataLen))
					mClient.OnGetAllChannels(message.GetTrackingNumber(), message.GetResult(), message.GetChannelsVec(), userData);
				break;
			}
			case MESSAGE_DELETE_CHANNEL_REPLY:
			{
				Message::DeleteChannelReply message;
				if(message.Read(data, dataLen))
					mClient.OnDeleteChannel(message.GetTrackingNumber(), message.GetResult(), userData);
				break;
			}
			case MESSAGE_SET_USER_DATA_REPLY:
			{
				Message::SetUserDataReply message;
				if(message.Read(data, dataLen))
					mClient.OnSetUserData(message.GetTrackingNumber(), message.GetResult(), userData);
				break;
			}
			case MESSAGE_SET_BAN_STATUS_REPLY:
			{
				Message::SetBanStatusReply message;
				if(message.Read(data, dataLen))
					mClient.OnSetBanStatus(message.GetTrackingNumber(), message.GetResult(), userData);
				break;
			}
			case MESSAGE_GET_CHANNEL_INFO_REPLY:
			{
				Message::GetChannelInfoReply message;
				if(message.Read(data, dataLen))
					mClient.OnGetChannelInfo(message.GetTrackingNumber(), message.GetResult(), message.GetChannelName(), message.GetChannelURI(), message.GetChannelID(), userData);
				break;
			}
			case MESSAGE_GET_CHANNEL_REPLY_V2:
			{
				Message::GetChannelReplyV2 message;
				if(message.Read(data, dataLen))
					mClient.OnGetChannelV2(message.GetTrackingNumber(), message.GetResult(), message.GetChannelName(), message.GetChannelURI(), message.GetChannelID(), message.GetIsNewChannel(),userData);
				break;
			}
			case MESSAGE_ADD_CHARACTER_CHANNEL_REPLY:
			{
				Message::AddCharacterChannelReply message;
				if(message.Read(data, dataLen))
					mClient.OnAddCharacterChannel(message.GetTrackingNumber(), message.GetResult(), userData);
				break;
			}
			case MESSAGE_REMOVE_CHARACTER_CHANNEL_REPLY:
			{
				Message::RemoveCharacterChannelReply message;
				if (message.Read(data, dataLen)) {
					mClient.OnRemoveCharacterChannel(message.GetTrackingNumber(), message.GetResult(), userData);
					break;
				}
			}
			case MESSAGE_GET_CHARACTER_CHANNEL_REPLY:
			{
				Message::GetCharacterChannelReply message;
				if (message.Read(data, dataLen)) {
					mClient.OnGetCharacterChannel(message.GetTrackingNumber(), message.GetResult(), message.GetChannelList(), userData);
					break;
				}
			}
			
			default:
				break;			
		}
	}


	/////////////////////////////////////////
	// TIMEOUT callbacks
	/////////////////////////////////////////
	void VChatAPICore::Callback(unsigned messageId, unsigned trackingNumber, unsigned result, void * userData)
	{
		switch (messageId)
		{		
			case MESSAGE_GET_ACCOUNT_REPLY:
			{
				mClient.OnGetAccount(trackingNumber, result, 0, 0, std::string(), std::string(), std::string(), userData);
				break;
			}
			case MESSAGE_GET_CHANNEL_REPLY:
			{				
				mClient.OnGetChannel(trackingNumber, result, std::string(), std::string(), 0, userData);
				break;
			}
			case MESSAGE_GET_PROXIMITY_CHANNEL_REPLY:
			{				
				mClient.OnGetProximityChannel(trackingNumber, result, std::string(), std::string(), 0, userData);
				break;
			}
			case MESSAGE_CHANNEL_COMMAND_REPLY:
			{
				mClient.OnChannelCommand(trackingNumber, result, userData);
				break;
			}
			case MESSAGE_DEACTIVATE_ACCOUNT_REPLY:
			{
				mClient.OnDeactivateVoiceAccount(trackingNumber, result, userData);
				break;
			}
			case MESSAGE_CHANGE_PASSWORD_REPLY:
			{
				mClient.OnChangePassword(trackingNumber, result, userData);
				break;
			}
			case MESSAGE_GET_ALL_CHANNELS_REPLY:
			{
				mClient.OnGetAllChannels(trackingNumber, result, ChannelVec_t(), userData);
				break;
			}
			case MESSAGE_DELETE_CHANNEL_REPLY:
			{
				mClient.OnDeleteChannel(trackingNumber, result, userData);
				break;
			}
			case MESSAGE_SET_USER_DATA_REPLY:
			{
				mClient.OnSetUserData(trackingNumber, result, userData);
				break;
			}
			case MESSAGE_SET_BAN_STATUS_REPLY:
			{
				mClient.OnSetBanStatus(trackingNumber, result, userData);
				break;
			}
			case MESSAGE_GET_CHANNEL_INFO_REPLY:
			{
				mClient.OnGetChannelInfo(trackingNumber, result, std::string(), std::string(), 0, userData);
				break;
			}
			case MESSAGE_GET_CHANNEL_REPLY_V2:
			{				
				mClient.OnGetChannelV2(trackingNumber, result, std::string(), std::string(), 0, 0, userData);
				break;
			}
			case MESSAGE_ADD_CHARACTER_CHANNEL_REPLY:
			{
				mClient.OnAddCharacterChannel(trackingNumber, result, userData);
				break;
			}
			case MESSAGE_REMOVE_CHARACTER_CHANNEL_REPLY:
			{
				mClient.OnRemoveCharacterChannel(trackingNumber, result,userData);
				break;
			}
			case MESSAGE_GET_CHARACTER_CHANNEL_REPLY:
			{
				mClient.OnGetCharacterChannel(trackingNumber, result, CharacterChannelVec_t(), userData);
				break;
			}
		default :
				break;
		}
	}	
}


