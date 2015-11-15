// ==================================================================
//
// CreateClientPathMessage.cpp
// Copyright 2002 Sony Online Entertainment
//
// ==================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/CreateClientPathMessage.h"

#include "sharedMathArchive/VectorArchive.h"

// ==================================================================

char const * const CreateClientPathMessage::cms_name = "CreateClientPathMessage";

// ==================================================================

CreateClientPathMessage::CreateClientPathMessage (std::vector<Vector> const & pointList) :
	GameNetworkMessage (cms_name),
	m_pointList ()
{
	m_pointList.set (pointList);
	
	addVariable (m_pointList);
}

// ------------------------------------------------------------------

CreateClientPathMessage::CreateClientPathMessage (Archive::ReadIterator & source) :
	GameNetworkMessage (cms_name),
	m_pointList ()
{
	addVariable (m_pointList);

	unpack (source);
}

// ------------------------------------------------------------------

CreateClientPathMessage::~CreateClientPathMessage ()
{
}

// ------------------------------------------------------------------

std::vector<Vector> const & CreateClientPathMessage::getPointList () const
{
	return m_pointList.get ();
}

// ==================================================================
