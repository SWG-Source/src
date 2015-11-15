//========================================================================
//
// ReloadTemplateMessage.h
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef	_INCLUDED_ReloadTemplateMessage_H
#define	_INCLUDED_ReloadTemplateMessage_H

//-----------------------------------------------------------------------

#include "sharedMathArchive/TransformArchive.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "unicodeArchive/UnicodeArchive.h"

//-----------------------------------------------------------------------

/**
 * Sent from:  Game Server
 * Sent to:  DBProcess
 * Action:  reload an object template from disk
 */
class ReloadTemplateMessage: public GameNetworkMessage
{
public:
	ReloadTemplateMessage(const std::string & templateName);
	ReloadTemplateMessage(Archive::ReadIterator & source);
	~ReloadTemplateMessage();

public:
	const std::string & getTemplate() const;
	
private:
	Archive::AutoVariable<std::string>  m_template;

private:
	ReloadTemplateMessage(const ReloadTemplateMessage&);
	ReloadTemplateMessage& operator= (const ReloadTemplateMessage&);
};

// ======================================================================

inline const std::string & ReloadTemplateMessage::getTemplate() const
{
	return m_template.get();
}

// ======================================================================

#endif	// _INCLUDED_ReloadTemplateMessage_H

