// ======================================================================
//
// PermissionListCreateMessage.h
// copyright (c) 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_PermissionListCreateMessage_H
#define INCLUDED_PermissionListCreateMessage_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"

#include "Unicode.h"
#include "unicodeArchive/UnicodeArchive.h"

//-----------------------------------------------------------------------

class PermissionListCreateMessage : public GameNetworkMessage
{
public:

	static const char * const MessageType;

	                                       PermissionListCreateMessage (const std::vector<Unicode::String> &currentMembers, const std::vector<Unicode::String> &nearbyPeople, Unicode::String listName);
	explicit                               PermissionListCreateMessage (Archive::ReadIterator & source);
	virtual                               ~PermissionListCreateMessage ();
	const std::vector<Unicode::String>& getCurrentMembers           () const;
	const std::vector<Unicode::String>& getNearbyPeople             () const;
	const Unicode::String&                 getListName                 () const;

private:
	//disabled
	PermissionListCreateMessage            (const PermissionListCreateMessage&);
	PermissionListCreateMessage& operator= (const PermissionListCreateMessage&);

private:
	Archive::AutoArray<Unicode::String>    m_currentMembers;
	Archive::AutoArray<Unicode::String>    m_nearbyPeople;
	Archive::AutoVariable<Unicode::String> m_listName;
};

// ======================================================================

#endif
