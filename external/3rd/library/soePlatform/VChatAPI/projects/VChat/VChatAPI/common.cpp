#include "common.h"
#include "Base/Base64.h"
#include "Base/Base32.h"
#include <algorithm>
#include "Base/stringutils.h"

using namespace Base;

namespace VChatSystem
{	

	unsigned Channel::Write(unsigned char * stream, unsigned size) const
	{
		unsigned bytesTotal = 0;
		unsigned bytes = 0;

		bytesTotal += (bytes = soe::Write(stream+bytesTotal, size-bytesTotal, m_channelName));
		if (!bytes) return 0;

		bytesTotal += (bytes = soe::Write(stream+bytesTotal, size-bytesTotal, m_channelURI));
		if (!bytes) return 0;

		bytesTotal += (bytes = soe::Write(stream+bytesTotal, size-bytesTotal, m_channelPassword));
		if (!bytes) return 0;

		bytesTotal += (bytes = soe::Write(stream+bytesTotal, size-bytesTotal, m_channelID));
		if (!bytes) return 0;

		bytesTotal += (bytes = soe::Write(stream+bytesTotal, size-bytesTotal, m_channelType));
		if (!bytes) return 0;

		return bytesTotal;
	}

	unsigned Channel::Read(const unsigned char * stream, unsigned size)
	{
		unsigned bytesTotal = 0;
		unsigned bytes = 0;

		bytesTotal += (bytes = soe::Read(stream+bytesTotal, size-bytesTotal, m_channelName, MAX_CHANNEL_LEN));
		if (!bytes) return 0;

		bytesTotal += (bytes = soe::Read(stream+bytesTotal, size-bytesTotal, m_channelURI, MAX_CHANNEL_LEN));
		if (!bytes) return 0;

		bytesTotal += (bytes = soe::Read(stream+bytesTotal, size-bytesTotal, m_channelPassword, MAX_PASSWORD_LEN));
		if (!bytes) return 0;

		bytesTotal += (bytes = soe::Read(stream+bytesTotal, size-bytesTotal, m_channelID));
		if (!bytes) return 0;

		bytesTotal += (bytes = soe::Read(stream+bytesTotal, size-bytesTotal, m_channelType));
		if (!bytes) return 0;

		return bytesTotal;
	}	

	unsigned CharacterChannel::Write(unsigned char * stream, unsigned size) const
	{
		unsigned bytesTotal = 0;
		unsigned bytes = 0;

		bytesTotal += (bytes = soe::Write(stream+bytesTotal, size-bytesTotal, m_stationID));
		if (!bytes) return 0;

		bytesTotal += (bytes = soe::Write(stream+bytesTotal, size-bytesTotal, m_avatarID));
		if (!bytes) return 0;

		bytesTotal += (bytes = soe::Write(stream+bytesTotal, size-bytesTotal, m_characterName));
		if (!bytes) return 0;

		bytesTotal += (bytes = soe::Write(stream+bytesTotal, size-bytesTotal, m_worldName));
		if (!bytes) return 0;

		bytesTotal += (bytes = soe::Write(stream+bytesTotal, size-bytesTotal, m_gameCode));
		if (!bytes) return 0;

		bytesTotal += (bytes = soe::Write(stream+bytesTotal, size-bytesTotal, m_channelType));
		if (!bytes) return 0;

		bytesTotal += (bytes = soe::Write(stream+bytesTotal, size-bytesTotal, m_channelDescription));
		if (!bytes) return 0;

		bytesTotal += (bytes = soe::Write(stream+bytesTotal, size-bytesTotal, m_channelAddress));
		if (!bytes) return 0;

		bytesTotal += (bytes = soe::Write(stream+bytesTotal, size-bytesTotal, m_locale));
		if (!bytes) return 0;

		bytesTotal += (bytes = soe::Write(stream+bytesTotal, size-bytesTotal, m_password));
		if (!bytes) return 0;

		return bytesTotal;
	}
	unsigned CharacterChannel::Read(const unsigned char * stream, unsigned size)
	{
		unsigned bytesTotal = 0;
		unsigned bytes = 0;

		bytesTotal += (bytes = soe::Read(stream+bytesTotal, size-bytesTotal, m_stationID));
		if (!bytes) return 0;

		bytesTotal += (bytes = soe::Read(stream+bytesTotal, size-bytesTotal, m_avatarID));
		if (!bytes) return 0;

		bytesTotal += (bytes = soe::Read(stream+bytesTotal, size-bytesTotal, m_characterName, MAX_FULL_NAME_LEN));
		if (!bytes) return 0;

		bytesTotal += (bytes = soe::Read(stream+bytesTotal, size-bytesTotal, m_worldName, MAX_WORLD_LEN));
		if (!bytes) return 0;

		bytesTotal += (bytes = soe::Read(stream+bytesTotal, size-bytesTotal, m_gameCode, MAX_GAME_LEN));
		if (!bytes) return 0;

		bytesTotal += (bytes = soe::Read(stream+bytesTotal, size-bytesTotal, m_channelType, MAX_FULL_NAME_LEN));
		if (!bytes) return 0;

		bytesTotal += (bytes = soe::Read(stream+bytesTotal, size-bytesTotal, m_channelDescription, MAX_DESCRIPTION_LEN));
		if (!bytes) return 0;

		bytesTotal += (bytes = soe::Read(stream+bytesTotal, size-bytesTotal, m_channelAddress, MAX_CHANNEL_LEN));
		if (!bytes) return 0;

		bytesTotal += (bytes = soe::Read(stream+bytesTotal, size-bytesTotal, m_locale, MAX_FULL_NAME_LEN));
		if (!bytes) return 0;
		
		bytesTotal += (bytes = soe::Read(stream+bytesTotal, size-bytesTotal, m_password, MAX_PASSWORD_LEN));
		if (!bytes) return 0;

		return bytesTotal;
	}

	
	const char * VIV0X_DOT_COM = ".vivox.com";

	void CreateUserURI(const std::string & avatarName, const std::string & game, const std::string & server, std::string & outUserURI)
	{
		outUserURI = server;
		outUserURI += ".";
		outUserURI += avatarName;
		outUserURI += "@";
		outUserURI += game;
		outUserURI += VIV0X_DOT_COM;
	}

	void CreateUserName(const std::string & avatarName, const std::string & server, std::string & outUserName)
	{
		outUserName = server;
		outUserName += ".";
		outUserName += avatarName;
	}


	void CreateChannelURI(const std::string & channelName, const std::string & game, const std::string & server, std::string & outChannelURI)
	{
		outChannelURI = game;
		outChannelURI += ".";
		outChannelURI += server;
		outChannelURI += ".";
		outChannelURI += channelName;
	}

	void GetChannelComponents(const std::string & channelURI, std::string & name, std::string & server, std::string & game)
	{
		char seps[]   = ".\n";
		char *token;

		// this will muck with the actual string so make a copy
		// not very efficient but who cares...
		std::string tmpTokenee = channelURI;
		token = strtok((char*)tmpTokenee.c_str(), seps );


		if( token != nullptr )
		{
			game = token;

			/* Get next token: */
			token = strtok( nullptr, seps );
		}

		if( token != nullptr )
		{
			server = token;

			/* Get next token: */
			token = strtok( nullptr, seps );
		}

		if( token != nullptr )
		{
			name = token;

			/* Get next token: */
			token = strtok( nullptr, seps );
		}

		while ( token != nullptr )
		{
			name += ".";
			name += token;

			/* Get next token: */
			token = strtok( nullptr, seps );
		}
	}

	void GetUserNameComponents(const std::string & userName, std::string & server, std::string & name)
	{

		char seps[]   = ".\n";
		char *token;

		// this will muck with the actual string so make a copy
		// not very efficient but who cares...
		std::string tmpTokenee = userName;
		token = strtok((char*)tmpTokenee.c_str(), seps );

		if( token != nullptr )
		{
			server = token;

			/* Get next token: */
			token = strtok( nullptr, seps );
		}

		if( token != nullptr )
		{
			name = token;

			/* Get next token: */
			token = strtok( nullptr, seps );
		}

		while ( token != nullptr )
		{
			name += ".";
			name += token;

			/* Get next token: */
			token = strtok( nullptr, seps );
		}
	}


	std::string HexEncodeString(const std::string & input)
	{
		std::string retString;
		soe::Base64::HexEncode((unsigned char *)input.c_str(), (unsigned)input.length(), retString, true);
		return retString;
	}

	void EncodeVivoxString(const std::string & input, std::string & output)
	{
		std::string tmpInput = soe::tolowercase(input);
		
		const unsigned outLen = Base32::GetEncode32Length((int)tmpInput.length());

		unsigned char * outBuffer = new unsigned char[outLen + 1];

		if(Base32::Encode32((unsigned char*)tmpInput.c_str(), (int)tmpInput.length(), outBuffer))
		{
			Base32::Map32(outBuffer, outLen);

			outBuffer[outLen] = 0;
			output = (char*)outBuffer;
		}


		delete [] outBuffer;
	}

	void DecodeVivoxString(const std::string & input, std::string & output)
	{
		std::string tmpInput = soe::touppercase(input);
		unsigned char tmpBuffer[256];

		int inputLen = (int)tmpInput.length();
		memset(tmpBuffer, 0, 256);
		memcpy(tmpBuffer, tmpInput.c_str(), inputLen);

		const unsigned outLen = Base32::GetDecode32Length((int)inputLen);

		unsigned char * outBuffer = new unsigned char[outLen + 1];
		
		Base32::Unmap32(tmpBuffer, inputLen);

		if(Base32::Decode32(tmpBuffer, inputLen, outBuffer))
		{		
			outBuffer[outLen] = 0;
			output = (char*)outBuffer;
		}

		delete [] outBuffer;
	}
	

	void GetEncodedUserName(const std::string & name, const std::string & server, std::string & encodedName)
	{
		std::string tmpName;
		EncodeVivoxString(name, tmpName);
		CreateUserName(tmpName, server, encodedName);
	}

	void GetDecodedUserName(const std::string & username, std::string & decodedName)
	{
		std::string name, server, decoded;
		GetUserNameComponents(username, server, name);
		DecodeVivoxString(name, decoded);
		CreateUserName(decoded, server, decodedName);
	}

	void GetEncodedChannelName(const std::string & name, const std::string & server, const std::string & game, std::string & encodedName)
	{
		std::string tmpName;
		EncodeVivoxString(name, tmpName);
		CreateChannelURI(tmpName, game, server, encodedName);
	}

	void GetDecodedChannelName(const std::string & channelName, std::string & decodedChannelName)
	{
		std::string name, server, game, decoded;
		GetChannelComponents(channelName, name, server, game);
		DecodeVivoxString(name, decoded);
		CreateChannelURI(decoded, game, server, decodedChannelName);
	}
};


