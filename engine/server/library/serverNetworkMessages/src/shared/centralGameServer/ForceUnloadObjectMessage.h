//========================================================================
//
// ForceUnloadObjectMessage.h - tells CentralServer to unload all instances of
// an object.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef	_INCLUDED_ForceUnloadObjectMessage_H
#define	_INCLUDED_ForceUnloadObjectMessage_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class ForceUnloadObjectMessage : public GameNetworkMessage
{
public:
	explicit ForceUnloadObjectMessage  (const NetworkId& id, bool permaDelete = false);
	ForceUnloadObjectMessage           (Archive::ReadIterator & source);
	~ForceUnloadObjectMessage ();

	const NetworkId&          getId(void) const;
	bool                getPermaDelete(void) const;
        
private:
	Archive::AutoVariable<NetworkId>     m_id;			///< id of object we want to unload
	Archive::AutoVariable<bool>    m_permaDelete;            ///< whether or not to delete this from the DB.

	ForceUnloadObjectMessage();
	ForceUnloadObjectMessage(const ForceUnloadObjectMessage&);
	ForceUnloadObjectMessage& operator= (const ForceUnloadObjectMessage&);
};


//-----------------------------------------------------------------------

inline const NetworkId& ForceUnloadObjectMessage::getId(void) const
{
	return m_id.get();
}	// ForceUnloadObjectMessage::getId

inline bool ForceUnloadObjectMessage::getPermaDelete(void) const
{
    return m_permaDelete.get();
}


// ----------------------------------------------------------------------


#endif	// _INCLUDED_ForceUnloadObjectMessage_H
