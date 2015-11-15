// ======================================================================
//
// AddMapLocationMessage.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/AddMapLocationMessage.h"

// ======================================================================

AddMapLocationMessage::AddMapLocationMessage(
	const std::string &planetName,
	const NetworkId &locationId, 
	const Unicode::String &locationName,
	const Vector2d &location,
	uint8 category,
	uint8 subCategory
) :
	GameNetworkMessage("AddMapLocationMessage"),
	m_planetName   (planetName),
	m_locationId   (locationId),
	m_locationName (locationName),
	m_locationX    (location.x),
	m_locationY    (location.y),
	m_category     (category),
	m_subCategory  (subCategory)
{
	AutoByteStream::addVariable(m_planetName);
	AutoByteStream::addVariable(m_locationId);
	AutoByteStream::addVariable(m_locationName);
	AutoByteStream::addVariable(m_locationX);
	AutoByteStream::addVariable(m_locationY);
	AutoByteStream::addVariable(m_category);
	AutoByteStream::addVariable(m_subCategory);
}

// ======================================================================

AddMapLocationMessage::AddMapLocationMessage(Archive::ReadIterator &source) :
	GameNetworkMessage ("AddMapLocationMessage"),
	m_planetName       (),
	m_locationId       (),
	m_locationName     (),
	m_locationX        (0.0f),
	m_locationY        (0.0f),
	m_category         (),
	m_subCategory      ()
{
	AutoByteStream::addVariable(m_planetName);
	AutoByteStream::addVariable(m_locationId);
	AutoByteStream::addVariable(m_locationName);
	AutoByteStream::addVariable(m_locationX);
	AutoByteStream::addVariable(m_locationY);
	AutoByteStream::addVariable(m_category);
	AutoByteStream::addVariable(m_subCategory);
	unpack(source);
}

// ======================================================================

AddMapLocationMessage::~AddMapLocationMessage()
{
}

//----------------------------------------------------------------------
