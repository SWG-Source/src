// ======================================================================
//
// PermissionListModifyMessage.h
// copyright (c) 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_PermissionListModifyMessage_H
#define INCLUDED_PermissionListModifyMessage_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "Unicode.h"
#include "unicodeArchive/UnicodeArchive.h"

//-----------------------------------------------------------------------

class PermissionListModifyMessage : public GameNetworkMessage
{
public:
	                                    PermissionListModifyMessage (const Unicode::String& person, const Unicode::String& listName, const Unicode::String& action);
	explicit                            PermissionListModifyMessage (Archive::ReadIterator & source);
	virtual                            ~PermissionListModifyMessage ();
	const Unicode::String &             getPerson                   () const;
	const Unicode::String &             getListName                 () const;
	const Unicode::String &             getAction                   () const;

private:
	//disabled
	                                    PermissionListModifyMessage (const PermissionListModifyMessage&);
	PermissionListModifyMessage&        operator=                   (const PermissionListModifyMessage&);

private:
	Archive::AutoVariable<Unicode::String> m_person;
	Archive::AutoVariable<Unicode::String> m_listName;
	Archive::AutoVariable<Unicode::String> m_action;
};

// ======================================================================

#endif
