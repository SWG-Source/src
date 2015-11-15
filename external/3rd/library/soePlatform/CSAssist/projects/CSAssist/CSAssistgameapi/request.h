#ifndef REQUEST_H
#define REQUEST_H

//------------------------------------------------------------------------------
//
// request.h
//
// Author:	Graeme Ing		(ging@soe.sony.com)
// Module:	CSAssist API
//
// copyright 2002 (c) Sony Online Entertainment Inc.
//
// Class definitions for all user-initiated requests
//
//------------------------------------------------------------------------------

#include "Base/Archive.h"
#include "Unicode/UnicodeUtils.h"
#include "CSAssistgameapi.h"
#include "CSAssistgameobjects.h"

namespace CSAssist
{

// ----- Add all new messages to this list! -----

// **** ADD CODE HERE ****
enum SubCMessage 
{
	CSASSIST_CALL_CONNECT,						// 0
	CSASSIST_CALL_DISCONNECT,					// 1
	CSASSIST_CALL_NEWTICKETACTIVITY,			// 2
	CSASSIST_CALL_REGISTERCHARACTER,			// 3
	CSASSIST_CALL_UNREGISTERCHARACTER,			// 4
	CSASSIST_CALL_ERRORTEXT,					// 5
	CSASSIST_CALL_GETISSUEHIERARCHY,			// 6
	CSASSIST_CALL_CREATETICKET,					// 7
	CSASSIST_CALL_APPENDCOMMENT,				// 8
	CSASSIST_CALL_GETTICKETBYID,				// 9
	CSASSIST_CALL_GETTICKETCOMMENTS,			// 10
	CSASSIST_CALL_GETTICKET,					// 11
	CSASSIST_CALL_MARKREAD,						// 12
	CSASSIST_CALL_CANCELTICKET,					// 13
	CSASSIST_CALL_COMMENTCOUNT,					// 14
	CSASSIST_CALL_REPLYLOCATION,				// 15
	CSASSIST_CALL_GETDOCUMENTLIST,				// 16
	CSASSIST_CALL_GETDOCUMENT,					// 17
	CSASSIST_CALL_GETTICKETXMLBLOCK,			// 18
	CSASSIST_CALL_GETKBARTICLE,					// 19
	CSASSIST_CALL_SEARCHKB,						// 20
	// insert API message types above this line
	CSASSIST_CALL_REQUEST_MAX,					// 21 - always needs to be last of API messages

	CSASSIST_CALL_CONNECTLB = 2001,				// only used internally to connect to connection-load balancing server

	CSASSIST_CALL_START_SERVER_MESSAGES=10000,
	// insert server-initiated message types below this line
	CSASSIST_SERVER_TICKETCHANGE,				// 10001
	CSASSIST_SERVER_BROADCASTTICKET,			// 10002
	CSASSIST_SERVER_BROADCASTCOMMENT,			// 10003
	CSASSIST_SERVER_REQUESTLOCATION,			// 10004
	CSASSIST_SERVER_REPORTLOCATION,				// 10005
	CSASSIST_SERVER_HIERARCHYCHANGE				// 10006
};

//----------------------------------------------
class Request
//----------------------------------------------
{
	public:
		Request(short type, unsigned track);
		virtual ~Request()					{ }
		unsigned getType()					{ return (unsigned)mType; }
		unsigned getTrack()					{ return mTrack; }
		void setTimeout(unsigned t)			{ timeout = t; }
		unsigned getTimeout()				{ return timeout; }
		virtual void pack(Base::ByteStream &msg);

	protected:
		short mType;
		unsigned mTrack;

	private:
		unsigned timeout;
};

//----------------------------------------------
//
// Add new Request objects for each API call here
//
// **** ADD CODE HERE ****
//----------------------------------------------

//----------------------------------------------
class RConnectCSAssist : public Request
//----------------------------------------------
{
	public:
		RConnectCSAssist(unsigned track, const Plat_Unicode::String GameName, const Plat_Unicode::String ServerName);
		virtual void pack(Base::ByteStream &msg);
	private:
		Plat_Unicode::String mGameName;
		Plat_Unicode::String mServerName;
};

//----------------------------------------------
class RDisconnectCSAssist : public Request
//----------------------------------------------
{
	public:
		RDisconnectCSAssist(unsigned track);
	private:
};

//----------------------------------------------
class RNewTicketActivity : public Request
//----------------------------------------------
{
	public:
		RNewTicketActivity(unsigned track, const unsigned uid, const Plat_Unicode::String character);
		virtual void pack(Base::ByteStream &msg);
	private:
		unsigned			 mUID;
		Plat_Unicode::String mCharacter;
};

//----------------------------------------------
class RRegisterCharacter : public Request
//----------------------------------------------
{
	public:
		RRegisterCharacter(unsigned track, const unsigned uid, const Plat_Unicode::String character, const unsigned avaconID);
		virtual void pack(Base::ByteStream &msg);
	private:
		unsigned				mUID;
		Plat_Unicode::String	mCharacter;
		unsigned				mAvaconID;
};

//----------------------------------------------
class RUnRegisterCharacter : public Request
//----------------------------------------------
{
	public:
		RUnRegisterCharacter(unsigned track, const unsigned uid, const Plat_Unicode::String character);
		virtual void pack(Base::ByteStream &msg);
	private:
		unsigned			  mUID;
		Plat_Unicode::String  mCharacter;
};

//----------------------------------------------
class RGetIssueHierarchy : public Request
//----------------------------------------------
{
	public:
		RGetIssueHierarchy(unsigned track, const Plat_Unicode::String version, const Plat_Unicode::String language);
		virtual void pack(Base::ByteStream &msg);
	private:
		Plat_Unicode::String mVersion;
		Plat_Unicode::String mLanguage;
};

//----------------------------------------------
class RCreateTicket : public Request
//----------------------------------------------
{
	public:
		RCreateTicket(unsigned track, const CSAssistGameAPITicket *ticketBody, const Plat_Unicode::String XMLBody, const unsigned uid);
		virtual void pack(Base::ByteStream &msg);
	private:
		CSAssistGameAPITicket	mTicketStruct;
		Plat_Unicode::String	mXMLBody;
		unsigned				mUID;
};

//----------------------------------------------
class RAppendComment : public Request
//----------------------------------------------
{
	public:
		RAppendComment(unsigned track, const unsigned ticket, const unsigned uid, const Plat_Unicode::String character, const Plat_Unicode::String comment);
		virtual void pack(Base::ByteStream &msg);
	private:
		unsigned			 mTicketID;
		unsigned			 mUID;
		Plat_Unicode::String mComment;
		Plat_Unicode::String mCharacter;
};

//----------------------------------------------
class RGetTicketByID : public Request
//----------------------------------------------
{
	public:
		RGetTicketByID(unsigned track, const unsigned ticket, const unsigned MarkAsRead);
		virtual void pack(Base::ByteStream &msg);
	private:
		unsigned mTicketID;
		unsigned mMarkAsRead;
};

//----------------------------------------------
class RGetTicketComments : public Request
//----------------------------------------------
{
	public:
		RGetTicketComments(unsigned track, const unsigned ticket, const unsigned start, const unsigned count, const unsigned offset);
		virtual void pack(Base::ByteStream &msg);
	private:
		unsigned mTicketID;
		unsigned mStart;
		unsigned mCount;
		unsigned mOffset;
};

//----------------------------------------------
class RGetTicketByCharacter : public Request
//----------------------------------------------
{
	public:
		RGetTicketByCharacter(unsigned track, const unsigned uid, const Plat_Unicode::String character, const unsigned Start, const unsigned Count, const unsigned MarkAsRead);
		virtual void pack(Base::ByteStream &msg);
	private:
		unsigned				mUID;
		Plat_Unicode::String	mCharacter;
		unsigned				mStart;
		unsigned				mCount;
		unsigned				mMarkAsRead;
};

//----------------------------------------------
class RMarkTicketRead : public Request
//----------------------------------------------
{
	public:
		RMarkTicketRead(unsigned track, const unsigned ticket);
		virtual void pack(Base::ByteStream &msg);
	private:
		unsigned mTicketID;
};

//----------------------------------------------
class RCancelTicket : public Request
//----------------------------------------------
{
	public:
		RCancelTicket(unsigned track, const unsigned ticket, const unsigned uid, const Plat_Unicode::String comment);
		virtual void pack(Base::ByteStream &msg);
	private:
		unsigned				mTicketID;
		unsigned				mUID;
		Plat_Unicode::String	mComment;
};

//----------------------------------------------
class RCommentCount : public Request
//----------------------------------------------
{
	public:
		RCommentCount(unsigned track, const unsigned ticket);
		virtual void pack(Base::ByteStream &msg);
	private:
		unsigned mTicketID;
};

//----------------------------------------------
class RReplyGameLocation : public Request
//----------------------------------------------
{
	public:
		RReplyGameLocation(unsigned track, const unsigned sourceTrack, const unsigned uid, const Plat_Unicode::String character, const unsigned csruid, const Plat_Unicode::String locationString);
		virtual void pack(Base::ByteStream &msg);
	private:
		unsigned				mSourceTrack;
		unsigned				mUID;
		Plat_Unicode::String	mCharacter;
		unsigned				mCSRUID;
		Plat_Unicode::String	mLocationString;
};

//----------------------------------------------
class RGetDocumentList : public Request
//----------------------------------------------
{
	public:
		RGetDocumentList(unsigned track, Plat_Unicode::String version, Plat_Unicode::String language);
		virtual void pack(Base::ByteStream &msg);
	private:
		Plat_Unicode::String mVersion;
		Plat_Unicode::String mLanguage;
};

//----------------------------------------------
class RGetDocument : public Request
//----------------------------------------------
{
	public:
		RGetDocument(unsigned track, const unsigned id);
		virtual void pack(Base::ByteStream &msg);
	private:
		unsigned mID;
};

//----------------------------------------------
class RGetTicketXMLBlock : public Request
//----------------------------------------------
{
	public:
		RGetTicketXMLBlock(unsigned track, const unsigned id);
		virtual void pack(Base::ByteStream &msg);
	private:
		unsigned mID;
};

//----------------------------------------------
class RGetKBArticle : public Request
//----------------------------------------------
{
	public:
		RGetKBArticle(unsigned track, const Plat_Unicode::String id, const Plat_Unicode::String language, const unsigned uid);
		virtual void pack(Base::ByteStream &msg);
	private:
		Plat_Unicode::String mID;
		Plat_Unicode::String mLanguage;
		unsigned mUID;
};

//----------------------------------------------
class RSearchKB : public Request
//----------------------------------------------
{
	public:
		RSearchKB(unsigned track, const Plat_Unicode::String searchstring, const Plat_Unicode::String language, const unsigned uid);
		virtual void pack(Base::ByteStream &msg);
	private:
		Plat_Unicode::String mSearchString;
		Plat_Unicode::String mLanguage;
		unsigned mUID;
};

//----------------------------------------------
class RConnectLB : public Request
//----------------------------------------------
{
	public:
		RConnectLB(unsigned track, const std::string connectString);
		virtual void pack(Base::ByteStream &msg);
	private:
		std::string mConnectString;
};

} // namespace CSAssist

#endif
