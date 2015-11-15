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

	                                       PermissionListCreateMessage (const stdvector<Unicode::String>::fwd &currentMembers, const stdvector<Unicode::String>::fwd &nearbyPeople, Unicode::String listName);
	explicit                               PermissionListCreateMessage (Archive::ReadIterator & source);
	virtual                               ~PermissionListCreateMessage ();
	const stdvector<Unicode::String>::fwd& getCurrentMembers           () const;
	const stdvector<Unicode::String>::fwd& getNearbyPeople             () const;
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
