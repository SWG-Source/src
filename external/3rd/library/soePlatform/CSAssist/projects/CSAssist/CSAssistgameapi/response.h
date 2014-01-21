#ifndef RESPONSE_H
#define RESPONSE_H

//------------------------------------------------------------------------------
//
// response.h
//
// Author:	Graeme Ing		(ging@soe.sony.com)
// Module:	CSAssist API
//
// copyright 2002 (c) Sony Online Entertainment Inc.
//
// Class definitions for all response objects from servers
//
//------------------------------------------------------------------------------

#include "Base/Archive.h"
#include "CSAssistgameapicore.h"
#include "request.h"

namespace CSAssist
{

//------------------------------------------------
class Response 
//------------------------------------------------
{
    public:
		Response(unsigned type, unsigned track, const void *muserData);
		virtual ~Response();
	    unsigned getType()									{ return (unsigned)mType; }
	    virtual CSAssistGameAPITrack getTrack()				{ return (CSAssistGameAPITrack)mTrack; }
		virtual CSAssistGameAPIResult getResult()			{ return (CSAssistGameAPIResult)mResult; }
		inline void *getUserData()							{ return muserData; }
		inline void init(short type, unsigned track, unsigned result)
															{ mType = type; mTrack = track; mResult = result; }
		inline void setResult(unsigned result)				{ mResult = result; }

		virtual void decode(Base::ByteStream::ReadIterator &msg);

    protected:
		short		mType;
	    unsigned	mTrack;
		unsigned	mResult;
		void		*muserData;


};

//----------------------------------------------
//
// Add new Response objects for each API call here
//
// **** ADD CODE HERE ****
//----------------------------------------------

//------------------------------------------------
class ResConnectCSAssist : public Response
//------------------------------------------------
{
	public:
		ResConnectCSAssist(unsigned track, const void *userData);
	private:
};

//------------------------------------------------
class ResDisconnectCSAssist : public Response
//------------------------------------------------
{
	public:
		ResDisconnectCSAssist(unsigned track, const void *userData);
	private:
};

//------------------------------------------------
class ResNewTicketActivity : public Response
//------------------------------------------------
{
	public:
		ResNewTicketActivity(unsigned track, const void *userData);
		~ResNewTicketActivity();
	    inline unsigned getNewActivityFlag()	{ return mNewActivityFlag; }
	    inline unsigned getHasTickets()			{ return mHasTickets; }
		virtual void decode(Base::ByteStream::ReadIterator &msg);
	private:
		unsigned mNewActivityFlag;
		unsigned mHasTickets;
};

//------------------------------------------------
class ResRegisterCharacter : public Response
//------------------------------------------------
{
	public:
		ResRegisterCharacter(unsigned track, const void *userData);
	private:
};

//------------------------------------------------
class ResUnRegisterCharacter : public Response
//------------------------------------------------
{
	public:
		ResUnRegisterCharacter(unsigned track, const void *userData);
	private:
};


//------------------------------------------------
class ResGetIssueHierarchy : public Response
//------------------------------------------------
{
	public:
		ResGetIssueHierarchy(unsigned track, const void *userData);
		virtual ~ResGetIssueHierarchy();
		inline CSAssistUnicodeChar *getHierarchyBody()	{ return rawStringPtr; }
		inline unsigned getModifyDate()					{ return mModifyDate; }
		virtual void decode(Base::ByteStream::ReadIterator &msg);
	private:
		Plat_Unicode::String	mHierarchyBody;
		unsigned				mModifyDate;
		CSAssistUnicodeChar	*rawStringPtr;
};

//------------------------------------------------
class ResCreateTicket : public Response
//------------------------------------------------
{
	public:
		ResCreateTicket(unsigned track, const void *userData);
		~ResCreateTicket();
	    inline unsigned getTicketID()		{ return mTicketID; }
		virtual void decode(Base::ByteStream::ReadIterator &msg);
	private:
		unsigned	mTicketID;
};

//------------------------------------------------
class ResAppendComment : public Response
//------------------------------------------------
{
	public:
		ResAppendComment(unsigned track, const void *userData);
		~ResAppendComment();
	    inline unsigned getTicketID()		{ return mTicketID; }
		virtual void decode(Base::ByteStream::ReadIterator &msg);
	private:
		unsigned mTicketID;
};

//------------------------------------------------
class ResGetTicketByID : public Response
//------------------------------------------------
{
	public:
		ResGetTicketByID(unsigned track, const void *userData);
		~ResGetTicketByID();
		inline CSAssistGameAPITicket &getTicketBody()		{ return (CSAssistGameAPITicket &)mTicketBody; }
		virtual void decode(Base::ByteStream::ReadIterator &msg);
	private:
		CSAssistGameAPITicket mTicketBody;
};

//------------------------------------------------
class ResGetTicketComments : public Response
//------------------------------------------------
{
	public:
		ResGetTicketComments(unsigned track, const void *userData);
		~ResGetTicketComments();
	    inline unsigned getNumberRead()							{ return mNumberRead; }
		inline CSAssistGameAPITicketComment *getCommentArray()	{ return mCommentArray; }
		virtual void decode(Base::ByteStream::ReadIterator &msg);

	private:
		unsigned					 mNumberRead;
		CSAssistGameAPITicketComment *mCommentArray;
};

//------------------------------------------------
class ResGetTicketByCharacter : public Response
//------------------------------------------------
{
	public:
		ResGetTicketByCharacter(unsigned track, const void *userData);
		~ResGetTicketByCharacter();
	    inline unsigned getNumberReturned()				{ return mNumberReturned; }
	    inline unsigned getTotalNumber()				{ return mTotalNumber; }
		inline CSAssistGameAPITicket *getTicketArray()	{ return mTicketArray; }
		virtual void decode(Base::ByteStream::ReadIterator &msg);

	private:
		unsigned				mNumberReturned;
		unsigned				mTotalNumber;
		CSAssistGameAPITicket	*mTicketArray;
};

//------------------------------------------------
class ResTicketChange : public Response
//------------------------------------------------
{
	public:
		ResTicketChange(unsigned track);
		~ResTicketChange();
	    inline unsigned getTicketID()				{ return mTicketID; }
	    inline unsigned getUID()					{ return mUID; }
		inline CSAssistUnicodeChar *getCharacter()	{ return rawCharacter; }
		virtual void decode(Base::ByteStream::ReadIterator &msg);
	private:
		unsigned				mTicketID;
		unsigned				mUID;
		Plat_Unicode::String	mCharacter;
		CSAssistUnicodeChar	*rawCharacter;
};

//------------------------------------------------
class ResMarkTicketRead : public Response
//------------------------------------------------
{
	public:
		ResMarkTicketRead(unsigned track, const void *userData);
	private:
};

//------------------------------------------------
class ResCancelTicket : public Response
//------------------------------------------------
{
	public:
		ResCancelTicket(unsigned track, const void *userData);
		~ResCancelTicket();
	    inline unsigned getTicketID()		{ return mTicketID; }
		virtual void decode(Base::ByteStream::ReadIterator &msg);
	private:
		unsigned mTicketID;
};

//------------------------------------------------
class ResCommentCount : public Response
//------------------------------------------------
{
	public:
		ResCommentCount(unsigned track, const void *userData);
		~ResCommentCount();
	    inline unsigned getTicketID()		{ return mTicketID; }
		inline unsigned getCount()			{ return mCount; };
		virtual void decode(Base::ByteStream::ReadIterator &msg);
	private:
		unsigned mTicketID;
		unsigned mCount;
};

//------------------------------------------------
class ResRequestGameLocation : public Response
//------------------------------------------------
{
	public:
		ResRequestGameLocation(unsigned track);
		~ResRequestGameLocation();
	    inline unsigned getSourceTrack()			{ return mSourceTrack; }
	    inline unsigned getUID()					{ return mUID; }
		inline CSAssistUnicodeChar *getCharacter()	{ return rawCharacter; }
	    inline unsigned getCSRUID()					{ return mCSRUID; }
		virtual void decode(Base::ByteStream::ReadIterator &msg);
	private:
		unsigned				mSourceTrack;
		unsigned				mUID;
		Plat_Unicode::String	mCharacter;
		unsigned				mCSRUID;
		CSAssistUnicodeChar	*rawCharacter;
};

//------------------------------------------------
class ResReplyGameLocation : public Response
//------------------------------------------------
{
	public:
		ResReplyGameLocation(unsigned track, const void *userData);
	private:
};

//------------------------------------------------
class ResGetDocumentList : public Response
//------------------------------------------------
{
	public:
		ResGetDocumentList(unsigned track, const void *userData);
		virtual ~ResGetDocumentList();
	    inline unsigned getNumberRead()				{ return mNumberRead; }
		inline CSAssistGameAPIDocumentHeader *getDocumentList()	{ return mDocumentArray; }
		virtual void decode(Base::ByteStream::ReadIterator &msg);

	private:
		unsigned mNumberRead;
		CSAssistGameAPIDocumentHeader *mDocumentArray;
};

//------------------------------------------------
class ResGetDocument : public Response
//------------------------------------------------
{
	public:
		ResGetDocument(unsigned track, const void *userData);
		virtual ~ResGetDocument();
		inline CSAssistUnicodeChar *getDocumentBody()	{ return rawDocument; }
		virtual void decode(Base::ByteStream::ReadIterator &msg);
	private:
		Plat_Unicode::String mDocumentBody;
		CSAssistUnicodeChar	*rawDocument;
};

//------------------------------------------------
class ResGetTicketXMLBlock : public Response
//------------------------------------------------
{
	public:
		ResGetTicketXMLBlock(unsigned track, const void *userData);
		~ResGetTicketXMLBlock();
		inline CSAssistUnicodeChar *getXMLBody()	{ return rawXML; }
		virtual void decode(Base::ByteStream::ReadIterator &msg);
	private:
		Plat_Unicode::String mXMLBody;
		CSAssistUnicodeChar *rawXML;
};

//------------------------------------------------
class ResGetKBArticle : public Response
//------------------------------------------------
{
	public:
		ResGetKBArticle(unsigned track, const void *userData);
		~ResGetKBArticle();
		inline CSAssistUnicodeChar *getArticleBody()	{ return rawArticle; }
		inline CSAssistUnicodeChar *getArticleTitle()	{ return rawTitle; }
		virtual void decode(Base::ByteStream::ReadIterator &msg);
	private:
		Plat_Unicode::String mArticleBody;
		CSAssistUnicodeChar *rawArticle;
		Plat_Unicode::String mArticleTitle;
		CSAssistUnicodeChar *rawTitle;
};

//------------------------------------------------
class ResSearchKB : public Response
//------------------------------------------------
{
	public:
		ResSearchKB(unsigned track, const void *userData);
		~ResSearchKB();
	    inline unsigned getNumberRead()							{ return mNumberRead; }
		inline CSAssistGameAPISearchResult *getArticleList()	{ return mArticleArray; }
		virtual void decode(Base::ByteStream::ReadIterator &msg);

	private:
		unsigned					mNumberRead;
		CSAssistGameAPISearchResult	*mArticleArray;
};

//------------------------------------------------
class ResHierarchyChange : public Response
//------------------------------------------------
{
	public:
		ResHierarchyChange(unsigned track);
		~ResHierarchyChange();
		inline CSAssistUnicodeChar *getGame()			{ return rawGame; }
		inline CSAssistUnicodeChar *getVersion()		{ return rawVersion; }
		inline CSAssistUnicodeChar *getLanguage()		{ return rawLanguage; }
		inline CSAssistGameAPIHierarchyChangeType getChangeType() { return (CSAssistGameAPIHierarchyChangeType)mChangeType; }
		virtual void decode(Base::ByteStream::ReadIterator &msg);
	private:
		Plat_Unicode::String mGame;
		Plat_Unicode::String mVersion;
		Plat_Unicode::String mLanguage;
		CSAssistUnicodeChar *rawGame;
		CSAssistUnicodeChar *rawVersion;
		CSAssistUnicodeChar *rawLanguage;
		unsigned mChangeType;
};

//------------------------------------------------
class ResConnectLB : public Response
//------------------------------------------------
{
	public:
		ResConnectLB(unsigned track, Request *req, Response *res);
		~ResConnectLB();
		virtual void		decode(Base::ByteStream::ReadIterator &msg);
		inline std::string	getServerName() { return mServerName; }
		inline unsigned		getServerPort() { return mServerPort; }
		Request				*getRequest() { return mConnectRequest; }
		Response			*getResponse() { return mConnectResponse; }

	private:
		std::string		mServerName;
		unsigned short	mServerPort;
		Request			*mConnectRequest;
		Response		*mConnectResponse;

};

} // namespace CSAssist
	
#endif

