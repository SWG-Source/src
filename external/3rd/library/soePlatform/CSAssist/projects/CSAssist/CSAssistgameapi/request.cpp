//------------------------------------------------------------------------------
//
// request.cpp
//
// Author:	Graeme Ing		(ging@soe.sony.com)
// Module:	CSAssist API
//
// copyright 2002 (c) Sony Online Entertainment Inc.
//
// Request functions, prepare data to send to servers
//
//------------------------------------------------------------------------------

#include "request.h"
#include "packdata.h"
#include <stdio.h>

namespace CSAssist
{

using namespace Base;
using namespace std;
using namespace Plat_Unicode;

const static unsigned CSASSIST_VERSION_MAGIC = (4 + (256 * 3) + (65536 * 2) + (65536 * 256 * 1));
const static unsigned API_VERSION = 201;

//------------------------------------------------
Request::Request(short type, unsigned track) :
mType(type), mTrack(track), timeout(0)
//----------------------------------------------
{
}

//----------------------------------------------
void Request::pack(ByteStream &msg)
//----------------------------------------------
{
	short num_args = 2;
	put(msg, num_args);
	put(msg, mType);
	put(msg, mTrack);
}

//----------------------------------------------
//
// Add new Request objects for each API call here
// **** ADD CODE HERE ****
//
//----------------------------------------------

//----------------------------------------------
RConnectCSAssist::RConnectCSAssist(unsigned track, Plat_Unicode::String GameName, const Plat_Unicode::String ServerName)
: Request(CSASSIST_CALL_CONNECT, track), 
  mGameName(GameName),
  mServerName(ServerName)
//----------------------------------------------
{
}

//----------------------------------------------
void RConnectCSAssist::pack(ByteStream &msg)
//----------------------------------------------
{
	short num_args = 4;
	put(msg, num_args);
	put(msg, mType);
	put(msg, mTrack);
	put(msg, (unsigned)CSASSIST_VERSION_MAGIC);
	put(msg, (unsigned)API_VERSION);
	put(msg, mGameName);
	put(msg, mServerName);
}

//----------------------------------------------
	RDisconnectCSAssist::RDisconnectCSAssist(unsigned track)
: Request(CSASSIST_CALL_DISCONNECT, track) 
//----------------------------------------------
{
}

//----------------------------------------------
	RNewTicketActivity::RNewTicketActivity(unsigned track, const unsigned uid, const Plat_Unicode::String character)
: Request(CSASSIST_CALL_NEWTICKETACTIVITY, track), 
  mUID(uid), 
  mCharacter(character)
//----------------------------------------------
{
}

//----------------------------------------------
void RNewTicketActivity::pack(ByteStream &msg)
//----------------------------------------------
{
	short num_args = 4;
	put(msg, num_args);
	put(msg, mType);
	put(msg, mTrack);
	put(msg, mUID);
	put(msg, mCharacter);
}

//----------------------------------------------
	RRegisterCharacter::RRegisterCharacter(unsigned track, const unsigned uid, const Plat_Unicode::String character, const unsigned avaconID)
: Request(CSASSIST_CALL_REGISTERCHARACTER, track), 
  mUID(uid), 
  mCharacter(character),
  mAvaconID(avaconID)
//----------------------------------------------
{
}

//----------------------------------------------
void RRegisterCharacter::pack(ByteStream &msg)
//----------------------------------------------
{
	short num_args = 5;
	put(msg, num_args);
	put(msg, mType);
	put(msg, mTrack);
	put(msg, mUID);
	put(msg, mCharacter);
	put(msg, mAvaconID);
}

//----------------------------------------------
	RUnRegisterCharacter::RUnRegisterCharacter(unsigned track, const unsigned uid, const Plat_Unicode::String character)
: Request(CSASSIST_CALL_UNREGISTERCHARACTER, track), 
  mUID(uid), 
  mCharacter(character)
//----------------------------------------------
{
}

//----------------------------------------------
void RUnRegisterCharacter::pack(ByteStream &msg)
//----------------------------------------------
{
	short num_args = 4;
	put(msg, num_args);
	put(msg, mType);
	put(msg, mTrack);
	put(msg, mUID);
	put(msg, mCharacter);
}

//----------------------------------------------
	RGetIssueHierarchy::RGetIssueHierarchy(unsigned track, const Plat_Unicode::String version, const Plat_Unicode::String language)
: Request(CSASSIST_CALL_GETISSUEHIERARCHY, track),
  mVersion(version), 
  mLanguage(language)
//----------------------------------------------
{
}

//----------------------------------------------
void RGetIssueHierarchy::pack(ByteStream &msg)
//----------------------------------------------
{
	short num_args = 4;
	put(msg, num_args);
	put(msg, mType);
	put(msg, mTrack);
	put(msg, mVersion);
	put(msg, mLanguage);
}

//----------------------------------------------
RCreateTicket::RCreateTicket(unsigned track, const CSAssistGameAPITicket *ticketBody, const Plat_Unicode::String XMLBody, const unsigned uid)
: Request(CSASSIST_CALL_CREATETICKET, track), 
  mXMLBody(XMLBody),
  mUID(uid)
//----------------------------------------------
{
	mTicketStruct = *ticketBody;
}
//----------------------------------------------
void RCreateTicket::pack(ByteStream &msg)
//----------------------------------------------
{
	short num_args = 5;
	put(msg, num_args);
	put(msg, mType);
	put(msg, mTrack);
	put(msg, mTicketStruct);
	put(msg, mXMLBody);
	put(msg, mUID);
}

//----------------------------------------------
RAppendComment::RAppendComment(unsigned track, const unsigned ticket, const unsigned uid, const Plat_Unicode::String character, const Plat_Unicode::String comment)
: Request(CSASSIST_CALL_APPENDCOMMENT, track), 
  mTicketID(ticket), 
  mUID(uid),
  mComment(comment),
  mCharacter(character)
//----------------------------------------------
{
}

//----------------------------------------------
void RAppendComment::pack(ByteStream &msg)
//----------------------------------------------
{
	short num_args = 6;
	put(msg, num_args);
	put(msg, mType);
	put(msg, mTrack);
	put(msg, mTicketID);
	put(msg, mUID);
	put(msg, mComment);
	put(msg, mCharacter);
}

//----------------------------------------------
	RGetTicketByID::RGetTicketByID(unsigned track, const unsigned ticket, const unsigned MarkAsRead)
: Request(CSASSIST_CALL_GETTICKETBYID, track), 
  mTicketID(ticket), 
  mMarkAsRead(MarkAsRead)
//----------------------------------------------
{
}

//----------------------------------------------
void RGetTicketByID::pack(ByteStream &msg)
//----------------------------------------------
{
	short num_args = 4;
	put(msg, num_args);
	put(msg, mType);
	put(msg, mTrack);
	put(msg, mTicketID);
	put(msg, mMarkAsRead);
}

//----------------------------------------------
	RGetTicketComments::RGetTicketComments(unsigned track, const unsigned ticket, const unsigned start, const unsigned count, const unsigned offset)
: Request(CSASSIST_CALL_GETTICKETCOMMENTS, track), 
  mTicketID(ticket), 
  mStart(start),
  mCount(count),
  mOffset(offset)
//----------------------------------------------
{
}

//----------------------------------------------
void RGetTicketComments::pack(ByteStream &msg)
//----------------------------------------------
{
	short num_args = 6;
	put(msg, num_args);
	put(msg, mType);
	put(msg, mTrack);
	put(msg, mTicketID);
	put(msg, mStart);
	put(msg, mCount);
	put(msg, mOffset);
}

//----------------------------------------------
RGetTicketByCharacter::RGetTicketByCharacter(unsigned track, const unsigned uid, const Plat_Unicode::String character, const unsigned Start, const unsigned Count, const unsigned MarkAsRead)
: Request(CSASSIST_CALL_GETTICKET, track), 
  mUID(uid), 
  mCharacter(character),
  mStart(Start),
  mCount(Count),
  mMarkAsRead(MarkAsRead)
//----------------------------------------------
{
}

//----------------------------------------------
void RGetTicketByCharacter::pack(ByteStream &msg)
//----------------------------------------------
{
	short num_args = 7;
	put(msg, num_args);
	put(msg, mType);
	put(msg, mTrack);
	put(msg, mUID);
	put(msg, mCharacter);
	put(msg, mStart);
	put(msg, mCount);
	put(msg, mMarkAsRead);
}

//----------------------------------------------
	RMarkTicketRead::RMarkTicketRead(unsigned track, const unsigned ticket)
: Request(CSASSIST_CALL_MARKREAD, track), 
  mTicketID(ticket) 
//----------------------------------------------
{
}

//----------------------------------------------
void RMarkTicketRead::pack(ByteStream &msg)
//----------------------------------------------
{
	short num_args = 3;
	put(msg, num_args);
	put(msg, mType);
	put(msg, mTrack);
	put(msg, mTicketID);
}

//----------------------------------------------
RCancelTicket::RCancelTicket(unsigned track, const unsigned ticket, const unsigned uid, const Plat_Unicode::String comment)
: Request(CSASSIST_CALL_CANCELTICKET, track), 
  mTicketID(ticket), 
  mUID(uid),
  mComment(comment)
//----------------------------------------------
{
}

//----------------------------------------------
void RCancelTicket::pack(ByteStream &msg)
//----------------------------------------------
{
	short num_args = 5;
	put(msg, num_args);
	put(msg, mType);
	put(msg, mTrack);
	put(msg, mTicketID);
	put(msg, mUID);
	put(msg, mComment);
}

//----------------------------------------------
	RCommentCount::RCommentCount(unsigned track, const unsigned ticket)
: Request(CSASSIST_CALL_COMMENTCOUNT, track), 
  mTicketID(ticket) 
//----------------------------------------------
{
}

//----------------------------------------------
void RCommentCount::pack(ByteStream &msg)
//----------------------------------------------
{
	short num_args = 3;
	put(msg, num_args);
	put(msg, mType);
	put(msg, mTrack);
	put(msg, mTicketID);
}

//----------------------------------------------
	RReplyGameLocation::RReplyGameLocation(unsigned track, const unsigned sourceTrack, const unsigned uid, const Plat_Unicode::String character, const unsigned csruid, const Plat_Unicode::String location)
: Request(CSASSIST_CALL_REPLYLOCATION, track), 
  mSourceTrack(sourceTrack),
  mUID(uid),
  mCharacter(character),
  mCSRUID(csruid),
  mLocationString(location)
//----------------------------------------------
{
}

//----------------------------------------------
void RReplyGameLocation::pack(ByteStream &msg)
//----------------------------------------------
{
	short num_args = 7;
	put(msg, num_args);
	put(msg, mType);
	put(msg, mTrack);
	put(msg, mSourceTrack);
	put(msg, mUID);
	put(msg, mCharacter);
	put(msg, mCSRUID);
	put(msg, mLocationString);
}

//----------------------------------------------
	RGetDocumentList::RGetDocumentList(unsigned track, Plat_Unicode::String version, Plat_Unicode::String language)
: Request(CSASSIST_CALL_GETDOCUMENTLIST, track), mVersion(version), mLanguage(language)
//----------------------------------------------
{
}

//----------------------------------------------
void RGetDocumentList::pack(ByteStream &msg)
//----------------------------------------------
{
	short num_args = 4;
	put(msg, num_args);
	put(msg, mType);
	put(msg, mTrack);
	put(msg, mVersion);
	put(msg, mLanguage);
}

//----------------------------------------------
	RGetDocument::RGetDocument(unsigned track, const unsigned id)
: Request(CSASSIST_CALL_GETDOCUMENT, track), mID(id)
//----------------------------------------------
{
}

//----------------------------------------------
void RGetDocument::pack(ByteStream &msg)
//----------------------------------------------
{
	short num_args = 3;
	put(msg, num_args);
	put(msg, mType);
	put(msg, mTrack);
	put(msg, mID);
}

//----------------------------------------------
	RGetTicketXMLBlock::RGetTicketXMLBlock(unsigned track, const unsigned id)
: Request(CSASSIST_CALL_GETTICKETXMLBLOCK, track),
  mID(id)
//----------------------------------------------
{
}

//----------------------------------------------
void RGetTicketXMLBlock::pack(ByteStream &msg)
//----------------------------------------------
{
	short num_args = 3;
	put(msg, num_args);
	put(msg, mType);
	put(msg, mTrack);
	put(msg, mID);
}

//----------------------------------------------
	RGetKBArticle::RGetKBArticle(unsigned track, const Plat_Unicode::String id, const Plat_Unicode::String language, const unsigned uid)
: Request(CSASSIST_CALL_GETKBARTICLE, track),
  mID(id),
  mLanguage(language),
  mUID(uid)
//----------------------------------------------
{
}

//----------------------------------------------
void RGetKBArticle::pack(ByteStream &msg)
//----------------------------------------------
{
	short num_args = 5;
	put(msg, num_args);
	put(msg, mType);
	put(msg, mTrack);
	put(msg, mID);
	put(msg, mLanguage);
	put(msg, mUID);
}

//----------------------------------------------
	RSearchKB::RSearchKB(unsigned track, const Plat_Unicode::String searchstring, const Plat_Unicode::String language, const unsigned uid)
: Request(CSASSIST_CALL_SEARCHKB, track),
  mSearchString(searchstring), 
  mLanguage(language),
  mUID(uid)
//----------------------------------------------
{
}

//----------------------------------------------
void RSearchKB::pack(ByteStream &msg)
//----------------------------------------------
{
	short num_args = 5;
	put(msg, num_args);
	put(msg, mType);
	put(msg, mTrack);
	put(msg, mSearchString);
	put(msg, mLanguage);
	put(msg, mUID);
}

//----------------------------------------------
RConnectLB::RConnectLB(unsigned track, const std::string connectString)
: Request(CSASSIST_CALL_CONNECTLB, track), 
mConnectString(connectString)
//----------------------------------------------
{
	setTimeout(time(0) + 5);
}

//----------------------------------------------
void RConnectLB::pack(ByteStream &msg)
//----------------------------------------------
{
	short num_args = 3;
	put(msg, num_args);
	put(msg, mType);
	put(msg, mTrack);
	put(msg, mConnectString);
}

} // namespace CSAssist
