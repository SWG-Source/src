#ifndef _VCHAT_CLIENT_H_
#define _VCHAT_CLIENT_H_

#include "VChatAPI/VChatAPI.h"

using namespace VChatSystem;

class VChatClient : public VChatAPI
{
public:
	VChatClient(const char * hostList);
	virtual ~VChatClient(void);

	bool IsConnected() const { return m_isConnected; }
	bool HasFailed() const { return m_failed; }
	bool IsDone() const { return m_isDone; }
	void Reset() { m_isDone = false; m_failed = false; }

	inline virtual void OnConnectionOpened( const char * address )
	{
		m_isConnected = true;
		printf("Connection opened to %s\n", address);
	}

	inline virtual void OnConnectionFailed( const char * address )
	{
		m_failed = true;
		m_isConnected = false;
		printf("Connection failed to %s\n", address);
	}

	inline virtual void OnConnectionClosed( const char * address, const char * reason )
	{
		m_isConnected = false;
		printf("Connection closed to %s because %s\n", address, reason);
	}

	virtual void OnGetAccount(unsigned track, 
						unsigned result, 
						unsigned userID,
						unsigned accountID, 
						const std::string &voicePassword,
						const std::string &encodedVoiceAccount,
						const std::string &URI,
						void *user);

	virtual void OnGetChannel(unsigned track, 
							  unsigned result, 
							  const std::string &channelName, 
							  const std::string &channelURI, 
							  unsigned channelID,
							  void *user);

	virtual void OnGetProximityChannel( unsigned track, 
										unsigned result, 
										const std::string &channelName, 
										const std::string &channelURI, 
										unsigned channelID,
										void *user );

	virtual void OnChannelCommand(unsigned track, 
								  unsigned result, 
								  void *user);

	virtual void OnDeactivateVoiceAccount(unsigned track, 
										  unsigned result, 
										  void *user);

	virtual void OnChangePassword(unsigned track, 
								 unsigned result, 
								 void *user);

	virtual void OnGetAllChannels(unsigned track, 
								  unsigned result, 
								  const ChannelVec_t & channels,
								  void *user);

	virtual void OnDeleteChannel(unsigned track, 
								 unsigned result, 
								 void *user);

	virtual void OnSetBanStatus(unsigned track, 
								unsigned result, 
								void *user);

	virtual void OnGetChannelV2(unsigned track, 
							unsigned result, 
							const std::string &channelName, 
							const std::string &channelURI, 
							unsigned channelID,
							unsigned isNewChannel,
							void *user);


	/////////////////////////////////////////////////////////////////////////////


	unsigned GetAccountEx(const std::string &avatarName, 
					const std::string &game, 
					const std::string &world, 
					unsigned userID,
					unsigned avatarID);

	unsigned DeactivateVoiceAccount(const std::string &avatarName, 
		const std::string &game, 
		const std::string &world);


	unsigned GetChannelEx(  const std::string &channelName, 
							const std::string &game, 
							const std::string &server, 
							const std::string &description, 
							const std::string &password,
							unsigned limit,
							bool persistent);

	unsigned GetProximityChannelEx( const std::string &channelName, 
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
									unsigned distModel);

	unsigned ChannelCommandEx(const std::string &srcUserName, 
							const std::string &destUserName, 
							const std::string &destChannelAddress, 
							unsigned command,
							unsigned banTimeout);

	
	unsigned ChangePasswordEx(const std::string &channelName, 
							  const std::string &game, 
							  const std::string &server, 
							  const std::string &password);

	unsigned GetAllChannelsEx();

	unsigned DeleteChannelEx( const std::string &channelName, 
							const std::string &game, 
							const std::string &server);

	unsigned SetBanStatusEx(unsigned userID,
							unsigned banStatus);


	virtual void OnSetUserData( unsigned track, 
		unsigned result, 
		void *user)
	{
		//XXX need to implement
	}

	unsigned GetChannelInfoEx(const std::string &channelName, 
							  const std::string &game, 
							  const std::string &server);


	virtual void OnGetChannelInfo(unsigned track, 
								  unsigned result, 
								  const std::string &channelName, 
								  const std::string &channelURI, 
								  unsigned channelID,
								  void *user);


	virtual unsigned GetChannelV2Ex(  const std::string &channelName, 
									const std::string &game, 
									const std::string &server, 
									const std::string &description, 
									const std::string &password,
									unsigned limit,
									bool persistent);

	unsigned AddCharacterChannelEx( const unsigned stationID,
									const unsigned avatarID,
									const std::string &characterName,
									const std::string &worldName,
									const std::string &gameCode,
									const std::string &channelType,
									const std::string &channelDescription,
									const std::string &password,
									const std::string &channelAddress,
									const std::string &locale );

	virtual void OnAddCharacterChannel(unsigned track, 
										unsigned result, 
										void *user);

	unsigned RemoveCharacterChannelEx(  const unsigned stationID,
										const unsigned avatarID,
										const std::string &characterName,
										const std::string &worldName,
										const std::string &gameCode,
										const std::string &channelType);

	virtual void OnRemoveCharacterChannel(unsigned track, 
											unsigned result, 
											void *user);

	unsigned GetCharacterChannelEx(const unsigned stationID,
									const std::string &characterName,
									const std::string &worldName,
									const std::string &gameCode);

	virtual void OnGetCharacterChannel(unsigned track,
										unsigned result,
										const CharacterChannelVec_t &characterChannels,
										void * user);

	
	unsigned m_result;
	unsigned m_userID;
	unsigned m_avatarID;
	unsigned m_channelID;
	std::string m_channelURI;
	unsigned m_track;
	std::string m_previousUser;

private:
	void PrintChannel(const CharacterChannel &characterChannel);
	bool m_isDone;
	bool m_failed;
	bool m_isConnected;

};

#endif


