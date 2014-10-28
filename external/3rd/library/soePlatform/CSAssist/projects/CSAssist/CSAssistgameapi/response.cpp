//------------------------------------------------------------------------------
//
// CSAssistgameapi.cpp
//
// Author:	Graeme Ing		(ging@soe.sony.com)
// Module:	CSAssist API
//
// copyright 2002 (c) Sony Online Entertainment Inc.
//
// Response functions, process data from the servers
//
//------------------------------------------------------------------------------
#pragma warning (disable: 4786)

#include "response.h"

namespace CSAssist
{

using namespace Base;
using namespace std;

extern CSAssistUnicodeChar *get_c_str(Plat_Unicode::String s);
extern CSAssistUnicodeChar *copy_c_str(Plat_Unicode::String s, CSAssistUnicodeChar *dest);


//----------------------------------------------
Response::Response(unsigned type, unsigned track, const void *userData)
: mType(type),
  mTrack(track), 
  mResult(CSASSIST_RESULT_TIMEOUT)
//----------------------------------------------
{
	muserData = (void *)userData;
	//fprintf(stderr,"Response(%p) type(%d),track(%d)\n", this, mType,mTrack);
}

Response::~Response()
{
	//fprintf(stderr,"~Response(%p) type(%d),track(%d)\n", this,mType,mTrack);
}

//----------------------------------------------
void Response::decode(ByteStream::ReadIterator &msg)
//----------------------------------------------
{
}


//----------------------------------------------
//
// Add new Response objects for each API call here
//
// **** ADD CODE HERE ****
//----------------------------------------------

//----------------------------------------------
ResConnectCSAssist::ResConnectCSAssist(unsigned track, const void *userData)
: Response(CSASSIST_CALL_CONNECT, track, userData)
//----------------------------------------------
{
}

//----------------------------------------------
ResDisconnectCSAssist::ResDisconnectCSAssist(unsigned track, const void *userData)
: Response(CSASSIST_CALL_DISCONNECT, track, userData)
//----------------------------------------------
{
}

//----------------------------------------------
ResNewTicketActivity::ResNewTicketActivity(unsigned track, const void *userData)
: Response(CSASSIST_CALL_NEWTICKETACTIVITY, track, userData), mNewActivityFlag(0), mHasTickets(0)
//----------------------------------------------
{
}

ResNewTicketActivity::~ResNewTicketActivity()
{
}

//----------------------------------------------
void ResNewTicketActivity::decode(Base::ByteStream::ReadIterator &msg)
//----------------------------------------------
{
	get(msg, mNewActivityFlag);
	get(msg, mHasTickets);
}

//----------------------------------------------
ResRegisterCharacter::ResRegisterCharacter(unsigned track, const void *userData)
: Response(CSASSIST_CALL_REGISTERCHARACTER, track, userData)
//----------------------------------------------
{
}

//----------------------------------------------
ResUnRegisterCharacter::ResUnRegisterCharacter(unsigned track, const void *userData)
: Response(CSASSIST_CALL_UNREGISTERCHARACTER, track, userData)
//----------------------------------------------
{
}

//----------------------------------------------
ResGetIssueHierarchy::ResGetIssueHierarchy(unsigned track, const void *userData)
: Response(CSASSIST_CALL_GETISSUEHIERARCHY, track, userData), mModifyDate(0), rawStringPtr(0)
//----------------------------------------------
{
}

//----------------------------------------------
ResGetIssueHierarchy::~ResGetIssueHierarchy()
//----------------------------------------------
{
	delete [] rawStringPtr;
}

//----------------------------------------------
void ResGetIssueHierarchy::decode(Base::ByteStream::ReadIterator &msg)
//----------------------------------------------
{
	get(msg, mHierarchyBody);
	get(msg, mModifyDate);
	rawStringPtr = get_c_str(mHierarchyBody);
}

//----------------------------------------------
ResCreateTicket::ResCreateTicket(unsigned track, const void *userData)
: Response(CSASSIST_CALL_CREATETICKET, track, userData), mTicketID(0)
//----------------------------------------------
{
}

ResCreateTicket::~ResCreateTicket()
{
}

//----------------------------------------------
void ResCreateTicket::decode(Base::ByteStream::ReadIterator &msg)
//----------------------------------------------
{
	get(msg, mTicketID);
}

//----------------------------------------------
ResAppendComment::ResAppendComment(unsigned track, const void *userData)
: Response(CSASSIST_CALL_APPENDCOMMENT, track, userData), mTicketID(0)
//----------------------------------------------
{
}

ResAppendComment::~ResAppendComment()
{
}

//----------------------------------------------
void ResAppendComment::decode(Base::ByteStream::ReadIterator &msg)
//----------------------------------------------
{
	get(msg, mTicketID);
}

//----------------------------------------------
ResGetTicketByID::ResGetTicketByID(unsigned track, const void *userData)
: Response(CSASSIST_CALL_GETTICKETBYID, track, userData), mTicketBody(CSAssist::CSAssistGameAPITicket())
//----------------------------------------------
{
}

ResGetTicketByID::~ResGetTicketByID()
{
}

//----------------------------------------------
void ResGetTicketByID::decode(Base::ByteStream::ReadIterator &msg)
//----------------------------------------------
{
	get(msg, mTicketBody);
}

//----------------------------------------------
ResGetTicketComments::ResGetTicketComments(unsigned track, const void *userData)
: Response(CSASSIST_CALL_GETTICKETCOMMENTS, track, userData), mNumberRead(0), mCommentArray(0)
//----------------------------------------------
{
}

//----------------------------------------------
ResGetTicketComments::~ResGetTicketComments()
//----------------------------------------------
{
	delete [] mCommentArray;		// be sure to free up any resources!
}

//----------------------------------------------
void ResGetTicketComments::decode(Base::ByteStream::ReadIterator &msg)
//----------------------------------------------
{
	get(msg,mNumberRead);

	mCommentArray = 0;
	if (mNumberRead > 0)
	{
		mCommentArray = new CSAssistGameAPITicketComment[mNumberRead];
		CSAssistGameAPITicketComment *pcomments = mCommentArray;
		for (unsigned i=0; i < mNumberRead; i++)
		{
			decodeTicketComment(msg, pcomments);
			pcomments++;
		}
	}
}

//----------------------------------------------
ResGetTicketByCharacter::ResGetTicketByCharacter(unsigned track, const void *userData)
: Response(CSASSIST_CALL_GETTICKET, track, userData), mNumberReturned(0), mTotalNumber(0), mTicketArray(0)
//----------------------------------------------
{
}

//----------------------------------------------
ResGetTicketByCharacter::~ResGetTicketByCharacter()
//----------------------------------------------
{
	delete [] mTicketArray;		// be sure to free up any resources!
}

//----------------------------------------------
void ResGetTicketByCharacter::decode(Base::ByteStream::ReadIterator &msg)
//----------------------------------------------
{
	get(msg, mNumberReturned);
	get(msg, mTotalNumber);

	mTicketArray = 0;
	if (mNumberReturned > 0)
	{
		mTicketArray = new CSAssistGameAPITicket[mNumberReturned];
		CSAssistGameAPITicket *ptickets = mTicketArray;
		for (unsigned i=0; i < mNumberReturned; i++)
		{
			get(msg, ptickets);
			ptickets++;
		}
	}
}

//----------------------------------------------
ResTicketChange::ResTicketChange(unsigned track)
: Response(CSASSIST_SERVER_TICKETCHANGE, track, 0), mTicketID(0), mUID(0), rawCharacter(0)
//----------------------------------------------
{
}

//----------------------------------------------
ResTicketChange::~ResTicketChange()
//----------------------------------------------
{
	delete [] rawCharacter;
}

//----------------------------------------------
void ResTicketChange::decode(Base::ByteStream::ReadIterator &msg)
//----------------------------------------------
{
	get(msg, mTicketID);
	get(msg, mUID);
	get(msg, mCharacter);
	rawCharacter = get_c_str(mCharacter);
}

//----------------------------------------------
ResMarkTicketRead::ResMarkTicketRead(unsigned track, const void *userData)
: Response(CSASSIST_CALL_MARKREAD, track, userData)
//----------------------------------------------
{
}

//----------------------------------------------
ResCancelTicket::ResCancelTicket(unsigned track, const void *userData)
: Response(CSASSIST_CALL_CANCELTICKET, track, userData), mTicketID(0)
//----------------------------------------------
{
}

ResCancelTicket::~ResCancelTicket()
{
}

//----------------------------------------------
void ResCancelTicket::decode(Base::ByteStream::ReadIterator &msg)
//----------------------------------------------
{
	get(msg, mTicketID);
}

//----------------------------------------------
ResCommentCount::ResCommentCount(unsigned track, const void *userData)
: Response(CSASSIST_CALL_COMMENTCOUNT, track, userData), mTicketID(0), mCount(0)
//----------------------------------------------
{
}

ResCommentCount::~ResCommentCount()
{
}

//----------------------------------------------
void ResCommentCount::decode(Base::ByteStream::ReadIterator &msg)
//----------------------------------------------
{
	get(msg, mTicketID);
	get(msg, mCount);
}

//----------------------------------------------
ResRequestGameLocation::ResRequestGameLocation(unsigned track)
: Response(CSASSIST_SERVER_REQUESTLOCATION, track, 0), mSourceTrack(0), mUID(0), mCSRUID(0),
rawCharacter(0)
//----------------------------------------------
{
}

//----------------------------------------------
ResRequestGameLocation::~ResRequestGameLocation()
//----------------------------------------------
{
	delete [] rawCharacter;
}

//----------------------------------------------
void ResRequestGameLocation::decode(Base::ByteStream::ReadIterator &msg)
//----------------------------------------------
{
	get(msg,mSourceTrack);
	get(msg,mUID);
	get(msg,mCharacter);
	get(msg,mCSRUID);
	rawCharacter = get_c_str(mCharacter);
}

//----------------------------------------------
ResReplyGameLocation::ResReplyGameLocation(unsigned track, const void *userData)
: Response(CSASSIST_CALL_REPLYLOCATION, track, userData)
//----------------------------------------------
{
}

//----------------------------------------------
ResGetDocumentList::ResGetDocumentList(unsigned track, const void *userData)
: Response(CSASSIST_CALL_GETDOCUMENTLIST, track, userData), mNumberRead(0), mDocumentArray(0)
//----------------------------------------------
{
}

//----------------------------------------------
ResGetDocumentList::~ResGetDocumentList()
//----------------------------------------------
{
	delete [] mDocumentArray;		// be sure to free up any resources!
}

//----------------------------------------------
void ResGetDocumentList::decode(Base::ByteStream::ReadIterator &msg)
//----------------------------------------------
{
	get(msg,mNumberRead);

	mDocumentArray = 0;
	if (mNumberRead > 0)
	{
		mDocumentArray = new CSAssistGameAPIDocumentHeader[mNumberRead];
		CSAssistGameAPIDocumentHeader *pdocs = mDocumentArray;
		for (unsigned i=0; i < mNumberRead; i++)
		{
			decodeDocumentHeader(msg, pdocs);
			pdocs++;
		}
	}

}

//----------------------------------------------
ResGetDocument::ResGetDocument(unsigned track, const void *userData)
: Response(CSASSIST_CALL_GETDOCUMENT, track, userData), rawDocument(0)
//----------------------------------------------
{
}

//----------------------------------------------
ResGetDocument::~ResGetDocument()
//----------------------------------------------
{
	delete [] rawDocument;
}

//----------------------------------------------
void ResGetDocument::decode(Base::ByteStream::ReadIterator &msg)
//----------------------------------------------
{
	get(msg,mDocumentBody);
	rawDocument = get_c_str(mDocumentBody);
}

//----------------------------------------------
ResGetTicketXMLBlock::ResGetTicketXMLBlock(unsigned track, const void *userData)
: Response(CSASSIST_CALL_GETTICKETXMLBLOCK, track, userData), rawXML(0)
//----------------------------------------------
{
}

//----------------------------------------------
ResGetTicketXMLBlock::~ResGetTicketXMLBlock()
//----------------------------------------------
{
	delete [] rawXML;
}

//----------------------------------------------
void ResGetTicketXMLBlock::decode(Base::ByteStream::ReadIterator &msg)
//----------------------------------------------
{
	get(msg,mXMLBody);
	rawXML = get_c_str(mXMLBody);
}

//----------------------------------------------
ResGetKBArticle::ResGetKBArticle(unsigned track, const void *userData)
: Response(CSASSIST_CALL_GETKBARTICLE, track, userData), rawArticle(0), rawTitle(0)
//----------------------------------------------
{
}

//----------------------------------------------
ResGetKBArticle::~ResGetKBArticle()
//----------------------------------------------
{
	delete [] rawArticle;
	delete [] rawTitle;
}

//----------------------------------------------
void ResGetKBArticle::decode(Base::ByteStream::ReadIterator &msg)
//----------------------------------------------
{
	get(msg,mArticleBody);
	rawArticle = get_c_str(mArticleBody);
	// don't try to read title from older servers
	if (msg.getSize() <= 0) 
		return;
	get(msg,mArticleTitle);
	rawTitle   = get_c_str(mArticleTitle);
}

//----------------------------------------------
ResSearchKB::ResSearchKB(unsigned track, const void *userData)
: Response(CSASSIST_CALL_SEARCHKB, track, userData), mNumberRead(0), mArticleArray(0)
//----------------------------------------------
{
}

//----------------------------------------------
ResSearchKB::~ResSearchKB()
//----------------------------------------------
{
	delete [] mArticleArray;		// be sure to free up any resources!
}

//----------------------------------------------
void ResSearchKB::decode(Base::ByteStream::ReadIterator &msg)
//----------------------------------------------
{
	get(msg, mNumberRead);

	mArticleArray = 0;
	if (mNumberRead > 0)
	{
		mArticleArray = new CSAssistGameAPISearchResult[mNumberRead];
		CSAssistGameAPISearchResult *pdocs = mArticleArray;
		for (unsigned i=0; i < mNumberRead; i++)
		{
			decodeSearchResult(msg, pdocs);
			pdocs++;
		}
	}

}

//----------------------------------------------
ResHierarchyChange::ResHierarchyChange(unsigned track)
: Response(CSASSIST_SERVER_HIERARCHYCHANGE, track, 0),
rawGame(0), rawVersion(0), rawLanguage(0), mChangeType(CSASSIST_HIERARCHY_NONE)
//----------------------------------------------
{
}

ResHierarchyChange::~ResHierarchyChange()
{
	delete [] rawGame;
	delete [] rawVersion;
	delete [] rawLanguage;
}

//----------------------------------------------
void ResHierarchyChange::decode(Base::ByteStream::ReadIterator &msg)
//----------------------------------------------
{
	get(msg,mGame);
	get(msg,mVersion);
	get(msg,mLanguage);
	rawGame = get_c_str(mGame);
	rawVersion = get_c_str(mVersion);
	rawLanguage = get_c_str(mLanguage);
	get(msg, mChangeType);
}

ResConnectLB::ResConnectLB(unsigned track, Request *req, Response *res) 
:	Response(CSASSIST_CALL_CONNECTLB, track, 0),
	mServerPort(0),
	mConnectRequest(req),
	mConnectResponse(res)
{
	//fprintf(stderr,"Const:Req=%p,Res=%p\n", mConnectRequest,mConnectResponse);
}

ResConnectLB::~ResConnectLB()
{
	if (mResult == CSASSIST_RESULT_TIMEOUT)
	{
		//fprintf(stderr,"Dest: Req=%p,Res=%p\n", mConnectRequest,mConnectResponse);
		delete mConnectRequest;
		delete mConnectResponse;

	}
}

void ResConnectLB::decode(Base::ByteStream::ReadIterator &msg)
{
	get(msg, mServerName);
	get(msg, mServerPort);
}

} // namespace CSAssist
