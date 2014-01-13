// ==================================================================
//
// NewbieTutorialRequest.h
// Copyright 2002 Sony Online Entertainment
//
// ==================================================================

#ifndef	INCLUDED_NewbieTutorialRequest_H
#define	INCLUDED_NewbieTutorialRequest_H

// ==================================================================

#include "Archive/AutoDeltaByteStream.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

// ==================================================================

class NewbieTutorialRequest : public GameNetworkMessage
{
public: 

	explicit NewbieTutorialRequest (const std::string& request);
	explicit NewbieTutorialRequest (Archive::ReadIterator &source);
	virtual ~NewbieTutorialRequest ();

	const std::string& getRequest () const;

public:

	static const char* const cms_name;

private:

	NewbieTutorialRequest ();
	NewbieTutorialRequest (const NewbieTutorialRequest&);
	NewbieTutorialRequest& operator= (const NewbieTutorialRequest&);

private:

	Archive::AutoVariable<std::string> m_request;
};

// ==================================================================

#endif 
