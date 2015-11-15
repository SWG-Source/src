// ==================================================================
//
// NewbieTutorialSetToolbarElement.h
// Copyright 2002 Sony Online Entertainment
//
// ==================================================================

#ifndef	INCLUDED_NewbieTutorialSetToolbarElement_H
#define	INCLUDED_NewbieTutorialSetToolbarElement_H

// ==================================================================

#include "Archive/AutoDeltaByteStream.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

// ==================================================================

class NewbieTutorialSetToolbarElement : public GameNetworkMessage
{
public: 

	NewbieTutorialSetToolbarElement (int slot, const std::string& commandName);
	NewbieTutorialSetToolbarElement (int slot, int64 object);
	explicit NewbieTutorialSetToolbarElement (Archive::ReadIterator &source);
	virtual ~NewbieTutorialSetToolbarElement ();

	int                getSlot () const;	
	const std::string& getCommandName () const;
	int64              getObject () const;
	
public:

	static const char* const cms_name;

private:

	NewbieTutorialSetToolbarElement ();
	NewbieTutorialSetToolbarElement (const NewbieTutorialSetToolbarElement&);
	NewbieTutorialSetToolbarElement& operator= (const NewbieTutorialSetToolbarElement&);

private:

	Archive::AutoVariable<int>         m_slot;
	Archive::AutoVariable<std::string> m_commandName;
	Archive::AutoVariable<int64>       m_object;
	
};

// ==================================================================

#endif 
