// ==================================================================
//
// NewbieTutorialEnableInterfaceElement.h
// Copyright 2002 Sony Online Entertainment
//
// ==================================================================

#ifndef	INCLUDED_NewbieTutorialEnableInterfaceElement_H
#define	INCLUDED_NewbieTutorialEnableInterfaceElement_H

// ==================================================================

#include "Archive/AutoDeltaByteStream.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

// ==================================================================

class NewbieTutorialEnableInterfaceElement : public GameNetworkMessage
{
public: 

	NewbieTutorialEnableInterfaceElement (const std::string& name, bool enable);
	explicit NewbieTutorialEnableInterfaceElement (Archive::ReadIterator &source);
	virtual ~NewbieTutorialEnableInterfaceElement ();

	const std::string& getName () const;
	bool               getEnable () const;

public:

	static const char* const cms_name;

private:

	NewbieTutorialEnableInterfaceElement ();
	NewbieTutorialEnableInterfaceElement (const NewbieTutorialEnableInterfaceElement&);
	NewbieTutorialEnableInterfaceElement& operator= (const NewbieTutorialEnableInterfaceElement&);

private:

	Archive::AutoVariable<std::string> m_name;
	Archive::AutoVariable<bool>        m_enable;
};

// ==================================================================

#endif 
