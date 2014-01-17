#ifndef _VCHAT_SERVER_COMMON_H_
#define _VCHAT_SERVER_COMMON_H_

#include <string>
#include <vector>
#ifndef SOE__DATE_H
 #include <Base/date.h>
#endif

namespace VChatSystem
{
    // Add common enumerated types or constants here

    typedef unsigned char HashValue_t;
    
    const unsigned MAX_PARAM_LEN = 6400;
    const unsigned MAX_STUFF_LEN = 64000;
	const unsigned MAX_NAME_PAIR_LEN = 256;
	const unsigned MAX_VALUE_PAIR_LEN = 256;
	const unsigned MAX_AVATAR_LEN = 50;
	const unsigned MAX_CHANNEL_LEN = 255;
	const unsigned MAX_DESCRIPTION_LEN = 256;
	const unsigned MAX_FULL_NAME_LEN = 1024;
	const unsigned MAX_GAME_LEN = 255;
	const unsigned MAX_PASSWORD_LEN = 30;
	const unsigned MAX_CHAT_AVATAR_LEN = 255;
	const unsigned MAX_WORLD_LEN = 255;
	const unsigned MAX_EMAIL_LEN = 255;
	const unsigned MAX_PHONE_NUMBER_LEN = 255;
	const unsigned MAX_PIN_LEN = 255;
	const unsigned MAX_SMS_ACCT_LEN = 255;
	const unsigned MAX_CHANNEL_ID_LEN = 100;
	const unsigned MAX_URI_LEN = 255;
	const unsigned VIVOX_NAME_LIMIT = 63;


	struct Channel
	{
		Channel(): m_channelID(0) , m_channelType(0) {}
		~Channel() {}

		std::string m_channelName;
		std::string m_channelURI;
		std::string m_channelPassword;
		unsigned	m_channelID;
		unsigned	m_channelType;

		unsigned Write(unsigned char * stream, unsigned size) const;		
		unsigned Read(const unsigned char * stream, unsigned size);		
	};

	typedef std::vector<Channel> ChannelVec_t;


	struct CharacterChannel
	{
		CharacterChannel(): m_stationID(0), m_avatarID(0){}
		~CharacterChannel() {}

		unsigned	m_stationID;
		unsigned	m_avatarID;
		std::string m_characterName;
		std::string m_worldName;
		std::string m_gameCode;
		std::string m_channelType;
		std::string m_channelDescription;
		std::string m_channelAddress;
		std::string m_locale;
		std::string m_password;

		unsigned Write(unsigned char * stream, unsigned size) const;		
		unsigned Read(const unsigned char * stream, unsigned size);		

	};

	typedef std::vector<CharacterChannel> CharacterChannelVec_t;

	// given avatarName, game and server returns outUserURI in the form of server.avatarName@game.vivox.com
	void CreateUserURI(const std::string & avatarName, const std::string & game, const std::string & server, std::string & outUserURI);

	// given avatarName and server returns outUserName in the form of server.avatarName 
	void CreateUserName(const std::string & avatarName, const std::string & server, std::string & outUserName);

	// given channelName, game and server returns outChannelURI in the form of game.server.channelName
	void CreateChannelURI(const std::string & channelName, const std::string & game, const std::string & server, std::string & outChannelURI);

	// given input of game.server.channel will break into seperate strings
	void GetChannelComponents(const std::string & channelURI, std::string & name, std::string & server, std::string & game);

	// given input of server.userName will break into seperate strings
	void GetUserNameComponents(const std::string & userName, std::string & server, std::string & name);

	std::string HexEncodeString(const std::string & input);

	// encodes the string using a modified base64 encoding.
	void EncodeVivoxString(const std::string & input, std::string & output);

	// reverses a string encoded with EncodeVivoxString
	void DecodeVivoxString(const std::string & input, std::string & output);

	// given a name and server will return encodedName in the form of game.server.EncodedName
	void GetEncodedUserName(const std::string & name, const std::string & server, std::string & encodedName);

	// given an input of server.EncodedName will return server.DecodedName
	void GetDecodedUserName(const std::string & username, std::string & decodedName);

	// given a name and server will return encodedName in the form of server.EncodedName
	void GetEncodedChannelName(const std::string & name, const std::string & server, const std::string & game, std::string & encodedName);

	// given an input of game.server.EncodedName will return game.server.DecodedName
	void GetDecodedChannelName(const std::string & channelName, std::string & decodedChannelName);
}

#endif //_VCHAT_SERVER_COMMON_H_
