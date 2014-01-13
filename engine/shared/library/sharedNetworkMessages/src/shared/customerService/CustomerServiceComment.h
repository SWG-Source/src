// CustomerServiceComment.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved.

#ifndef _INCLUDED_CustomerServiceComment_H
#define _INCLUDED_CustomerServiceComment_H

#include <string>
#include <vector>

//--------------------------------------------------------

class CustomerServiceComment
{
private:

protected:

public:
	CustomerServiceComment();
	CustomerServiceComment(unsigned int ticketId, unsigned int commentId, bool fromR, const Unicode::String &comment, const std::string &commentorName);
	~CustomerServiceComment();

	int                    getTicketId()       const {return m_ticketId;}
	int                    getCommentId()      const {return m_commentId;}
	bool                   isFromCsr()         const {return m_fromCsr;}
	const Unicode::String &getComment()        const {return m_comment;}
	const std::string &    getCommentorName()  const {return m_commentorName;}

	unsigned int                          m_ticketId;
	unsigned int                          m_commentId;
	bool                                  m_fromCsr;
	Unicode::String                       m_comment;
	std::string                           m_commentorName;
};

#endif //_INCLUDED_CustomerServiceComment_H
