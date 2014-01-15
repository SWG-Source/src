//========================================================================
//
// SetObjectPositionMessage.h - tells where to put an object.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef	_INCLUDED_SetObjectPositionMessage_H
#define	_INCLUDED_SetObjectPositionMessage_H

//-----------------------------------------------------------------------

#include <string>
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedMath/Vector.h"

//-----------------------------------------------------------------------

class SetObjectPositionMessage : public GameNetworkMessage
{
public:
	SetObjectPositionMessage  (const NetworkId& id, const std::string &scene, bool added, bool authoritative);
	SetObjectPositionMessage  (Archive::ReadIterator & source);
	~SetObjectPositionMessage ();

	bool                      getAdded() const;
	bool                      getAuthoritative() const;
	const NetworkId&          getId(void) const;
	const std::string&        getScene(void) const;

private:
	Archive::AutoVariable<bool>              m_added;
	Archive::AutoVariable<bool>              m_authoritative;
	Archive::AutoVariable<NetworkId>         m_id;			// id of object that was created
	Archive::AutoVariable<std::string>       m_scene;		// scene (planet) the object is in

	SetObjectPositionMessage();
	SetObjectPositionMessage(const SetObjectPositionMessage&);
	SetObjectPositionMessage& operator= (const SetObjectPositionMessage&);
};


//-----------------------------------------------------------------------
inline bool SetObjectPositionMessage::getAdded() const
{
	return m_added.get();
}	
//-----------------------------------------------------------------------

inline bool SetObjectPositionMessage::getAuthoritative() const
{
	return m_authoritative.get();
}	
//-----------------------------------------------------------------------

inline const NetworkId& SetObjectPositionMessage::getId(void) const
{
	return m_id.get();
}	
//-----------------------------------------------------------------------

inline const std::string& SetObjectPositionMessage::getScene(void) const
{
	return m_scene.get();
}	
//-----------------------------------------------------------------------


#endif	// _INCLUDED_SetObjectPositionMessage_H
