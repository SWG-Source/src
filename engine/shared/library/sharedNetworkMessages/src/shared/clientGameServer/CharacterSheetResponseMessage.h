// ======================================================================
//
// CharacterSheetResponseMessage.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef	_CharacterSheetResponseMessage_H
#define	_CharacterSheetResponseMessage_H

//-----------------------------------------------------------------------

#include "Archive/AutoDeltaByteStream.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

#include "sharedMath/Vector.h"
#include "sharedMathArchive/VectorArchive.h"

//-----------------------------------------------------------------------

class CharacterSheetResponseMessage : public GameNetworkMessage
{
public:
	CharacterSheetResponseMessage (int bornDate, int played, const Vector& bindLoc, 
	                              const std::string& bindPlanet, const Vector& bankLoc, 
	                              const std::string& bankPlanet,  const Vector& residenceLoc, 
	                              const std::string& residencePlanet, const std::string& citizensOf,
	                              const Unicode::String& spouseName, int lotsUsed);
	explicit CharacterSheetResponseMessage(Archive::ReadIterator &source);
	~CharacterSheetResponseMessage();

public: // methods
	int                    getBornDate           () const;
	int                    getPlayed             () const;
	const Vector&          getBindLoc            () const;
	const std::string&     getBindPlanet         () const;
	const Vector&          getBankLoc            () const;
	const std::string&     getBankPlanet         () const;
	const Vector&          getResidenceLoc       () const;
	const std::string&     getResidencePlanet    () const;
	const std::string&     getCitizensOf         () const;
	const Unicode::String& getSpouseName         () const;
	int                    getLotsUsed           () const;

public:
	static const char* const cms_name;

private: 
	Archive::AutoVariable<int>             m_bornDate;
	Archive::AutoVariable<int>             m_played;
	Archive::AutoVariable<Vector>          m_bindLocation;
	Archive::AutoVariable<std::string>     m_bindPlanet;
	Archive::AutoVariable<Vector>          m_bankLocation;
	Archive::AutoVariable<std::string>     m_bankPlanet;
	Archive::AutoVariable<Vector>          m_residenceLocation;
	Archive::AutoVariable<std::string>     m_residencePlanet;
	Archive::AutoVariable<std::string>     m_citizensOf;
	Archive::AutoVariable<Unicode::String> m_spouseName;
	Archive::AutoVariable<int>             m_lotsUsed;
};

// ----------------------------------------------------------------------

inline int                    CharacterSheetResponseMessage::getBornDate        () const
{
	return m_bornDate.get();
}

// ----------------------------------------------------------------------

inline int                    CharacterSheetResponseMessage::getPlayed          () const
{
	return m_played.get();
}

// ----------------------------------------------------------------------

inline int                    CharacterSheetResponseMessage::getLotsUsed        () const
{
	return m_lotsUsed.get();
}

// ----------------------------------------------------------------------

#endif // _CharacterSheetResponseMessage_H

