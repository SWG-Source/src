// VChatInterface.h

#ifndef	_INCLUDED_VChatInterface_H
#define	_INCLUDED_VChatInterface_H

#include "sharedFoundation/NetworkId.h"

#include <set>
#include <string>
#include <sstream>
#include <map>

#pragma warning(disable:4100 4244)
#include "VChatAPI/VChatAPI.h"
#pragma warning(default:4100 4244)

struct ReturnAddress
{
	enum ReturnAddressType
	{
		RAT_invalid,
		RAT_client,
		RAT_gameserver,
	};

	ReturnAddressType type;
	NetworkId clientId;
	unsigned gameServerId;

	ReturnAddress() : type(RAT_invalid), clientId(), gameServerId(0) {}
	ReturnAddress(NetworkId const & id):type(RAT_client), clientId(id), gameServerId(0) {}
	ReturnAddress(unsigned const id) : type(RAT_gameserver), clientId(), gameServerId(id) {}
	inline std::string debugString() const
	{
		switch(type)
		{
		case RAT_invalid: return "invalid"; break;
		case RAT_client: return std::string("client") + clientId.getValueString(); break;
		case RAT_gameserver:
			{
				std::stringstream ss;
				ss << "gameserver";
				ss << gameServerId;
				return ss.str();
			}
			break;
		default: return "unknown type";
		}
	}
};

//-----------------------------------------------------------------------

class VChatInterface : public VChatSystem::VChatAPI
{

public:

	VChatInterface(const char * hostList);
	virtual ~VChatInterface();

	//functions that need looking at before this goes final
	bool addClientToChannel(std::string const & roomName, NetworkId const & id, std::string const & playerName, bool forceShortlist = false);
	bool removeClientFromChannel(std::string const & roomName, NetworkId const & id, std::string const & playerName);
	
	void sendChannelData(ReturnAddress const & toWhom, std::string const & channelName, std::string const & messageText, bool targetIsModerator, bool notifyIfNoChannel, bool doGetChannelInfo = false);

	//personal channel stuff
	std::string buildPersonalChannelName(NetworkId const & ownerId);
	void requestInvite(NetworkId const & sourceId, NetworkId const & targetId, std::string const & channelName);
	void requestKick(NetworkId const & sourceId, NetworkId const & targetId, std::string const & channelName);

	void broadcastGlobalChannelMessage(std::string const & channelName, std::string const & textMessage, bool isRemove);

	//end questionable functions

	inline bool isConnected() { return m_connected; }

	//////////////
	// Requests //
	//////////////

	unsigned requestGetChannel(const std::string &channelName, 
		const std::string &description, 
		const std::string &password,
		unsigned limit,
		bool isPublic,
		bool persistent,
		std::list<std::string> const & moderators,
		ReturnAddress const & requester);

	unsigned requestDeleteChannel(const std::string &channelName, ReturnAddress const & requester);

	unsigned requestChannelCommand(ReturnAddress const & request,
		const std::string &srcUserName, 
		const std::string &destUserName, 
		const std::string &destChannelAddress, 
		unsigned command,
		unsigned banTimeout);

	unsigned requestConnectPlayer(unsigned suid, std::string const & characterName, NetworkId const & netId, unsigned previousAttempts = 0);

	unsigned requestChannelInfo(std::string const & channelName, ReturnAddress const & requester, bool isForGlobalBroadcast = false, std::string const & messageText = "");

	///////////////
	// Callbacks //
	///////////////
	virtual void OnConnectionOpened( const char * address );
	virtual void OnConnectionFailed( const char * address );
	virtual void OnConnectionClosed( const char * address, const char * reason );
	virtual void OnConnectionShutdownNotified( const char * address, unsigned outstandingRequests );

	virtual void OnGetAccount(unsigned track, 
		unsigned result, 
		unsigned userID,
		unsigned accountID, 
		const std::string &voicePassword,
		const std::string &encodedVoiceAccount,
		const std::string &URI,
		void *user);

	virtual void OnGetChannelV2(unsigned track, 
		unsigned result, 
		const std::string &channelName, 
		const std::string &channelURI, 
		unsigned channelID,
		unsigned isNewChannel,
		void *user);

	virtual void OnChannelCommand(unsigned track, 
		unsigned result, 
		void *user);

	virtual void OnGetAllChannels(unsigned track, 
		unsigned result, 
		const VChatSystem::ChannelVec_t & channels,
		void *user);

	virtual void OnDeleteChannel(unsigned track, 
		unsigned result, 
		void *user);

	virtual void OnGetChannelInfo(unsigned track, 
		unsigned result, 
		const std::string & channelName, 
		const std::string & channelURI, 
		unsigned channelID,
		void * user);

	virtual void OnAddCharacterChannel(unsigned int track, 
		unsigned int result, 
		void * user);

	virtual void OnRemoveCharacterChannel(unsigned track, 
		unsigned result, 
		void *user);

	virtual void OnGetCharacterChannel(unsigned track, 
		unsigned result, 
		const VChatSystem::CharacterChannelVec_t &characterChannels, 
		void * user);

	virtual void OnUpdateCharacterChannel(unsigned track, 
		unsigned result, 
		void *user);

	//callbacks for things we don't care about but have to implement
	//because they are virtual in vchatapi
	virtual void OnGetProximityChannel (unsigned /*track*/, 
		unsigned /*result*/, 
		const std::string & /*channelName*/, 
		const std::string & /*channelURI*/, 
		unsigned /*channelID*/,
		void * /*user*/) {};


	virtual void OnDeactivateVoiceAccount(unsigned /*track*/, 
		unsigned /*result*/, 
		void * /*user*/) {};

	virtual void OnChangePassword(unsigned /*track*/, 
		unsigned /*result*/, 
		void * /*user*/) {};

	virtual void OnSetUserData( unsigned /*track*/, 
		unsigned /*result*/, 
		void * /*user*/) {};

	virtual void OnSetBanStatus(unsigned /*track*/, 
		unsigned /*result*/, 
		void * /*user*/) {};

	virtual void OnGetChannel(unsigned /*track*/, 
		unsigned /*result*/, 
		const std::string & /*channelName*/, 
		const std::string & /*channelURI*/, 
		unsigned /*channelID*/,
		void * /*user*/) {};

private:

	std::string const & getSystemLoginName();
	std::string parseWorldName(std::string const & input);

	// these functions all convert the name to lower case for the lookup
	// but preserves case in the data
	bool findChannelDataByName(std::string const & name, VChatSystem::Channel & data);
	void setChannelData(VChatSystem::Channel const & data);
	bool eraseChannelData(std::string const & name);

	//Helper functions for character channel management.
	void checkForCharacterChannelAdd(std::string const & name, std::string const & channelName);
	void checkForCharacterChannelRemove(std::string const & name, std::string const & channelName);

	unsigned internalGetChannel(const std::string &channelName, 
		const std::string &description, 
		const std::string &password,
		unsigned limit,
		bool isPublic,
		bool persistent,
		std::list<std::string> const & moderators,
		ReturnAddress const & requester);


	typedef std::map<std::string,VChatSystem::Channel> ChannelDataMap;
	ChannelDataMap m_channelData;
	typedef std::map<std::string, std::string> GlobalChannelMap;
	GlobalChannelMap m_globalChannels;
	bool m_connected;
	uint32 m_connectionFailedCount;
	typedef std::map<std::string, std::vector<NetworkId> > VoiceChannelIdMap;
	VoiceChannelIdMap m_voiceChatChannelNameToNetworkIdMap;

	struct PendingGetChannelRequest
	{
		std::string m_channelName;
		std::string m_description;
		std::string m_password;
		unsigned int m_limit;
		bool m_isPublic;
		bool m_persistent;
		std::list<std::string> m_moderators;
		ReturnAddress m_requester;
	};

	typedef std::map<std::string, std::list<PendingGetChannelRequest> > PendingGetChannelRequestMap;
	PendingGetChannelRequestMap m_pendingGetChannelRequests;
};

//-----------------------------------------------------------------------

#endif	// _INCLUDED_VChatInterface_H
