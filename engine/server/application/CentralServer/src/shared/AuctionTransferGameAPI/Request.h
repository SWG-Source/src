#ifndef REQUEST_H
#define REQUEST_H

#include <ATGenericAPI/GenericMessage.h>
#include <Base/Archive.h>
#include "AuctionTransferEnum.h"
#include "Character.h"

//////////////////////////////////////////////////////////////////////////////////////
namespace AuctionTransfer
{

	class Blob
	{
	public:
		Blob() : m_data(0), m_len(0) {}
		Blob(const unsigned char *data, unsigned len);
		Blob(const Blob &cpy);
		~Blob();

		Blob & operator =(const Blob &cpy);

		const unsigned char *getData() const { return m_data; }
		unsigned getLen() const { return m_len; }

		unsigned char *m_data;
		unsigned m_len;
	};

	//////////////////////////////////////////////////////////////////////////////////////
	class ReplyRequest: public GenericRequest
	{
	public:
		ReplyRequest(RequestTypes type, unsigned serverTrack, unsigned responseCode);
		virtual ~ReplyRequest()	{};

		void pack(Base::ByteStream &msg);
	private:
		unsigned m_responseCode;
	};

	//////////////////////////////////////////////////////////////////////////////////////
	class CommonRequest: public GenericRequest
	{
	public:
		CommonRequest(RequestTypes type, unsigned serverTrack, long long transactionID);
		virtual ~CommonRequest()	{};

		void pack(Base::ByteStream &msg);
	private:
		long long m_transactionID;
	};

	//////////////////////////////////////////////////////////////////////////////////////
	class GetIDRequest: public GenericRequest
	{
	public:
		GetIDRequest(RequestTypes type, unsigned serverTrack);
		virtual ~GetIDRequest()	{};

		void pack(Base::ByteStream &msg);
	private:
	};

	//////////////////////////////////////////////////////////////////////////////////////
	class SendPrepareRequest: public GenericRequest
	{
	public:
		SendPrepareRequest(RequestTypes type, unsigned serverTrack, const char *serverID, long long transactionID, unsigned stationID, unsigned characterID, long long assetID, const char *xmlAsset);
		virtual ~SendPrepareRequest()	{};

		void pack(Base::ByteStream &msg);
	private:
		long long m_transactionID;
		unsigned m_stationID;
		unsigned m_characterID;
		long long m_assetID;
		std::string m_xml;
		std::string m_serverID;
	};

	//////////////////////////////////////////////////////////////////////////////////////
	class SendPrepareCompressedRequest: public GenericRequest
	{
	public:
		SendPrepareCompressedRequest(RequestTypes type, unsigned serverTrack, const char *serverID, long long transactionID, unsigned stationID, unsigned characterID, long long assetID, const unsigned char *xmlAsset, unsigned length);
		virtual ~SendPrepareCompressedRequest()	{};

		void pack(Base::ByteStream &msg);
	private:
		long long m_transactionID;
		unsigned m_stationID;
		unsigned m_characterID;
		long long m_assetID;
		//std::string m_xml;
		std::string m_serverID;
		Blob m_data;
	};

	//////////////////////////////////////////////////////////////////////////////////////
	class ReplyGetCharacterListRequest: public GenericRequest
	{
	public:
		ReplyGetCharacterListRequest(RequestTypes type, unsigned serverTrack, unsigned responseCode, const Character characters[], unsigned numCharacters);
		virtual ~ReplyGetCharacterListRequest()	{};

		void pack(Base::ByteStream &msg);
	private:
		unsigned m_responseCode;
		std::vector<Character> m_characters;
	};

	//////////////////////////////////////////////////////////////////////////////////////
	class IdentifyServerRequest: public GenericRequest
	{
	public:
		IdentifyServerRequest(RequestTypes type, unsigned serverTrack,  const char *serverID[], unsigned numIDs);
		virtual ~IdentifyServerRequest()	{};

		void pack(Base::ByteStream &msg);
	private:
		std::vector<std::string> m_serverIDs;
	};

	//////////////////////////////////////////////////////////////////////////////////////
	class SendAuditRequest : public GenericRequest
	{
	public:
		SendAuditRequest(	RequestTypes type, unsigned serverTrack, const char *gameCode, const char *serverCode, 
							long long inGameAssetID, unsigned stationID, const char *event, const char *message);
		~SendAuditRequest() {};
		void pack(Base::ByteStream &msg);
	private:
		std::string m_gameCode;
		std::string m_serverCode;
		long long m_assetID;
		unsigned m_userID;
		std::string m_event;
		std::string m_message;

	};
}; // namespace
//////////////////////////////////////////////////////////////////////////////////////


#endif //REQUEST_H

