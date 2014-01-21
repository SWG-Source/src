#ifndef __CSASSISTGAMEAPI_H
#define __CSASSISTGAMEAPI_H

//------------------------------------------------------------------------------
//
// CSAssistgameapi.h
//
// Author:	Graeme Ing		(ging@soe.sony.com)
// Module:	CSAssist API
//
// copyright 2002 (c) Sony Online Entertainment Inc.
//
// Main Header file for use by the API client
//
//------------------------------------------------------------------------------
#if defined(BUILD_CSADLL)
#define CSADLL_EXPORT __declspec(dllexport)
#elif defined(USE_CSADLL)
#define CSADLL_EXPORT __declspec(dllimport)
#else
#define CSADLL_EXPORT
#endif


#include "time.h"

namespace CSAssist
{

//------------------------------------------------------------------------------
// Define some API types here to identify CSAssist variables
//------------------------------------------------------------------------------

typedef unsigned  CSAssistGameAPITrack;			// An API tracking number
typedef unsigned  CSAssistGameAPIUID;			// A station UID
typedef unsigned  CSAssistGameAPITicketID;		// A ticket ID
typedef unsigned short CSAssistUnicodeChar;		// base type for unicode strings (uint16)
typedef CSAssistUnicodeChar *CSAssistGameAPIXML;// A pointer to an XML block

//------------------------------------------------------------------------------
// Most API callbacks will return one of these result codes
// be sure to check for CSASSIST_RESULT_SUCCESS
//------------------------------------------------------------------------------

CSADLL_EXPORT enum CSAssistGameAPIResult						// Result codes from all API calls
{
	CSASSIST_RESULT_SUCCESS	= 0,				// 0  operation was successful
	CSASSIST_RESULT_TIMEOUT,					// 1  operation resulted in an API side timeout
	CSASSIST_RESULT_FAIL,						// 2  general failure
	CSASSIST_RESULT_BADPARAMS,					// 3  Client passed in bad parameters (strings too long etc)
	CSASSIST_RESULT_OUTOFMEMORY,				// 4  failed to allocate some memory somehow
	CSASSIST_RESULT_NOTICKET,					// 5  ticket not found, probably illegal ticket ID
	CSASSIST_RESULT_NOCREATE_TICKET,			// 6  failed to create ticket
	CSASSIST_RESULT_UNKNOWN_USER,				// 7  request cannot be made by an anonymous user (user connectCSAssist)
	CSASSIST_RESULT_NOCREATE_COMMENT,			// 8  failed to create ticket comment
	CSASSIST_RESULT_TICKETLOCKED,				// 9  ticked locked by another CSR
	CSASSIST_RESULT_DBERROR,					// 10 operation failed due to a database problem
	CSASSIST_RESULT_TOOMANYOBJECTS,				// 11 trying to obtain too many tickets/comments!
	CSASSIST_RESULT_INVALID_ACCOUNT,			// 12 unvalidated user, probably wrong login name and/or password
	CSASSIST_RESULT_ACCOUNT_CLOSED,				// 13 validated account but marked closed/banned
	CSASSIST_RESULT_INVALID_DOCUMENTID,			// 14 document ID requested does not exist
	CSASSIST_RESULT_TICKET_XMLFAILED,			// 15 failed to write XML block when creating a ticket.
												//    main ticket was still created, so this error *could* be ignored
	CSASSIST_RESULT_ALREADY_REGISTERED,			// 16 player character/UID was already registered for this game/server
												//    he will remain registered, but this indicates a potential application bug
	CSASSIST_RESULT_PRIMUS_ERROR,				// 17 Primus Knowledge Base returned an error, or malformed XML returned
	CSASSIST_RESULT_TOOMANY_FILTERS,			// 18 CSR cannot have any more filters
	CSASSIST_RESULT_ALREADY_LOGGEDIN,			// 19 CSR or game server is already connected and cannot have multiple connections
	CSASSIST_RESULT_UNKNOWN_FILTER,				// 20 Tried to manipulate or remove a filter that does not exist
	CSASSIST_RESULT_BAD_VERSION,				// 21 Incorrect API for this server, probably an old API!
	CSASSIST_RESULT_SERVER_DISCONNECT			// 22 The API was disconnected from the backend. Not a user requested disconnect.
};

CSADLL_EXPORT enum CSAssistGameAPIOffset						// offset for First/Last type commands
{
	CSASSIST_OFFSET_START	= 0,
	CSASSIST_OFFSET_END		= 1
};

CSADLL_EXPORT enum CSAssistTicketStatus
{
	TICKET_STATUS_NEWTICKET = 0,
	TICKET_STATUS_UNUSED,						// pad
	TICKET_STATUS_WAIT_PLAYER,					// waiting for response from player
	TICKET_STATUS_WAIT_CSR,						// waiting for response from CSR
	TICKET_STATUS_PENDING_CLOSED,				// CSR has closed ticket, awaiting final read by player first
	TICKET_STATUS_CLOSED						// closed MUST remain at 5 for DB stored procs!
};

CSADLL_EXPORT enum CSAssistBugStatus
{
	BUG_STATUS_NONE = 0,						// not a bug
	BUG_STATUS_BUG,								// A bug but not assigned another category yet
	BUG_STATUS_REPEATABLE,
	BUG_STATUS_UNREPEATABLE,
	BUG_STATUS_PENDING,
	BUG_STATUS_DUPE,
	BUG_STATUS_CLOSED
};

CSADLL_EXPORT enum CSAssistGameAPICommentType
{
	CSASSIST_COMMENT_NONE = 0,
	CSASSIST_COMMENT_FROM_CSR,
	CSASSIST_COMMENT_FROM_PLAYER
};

CSADLL_EXPORT enum CSAssistGameAPIHierarchyChangeType
{
	CSASSIST_HIERARCHY_NONE = 0,
	CSASSIST_HIERARCHY_ADDED,
	CSASSIST_HIERARCHY_UPDATED,
	CSASSIST_HIERARCHY_DELETED
};

//------------------------------------------------------------------------------
// Flags passed to the instantiation of the API
//------------------------------------------------------------------------------

static const unsigned	CSASSIST_APIFLAG_ASSUME_RECONNECT = 0x001;
// If communication is lost with the server, the API may have already queued
// up many requests. This flag determines whether they will be immediately
// returned as an error to the calling application, or left on their original
// timeout, to see if a server reconnect occurs quickly.
//      Set this flag to let the reconnect try to happen. Connection to the server
// may reoccur with 5 seconds which is substantially less than the usual
// timeout values of 60 secs or more.
//      Default is for this flag NOT to be set. All queued requests will be
// immediately timed out.

static const unsigned	CSASSIST_APIFLAG_NO_REDIRECT = 0x002;
// This flag will not make the API make one hop via a RedirectServer to connect
// to the actual CSAssist backend. If the RECONNECT flag above is set and this is not,
// the API will always try to first connect to a RedirectServer to re-connect
// to CSAssist. 
//		Defailt is for this flag NOT to be set. All connect requests will attempt
// to connect to a RedirectServer initially.
 
//------------------------------------------------------------------------------
// String lengths within objects
//------------------------------------------------------------------------------

static const unsigned	CSASSIST_TICKET_LENGTH_GAME = 25;
static const unsigned	CSASSIST_TICKET_LENGTH_SERVER = 50;
static const unsigned	CSASSIST_TICKET_LENGTH_CHARACTER = 64;
static const unsigned	CSASSIST_TICKET_LENGTH_DETAILS = 4000;
static const unsigned	CSASSIST_TICKET_LENGTH_LANGUAGE = 2;
static const unsigned	CSASSIST_TICKET_LENGTH_COMMENT = 4000;
static const unsigned	CSASSIST_TICKET_LENGTH_NAME = 64;
static const unsigned	CSASSIST_TICKET_LENGTH_LOCATION = 128;

static const short	CSASSIST_NUM_CATEGORIES = 4;

static const unsigned	CSASSIST_DOCUMENT_LENGTH_NAME = 25;
static const unsigned	CSASSIST_DOCUMENT_LENGTH_LANGUAGE = 2;
static const unsigned	CSASSIST_DOCUMENT_LENGTH_TITLE = 64;
static const unsigned	CSASSIST_DOCUMENT_LENGTH_GAMESUBCODE = 50;

static const unsigned	CSASSIST_KBSEARCH_LENGTH_TITLE = 255;
static const unsigned	CSASSIST_KBSEARCH_LENGTH_ID	 = 64;

class CSAssistGameAPIcore;
class CSAssistGameAPITicket;
class CSAssistGameAPITicketComment;
class CSAssistGameAPIDocumentHeader;
class CSAssistGameAPISearchResult;

//---------------------------
class CSADLL_EXPORT CSAssistGameAPI
//---------------------------
{
public:
	CSAssistGameAPI(const char *host, const unsigned port, const unsigned timeout, const unsigned flags);
	CSAssistGameAPI(const char *serverList, const unsigned timeout = 0, const unsigned flags = CSASSIST_APIFLAG_ASSUME_RECONNECT);
	virtual ~CSAssistGameAPI();

	// **** ADD CODE HERE ****
	CSAssistGameAPITrack connectCSAssist(const void *userData, const CSAssistUnicodeChar *GameName, const CSAssistUnicodeChar *ServerName);
	CSAssistGameAPITrack disconnectCSAssist(const void *userData);
	CSAssistGameAPITrack requestNewTicketActivity(const void *userData, const CSAssistGameAPIUID uid, const CSAssistUnicodeChar *character);
	CSAssistGameAPITrack requestRegisterCharacter(const void *userData, const CSAssistGameAPIUID uid, const CSAssistUnicodeChar *character, const unsigned avaconID);
	CSAssistGameAPITrack requestUnRegisterCharacter(const void *userData, const CSAssistGameAPIUID uid, const CSAssistUnicodeChar *character);
	CSAssistGameAPITrack requestGetIssueHierarchy(const void *userData, const CSAssistUnicodeChar *version, const CSAssistUnicodeChar *language);
	CSAssistGameAPITrack requestCreateTicket(const void *userData, const CSAssistGameAPITicket *ticketBody, const CSAssistUnicodeChar *XMLBody, const CSAssistGameAPIUID uid);
	CSAssistGameAPITrack requestAppendTicketComment(const void *userData, const CSAssistGameAPITicketID ticket, const CSAssistGameAPIUID uid, const CSAssistUnicodeChar *character, const CSAssistUnicodeChar *comment);
	CSAssistGameAPITrack requestGetTicketByID(const void *userData, const CSAssistGameAPITicketID ticket, const unsigned FlagAsRead);
	CSAssistGameAPITrack requestGetTicketComments(const void *userData, const CSAssistGameAPITicketID ticket, const unsigned Start, const unsigned Count, const CSAssistGameAPIOffset offset);
	CSAssistGameAPITrack requestGetTicketByCharacter(const void *userData, const CSAssistGameAPIUID uid, const CSAssistUnicodeChar *character, const unsigned Start, const unsigned Count, const unsigned FlagAsRead);
	CSAssistGameAPITrack requestMarkTicketRead(const void *userData, const CSAssistGameAPITicketID ticket);
	CSAssistGameAPITrack requestCancelTicket(const void *userData, const CSAssistGameAPITicketID ticket, const CSAssistGameAPIUID uid, const CSAssistUnicodeChar *comment);
	CSAssistGameAPITrack requestGetTicketCommentsCount(const void *userData, const CSAssistGameAPITicketID ticket);
	CSAssistGameAPITrack replyGameLocation(const void *userData, const CSAssistGameAPITrack sourceTrack, const CSAssistGameAPIUID uid, const CSAssistUnicodeChar *character, const CSAssistGameAPIUID csruid, const CSAssistUnicodeChar *locationString);
	CSAssistGameAPITrack requestGetDocumentList(const void *userData, const CSAssistUnicodeChar *version, const CSAssistUnicodeChar *language);
	CSAssistGameAPITrack requestGetDocument(const void *userData, const unsigned id);
	CSAssistGameAPITrack requestGetTicketXMLBlock(const void *userData, const CSAssistGameAPITicketID id);
	CSAssistGameAPITrack requestGetKBArticle(const void *userData, const CSAssistUnicodeChar *id, const CSAssistUnicodeChar *language, const CSAssistGameAPIUID uid);
	CSAssistGameAPITrack requestSearchKB(const void *userData, const CSAssistUnicodeChar *searchstring, const CSAssistUnicodeChar *language, const CSAssistGameAPIUID uid);
	void				 setMaxPacketsPerSecond(const unsigned n);

	// ----- remember to copy off any pointer based variables for your own use -----
	// ----- when the callback returns, this data will be deleted!! -----

	virtual void OnConnectCSAssist(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData);
	virtual void OnConnectRejectedCSAssist(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData);
	virtual void OnDisconnectCSAssist(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData);
	virtual void OnNewTicketActivity(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData, const unsigned NewActivityFlag, const unsigned HasTickets);
	virtual void OnRegisterCharacter(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData);
	virtual void OnUnRegisterCharacter(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData);
	virtual void OnGetIssueHierarchy(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData, const CSAssistGameAPIXML hierarchyBody, const unsigned modifyDate);
	virtual void OnCreateTicket(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData, const CSAssistGameAPITicketID ticket);
	virtual void OnAppendTicketComment(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData, const CSAssistGameAPITicketID ticket);
	virtual void OnGetTicketByID(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData, const CSAssistGameAPITicket *ticketBody);
	virtual void OnGetTicketComments(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData, const CSAssistGameAPITicketComment *comments, const unsigned numberRead);
	virtual void OnGetTicketByCharacter(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData, const unsigned NumberReturned, const unsigned TotalNumber, const CSAssistGameAPITicket *ticketBody);
	virtual void OnTicketChange(const CSAssistGameAPITicketID ticketID, const CSAssistGameAPIUID uid, const CSAssistUnicodeChar *character);
	virtual void OnMarkTicketRead(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData);
	virtual void OnCancelTicket(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData, const CSAssistGameAPITicketID ticket);
	virtual void OnGetTicketCommentsCount(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData, const CSAssistGameAPITicketID ticket, const unsigned count);
	virtual void OnRequestGameLocation(const CSAssistGameAPITrack sourceTrack, const CSAssistGameAPIUID uid, const CSAssistUnicodeChar *character, const CSAssistGameAPIUID CSRUID);
	virtual void OnGetDocumentList(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData, const CSAssistGameAPIDocumentHeader *docList, const unsigned numberRead);
	virtual void OnGetDocument(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData, const CSAssistUnicodeChar *documentBody);
	virtual void OnGetTicketXMLBlock(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData, const CSAssistGameAPIXML XMLBody);
	virtual void OnGetKBArticle(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData, const CSAssistGameAPIXML articleBody, const CSAssistUnicodeChar *articleTitle);
	virtual void OnSearchKB(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData, const CSAssistGameAPISearchResult *results, const unsigned numberRead);
	virtual void OnIssueHierarchyChanged(const CSAssistUnicodeChar *version, const CSAssistUnicodeChar *language, const CSAssistGameAPIHierarchyChangeType type);

	void Update();
	const char *getErrorString(const CSAssistGameAPIResult result);
	const char *GetConnectedHost();
	int			GetConnectedPort();
	
private:
	CSAssistGameAPIcore *core;
};

} // namespace CSAssist

#endif

