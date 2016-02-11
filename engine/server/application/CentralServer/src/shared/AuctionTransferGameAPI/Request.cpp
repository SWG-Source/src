#include "Request.h"

//////////////////////////////////////////////////////////////////////////////////////
namespace AuctionTransfer
{
void put(Base::ByteStream &msg, const Blob &source);

//////////////////////////////////////////////////////////////////////////////////////
	Blob::Blob(const unsigned char *data, unsigned len)
		: m_data(nullptr),
		m_len(len)      
	{
		if (m_len > 0)
		{
			m_data = new unsigned char [m_len];
			memcpy(m_data, data, m_len);
		}
	}


//////////////////////////////////////////////////////////////////////////////////////
	Blob::Blob(const Blob &cpy)
		: m_data(nullptr),
		m_len(cpy.m_len)
	{
		if (m_len > 0)
		{
			m_data = new unsigned char [m_len];
			memcpy(m_data, cpy.m_data, m_len);
		}
	}

//////////////////////////////////////////////////////////////////////////////////////
	Blob::~Blob()
	{
		delete [] m_data;
	}

//////////////////////////////////////////////////////////////////////////////////////
	Blob & Blob::operator =(const Blob &cpy)
	{
		if (this != &cpy)
		{
			//copy 
			if (m_data)
			{
				delete [] m_data;
				m_data = nullptr;
			}

			m_len = cpy.m_len;

			if (m_len > 0)
			{
				m_data = new unsigned char [m_len];
				memcpy(m_data, cpy.m_data, m_len);
			}
		}
		return *this;
	}
//////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////
ReplyRequest::ReplyRequest( RequestTypes type, unsigned serverTrack, unsigned responseCode )
	: GenericRequest((short)type, serverTrack), m_responseCode(responseCode)
{
}

void ReplyRequest::pack(Base::ByteStream &msg)
{
	put(msg, m_type);
	put(msg, m_track);
	put(msg, m_server_track);
	put(msg, m_responseCode);
}

//////////////////////////////////////////////////////////////////////////////////////
CommonRequest::CommonRequest( RequestTypes type, unsigned serverTrack, long long transactionID )
	: GenericRequest((short)type, serverTrack), m_transactionID(transactionID)
{
}

void CommonRequest::pack(Base::ByteStream &msg)
{
	put(msg, m_type);
	put(msg, m_track);
	put(msg, m_transactionID);
}

//////////////////////////////////////////////////////////////////////////////////////
GetIDRequest::GetIDRequest( RequestTypes type, unsigned serverTrack )
	: GenericRequest((short)type, serverTrack)
{
}

void GetIDRequest::pack(Base::ByteStream &msg)
{
	put(msg, m_type);
	put(msg, m_track);
}

//////////////////////////////////////////////////////////////////////////////////////
SendPrepareCompressedRequest::SendPrepareCompressedRequest(RequestTypes type, unsigned serverTrack, const char *serverID, long long transactionID, unsigned stationID, unsigned characterID, long long assetID, const unsigned char *xmlAsset, unsigned length)
  : GenericRequest((short)type, serverTrack),  
	m_transactionID(transactionID), 
	m_stationID(stationID),
	m_characterID(characterID),
	m_assetID(assetID),
	m_serverID(serverID),
	m_data(xmlAsset, length)
{
}

void SendPrepareCompressedRequest::pack(Base::ByteStream &msg)
{
	put(msg, m_type);
	put(msg, m_track);
	put(msg, m_serverID);
	put(msg, m_transactionID);
	put(msg, m_stationID);
	put(msg, m_characterID);
	put(msg, m_assetID);
	put(msg, m_data);
}

//////////////////////////////////////////////////////////////////////////////////////
SendPrepareRequest::SendPrepareRequest( RequestTypes type, unsigned serverTrack, const char *serverID, long long transactionID, unsigned stationID, unsigned characterID, long long assetID, const char *xml )
	: GenericRequest((short)type, serverTrack),  
	  m_transactionID(transactionID), 
	  m_stationID(stationID),
	  m_characterID(characterID),
	  m_assetID(assetID),
	  m_xml(xml), 
	  m_serverID(serverID)
{
}

void SendPrepareRequest::pack(Base::ByteStream &msg)
{
	put(msg, m_type);
	put(msg, m_track);
	put(msg, m_serverID);
	put(msg, m_transactionID);
	put(msg, m_stationID);
	put(msg, m_characterID);
	put(msg, m_assetID);
	put(msg, m_xml);
}

//////////////////////////////////////////////////////////////////////////////////////
ReplyGetCharacterListRequest::ReplyGetCharacterListRequest(RequestTypes type, unsigned serverTrack, unsigned responseCode, const Character characters[], unsigned numCharacters)
: GenericRequest((short)type, serverTrack), m_responseCode(responseCode)
{
	for(unsigned i = 0; i < numCharacters; i++)
	{
		m_characters.push_back(characters[i]);
	}
}

void ReplyGetCharacterListRequest::pack(Base::ByteStream &msg)
{
	put(msg, m_type);
	put(msg, m_track);
	put(msg, m_server_track);
	put(msg, m_responseCode);
	put(msg, (unsigned)m_characters.size());
	for(unsigned i = 0; i < m_characters.size(); i++)
	{
		put(msg, m_characters[i]);
	}
}

//////////////////////////////////////////////////////////////////////////////////////
IdentifyServerRequest::IdentifyServerRequest(RequestTypes type, unsigned serverTrack, const char *serverID[], unsigned numIDs)
: GenericRequest((short)type, serverTrack)
{
	for(unsigned i = 0; i < numIDs; i++)
	{
		m_serverIDs.push_back(serverID[i]);
	}
}

void IdentifyServerRequest::pack(Base::ByteStream &msg)
{
	put(msg, m_type);
	put(msg, m_track);
	put(msg, (unsigned)m_serverIDs.size());
	for(unsigned i = 0; i < m_serverIDs.size(); i++)
	{
		put(msg, m_serverIDs[i]);
	}
}

//////////////////////////////////////////////////////////////////////////////////////
SendAuditRequest::SendAuditRequest(	RequestTypes type, unsigned serverTrack, const char *gameCode, const char *serverCode, 
					long long inGameAssetID, unsigned stationID, const char *event, const char *message)
: GenericRequest((short)type, serverTrack), m_gameCode(gameCode), m_serverCode(serverCode), m_assetID(inGameAssetID),
	m_userID(stationID), m_event(event), m_message(message)
{
}

void SendAuditRequest::pack(Base::ByteStream &msg)
{
	put(msg, m_type);
	put(msg, m_track);
	put(msg, m_gameCode);
	put(msg, m_serverCode);
	put(msg, m_assetID);
	put(msg, m_userID);
	put(msg, m_event);
	put(msg, m_message);
}

void put(Base::ByteStream &msg, const Blob &source)
{
	put(msg, source.getLen());

	if (source.getLen() > 0)
	{
		put(msg, source.getData(), source.getLen());
	}
}


}; // namespace
//////////////////////////////////////////////////////////////////////////////////////

