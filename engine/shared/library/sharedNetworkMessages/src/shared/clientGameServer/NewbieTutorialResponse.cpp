// ==================================================================
//
// NewbieTutorialResponse.cpp
// Copyright 2002 Sony Online Entertainment
//
// ==================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/NewbieTutorialResponse.h"

// ==================================================================

const char* const NewbieTutorialResponse::cms_name = "NewbieTutorialResponse";

// ==================================================================

NewbieTutorialResponse::NewbieTutorialResponse (const std::string& response) :
	GameNetworkMessage ("NewbieTutorialResponse"),
	m_response (response)
{
	addVariable (m_response);
}

// ------------------------------------------------------------------

NewbieTutorialResponse::NewbieTutorialResponse (Archive::ReadIterator& source) :
	GameNetworkMessage ("NewbieTutorialResponse"),
	m_response ()
{
	addVariable (m_response);
	unpack (source);
}

// ------------------------------------------------------------------

NewbieTutorialResponse::~NewbieTutorialResponse ()
{
}

// ------------------------------------------------------------------

const std::string& NewbieTutorialResponse::getResponse () const
{
	return m_response.get ();
}

// ==================================================================
