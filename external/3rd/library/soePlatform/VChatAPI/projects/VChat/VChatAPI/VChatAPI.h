#ifndef _VCHAT_SERVER_API_H_
#define _VCHAT_SERVER_API_H_

#include <vector>
#include "VChatAPITypes.h"
//#include "CardinalServerUtils.h"


namespace VChatSystem
{
	// forward declarations
	class VChatAPICore;

	static const unsigned DEFAULT_VCHAT_API_TIMEOUT = 25;

	class VChatAPI
	{
		protected:
			VChatAPICore * mCore;

		protected:
			unsigned mConnectionType;
		public:
			VChatAPI( const char * hostList, unsigned connectionType = CONNECTION_TYPE_VHCAT_SYSTEM,const char * failoverHostList = 0, unsigned connectionLimit = 0, unsigned maxMsgSize = 1*1024, unsigned bufferSize = 64*1024 );
			virtual ~VChatAPI();

			void		 Process();
			unsigned	 ConnectionCount() const;

			////////////////////////////////////////
			//  Connection Callbacks
			virtual void OnConnectionOpened( const char * address ) = 0;
			virtual void OnConnectionFailed( const char * address ) = 0;
			virtual void OnConnectionClosed( const char * address, const char * reason ) = 0;
			virtual void OnConnectionShutdownNotified( const char * address, unsigned outstandingRequests ) { }

			////////////////////////////////////////
			

			//////////////////////////////////////////
			// GetAccount - will get a players login credentials.  If none exist
			// it will create the player with Vivox.
			//  avatarName - Name of the avatar to login.
			//	game – the game code of the calling game
			//	server – the world, server or some other identifier for the area of the game that the character is tied to.
			//
			//	The name will parsed and transformed into:
			//        “<WorldName>.<avatarName>@<GameCode>.vivox.com”
			//
			//	userID – station ID of the player
			//	avatarID – the ID of the chat avatar, or some other character identifier. 
			// 
			unsigned GetAccount(const std::string &avatarName, 
						        const std::string &game, 
								const std::string &server, 
								unsigned userID,
								unsigned avatarID,
								void *user,
								unsigned timeout = DEFAULT_VCHAT_API_TIMEOUT);

			///////////////////////////////////////////////
			// OnGetAccount
			// 
			//  userID – station ID of the player.
			//  accountID – the Vivox account ID.  
			//	voicePassword – the password for this user’s Vivox account.
			//  encodedVoiceAccount - the account name encoded with modified 
			//  base64 algorithm.  This should be used to login.
			//  URI - the url that this server is connected to.
			virtual void OnGetAccount(unsigned track, 
									  unsigned result, 
									  unsigned userID,
									  unsigned accountID, 
									  const std::string &voicePassword,
									  const std::string &encodedVoiceAccount,
									  const std::string &URI,
									  void *user) = 0;
			
			//////////////////////////////////////////
			// channelName - Name of the channel to retrieve.  See below for examples.
			//	game – the game code of the calling game
			//	server – the world, server or some other identifier for the area of the game that the channel is tied to.
			//
			//	The name will parsed and transformed into:
			//  group (private)		- <game>.<server>.group.<id>  channelName would be: group.<id>
			//	raid (private)		- <game>.<server>.raid.<id>  channelName would be: raid.<id>
			//	raid leader (optional, private) - <game>.<server>.raidleader.<id>  channelName would be: raidleader.<id>
			//	guild (private, persistent)	- <game>.<server>.guild.<name> channelName would be: guild.<name>
			//	guild leader (private, persistent)	- <game>.<server>.guildleader.<name>  channelName would be: guildleader.<name>
			//	tell (private)		- game.server.characterA_characterB  channelName would be: characterA_characterB 
			//	say (public, proximity)    - game.server.zone  channelName would be: zone
			//	user generated (public/private) - <game>.<server>.user.<channel>  channelName would be: user.<channel>

			//	description – description of the room.
			//	password – password needed to enter the channel.  If the password is empty it will not be passed to Vivox. (optional)
			//	limit - Optional hard limit on the number of users allowed on this channel
			//	persistent – true – persistent channel, false – temporary channel.
			unsigned GetChannel(const std::string &channelName, 
								const std::string &game, 
								const std::string &server, 
								const std::string &description, 
								const std::string &password,
								unsigned limit,
								bool persistent,
								void *user,
								unsigned timeout = DEFAULT_VCHAT_API_TIMEOUT);


			//////////////////////////////////////////
			//  channelName – the channel name.  Form is above.
			//  channelURI - uri returned by Vivox
			//	channelID – Vivox assigned channel id.
			virtual void OnGetChannel(unsigned track, 
									  unsigned result, 
									  const std::string &channelName, 
									  const std::string &channelURI, 
									  unsigned channelID,
									  void *user) = 0;

			///////////////////////////////////////////////
			//  channelName - Name of the channel to retrieve.
			//	game – the game code of the calling game
			//	server – the world, server or some other identifier for the area of the game that the channel is tied to.
			//	description – description of the room.
			//	password – password needed to enter the channel.  If the password is empty it will not be passed to Vivox. (optional)
			//	limit - Optional hard limit on the number of users allowed on this channel
			//	persistent – true – persistent channel, false – temporary channel.
			//	maxRange - Also known as the audible threshold distance. This is the distance beyond which a participant is considered 'out of range'. When participants cross this threshold distance from a particular listening position in a channel, a roster list updates for the channel occur, which indi-cate an entry being added (or removed, as the case may be) from the participants list. No audio is received by the listener for participants beyond this range. (The default value 80).
			//	clamping  - This is the distance from the listener below which the 'gain roll-off' effects for a given audio roll-off model (see below) are not applied. In effect, it is the 'audio plateau' distance (in the sense that the gain is constant up this distance, and then falls off). (The default value is 10)
			//	rollOff - This value indicates how sharp the audio attenuation will 'roll-off' between the clamping and maximum distances. Larger values will result in steeper roll-off. The extent of roll-off will depend on the distance model chosen. (Default value is 2.0).
			//	maxGain - The gain setting for all speakers in this channel. Note, that in practice this should not be raised above 2.5 or auditory distress may result.  (The default value is 1.0).
			//	distModel =<0 | 1| 2 | 3> - There are four possible values as described below:
			//(The default model is Inverse Distance Clamped.)
			//	•	None: No distance based attenuation is applied. All speakers are rendered as if they were in the same position as the listener. The audio from speakers will drop to 0 abruptly at the maximum distance.
			//	•	(1) Inverse Distance Clamped: The attenuation increases in inverse proportion to the distance. The roll-off factor n is the inverse of the slope of the attenuation curve.
			//	•	(2) Linear Distance Clamped: The attenuation increases in linear proportion to the distance. The roll-off factor is the negative slope of the attenuation curve.
			//	•	(3) Exponent Distance Clamped: The attenuation increases in inverse proportion to the distance raised to the power of the roll-off factor.

			unsigned GetProximityChannel(const std::string &channelName, 
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
										unsigned timeout = DEFAULT_VCHAT_API_TIMEOUT);

			///////////////////////////////////////////////
			//  channelName – the channel name.  
			//  channelURI - uri returned by Vivox
            //	channelID – Vivox assigned channel id.
			virtual void OnGetProximityChannel (unsigned track, 
												unsigned result, 
												const std::string &channelName, 
												const std::string &channelURI, 
												unsigned channelID,
												void *user)  = 0;


			//////////////////////////////////////////
			//  This will be used to operate on existing active channels and to provide moderators and ‘owner’ the ability to mute, kick, ban users along with other commands.
			//	srcUserName – the owner and/or moderator of this channel with the ability to perform this command.
			//	destUserName – the user on whom to perform the operation
			//		Both user names should be of the form:
			//        “<WorldName>.<avatarName>”
			//	destChannelAddress – Full address of the channel where the operation will be performed.
			//	command – The commands below will be placed in an enum that will be distributed with the API.
			//	•	mute - mute user
			//	•	unmute - unmute user
			//	•	kick - drop user from the channel, reentry only after 60 seconds
			//	•	hangup – drop user from the channel without banning them
			//	•	invite - add user to the channel
			//	•	ban - ban user, permanently from the channel
			//	•	unban - unban user from the channel
			//	•	chan_lock - lock the channel for any additional participants
			//	•	chan_unlock - unlock the channel for any additional participants
			//	•	mute_all – mute all callers except the requestor
			//	•	unmute_all – unmute all callers
			//  banTimeout - if command is ban, timeout for ban in seconds.
			 unsigned ChannelCommand(const std::string &srcUserName, 
									const std::string &destUserName, 
									const std::string &destChannelAddress, 
									unsigned command,
									unsigned banTimeout,
									void *user,
									unsigned timeout = DEFAULT_VCHAT_API_TIMEOUT);

			virtual void OnChannelCommand(unsigned track, 
										  unsigned result, 
										  void *user) = 0;


			///////////////////////////////////////////////
			// DeactivateVoiceAccount
			// In the event that a character is moved, transferred or 
			// banned the account needs to be de-activated.  A new voice 
			// account will be created for the moved/transferred user when 
			// they log in with their new character.
        	unsigned DeactivateVoiceAccount(const std::string &avatarName, 
											const std::string &game, 
											const std::string &server, 
											void *user,
											unsigned timeout = DEFAULT_VCHAT_API_TIMEOUT);

			virtual void OnDeactivateVoiceAccount(unsigned track, 
												  unsigned result, 
												  void *user) = 0;


			///////////////////////////////////////////////
			// 	ChangePassword
			// 	This will reset the password for the channel.  
			//  NOTE:  If a blank password is passed in this will
			//  be sent to the Vivox API.
			//
			// 	channelName - Name of the channel to retrieve.
			//	game – the game code of the calling game
			//	server – the world, server or some other identifier for the area of the game that the channel is tied to.
			//	password – password needed to enter the channel.  This will modify the password for the channel

			unsigned ChangePassword(const std::string &channelName, 
									const std::string &game, 
									const std::string &server, 
									const std::string &password,
									void *user,
									unsigned timeout = DEFAULT_VCHAT_API_TIMEOUT);
				
			///////////////////////////////////////////////
			// OnChangePassword
			virtual void OnChangePassword(unsigned track, 
										 unsigned result, 
										 void *user) = 0;

			////////////////////////////////////////////////////////////
			// 	GetAllChannels
			//  Will get all channels from the Vivox server.
			//
			unsigned GetAllChannels(void *user,
									unsigned timeout = DEFAULT_VCHAT_API_TIMEOUT);

			///////////////////////////////////////////////
			// OnGetAllChannels
			// 
			// channels - a vector of channels that Vivox returned to the server.
			// 
			//  from common.h:
			// 	struct Channel
			// 	{
			// 		std::string m_channelName;
			// 		std::string m_channelURI;
			// 		std::string m_channelPassword;
			// 		unsigned	m_channelID;
			// 		unsigned	m_channelType;
			// 	};
			//
			// Channel Type can be the following:
			// 	typedef enum ChannelType
			// 	{
			// 		CHAN_TYPE_UNKNOWN = 0,
			// 		CHAN_TYPE_DIR,
			// 		CHAN_TYPE_CHANNEL,
			// 		CHAN_TYPE_PROXIMITY
			// 	};
			//
			virtual void OnGetAllChannels(unsigned track, 
										  unsigned result, 
										  const ChannelVec_t & channels,
										  void *user) = 0;


			///////////////////////////////////////////////
			// DeleteChannel
			// 
			// Removes the channel from the Vivox server.
			//
			//	channelName - Name of the channel to retrieve.
			//	game – the game code of the calling game
			//	server – the world, server or some other identifier for the area of the game that the channel is tied to.
			//
			unsigned DeleteChannel( const std::string &channelName, 
									const std::string &game, 
									const std::string &server, 
									void *user,
									unsigned timeout = DEFAULT_VCHAT_API_TIMEOUT);

			///////////////////////////////////////////////
			// OnDeleteChannel
			virtual void OnDeleteChannel(unsigned track, 
										 unsigned result, 
										 void *user) = 0;


			///////////////////////////////////////////////
			// SetUserData
			// 
			// Sets data at the user level (ie. Station Account) that is valid
			// valid across all characters for that user.
			//
			//  userID - the userID that is tied to the station account
			unsigned SetUserData(unsigned userID,
								 unsigned attributes,
								 const std::string & email, 
								 const std::string & phoneNumber, 
								 const std::string & PIN, 
								 const std::string & smsAccount, 
								 void *user,
								 unsigned timeout = DEFAULT_VCHAT_API_TIMEOUT);

			///////////////////////////////////////////////
			// OnSetUserData
			virtual void OnSetUserData( unsigned track, 
										unsigned result, 
										void *user) = 0;

			///////////////////////////////////////////////
			// SetBanStatus
			// 
			// Sets the ban status at the user level (ie. Station Account) 
			// This primarily for CS to ban users from Voice for all of 
			// their characters.
			//
			//  userID - the userID that is tied to the station account
			//  banStatus - one of the following:
			//		typedef enum BanStatus
			//		{
			//			BAN = 0,
			//			UNBAN
			//		};
			//
			unsigned SetBanStatus(unsigned userID,
								  unsigned banStatus, 
								  void *user,
								  unsigned timeout = DEFAULT_VCHAT_API_TIMEOUT);

			///////////////////////////////////////////////
			// OnSetBanStatus
			virtual void OnSetBanStatus(unsigned track, 
										unsigned result, 
										void *user) = 0;


			//////////////////////////////////////////
			// channelName - Name of the channel to retrieve.  See below for examples.
			//	game – the game code of the calling game
			//	server – the world, server or some other identifier for the area of the game that the channel is tied to.
			//
			//	The name will parsed and transformed into:
			//  group (private)		- <game>.<server>.group.<id>  channelName would be: group.<id>
			//	raid (private)		- <game>.<server>.raid.<id>  channelName would be: raid.<id>
			//	raid leader (optional, private) - <game>.<server>.raidleader.<id>  channelName would be: raidleader.<id>
			//	guild (private, persistent)	- <game>.<server>.guild.<name> channelName would be: guild.<name>
			//	guild leader (private, persistent)	- <game>.<server>.guildleader.<name>  channelName would be: guildleader.<name>
			//	tell (private)		- game.server.characterA_characterB  channelName would be: characterA_characterB 
			//	say (public, proximity)    - game.server.zone  channelName would be: zone
			//	user generated (public/private) - <game>.<server>.user.<channel>  channelName would be: user.<channel>
			unsigned GetChannelInfo(const std::string &channelName, 
									const std::string &game, 
									const std::string &server, 
									void *user,
									unsigned timeout = DEFAULT_VCHAT_API_TIMEOUT);


			//////////////////////////////////////////
			//  channelName – the channel name.  Form is above.
			//  channelURI - uri returned by Vivox
			//	channelID – Vivox assigned channel id.
			virtual void OnGetChannelInfo(unsigned track, 
										  unsigned result, 
										  const std::string &channelName, 
										  const std::string &channelURI, 
										  unsigned channelID,
										  void *user) = 0;

			//////////////////////////////////////////
			// This is identical to GetChannel except the callback will 
			// let the caller know if the channel was created or an existing
			// channel was retrieved.
			//
			// channelName - Name of the channel to retrieve.  See below for examples.
			//	game – the game code of the calling game
			//	server – the world, server or some other identifier for the area of the game that the channel is tied to.
			//
			//	The name will parsed and transformed into:
			//  group (private)		- <game>.<server>.group.<id>  channelName would be: group.<id>
			//	raid (private)		- <game>.<server>.raid.<id>  channelName would be: raid.<id>
			//	raid leader (optional, private) - <game>.<server>.raidleader.<id>  channelName would be: raidleader.<id>
			//	guild (private, persistent)	- <game>.<server>.guild.<name> channelName would be: guild.<name>
			//	guild leader (private, persistent)	- <game>.<server>.guildleader.<name>  channelName would be: guildleader.<name>
			//	tell (private)		- game.server.characterA_characterB  channelName would be: characterA_characterB 
			//	say (public, proximity)    - game.server.zone  channelName would be: zone
			//	user generated (public/private) - <game>.<server>.user.<channel>  channelName would be: user.<channel>

			//	description – description of the room.
			//	password – password needed to enter the channel.  If the password is empty it will not be passed to Vivox. (optional)
			//	limit - Optional hard limit on the number of users allowed on this channel
			//	persistent – true – persistent channel, false – temporary channel.
			unsigned GetChannelV2(const std::string &channelName, 
				const std::string &game, 
				const std::string &server, 
				const std::string &description, 
				const std::string &password,
				unsigned limit,
				bool persistent,
				void *user,
				unsigned timeout = DEFAULT_VCHAT_API_TIMEOUT);


			//////////////////////////////////////////
			//  channelName – the channel name.  Form is above.
			//  channelURI - uri returned by Vivox
			//	channelID – Vivox assigned channel id.
			//	isNewChannel
			virtual void OnGetChannelV2(unsigned track, 
				unsigned result, 
				const std::string &channelName, 
				const std::string &channelURI, 
				unsigned channelID,
				unsigned isNewChannel,
				void *user) = 0;


			//////////////////////////////////////////
			// Character channels are to be maintained by the game teams.  
			// These will be used to store channels that the customer is allowed to access in game.
			// the descriptions will be localized so that the display will match what the customer sees in game
			// There will be just straight through db access on each of these calls
			// When a character is deleted they should all be removed, when it is renamed these should be updated, etc.
			//	Common params:
			//	stationID		-- the station id associated with the character
			//	avatarID		-- the internal id for the character uint64
			//	characterName	-- utf8 character name
			//	worldName		-- utf8 string for the world name
			//	gameCode		-- utf8 string with the gamecode such as EQ, EQ2, SWG, etc.
			//	channelType		-- (utf8 internal description to identify the channel type (guild, officers, etc.)
			//	channelDescription -- localized description will be displayed to the user from station launcher
			//	channelID		-- unsigned channelID
			//	channelURI		-- URI for the channel with vivox
			//	locale			-- locale for the description such as en_US
			
			//////////////////////////////////////////

			//////////////////////////////////////////
			// AddCharacterChannel
			// 
			// params:
			//	stationID
			//	avatarID
			//	characterName
			//	channelType
			//	channelDescription -- localized
			//	channelID
			//	channelURI
			//	locale
			unsigned AddCharacterChannel(const unsigned stationID,
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
				unsigned timeout = DEFAULT_VCHAT_API_TIMEOUT);
			
			virtual void OnAddCharacterChannel(unsigned track, 
				unsigned result, 
				void *user) = 0;

			//////////////////////////////////////////
			// RemoveCharacterChannel
			// 
			// params:
			//	stationID
			//	avatarID
			//	characterName
			//	worldName
			//	gameCode
			//	channelType
			//	channelDescription -- localized
			//	channelID
			//	channelURI
			//	locale
			unsigned RemoveCharacterChannel(const unsigned stationID,
				const unsigned avatarID,
				const std::string &characterName,
				const std::string &worldName,
				const std::string &gameCode,
				const std::string &channelType,
				void *user,
				unsigned timeout = DEFAULT_VCHAT_API_TIMEOUT);

			virtual void OnRemoveCharacterChannel(unsigned track, 
				unsigned result, 
				void *user) = 0;

			//////////////////////////////////////////
			// GetCharacterChannel
			// 
			// params:
			//	stationID
			//	avatarID
			//	characterName
			//	channelType
			//	channelDescription -- localized
			//	channelID
			//	channelURI
			//	locale
			unsigned GetCharacterChannel(const unsigned stationID,
										 const std::string &characterName,
										 const std::string &worldName,
										 const std::string &gameCode,
										 void *user,
										 unsigned timeout = DEFAULT_VCHAT_API_TIMEOUT);

			virtual void OnGetCharacterChannel(unsigned track,
				unsigned result,
				const CharacterChannelVec_t &characterChannels,
				void * user) = 0;

};

} // end namespace 

#endif // end #ifndef _VCHAT_SERVER_API_H_
