// ======================================================================
//
// ConsentRequestMessage.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ConsentRequestMessage_H
#define INCLUDED_ConsentRequestMessage_H

// ======================================================================

#include "sharedGame/ProsePackage.h"
#include "sharedGame/ProsePackageArchive.h"

#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

/**
 * Sent from:  Client
 * Sent to:    Connection Server
 * Action:     Requests the state of the player's stomach(s).
 */
class ConsentRequestMessage : public GameNetworkMessage
{
public:
	static const char * const cms_name;

public:
	ConsentRequestMessage         (const ProsePackage& question, int id);
	explicit ConsentRequestMessage(Archive::ReadIterator & source);
	virtual ~ConsentRequestMessage();

public:
	const ProsePackage& getQuestion() const;
	int                 getId      () const;

 private:
	ConsentRequestMessage(const ConsentRequestMessage&);
	ConsentRequestMessage& operator= (const ConsentRequestMessage&);

private:
	Archive::AutoVariable<ProsePackage> m_question;
	Archive::AutoVariable<int> m_id;
};

// ----------------------------------------------------------------------

inline const ProsePackage& ConsentRequestMessage::getQuestion() const
{
	return m_question.get();
}

// ----------------------------------------------------------------------

inline int ConsentRequestMessage::getId() const
{
	return m_id.get();
}

// ======================================================================

#endif
