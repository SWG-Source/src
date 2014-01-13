// ======================================================================
//
// EditAppearanceMessage.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef	_EditAppearanceMessage_H
#define	_EditAppearanceMessage_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkIdArchive.h"

//-----------------------------------------------------------------------

class EditAppearanceMessage : public GameNetworkMessage
{
public:
	static const char * const MessageType;

public:
	explicit EditAppearanceMessage(const NetworkId & id);
	explicit EditAppearanceMessage(Archive::ReadIterator &source);

public:
	const NetworkId &                                 getTarget () const;

private:
	Archive::AutoVariable<NetworkId>                  m_target;
};

// ----------------------------------------------------------------------

inline const NetworkId & EditAppearanceMessage::getTarget () const
{
	return m_target.get ();
}

// ----------------------------------------------------------------------

#endif // _EditAppearanceMessage_H

