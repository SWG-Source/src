// ======================================================================
//
// StatMigrationTargetsMessage.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef	_StatMigrationTargetsMessage_H
#define	_StatMigrationTargetsMessage_H

//-----------------------------------------------------------------------

#include "Archive/AutoDeltaByteStream.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class StatMigrationTargetsMessage : public GameNetworkMessage
{
public:
	StatMigrationTargetsMessage(const stdvector<int>::fwd& targets, int pointsLeft);
	explicit StatMigrationTargetsMessage(Archive::ReadIterator &source);
	~StatMigrationTargetsMessage();

public: // methods
	int                               getPointsLeft() const;
	stdvector<int>::fwd getTargets   () const;

public:
	static const char* const cms_name;

private: 
	Archive::AutoVariable<int> m_health;
	Archive::AutoVariable<int> m_constitution;
	Archive::AutoVariable<int> m_action;
	Archive::AutoVariable<int> m_stamina;
	Archive::AutoVariable<int> m_mind;
	Archive::AutoVariable<int> m_willpower;
	Archive::AutoVariable<int> m_pointsLeft;
};

// ----------------------------------------------------------------------

inline int StatMigrationTargetsMessage::getPointsLeft() const
{
	return m_pointsLeft.get();
}

// ----------------------------------------------------------------------

#endif // _StatMigrationTargetsMessage_H

