#include "Response.h"

namespace CTService 
{

using namespace Base;

/*
//----------- TEST CODE ONLY ------------
//-----------------------------------------
ResTest::ResTest(void *user)
: GenericResponse(CTGAME_REQUEST_TEST, CT_RESULT_TIMEOUT, user),
  m_value(0)
//-----------------------------------------
{
}

//-----------------------------------------
void ResTest::unpack(ByteStream::ReadIterator &iter)
//-----------------------------------------
{
    get(iter, m_type);
    get(iter, m_track);
    get(iter, m_result);
    get(iter, m_value);
}

//-----------------------------------------
ResReplyTest::ResReplyTest(void *user)
: GenericResponse(CTGAME_REPLY_TEST, CT_RESULT_TIMEOUT, user)
//-----------------------------------------
{
}
//----------- TEST CODE ONLY ------------
*/

//-----------------------------------------
ResReplyMoveStatus::ResReplyMoveStatus(void *user)
: GenericResponse(CTGAME_REPLY_MOVESTATUS, CT_RESULT_TIMEOUT, user)
//-----------------------------------------
{
}

//-----------------------------------------
ResReplyValidateMove::ResReplyValidateMove(void *user)
: GenericResponse(CTGAME_REPLY_VALIDATEMOVE, CT_RESULT_TIMEOUT, user)
//-----------------------------------------
{
}

//-----------------------------------------
ResReplyMove::ResReplyMove(void *user)
: GenericResponse(CTGAME_REPLY_MOVE, CT_RESULT_TIMEOUT, user)
//-----------------------------------------
{
}

//-----------------------------------------
ResReplyDelete::ResReplyDelete(void *user)
: GenericResponse(CTGAME_REPLY_DELETE, CT_RESULT_TIMEOUT, user)
//-----------------------------------------
{
}

//-----------------------------------------
ResReplyRestore::ResReplyRestore(void *user)
: GenericResponse(CTGAME_REPLY_RESTORE, CT_RESULT_TIMEOUT, user)
//-----------------------------------------
{
}

//-----------------------------------------
ResReplyTransferAccount::ResReplyTransferAccount(void *user)
: GenericResponse(CTGAME_REPLY_TRANSFER_ACCOUNT, CT_RESULT_TIMEOUT, user)
//-----------------------------------------
{
}

//-----------------------------------------
ResReplyCharacterList::ResReplyCharacterList(void *user)
: GenericResponse(CTGAME_REPLY_CHARACTERLIST, CT_RESULT_TIMEOUT, user)
//-----------------------------------------
{
}

//-----------------------------------------
ResReplyServerList::ResReplyServerList(void *user)
: GenericResponse(CTGAME_REPLY_SERVERLIST, CT_RESULT_TIMEOUT, user)
//-----------------------------------------
{
}

//-----------------------------------------
ResReplyDestinationServerList::ResReplyDestinationServerList(void *user)
: GenericResponse(CTGAME_REPLY_DESTSERVERLIST, CT_RESULT_TIMEOUT, user)
//-----------------------------------------
{
}

}; // namespace
