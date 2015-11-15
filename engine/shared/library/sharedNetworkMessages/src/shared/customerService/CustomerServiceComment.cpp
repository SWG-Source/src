// CustomerServiceComment.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved.

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/CustomerServiceComment.h"

//-----------------------------------------------------------------------

CustomerServiceComment::CustomerServiceComment(
	unsigned int ticketId, 
	unsigned int commentId, 
	bool fromCsr, 
	const Unicode::String &comment, 
	const std::string &commentorName
) :
m_ticketId(ticketId),
m_commentId(commentId),
m_fromCsr(fromCsr),
m_comment(comment),
m_commentorName(commentorName)
{
}

//-----------------------------------------------------------------------

CustomerServiceComment::CustomerServiceComment() :
m_ticketId(0),
m_commentId(0),
m_fromCsr(0),
m_comment(),
m_commentorName()
{
}

//-----------------------------------------------------------------------

CustomerServiceComment::~CustomerServiceComment()
{
}

//-----------------------------------------------------------------------
