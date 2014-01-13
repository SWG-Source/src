// ==================================================================
//
// NewbieTutorialResponse.h
// Copyright 2002 Sony Online Entertainment
//
// ==================================================================

#ifndef	INCLUDED_NewbieTutorialResponse_H
#define	INCLUDED_NewbieTutorialResponse_H

// ==================================================================

#include "Archive/AutoDeltaByteStream.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

// ==================================================================

class NewbieTutorialResponse : public GameNetworkMessage
{
public: 

	explicit NewbieTutorialResponse (const std::string& response);
	explicit NewbieTutorialResponse (Archive::ReadIterator &source);
	virtual ~NewbieTutorialResponse ();

	const std::string& getResponse () const;

public:

	static const char* const cms_name;

private:

	NewbieTutorialResponse ();
	NewbieTutorialResponse (const NewbieTutorialResponse&);
	NewbieTutorialResponse& operator= (const NewbieTutorialResponse&);

private:

	Archive::AutoVariable<std::string> m_response;
};

// ==================================================================

#endif 
