// ==================================================================
//
// NewbieTutorialEnableHudElement.h
// Copyright 2002 Sony Online Entertainment
//
// ==================================================================

#ifndef	INCLUDED_NewbieTutorialEnableHudElement_H
#define	INCLUDED_NewbieTutorialEnableHudElement_H

// ==================================================================

#include "Archive/AutoDeltaByteStream.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

// ==================================================================

class NewbieTutorialEnableHudElement : public GameNetworkMessage
{
public: 

	NewbieTutorialEnableHudElement (const std::string& name, bool enable, float blinkTime);
	explicit NewbieTutorialEnableHudElement (Archive::ReadIterator &source);
	virtual ~NewbieTutorialEnableHudElement ();

	const std::string& getName () const;
	bool               getEnable () const;
	float              getBlinkTime () const;

public:

	static const char* const cms_name;

private:

	NewbieTutorialEnableHudElement ();
	NewbieTutorialEnableHudElement (const NewbieTutorialEnableHudElement&);
	NewbieTutorialEnableHudElement& operator= (const NewbieTutorialEnableHudElement&);

private:

	Archive::AutoVariable<std::string> m_name;
	Archive::AutoVariable<bool>        m_enable;
	Archive::AutoVariable<float>       m_blinkTime;
};

// ==================================================================

#endif 
