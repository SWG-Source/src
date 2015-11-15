// ======================================================================
//
// SetAuthoritativeMessage.h - tells a Gameserver which server is authoritative
// over an object.
//
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef	_INCLUDED_SetAuthoritativeMessage_H
#define	_INCLUDED_SetAuthoritativeMessage_H

// ======================================================================

#include "sharedMath/Transform.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

// ======================================================================

class SetAuthoritativeMessage: public GameNetworkMessage
{
public:
	SetAuthoritativeMessage(NetworkId const &id, uint32 process, bool sceneChange, bool handlingCrash, NetworkId const &goalCell, Transform const &goalTransform, bool goalIsValid);
	SetAuthoritativeMessage(Archive::ReadIterator & source);
	~SetAuthoritativeMessage();

	NetworkId const &  getId() const;
	uint32             getProcess() const;
	bool               getSceneChange() const;
	bool               getHandlingCrash() const;
	NetworkId const &  getGoalCell() const;
	Transform const &  getGoalTransform() const;
	bool               getGoalIsValid() const;
	
private:
	Archive::AutoVariable<NetworkId> m_id;             // id of object being made authoritative
	Archive::AutoVariable<uint32>    m_process;        // process that is authoritative
	Archive::AutoVariable<bool>      m_sceneChange;    // whether authority transfer due to scene change
	Archive::AutoVariable<bool>      m_handlingCrash;  // whether authority transfer due to server crash
	Archive::AutoVariable<NetworkId> m_goalCell;       // Cell we're trying to move into
	Archive::AutoVariable<Transform> m_goalTransform;  // Transform we'd like to have when we move into the cell
	Archive::AutoVariable<bool>      m_goalIsValid;    // Whether the goalCell and goalTransform should be used

	SetAuthoritativeMessage();
	SetAuthoritativeMessage(SetAuthoritativeMessage const &);
	SetAuthoritativeMessage &operator=(SetAuthoritativeMessage const &);
};

// ----------------------------------------------------------------------

inline NetworkId const &SetAuthoritativeMessage::getId() const
{
	return m_id.get();
}

// ----------------------------------------------------------------------

inline uint32 SetAuthoritativeMessage::getProcess() const
{
	return m_process.get();
}

// ----------------------------------------------------------------------

inline bool SetAuthoritativeMessage::getSceneChange() const
{
	return m_sceneChange.get();
}

// ----------------------------------------------------------------------

inline bool SetAuthoritativeMessage::getHandlingCrash() const
{
	return m_handlingCrash.get();
}

// ----------------------------------------------------------------------

inline NetworkId const &SetAuthoritativeMessage::getGoalCell() const
{
	return m_goalCell.get();
}

// ----------------------------------------------------------------------

inline Transform const &SetAuthoritativeMessage::getGoalTransform() const
{
	return m_goalTransform.get();
}

// ----------------------------------------------------------------------

inline bool SetAuthoritativeMessage::getGoalIsValid() const
{
	return m_goalIsValid.get();
}

// ======================================================================

#endif // _INCLUDED_SetAuthoritativeMessage_H

