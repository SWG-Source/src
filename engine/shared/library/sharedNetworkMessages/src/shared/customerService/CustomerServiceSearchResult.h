// CustomerServiceSearchResult.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved.

#ifndef _INCLUDED_CustomerServiceSearchResult_H
#define _INCLUDED_CustomerServiceSearchResult_H

#include <string>
#include <vector>

//--------------------------------------------------------

class CustomerServiceSearchResult
{
private:

protected:

public:
	CustomerServiceSearchResult();
	CustomerServiceSearchResult(const Unicode::String &title, const std::string &id, short matchPercent);
	~CustomerServiceSearchResult();

	const Unicode::String &getTitle()        const {return m_title;}
	const std::string &    getId()           const {return m_id;}
	short                  getMatchPercent() const {return m_matchPercent;}

	Unicode::String                       m_title;
	std::string                           m_id;
	short                                 m_matchPercent;
};

#endif //_INCLUDED_CustomerServiceSearchResult_H
