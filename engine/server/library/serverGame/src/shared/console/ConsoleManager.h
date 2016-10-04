// ConsoleMgr.h
// copyright 2000 Verant Interactive
// Author: Chris Mayer

#ifndef INCLUDED_ConsoleMgr_H
#define INCLUDED_ConsoleMgr_H

#include "sharedCommandParser/CommandParser.h"
#include "UnicodeUtils.h"

class Client;


class ConsoleMgr
{
public:
	static void install();
	static void remove();

	static void processString(const std::string & msg, Client *from = nullptr,uint32 msgId = 0);
	static void broadcastString(const std::string & msg, Client *to = nullptr, uint32 msgId = 0);
	static void broadcastString(const CommandParser::String_t & msg, Client *to = nullptr, uint32 msgId = 0);
	static void broadcastString(const std::string & msg, NetworkId to, uint32 msgId = 0);

private:
    ConsoleMgr();
    ~ConsoleMgr();

    static CommandParser *          ms_parser;
};


inline void ConsoleMgr::broadcastString(const CommandParser::String_t & msg, Client *to, uint32 msgId)
{
    broadcastString(Unicode::wideToNarrow(msg), to, msgId);
}


#endif // INCLUDED_ConsoleMgr_H


