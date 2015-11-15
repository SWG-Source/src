// ==================================================================
//
// NewbieTutorialHighlightUIElement.h
// Copyright 2002 Sony Online Entertainment
//
// ==================================================================

#ifndef	INCLUDED_NewbieTutorialHighlightUIElement_H
#define	INCLUDED_NewbieTutorialHighlightUIElement_H

// ==================================================================

#include "Archive/AutoDeltaByteStream.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

// ==================================================================

class NewbieTutorialHighlightUIElement : public GameNetworkMessage
{
public: 

	NewbieTutorialHighlightUIElement (const std::string &widgetPath, float time);
	explicit NewbieTutorialHighlightUIElement (Archive::ReadIterator &source);
	virtual ~NewbieTutorialHighlightUIElement ();

	float              getTime () const;	
	const std::string& getWidgetPath () const;
	
public:

	static const char* const cms_name;

private:

	NewbieTutorialHighlightUIElement ();
	NewbieTutorialHighlightUIElement (const NewbieTutorialHighlightUIElement&);
	NewbieTutorialHighlightUIElement& operator= (const NewbieTutorialHighlightUIElement&);

private:

	Archive::AutoVariable<float>       m_time;
	Archive::AutoVariable<std::string> m_widgetPath;
	
};

// ==================================================================

#endif 
