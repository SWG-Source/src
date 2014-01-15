//========================================================================
//
// CreateNewObjectMessage.h - tells Centralserver a new object is being created.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef	_INCLUDED_CreateNewObjectMessage_H
#define	_INCLUDED_CreateNewObjectMessage_H

//-----------------------------------------------------------------------

#include <string>
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedMath/Vector.h"

//-----------------------------------------------------------------------

class CreateNewObjectMessage : public GameNetworkMessage
{
public:
	CreateNewObjectMessage  (const NetworkId& id, uint32 process, const std::string &scene, const Vector &pos);
	CreateNewObjectMessage  (Archive::ReadIterator & source);
	~CreateNewObjectMessage ();

	const NetworkId&          getId(void) const;
	uint32              getProcess(void) const;
	const std::string&  getScene(void) const;
	const Vector&       getPos(void) const;

private:
	Archive::AutoVariable<NetworkId>         m_id;			// id of object that was created
	Archive::AutoVariable<uint32>      m_process;		// process the object was created on
	Archive::AutoVariable<std::string> m_scene;		// scene (planet) the object is in
	Archive::AutoVariable<Vector>      m_pos;			// position of the object

	CreateNewObjectMessage();
	CreateNewObjectMessage(const CreateNewObjectMessage&);
	CreateNewObjectMessage& operator= (const CreateNewObjectMessage&);
};


//-----------------------------------------------------------------------

inline const NetworkId& CreateNewObjectMessage::getId(void) const
{
	return m_id.get();
}	// CreateNewObjectMessage::getId

inline uint32 CreateNewObjectMessage::getProcess(void) const
{
	return m_process.get();
}	// CreateNewObjectMessage::getProcess

inline const std::string& CreateNewObjectMessage::getScene(void) const
{
	return m_scene.get();
}	// CreateNewObjectMessage::getScene

inline const Vector& CreateNewObjectMessage::getPos(void) const
{
	return m_pos.get();
}	// CreateNewObjectMessage::getPos

#endif	// _INCLUDED_CreateNewObjectMessage_H
