//======================================================================
//
// ChatPersistentMessageToClientData.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ChatPersistentMessageToClientData_H
#define INCLUDED_ChatPersistentMessageToClientData_H

//======================================================================

struct ChatPersistentMessageToClientData
{
	std::string      fromCharacterName;
	std::string      fromGameCode;
	std::string      fromServerCode;
	unsigned int     id;
	bool             isHeader;
	Unicode::String  message;
	Unicode::String  subject;
	Unicode::String  outOfBand;
	signed char      status;
	unsigned int     timeStamp;

	ChatPersistentMessageToClientData ();
};

//======================================================================

#endif
