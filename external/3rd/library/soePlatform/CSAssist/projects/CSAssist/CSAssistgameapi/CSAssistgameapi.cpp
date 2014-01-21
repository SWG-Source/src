//------------------------------------------------------------------------------
//
// CSAssistgameapi.cpp
//
// Author:	Graeme Ing		(ging@soe.sony.com)
// Module:	CSAssist API
//
// copyright 2002 (c) Sony Online Entertainment Inc.
//
// Users layer for API
//
//------------------------------------------------------------------------------
#pragma warning (disable: 4786)


#include "CSAssistgameapi.h"
#include "CSAssistgameapicore.h"

namespace CSAssist
{

//---------------------------
CSAssistGameAPI::CSAssistGameAPI(const char *host, const unsigned port, const unsigned timeout, const unsigned flags)
//---------------------------
{
	core = new CSAssistGameAPIcore(this, host, port, timeout, flags);
}

//---------------------------
CSAssistGameAPI::CSAssistGameAPI(const char *serverList, const unsigned timeout, const unsigned flags)
//---------------------------
{
	core = new CSAssistGameAPIcore(this, serverList, timeout, flags);
}


//---------------------------
CSAssistGameAPI::~CSAssistGameAPI()
//---------------------------
{
	delete core;
}

//---------------------------
void CSAssistGameAPI::Update()
//---------------------------
{
	core->Update();
}

//---------------------------
const char *CSAssistGameAPI::getErrorString(const CSAssistGameAPIResult result)
//---------------------------
{
	switch (result)
	{
		case CSASSIST_RESULT_SUCCESS:			return "CSASSIST_RESULT_SUCCESS";
		case CSASSIST_RESULT_TIMEOUT:			return "CSASSIST_RESULT_TIMEOUT";
		case CSASSIST_RESULT_FAIL:				return "CSASSIST_RESULT_FAIL";
		case CSASSIST_RESULT_BADPARAMS:			return "CSASSIST_RESULT_BADPARAMS";
		case CSASSIST_RESULT_OUTOFMEMORY:		return "CSASSIST_RESULT_OUTOFMEMORY";
		case CSASSIST_RESULT_NOTICKET:			return "CSASSIST_RESULT_NOTICKET";
		case CSASSIST_RESULT_NOCREATE_TICKET:	return "CSASSIST_RESULT_NOCREATE_TICKET";
		case CSASSIST_RESULT_UNKNOWN_USER:		return "CSASSIST_RESULT_UNKNOWN_USER";
		case CSASSIST_RESULT_NOCREATE_COMMENT:	return "CSASSIST_RESULT_NOCREATE_COMMENT";
		case CSASSIST_RESULT_TICKETLOCKED:		return "CSASSIST_RESULT_TICKETLOCKED";
		case CSASSIST_RESULT_DBERROR:			return "CSASSIST_RESULT_DBERROR";
		case CSASSIST_RESULT_TOOMANYOBJECTS:	return "CSASSIST_RESULT_TOOMANYOBJECTS";
		case CSASSIST_RESULT_INVALID_ACCOUNT:	return "CSASSIST_RESULT_INVALID_ACCOUNT";
		case CSASSIST_RESULT_ACCOUNT_CLOSED:	return "CSASSIST_RESULT_ACCOUNT_CLOSED";
		case CSASSIST_RESULT_INVALID_DOCUMENTID:return "CSASSIST_RESULT_INVALID_DOCUMENTID";
		case CSASSIST_RESULT_TICKET_XMLFAILED:	return "CSASSIST_RESULT_TICKET_XMLFAILED";
		case CSASSIST_RESULT_ALREADY_REGISTERED:return "CSASSIST_RESULT_ALREADY_REGISTERED";
		case CSASSIST_RESULT_PRIMUS_ERROR:	return "CSASSIST_RESULT_PRIMUS_ERROR";
		case CSASSIST_RESULT_TOOMANY_FILTERS:	return "CSASSIST_RESULT_TOOMANY_FILTERS";
		case CSASSIST_RESULT_ALREADY_LOGGEDIN:	return "CSASSIST_RESULT_ALREADY_LOGGEDIN";
		case CSASSIST_RESULT_UNKNOWN_FILTER:	return "CSASSIST_RESULT_UNKNWON_FILTER";
		case CSASSIST_RESULT_BAD_VERSION:		return "CSASSIST_RESULT_BAD_VERSION";
		case CSASSIST_RESULT_SERVER_DISCONNECT: return "CSASSIST_RESULT_SERVER_DISCONNECT";
	}
	return "CSASSIST_RESULT_UNKNOWN";
}

//----------------------------------------------------
//
// Override API calls here! **** ADD CODE HERE ****
//
//----------------------------------------------------

//---------------------------
void CSAssistGameAPI::setMaxPacketsPerSecond(const unsigned n)
//---------------------------
{
	core->setMaxPacketsPerSecond(n);
}

//------------------------------------------------------
const char *CSAssistGameAPI::GetConnectedHost()
//------------------------------------------------------
{
	return core->GetConnectedHost();
}

//------------------------------------------------------
int	CSAssistGameAPI::GetConnectedPort()
//------------------------------------------------------
{
	return core->GetConnectedPort();
}

//---------------------------
CSAssistGameAPITrack CSAssistGameAPI::connectCSAssist(const void *userData, const CSAssistUnicodeChar *GameName, const CSAssistUnicodeChar *ServerName)
//---------------------------
{
	return(core->connectCSAssist(userData, GameName, ServerName));
}

//---------------------------
CSAssistGameAPITrack CSAssistGameAPI::disconnectCSAssist(const void *userData)
//---------------------------
{
	return(core->disconnectCSAssist(userData));
}

//---------------------------
CSAssistGameAPITrack CSAssistGameAPI::requestNewTicketActivity(const void *userData, const CSAssistGameAPIUID uid, const CSAssistUnicodeChar *character)
//---------------------------
{
	return(core->requestNewTicketActivity(userData, uid, character));
}

//---------------------------
CSAssistGameAPITrack CSAssistGameAPI::requestRegisterCharacter(const void *userData, const CSAssistGameAPIUID uid, const CSAssistUnicodeChar *character, const unsigned avaconID)
//---------------------------
{
	return(core->requestRegisterCharacter(userData, uid, character, avaconID));
}

//---------------------------
CSAssistGameAPITrack CSAssistGameAPI::requestUnRegisterCharacter(const void *userData, const CSAssistGameAPIUID uid, const CSAssistUnicodeChar *character)
//---------------------------
{
	return(core->requestUnRegisterCharacter(userData, uid, character));
}

//---------------------------
CSAssistGameAPITrack CSAssistGameAPI::requestGetIssueHierarchy(const void *userData, const CSAssistUnicodeChar *version, const CSAssistUnicodeChar *language)
//---------------------------
{
	return(core->requestGetIssueHierarchy(userData, version, language));
}

//---------------------------
CSAssistGameAPITrack CSAssistGameAPI::requestCreateTicket(const void *userData, const CSAssistGameAPITicket *ticketBody, const CSAssistUnicodeChar *XMLBody, const CSAssistGameAPIUID uid)
//---------------------------
{
	return(core->requestCreateTicket(userData, ticketBody, XMLBody, uid));
}

//---------------------------
CSAssistGameAPITrack CSAssistGameAPI::requestAppendTicketComment(const void *userData, const CSAssistGameAPITicketID ticket, const CSAssistGameAPIUID uid, const CSAssistUnicodeChar *character, const CSAssistUnicodeChar *comment)
//---------------------------
{
	return(core->requestAppendTicketComment(userData, ticket, uid, character, comment));
}

//---------------------------
CSAssistGameAPITrack CSAssistGameAPI::requestGetTicketByID(const void *userData, const CSAssistGameAPITicketID ticket, const unsigned FlagAsRead)
//---------------------------
{
	return(core->requestGetTicketByID(userData, ticket, FlagAsRead));
}

//---------------------------
CSAssistGameAPITrack CSAssistGameAPI::requestGetTicketComments(const void *userData, const CSAssistGameAPITicketID ticket, const unsigned Start, const unsigned Count, const CSAssistGameAPIOffset offset)
//---------------------------
{
	return(core->requestGetTicketComments(userData, ticket, Start, Count, offset));
}

//---------------------------
CSAssistGameAPITrack CSAssistGameAPI::requestGetTicketByCharacter(const void *userData, const CSAssistGameAPIUID uid, const CSAssistUnicodeChar * character, const unsigned Start, const unsigned Count, const unsigned FlagAsRead)
//---------------------------
{
	return(core->requestGetTicketByCharacter(userData, uid, character, Start, Count, FlagAsRead));
}

//---------------------------
CSAssistGameAPITrack CSAssistGameAPI::requestMarkTicketRead(const void *userData, const CSAssistGameAPITicketID ticket)
//---------------------------
{
	return(core->requestMarkTicketRead(userData, ticket));
}

//---------------------------
CSAssistGameAPITrack CSAssistGameAPI::requestCancelTicket(const void *userData, const CSAssistGameAPITicketID ticket, const CSAssistGameAPIUID uid, const CSAssistUnicodeChar *comment)
//---------------------------
{
	return(core->requestCancelTicket(userData, ticket, uid, comment));
}

//---------------------------
CSAssistGameAPITrack CSAssistGameAPI::requestGetTicketCommentsCount(const void *userData, const CSAssistGameAPITicketID ticket)
//---------------------------
{
	return(core->requestGetTicketCommentsCount(userData, ticket));
}

//---------------------------
CSAssistGameAPITrack CSAssistGameAPI::replyGameLocation(const void *userData, const CSAssistGameAPITrack sourceTrack, const CSAssistGameAPIUID uid, const CSAssistUnicodeChar *character, const CSAssistGameAPIUID csruid, const CSAssistUnicodeChar *locationString)
//---------------------------
{
	return(core->replyGameLocation(userData, sourceTrack, uid, character, csruid, locationString));
}

//---------------------------
CSAssistGameAPITrack CSAssistGameAPI::requestGetDocumentList(const void *userData, const CSAssistUnicodeChar *version, const CSAssistUnicodeChar *language)
//---------------------------
{
	return(core->requestGetDocumentList(userData, version, language));
}

//---------------------------
CSAssistGameAPITrack CSAssistGameAPI::requestGetDocument(const void *userData, const unsigned id)
//---------------------------
{
	return(core->requestGetDocument(userData, id));
}

//---------------------------
CSAssistGameAPITrack CSAssistGameAPI::requestGetTicketXMLBlock(const void *userData, const CSAssistGameAPITicketID id)
//---------------------------
{
	return(core->requestGetTicketXMLBlock(userData, id));
}

//---------------------------
CSAssistGameAPITrack CSAssistGameAPI::requestGetKBArticle(const void *userData, const CSAssistUnicodeChar *id, const CSAssistUnicodeChar *language, const CSAssistGameAPIUID uid)
//---------------------------
{
	return(core->requestGetKBArticle(userData, id, language, uid));
}

//---------------------------
CSAssistGameAPITrack CSAssistGameAPI::requestSearchKB(const void *userData, const CSAssistUnicodeChar *searchstring, const CSAssistUnicodeChar *language, const CSAssistGameAPIUID uid)
//---------------------------
{
	return(core->requestSearchKB(userData, searchstring, language, uid));
}

//----------------------------------------------------
//
// Override API callbacks here! **** ADD CODE HERE ****
//
//----------------------------------------------------

//---------------------------
void CSAssistGameAPI::OnConnectCSAssist(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData)
//---------------------------
{
}

//---------------------------
void CSAssistGameAPI::OnConnectRejectedCSAssist(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData)
//---------------------------
{
}

//---------------------------
void CSAssistGameAPI::OnDisconnectCSAssist(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData)
//---------------------------
{
}

//---------------------------
void CSAssistGameAPI::OnNewTicketActivity(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData, const unsigned NewActivityFlag, const unsigned HasTickets)
//---------------------------
{
}

//---------------------------
void CSAssistGameAPI::OnRegisterCharacter(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData)
//---------------------------
{
}

//---------------------------
void CSAssistGameAPI::OnUnRegisterCharacter(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData)
//---------------------------
{
}

//---------------------------
void CSAssistGameAPI::OnGetIssueHierarchy(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData, const CSAssistGameAPIXML hierarchyBody, const unsigned modifyDate)
//---------------------------
{
}

//---------------------------
void CSAssistGameAPI::OnCreateTicket(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData, const CSAssistGameAPITicketID ticket)
//---------------------------
{
}

//---------------------------
void CSAssistGameAPI::OnAppendTicketComment(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData, const CSAssistGameAPITicketID ticket)
//---------------------------
{
}

//---------------------------
void CSAssistGameAPI::OnGetTicketByID(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData, const CSAssistGameAPITicket *ticketBody)
//---------------------------
{
}

//---------------------------
void CSAssistGameAPI::OnGetTicketComments(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData, const CSAssistGameAPITicketComment *comments, const unsigned numberRead)
//---------------------------
{
}

//---------------------------
void CSAssistGameAPI::OnGetTicketByCharacter(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData, const unsigned NumberReturned, const unsigned TotalNumber, const CSAssistGameAPITicket *ticketBody)
//---------------------------
{
}

//---------------------------
void CSAssistGameAPI::OnTicketChange(const CSAssistGameAPITicketID ticketID, const CSAssistGameAPIUID uid, const CSAssistUnicodeChar *character)
//---------------------------
{
}

//---------------------------
void CSAssistGameAPI::OnMarkTicketRead(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData)
//---------------------------
{
}

//---------------------------
void CSAssistGameAPI::OnCancelTicket(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData, const CSAssistGameAPITicketID ticket)
//---------------------------
{
}

//---------------------------
void CSAssistGameAPI::OnGetTicketCommentsCount(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData, const CSAssistGameAPITicketID ticket, const unsigned count)
//---------------------------
{
}

//---------------------------
void CSAssistGameAPI::OnRequestGameLocation(const CSAssistGameAPITrack sourceTrack, const CSAssistGameAPIUID uid, const CSAssistUnicodeChar *character, const CSAssistGameAPIUID CSRUID)
//---------------------------
{
}

//---------------------------
void CSAssistGameAPI::OnGetDocumentList(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData, const CSAssistGameAPIDocumentHeader *docList, const unsigned numberRead)
//---------------------------
{
}

//---------------------------
void CSAssistGameAPI::OnGetDocument(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData, const CSAssistUnicodeChar *documentBody)
//---------------------------
{
}

//---------------------------
void CSAssistGameAPI::OnGetTicketXMLBlock(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData, const CSAssistGameAPIXML XMLBody)
//---------------------------
{
}

//---------------------------
void CSAssistGameAPI::OnGetKBArticle(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData, const CSAssistGameAPIXML articleBody, const CSAssistUnicodeChar *articleTitle)
//---------------------------
{
}

//---------------------------
void CSAssistGameAPI::OnSearchKB(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData, const CSAssistGameAPISearchResult *results, const unsigned numberRead)
//---------------------------
{
}

//---------------------------
void CSAssistGameAPI::OnIssueHierarchyChanged(const CSAssistUnicodeChar *version, const CSAssistUnicodeChar *language, const CSAssistGameAPIHierarchyChangeType type)
//---------------------------
{
}

} // namespace CSAssist

