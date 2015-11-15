//========================================================================
//
// ReloadScriptMessage.h
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef	_INCLUDED_ReloadScriptMessage_H
#define	_INCLUDED_ReloadScriptMessage_H

//-----------------------------------------------------------------------

#include "sharedMathArchive/TransformArchive.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "unicodeArchive/UnicodeArchive.h"

//-----------------------------------------------------------------------

/**
 * Sent from:  Game Server
 * Sent to:  DBProcess
 * Action:  teleport a person to a target objectid
 */
class ReloadScriptMessage: public GameNetworkMessage
{
public:
	ReloadScriptMessage(const std::string & script);
	ReloadScriptMessage(Archive::ReadIterator & source);
	~ReloadScriptMessage();

public:
	const std::string & getScript() const;
	
private:
	Archive::AutoVariable<std::string>  m_script;

private:
	ReloadScriptMessage(const ReloadScriptMessage&);
	ReloadScriptMessage& operator= (const ReloadScriptMessage&);
};

// ======================================================================

inline const std::string & ReloadScriptMessage::getScript() const
{
	return m_script.get();
}

// ======================================================================

#endif	// _INCLUDED_ReloadScriptMessage_H

