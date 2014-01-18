#if !defined (REQUEST_H_)
#define REQUEST_H_

#pragma warning (disable : 4786)

#include "CTGenericAPI/GenericMessage.h"
#include <Base/Archive.h>
#include <Unicode/UnicodeUtils.h>
#include "CTServiceAPI.h"

namespace CTService 
{

/*
//----------- TEST CODE ONLY ------------
//-----------------------------------------
class ReqTest : public GenericRequest
//-----------------------------------------
{
public:
	ReqTest(std::string &astring, unsigned anint);
	virtual ~ReqTest()	{};

	void pack(Base::ByteStream &msg);

private:
	std::string	m_astring;
	unsigned	m_anint;
};

//-----------------------------------------
class ReqReplyTest : public GenericRequest
//-----------------------------------------
{
public:
	ReqReplyTest(unsigned server_track, unsigned value);
	virtual ~ReqReplyTest()	{};

	void pack(Base::ByteStream &msg);

private:
	unsigned	m_value;
};
//----------- TEST CODE ONLY ------------
*/


//-----------------------------------------
class ReqReplyMoveStatus : public GenericRequest
//-----------------------------------------
{
public:
	ReqReplyMoveStatus(unsigned server_track, const unsigned status, const unsigned result, const CTUnicodeChar *reason);
	virtual ~ReqReplyMoveStatus()	{};

	void pack(Base::ByteStream &msg);

private:
	unsigned				m_status;
	unsigned				m_result;
	Plat_Unicode::String	m_reason;
};

//-----------------------------------------
class ReqReplyValidateMove : public GenericRequest
//-----------------------------------------
{
public:
	ReqReplyValidateMove(unsigned server_track, const unsigned result, const CTUnicodeChar *reason, const CTUnicodeChar *suggestedName);
	virtual ~ReqReplyValidateMove()	{};

	void pack(Base::ByteStream &msg);

private:
	unsigned				m_result;
	Plat_Unicode::String	m_reason;
	Plat_Unicode::String	m_suggestedName;
};

//-----------------------------------------
class ReqReplyMove : public GenericRequest
//-----------------------------------------
{
public:
	ReqReplyMove(unsigned server_track, const unsigned result, const CTUnicodeChar *reason);
	virtual ~ReqReplyMove()	{};

	void pack(Base::ByteStream &msg);

private:
	unsigned				m_result;
	Plat_Unicode::String	m_reason;
};

//-----------------------------------------
class ReqReplyDelete : public GenericRequest
//-----------------------------------------
{
public:
	ReqReplyDelete(unsigned server_track, const unsigned result, const CTUnicodeChar *reason);
	virtual ~ReqReplyDelete()	{};

	void pack(Base::ByteStream &msg);

private:
	unsigned				m_result;
	Plat_Unicode::String	m_reason;
};

//-----------------------------------------
class ReqReplyRestore : public GenericRequest
//-----------------------------------------
{
public:
	ReqReplyRestore(unsigned server_track, const unsigned result, const CTUnicodeChar *reason);
	virtual ~ReqReplyRestore()	{};

	void pack(Base::ByteStream &msg);

private:
	unsigned				m_result;
	Plat_Unicode::String	m_reason;
};

//-----------------------------------------
class ReqReplyTransferAccount : public GenericRequest
//-----------------------------------------
{
public:
	ReqReplyTransferAccount(unsigned server_track, const unsigned result, const CTUnicodeChar *reason);
	virtual ~ReqReplyTransferAccount()	{};

	void pack(Base::ByteStream &msg);

private:
	unsigned				m_result;
	Plat_Unicode::String	m_reason;
};

//-----------------------------------------
class ReqReplyCharacterList : public GenericRequest
//-----------------------------------------
{
public:
	ReqReplyCharacterList(unsigned server_track, const unsigned result, const unsigned count, const CTServiceCharacter *characters);
	virtual ~ReqReplyCharacterList()	{};

	void pack(Base::ByteStream &msg);

private:
	unsigned						m_result;
	unsigned						m_count;
	std::vector<CTServiceCharacter>	m_characterArray;
};

//-----------------------------------------
class ReqReplyServerList : public GenericRequest
//-----------------------------------------
{
public:
	ReqReplyServerList(unsigned server_track, const unsigned result, const unsigned count, const CTServiceServer *servers);
	virtual ~ReqReplyServerList()	{};

	void pack(Base::ByteStream &msg);

private:
	unsigned						m_result;
	unsigned						m_count;
	std::vector<CTServiceServer>	m_serverArray;
};

//-----------------------------------------
class ReqReplyDestinationServerList : public GenericRequest
//-----------------------------------------
{
public:
	ReqReplyDestinationServerList(unsigned server_track, const unsigned result, const unsigned count, const CTServiceServer *servers);
	virtual ~ReqReplyDestinationServerList()	{};

	void pack(Base::ByteStream &msg);

private:
	unsigned						m_result;
	unsigned						m_count;
	std::vector<CTServiceServer>	m_serverArray;
};

}; // namespace

#endif 



