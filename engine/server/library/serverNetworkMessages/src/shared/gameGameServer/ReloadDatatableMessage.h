//========================================================================
//
// ReloadDatatableMessage.h
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef	_INCLUDED_ReloadDatatableMessage_H
#define	_INCLUDED_ReloadDatatableMessage_H

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
class ReloadDatatableMessage: public GameNetworkMessage
{
public:
	ReloadDatatableMessage(const std::string & table);
	ReloadDatatableMessage(Archive::ReadIterator & source);
	~ReloadDatatableMessage();

public:
	const std::string & getTable() const;
	
public:
	// message name
	static const char * const ms_messageName;

private:
	Archive::AutoVariable<std::string>  m_table;

private:
	ReloadDatatableMessage(const ReloadDatatableMessage&);
	ReloadDatatableMessage& operator= (const ReloadDatatableMessage&);
};

// ======================================================================

inline const std::string & ReloadDatatableMessage::getTable() const
{
	return m_table.get();
}

// ======================================================================

#endif	// _INCLUDED_ReloadDatatableMessage_H

