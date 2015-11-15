// ======================================================================
//
// RequestCategoriesMessage.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef	INCLUDED_RequestCategoriesMessage_H
#define	INCLUDED_RequestCategoriesMessage_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class RequestCategoriesMessage : public GameNetworkMessage
{
public:

	RequestCategoriesMessage(std::string const & language);
	explicit RequestCategoriesMessage(Archive::ReadIterator & source);
	~RequestCategoriesMessage();

	std::string getLanguage() const;

private: 

	Archive::AutoVariable<std::string> m_language;
};

// ----------------------------------------------------------------------

#endif // INCLUDED_RequestCategoriesMessage_H
