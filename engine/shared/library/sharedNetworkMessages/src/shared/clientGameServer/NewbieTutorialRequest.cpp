// ==================================================================
//
// NewbieTutorialRequest.cpp
// Copyright 2002 Sony Online Entertainment
//
// ==================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/NewbieTutorialRequest.h"

// ==================================================================

const char* const NewbieTutorialRequest::cms_name = "NewbieTutorialRequest";

// ==================================================================

NewbieTutorialRequest::NewbieTutorialRequest (const std::string& request) :
	GameNetworkMessage ("NewbieTutorialRequest"),
	m_request (request)
{
	addVariable (m_request);
}

// ------------------------------------------------------------------

NewbieTutorialRequest::NewbieTutorialRequest (Archive::ReadIterator& source) :
	GameNetworkMessage ("NewbieTutorialRequest"),
	m_request ()
{
	addVariable (m_request);
	unpack (source);
}

// ------------------------------------------------------------------

NewbieTutorialRequest::~NewbieTutorialRequest ()
{
}

// ------------------------------------------------------------------

const std::string& NewbieTutorialRequest::getRequest () const
{
	return m_request.get ();
}

// ==================================================================
