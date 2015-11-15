// ======================================================================
//
// CharacterSheetResponseMessage.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/CharacterSheetResponseMessage.h"

// ======================================================================

const char* const CharacterSheetResponseMessage::cms_name = "CharacterSheetResponseMessage";

// ======================================================================

CharacterSheetResponseMessage::CharacterSheetResponseMessage(int bornDate, int played, const Vector& bindLoc, 
                                                             const std::string& bindPlanet, const Vector& bankLoc, 
                                                             const std::string& bankPlanet, const Vector& residenceLoc, 
                                                             const std::string& residencePlanet, const std::string& citizensOf,
                                                             const Unicode::String& spouseName, int lotsUsed)
: GameNetworkMessage(cms_name),
  m_bornDate(bornDate),
  m_played(played),
  m_bindLocation(bindLoc),
  m_bindPlanet(bindPlanet),
  m_bankLocation(bankLoc),
  m_bankPlanet(bankPlanet),
  m_residenceLocation(residenceLoc),
  m_residencePlanet(residencePlanet),
  m_citizensOf(citizensOf),
  m_spouseName(spouseName),
  m_lotsUsed(lotsUsed)
{
	AutoByteStream::addVariable(m_bornDate);
	AutoByteStream::addVariable(m_played);
	AutoByteStream::addVariable(m_bindLocation);
	AutoByteStream::addVariable(m_bindPlanet);
	AutoByteStream::addVariable(m_bankLocation);
	AutoByteStream::addVariable(m_bankPlanet);
	AutoByteStream::addVariable(m_residenceLocation);
	AutoByteStream::addVariable(m_residencePlanet);
	AutoByteStream::addVariable(m_citizensOf);
	AutoByteStream::addVariable(m_spouseName);
	AutoByteStream::addVariable(m_lotsUsed);
}

// ======================================================================

CharacterSheetResponseMessage::CharacterSheetResponseMessage(Archive::ReadIterator &source)
: GameNetworkMessage(cms_name),
  m_bornDate(0),
  m_played(0),
  m_bindLocation(),
  m_bindPlanet(),
  m_bankLocation(),
  m_bankPlanet(),
  m_residenceLocation(),
  m_residencePlanet(),
  m_citizensOf(),
  m_spouseName(),
  m_lotsUsed(0)
{
	AutoByteStream::addVariable(m_bornDate);
	AutoByteStream::addVariable(m_played);
	AutoByteStream::addVariable(m_bindLocation);
	AutoByteStream::addVariable(m_bindPlanet);
	AutoByteStream::addVariable(m_bankLocation);
	AutoByteStream::addVariable(m_bankPlanet);
	AutoByteStream::addVariable(m_residenceLocation);
	AutoByteStream::addVariable(m_residencePlanet);
	AutoByteStream::addVariable(m_citizensOf);
	AutoByteStream::addVariable(m_spouseName);
	AutoByteStream::addVariable(m_lotsUsed);
	unpack(source);
}

// ----------------------------------------------------------------------

CharacterSheetResponseMessage::~CharacterSheetResponseMessage()
{
}


// ----------------------------------------------------------------------

const Vector&          CharacterSheetResponseMessage::getBindLoc         () const
{
	return m_bindLocation.get();
}

// ----------------------------------------------------------------------

const std::string&     CharacterSheetResponseMessage::getBindPlanet      () const
{
	return m_bindPlanet.get();
}

// ----------------------------------------------------------------------

const Vector&          CharacterSheetResponseMessage::getBankLoc         () const
{
	return m_bankLocation.get();
}

// ----------------------------------------------------------------------

const std::string&     CharacterSheetResponseMessage::getResidencePlanet () const
{
	return m_residencePlanet.get();
}

// ----------------------------------------------------------------------

const std::string&     CharacterSheetResponseMessage::getCitizensOf      () const
{
	return m_citizensOf.get();
}

// ----------------------------------------------------------------------

const Vector&          CharacterSheetResponseMessage::getResidenceLoc    () const
{
	return m_residenceLocation.get();
}

// ----------------------------------------------------------------------

const std::string&     CharacterSheetResponseMessage::getBankPlanet      () const
{
	return m_bankPlanet.get();
}

// ----------------------------------------------------------------------

const Unicode::String& CharacterSheetResponseMessage::getSpouseName      () const
{
	return m_spouseName.get();
}

// ======================================================================
