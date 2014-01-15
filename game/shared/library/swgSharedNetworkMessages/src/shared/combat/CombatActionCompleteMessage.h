
// ======================================================================
//
// CombatActionCompleteMessage.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_CombatActionCompleteMessage_H
#define INCLUDED_CombatActionCompleteMessage_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "Unicode.h"
#include "unicodeArchive/UnicodeArchive.h"

//-----------------------------------------------------------------------

class GameInByteStream;
class GameOutByteStream;

//-----------------------------------------------------------------------

class CombatActionCompleteMessage : public GameNetworkMessage
{
public:

	static const char * const MESSAGE_TYPE;

	           CombatActionCompleteMessage (uint32 sequenceId, bool nack = false);
	explicit   CombatActionCompleteMessage (Archive::ReadIterator & source);
	virtual   ~CombatActionCompleteMessage ();

	
	uint32     getSequenceId () const;
	bool       isNack () const;

private:

	//-- high bit is the nack flag, lower 31 bits is the id
	Archive::AutoVariable<uint32> m_sequenceId;

private:
	CombatActionCompleteMessage(const CombatActionCompleteMessage&);
	CombatActionCompleteMessage& operator= (const CombatActionCompleteMessage&);
};

//-----------------------------------------------------------------------

inline uint32 CombatActionCompleteMessage::getSequenceId () const
{
	return m_sequenceId.get () & ~(0x80000000);
}

//----------------------------------------------------------------------

inline bool CombatActionCompleteMessage::isNack () const
{
	return (m_sequenceId.get () & 0x80000000) != 0;
}

//-----------------------------------------------------------------

#endif
