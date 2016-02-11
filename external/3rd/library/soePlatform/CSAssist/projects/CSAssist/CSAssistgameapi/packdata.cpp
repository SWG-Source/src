//------------------------------------------------------------------------------
//
// packdata.cpp
//
// Author:	Graeme Ing		(ging@soe.sony.com)
// Module:	CSAssist API
//
// copyright 2002 (c) Sony Online Entertainment Inc.
//
// decode/encode class data for sending over the wire
//
//------------------------------------------------------------------------------

#include "Base/Archive.h"
#include "CSAssistgameapi.h"
#include "CSAssistgameobjects.h"
#include "Unicode/UnicodeUtils.h"
#include "packdata.h"
#include <stdio.h>

namespace CSAssist
{

using namespace Base;
using namespace std;
using namespace CSAssist;
using namespace Plat_Unicode;


//----------------------------------------------
CSAssistUnicodeChar *get_c_str(Plat_Unicode::String s)
// This replaces c_str() by allocating a new buffer, erasing it, and then
// using the non-nullptr terminated data() to copy the data into the buffer.
//----------------------------------------------
{
	unsigned length = s.size() + 1;
	CSAssistUnicodeChar *temp = new CSAssistUnicodeChar[length];
	memset(temp, 0, length * sizeof(CSAssistUnicodeChar));
	memcpy(temp, s.data(), (length - 1) * sizeof(CSAssistUnicodeChar));
	return temp;
}

//----------------------------------------------
CSAssistUnicodeChar *copy_c_str(Plat_Unicode::String s, CSAssistUnicodeChar *dest)
//----------------------------------------------
{
	unsigned length = s.size() + 1;
	memset(dest, 0, length * sizeof(CSAssistUnicodeChar));
	memcpy(dest, s.data(), (length - 1) * sizeof(CSAssistUnicodeChar));
	return dest;
}

namespace Base
{
//----------------------------------------------
void put(Base::ByteStream & target, const CSAssistGameAPITicket & source)
//----------------------------------------------
{
	Plat_Unicode::String sgame = source.game;
	Plat_Unicode::String sserver = source.server;
	Plat_Unicode::String sdetails = source.details;
	Plat_Unicode::String slanguage = source.language;
	Plat_Unicode::String scharacter = source.character;
	Plat_Unicode::String slocation = source.location;

	put(target, source.ticketID);
	put(target, source.uid);
	put(target, (unsigned)source.createDate);
	put(target, (unsigned)source.modifiedDate);
	put(target, sgame);
	put(target, sserver);
	put(target, scharacter);
	put(target, (unsigned)source.status);
	put(target, (unsigned)source.bugstatus);
	for (short i=0; i < CSASSIST_NUM_CATEGORIES; i++)
		put(target, source.category[i]);
	put(target, sdetails);
	put(target, slanguage);
	put(target, source.isRead);
	put(target, slocation);
}

//----------------------------------------------
void get(ByteStream::ReadIterator & source, CSAssistGameAPITicket & target)
//----------------------------------------------
	{
		Plat_Unicode::String sgame;
		Plat_Unicode::String sserver;
		Plat_Unicode::String scharacter;
		Plat_Unicode::String sdetails;
		Plat_Unicode::String slanguage;
		Plat_Unicode::String slocation;
		unsigned create, modified, gstatus, bstatus;

		get(source, target.ticketID);
		get(source, target.uid);
		get(source, create);
		get(source, modified);
		get(source, sgame);
		get(source, sserver);
		get(source, scharacter);
		get(source, gstatus);
		get(source, bstatus);
		for (short i=0; i < CSASSIST_NUM_CATEGORIES; i++)
			get(source, target.category[i]);
		get(source, sdetails);
		get(source, slanguage);
		get(source, target.isRead);
		get(source, slocation);

		copy_c_str(sgame, target.game);
		copy_c_str(sserver, target.server);
		copy_c_str(scharacter, target.character);
		copy_c_str(sdetails, target.details);
		copy_c_str(slanguage, target.language);
		copy_c_str(slocation, target.location);

		target.createDate	= (time_t)create;
		target.modifiedDate	= (time_t)modified;
		target.status		= (CSAssistTicketStatus)gstatus;
		target.bugstatus	= (CSAssistBugStatus)bstatus;
	}

//----------------------------------------------
void get(ByteStream::ReadIterator & source, CSAssistGameAPITicket * target)
//----------------------------------------------
	{
		CSAssistGameAPITicket &t = *target;
		get(source, t);
	}
} // namespace Base

//----------------------------------------------
void decodeTicketComment(ByteStream::ReadIterator & msg,CSAssistGameAPITicketComment *dest)
//----------------------------------------------
{
	Plat_Unicode::String comment, name;
	unsigned modified, type;

	get(msg, dest->ticketID);
	get(msg, dest->commentID);
	get(msg, dest->uid);
	get(msg, modified);
	get(msg, type);
	get(msg, comment);
	get(msg, name);

	copy_c_str(comment, dest->comment);
	copy_c_str(name, dest->name);
	dest->modifiedDate	= (time_t)modified;
	dest->type = (CSAssistGameAPICommentType)type;
}

//----------------------------------------------
void decodeDocumentHeader(ByteStream::ReadIterator & msg,CSAssistGameAPIDocumentHeader *dest)
//----------------------------------------------
{
	Plat_Unicode::String game, language, title, version;
	unsigned modified;

	get(msg, dest->id);
	get(msg, game);
	get(msg, language);
	get(msg, title);
	get(msg, version);
	get(msg, modified);

	copy_c_str(game, dest->game);
	copy_c_str(language, dest->language);
	copy_c_str(title, dest->title);
	copy_c_str(version, dest->version);
	dest->modifiedDate	= (time_t)modified;
}

//----------------------------------------------
void decodeSearchResult(ByteStream::ReadIterator & msg,CSAssistGameAPISearchResult *dest)
//----------------------------------------------
{
	Plat_Unicode::String atitle, id;

	get(msg, id);
	get(msg, dest->matchPercent);
	get(msg, atitle);

	copy_c_str(id, dest->idstring);
	copy_c_str(atitle, dest->title);
}

} // namespace CSAssist

