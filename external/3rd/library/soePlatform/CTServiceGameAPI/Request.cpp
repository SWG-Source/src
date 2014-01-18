#include "Request.h"
#include "CTCommon/CTServiceObjects.h"

namespace CTService 
{

using namespace Base;
using namespace Plat_Unicode;

/*
//----------- TEST CODE ONLY ------------
//-----------------------------------------
ReqTest::ReqTest(std::string &astring, unsigned anint)
: GenericRequest(CTGAME_REQUEST_TEST),
m_astring(astring), m_anint(anint)
//-----------------------------------------
{
}

//-----------------------------------------
void ReqTest::pack(Base::ByteStream &msg)
//-----------------------------------------
{
	put(msg, m_type);
	put(msg, m_track);
	put(msg, m_astring);
	put(msg, m_anint);
}

//-----------------------------------------
ReqReplyTest::ReqReplyTest(unsigned server_track, unsigned value)
: GenericRequest(CTGAME_REPLY_TEST, server_track),
m_value(value)
//-----------------------------------------
{
}

//-----------------------------------------
void ReqReplyTest::pack(Base::ByteStream &msg)
//-----------------------------------------
{
	put(msg, m_type);
	put(msg, m_track);
	put(msg, m_server_track);
	put(msg, m_value);
}
//----------- TEST CODE ONLY ------------
*/


//-----------------------------------------
ReqReplyMoveStatus::ReqReplyMoveStatus(unsigned server_track, const unsigned status, const unsigned result, const CTUnicodeChar *reason)
: GenericRequest(CTGAME_REPLY_MOVESTATUS, server_track),
m_status(status), m_result(result), m_reason(reason)
//-----------------------------------------
{
}

//-----------------------------------------
void ReqReplyMoveStatus::pack(Base::ByteStream &msg)
//-----------------------------------------
{
	put(msg, m_type);
	put(msg, m_track);
	put(msg, m_server_track);
	put(msg, m_status);
	put(msg, m_result);
	put(msg, m_reason);
}

//-----------------------------------------
ReqReplyValidateMove::ReqReplyValidateMove(unsigned server_track, const unsigned result, const CTUnicodeChar *reason, const CTUnicodeChar *suggestedName)
: GenericRequest(CTGAME_REPLY_VALIDATEMOVE, server_track),
m_result(result), m_reason(reason), m_suggestedName(suggestedName)
//-----------------------------------------
{
}

//-----------------------------------------
void ReqReplyValidateMove::pack(Base::ByteStream &msg)
//-----------------------------------------
{
	put(msg, m_type);
	put(msg, m_track);
	put(msg, m_server_track);
	put(msg, m_result);
	put(msg, m_reason);
	put(msg, m_suggestedName);
}

//-----------------------------------------
ReqReplyMove::ReqReplyMove(unsigned server_track, const unsigned result, const CTUnicodeChar *reason)
: GenericRequest(CTGAME_REPLY_MOVE, server_track),
m_result(result), m_reason(reason)
//-----------------------------------------
{
}

//-----------------------------------------
void ReqReplyMove::pack(Base::ByteStream &msg)
//-----------------------------------------
{
	put(msg, m_type);
	put(msg, m_track);
	put(msg, m_server_track);
	put(msg, m_result);
	put(msg, m_reason);
}

//-----------------------------------------
ReqReplyDelete::ReqReplyDelete(unsigned server_track, const unsigned result, const CTUnicodeChar *reason)
: GenericRequest(CTGAME_REPLY_DELETE, server_track),
m_result(result), m_reason(reason)
//-----------------------------------------
{
}

//-----------------------------------------
void ReqReplyDelete::pack(Base::ByteStream &msg)
//-----------------------------------------
{
	put(msg, m_type);
	put(msg, m_track);
	put(msg, m_server_track);
	put(msg, m_result);
	put(msg, m_reason);
}

//-----------------------------------------
ReqReplyRestore::ReqReplyRestore(unsigned server_track, const unsigned result, const CTUnicodeChar *reason)
: GenericRequest(CTGAME_REPLY_RESTORE, server_track),
m_result(result), m_reason(reason)
//-----------------------------------------
{
}

//-----------------------------------------
void ReqReplyRestore::pack(Base::ByteStream &msg)
//-----------------------------------------
{
	put(msg, m_type);
	put(msg, m_track);
	put(msg, m_server_track);
	put(msg, m_result);
	put(msg, m_reason);
}

//-----------------------------------------
ReqReplyTransferAccount::ReqReplyTransferAccount(unsigned server_track, const unsigned result, const CTUnicodeChar *reason)
: GenericRequest(CTGAME_REPLY_TRANSFER_ACCOUNT, server_track),
m_result(result), m_reason(reason)
//-----------------------------------------
{
}

//-----------------------------------------
void ReqReplyTransferAccount::pack(Base::ByteStream &msg)
//-----------------------------------------
{
	put(msg, m_type);
	put(msg, m_track);
	put(msg, m_server_track);
	put(msg, m_result);
	put(msg, m_reason);
}

//-----------------------------------------
ReqReplyCharacterList::ReqReplyCharacterList(unsigned server_track, const unsigned result, const unsigned count, const CTServiceCharacter *characters)
: GenericRequest(CTGAME_REPLY_CHARACTERLIST, server_track),
m_result(result), m_count(count)
//-----------------------------------------
{
	for (unsigned i=0; i < count; i++)
	{
		m_characterArray.push_back(characters[i]);
	}
}

//-----------------------------------------
void ReqReplyCharacterList::pack(Base::ByteStream &msg)
//-----------------------------------------
{
	put(msg, m_type);
	put(msg, m_track);
	put(msg, m_server_track);
	put(msg, m_result);
	put(msg, m_count);
	for (unsigned i=0; i < m_count; i++)
		put(msg, m_characterArray[i]);
	printf("\nReqReplyCharacterList::pack: m_track(%d) m_server_track(%d)\n", m_track, m_server_track);
}

//-----------------------------------------
ReqReplyServerList::ReqReplyServerList(unsigned server_track, const unsigned result, const unsigned count, const CTServiceServer *servers)
: GenericRequest(CTGAME_REPLY_SERVERLIST, server_track),
m_result(result), m_count(count)
//-----------------------------------------
{
	for (unsigned i=0; i < count; i++)
	{
		m_serverArray.push_back(servers[i]);
	}
}

//-----------------------------------------
void ReqReplyServerList::pack(Base::ByteStream &msg)
//-----------------------------------------
{
	put(msg, m_type);
	put(msg, m_track);
	put(msg, m_server_track);
	put(msg, m_result);
	put(msg, m_count);
	for (unsigned i=0; i < m_count; i++)
		put(msg, m_serverArray[i]);
}

//-----------------------------------------
ReqReplyDestinationServerList::ReqReplyDestinationServerList(unsigned server_track, const unsigned result, const unsigned count, const CTServiceServer *servers)
: GenericRequest(CTGAME_REPLY_DESTSERVERLIST, server_track),
m_result(result), m_count(count)
//-----------------------------------------
{
	for (unsigned i=0; i < count; i++)
	{
		m_serverArray.push_back(servers[i]);
	}
}

//-----------------------------------------
void ReqReplyDestinationServerList::pack(Base::ByteStream &msg)
//-----------------------------------------
{
	put(msg, m_type);
	put(msg, m_track);
	put(msg, m_server_track);
	put(msg, m_result);
	put(msg, m_count);
	for (unsigned i=0; i < m_count; i++)
		put(msg, m_serverArray[i]);
}

}; // namespace
