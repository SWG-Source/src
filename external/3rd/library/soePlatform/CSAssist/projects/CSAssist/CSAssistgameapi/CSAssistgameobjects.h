#ifndef __GAMEOBJECTS_H
#define __GAMEOBJECTS_H

//------------------------------------------------------------------------------
//
// CSAssistgameobjects.h
//
// Header file for objects used by API clients
//
//------------------------------------------------------------------------------

#include "CSAssistgameapi.h"

namespace CSAssist
{

//------------------------------------------------------------------------------
// CSAssistGameAPITicket
//
// This is what a ticket looks like to the client
//------------------------------------------------------------------------------

class CSADLL_EXPORT CSAssistGameAPITicket
{
public:
	CSAssistGameAPITicket();

	void					setBug()			{ bugstatus = BUG_STATUS_BUG; }
	CSAssistGameAPIResult	setUID(CSAssistGameAPIUID u);
	CSAssistGameAPIResult	setGame(const CSAssistUnicodeChar *string);
	CSAssistGameAPIResult	setServer(const CSAssistUnicodeChar *string);
	CSAssistGameAPIResult	setCharacter(const CSAssistUnicodeChar *string);
	CSAssistGameAPIResult	setDetails(const CSAssistUnicodeChar *string);
	CSAssistGameAPIResult	setLanguage(const CSAssistUnicodeChar *string);
	CSAssistGameAPIResult	setLocation(const CSAssistUnicodeChar *string);
	CSAssistGameAPIResult	setCategory(int index, unsigned value);

	CSAssistGameAPITicketID	ticketID;										// unique ID
	CSAssistGameAPIUID		uid;											// station UID
	time_t					createDate;
	time_t					modifiedDate;
	CSAssistUnicodeChar 	game[CSASSIST_TICKET_LENGTH_GAME+1];			// game name
	CSAssistUnicodeChar 	server[CSASSIST_TICKET_LENGTH_SERVER+1];		// server name
	CSAssistUnicodeChar 	character[CSASSIST_TICKET_LENGTH_CHARACTER+1];	// character name
	CSAssistTicketStatus	status;											// status of this ticket (CSAssistTicketStatus)
	CSAssistBugStatus		bugstatus;										// whether this is a bug report (CSAssistBugStatus)
	unsigned				category[CSASSIST_NUM_CATEGORIES];				// 'n' category codes
	CSAssistUnicodeChar		details[CSASSIST_TICKET_LENGTH_DETAILS+1];		// petition text from player
	CSAssistUnicodeChar		language[CSASSIST_TICKET_LENGTH_LANGUAGE+1];	// language code, e.g. en, jp, fr
	unsigned				isRead;											// whether ticket has been read by player or not
	CSAssistUnicodeChar		location[CSASSIST_TICKET_LENGTH_LOCATION+1];	// location string
							// it is recommended that location take the form of:
							// zone: X=xx.xx Y=yy.yy Z=zz.zz
							// where zone is a textual name for a region of the game and X,Y,Z are coords
							// e.g.: 'dreadlands: X=-126.89 Y=1022.67 Z=301.65
};

//------------------------------------------------------------------------------
// CSAssistGameAPITicketComment
//
// This is what a ticket comment looks like to the client
//------------------------------------------------------------------------------

class CSADLL_EXPORT CSAssistGameAPITicketComment
{
public:
	CSAssistGameAPITicketComment();

	CSAssistGameAPITicketID		ticketID;									// ticket to which this comment belongs
	unsigned					commentID;									// unique ID (can be used to sort comments)
	CSAssistGameAPIUID			uid;										// station UID (will be 0 if a CSR comment)
	CSAssistGameAPICommentType	type;										// whether CSR or player comment
	time_t						modifiedDate;								// date of this comment
	CSAssistUnicodeChar 		comment[CSASSIST_TICKET_LENGTH_COMMENT+1];	
	CSAssistUnicodeChar			name[CSASSIST_TICKET_LENGTH_NAME+1];
};

//------------------------------------------------------------------------------
// CSAssistGameAPIDocumentHeader
//
// This is what a DocumentHeader looks like to the client
//------------------------------------------------------------------------------

class CSADLL_EXPORT CSAssistGameAPIDocumentHeader
{
public:
	CSAssistGameAPIDocumentHeader();

	unsigned			id;
	CSAssistUnicodeChar	game[CSASSIST_DOCUMENT_LENGTH_NAME+1];
	CSAssistUnicodeChar	language[CSASSIST_DOCUMENT_LENGTH_LANGUAGE+1];
	CSAssistUnicodeChar	title[CSASSIST_DOCUMENT_LENGTH_TITLE+1];
	CSAssistUnicodeChar	version[CSASSIST_DOCUMENT_LENGTH_GAMESUBCODE+1];
	time_t				modifiedDate;
};

//------------------------------------------------------------------------------
// CSAssistGameAPISearchResult
//
// This is what a Knowledge Base Search Result looks like to the client
//------------------------------------------------------------------------------

class CSADLL_EXPORT CSAssistGameAPISearchResult
{
public:
	CSAssistGameAPISearchResult();

	CSAssistUnicodeChar	title[CSASSIST_KBSEARCH_LENGTH_TITLE+1];
	CSAssistUnicodeChar	idstring[CSASSIST_KBSEARCH_LENGTH_ID+1];
	short				matchPercent;
};

} // namespace CSAssist

#endif
