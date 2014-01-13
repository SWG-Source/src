// CustomerServiceTicket.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved.

#ifndef _INCLUDED_CustomerServiceTicket_H
#define _INCLUDED_CustomerServiceTicket_H

#include <string>
#include <vector>

//--------------------------------------------------------

class CustomerServiceTicket
{
private:

protected:

public:
	CustomerServiceTicket();
	CustomerServiceTicket(unsigned int categoryId, unsigned int subCategoryId, const std::string &characterName, const Unicode::String &details, const std::string &language, unsigned int ticketId, int64 modifiedDate, bool read, bool closed);
	~CustomerServiceTicket();

	int                    getCategoryId()      const {return m_categoryId;}
	int                    getSubCategoryId()   const {return m_subCategoryId;}
	const std::string &    getCharacterName()   const {return m_characterName;}
	const Unicode::String &getDetails()         const {return m_details;}
	const std::string &    getLanguage()        const {return m_language;}
	unsigned int           getTicketId()        const {return m_ticketId;}
	int64                  getModifiedDate()    const {return m_modifiedDate;}
	bool                   isRead() const { return m_read; }
	bool                   isClosed() const { return m_closed; }

	unsigned int    m_categoryId;
	unsigned int    m_subCategoryId;
	std::string     m_characterName;
	Unicode::String m_details;
	std::string     m_language;
	unsigned int    m_ticketId;
	int64           m_modifiedDate;
	bool            m_read;
	bool            m_closed;
};

#endif //_INCLUDED_CustomerServiceTicket_H
