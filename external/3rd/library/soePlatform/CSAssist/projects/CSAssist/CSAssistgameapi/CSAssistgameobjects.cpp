//------------------------------------------------------------------------------
//
// CSAssistgameobjects.cpp
//
// Class member functions for user objects
//
//------------------------------------------------------------------------------

#include "CSAssistgameobjects.h"
#include <stdio.h>
#include <string.h>
#include <Unicode/UnicodeUtils.h>

namespace CSAssist
{

using namespace Plat_Unicode;

extern CSAssistUnicodeChar *get_c_str(Plat_Unicode::String s);
extern CSAssistUnicodeChar *copy_c_str(Plat_Unicode::String s, CSAssistUnicodeChar *dest);

//-----------------------------------------------------------
unsigned UnicodeLength(const CSAssistUnicodeChar *string)
// Return the character length of a raw uint16 string
//-----------------------------------------------------------
{
	CSAssistUnicodeChar *p = const_cast<CSAssistUnicodeChar *>(string);
	unsigned count = 0;
	while (*(p++) != 0)
		count++;
	return count;
}

//-----------------------------------------------------------
CSAssistGameAPITicket::CSAssistGameAPITicket() :
ticketID(0), uid(0), createDate(0), modifiedDate(0), status(TICKET_STATUS_NEWTICKET),
bugstatus(BUG_STATUS_NONE), isRead(0)
//-----------------------------------------------------------
{
	game[0]			= 0;
	server[0]		= 0;
	character[0]	= 0;
	details[0]		= 0;
	Plat_Unicode::String lang = narrowToWide("en");
	copy_c_str(lang, language);
	location[0]		= 0;
	for (short i=0; i < CSASSIST_NUM_CATEGORIES; i++)
		category[i] = 0;
}

//-----------------------------------------------------------
CSAssistGameAPIResult CSAssistGameAPITicket::setUID(CSAssistGameAPIUID u)
//-----------------------------------------------------------
{
	if (u == 0)
		return CSASSIST_RESULT_BADPARAMS;
	uid = u;
	return CSASSIST_RESULT_SUCCESS;
}

//-----------------------------------------------------------
CSAssistGameAPIResult CSAssistGameAPITicket::setGame(const CSAssistUnicodeChar *string)
//-----------------------------------------------------------
{
	unsigned length = UnicodeLength(string);
	if (length > CSASSIST_TICKET_LENGTH_GAME)
		return CSASSIST_RESULT_BADPARAMS;
	memcpy(game, string, (length+1) * sizeof(CSAssistUnicodeChar));
	return CSASSIST_RESULT_SUCCESS;
}

//-----------------------------------------------------------
CSAssistGameAPIResult CSAssistGameAPITicket::setServer(const CSAssistUnicodeChar *string)
//-----------------------------------------------------------
{
	unsigned length = UnicodeLength(string);
	if (length > CSASSIST_TICKET_LENGTH_SERVER)
		return CSASSIST_RESULT_BADPARAMS;
	memcpy(server, string, (length+1) * sizeof(CSAssistUnicodeChar));
	return CSASSIST_RESULT_SUCCESS;
}

//-----------------------------------------------------------
CSAssistGameAPIResult CSAssistGameAPITicket::setCharacter(const CSAssistUnicodeChar *string)
//-----------------------------------------------------------
{
	unsigned length = UnicodeLength(string);
	if (length > CSASSIST_TICKET_LENGTH_CHARACTER)
		return CSASSIST_RESULT_BADPARAMS;
	memcpy(character, string, (length+1) * sizeof(CSAssistUnicodeChar));
	return CSASSIST_RESULT_SUCCESS;
}

//-----------------------------------------------------------
CSAssistGameAPIResult CSAssistGameAPITicket::setDetails(const CSAssistUnicodeChar *string)
//-----------------------------------------------------------
{
	unsigned length = UnicodeLength(string);
	if (length > CSASSIST_TICKET_LENGTH_DETAILS)
		return CSASSIST_RESULT_BADPARAMS;
	memcpy(details, string, (length+1) * sizeof(CSAssistUnicodeChar));
	return CSASSIST_RESULT_SUCCESS;
}

//-----------------------------------------------------------
CSAssistGameAPIResult CSAssistGameAPITicket::setLanguage(const CSAssistUnicodeChar *string)
//-----------------------------------------------------------
{
	unsigned length = UnicodeLength(string);
	if (length > CSASSIST_TICKET_LENGTH_LANGUAGE)
		return CSASSIST_RESULT_BADPARAMS;
	memcpy(language, string, (length+1) * sizeof(CSAssistUnicodeChar));
	return CSASSIST_RESULT_SUCCESS;
}

//-----------------------------------------------------------
CSAssistGameAPIResult CSAssistGameAPITicket::setLocation(const CSAssistUnicodeChar *string)
//-----------------------------------------------------------
{
	unsigned length = UnicodeLength(string);
	if (length > CSASSIST_TICKET_LENGTH_LOCATION)
		return CSASSIST_RESULT_BADPARAMS;
	memcpy(location, string, (length+1) * sizeof(CSAssistUnicodeChar));
	return CSASSIST_RESULT_SUCCESS;
}

//-----------------------------------------------------------
CSAssistGameAPIResult CSAssistGameAPITicket::setCategory(int index, unsigned value)
//-----------------------------------------------------------
{
	if (index < 0 || index >= (short)CSASSIST_NUM_CATEGORIES)
		return CSASSIST_RESULT_BADPARAMS;
	category[index] = value;
	return CSASSIST_RESULT_SUCCESS;
}



//-----------------------------------------------------------
CSAssistGameAPITicketComment::CSAssistGameAPITicketComment() :
ticketID(0), commentID(0), uid(0), type(CSASSIST_COMMENT_NONE), modifiedDate(0)
//-----------------------------------------------------------
{
	comment[0] = 0;
	name[0] = 0;
}

//-----------------------------------------------------------
CSAssistGameAPIDocumentHeader::CSAssistGameAPIDocumentHeader() :
id(0), modifiedDate(0)
//-----------------------------------------------------------
{
	game[0]		= 0;
	language[0]	= 0;
	title[0]	= 0;
	version[0]	= 0;
}
	
//--------------------------------------------
CSAssistGameAPISearchResult::CSAssistGameAPISearchResult()
: matchPercent(0)
//--------------------------------------------
{
	title[0] = 0;
	idstring[0] = 0;
}

} // namespace CSAssist
