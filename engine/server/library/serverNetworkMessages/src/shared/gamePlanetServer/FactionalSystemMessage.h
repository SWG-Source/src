// ======================================================================
//
// FactionalSystemMessage.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_FactionalSystemMessage_H
#define INCLUDED_FactionalSystemMessage_H

// ======================================================================

#include "sharedNetworkMessages/GameNetworkMessage.h"

// ======================================================================

class FactionalSystemMessage : public GameNetworkMessage
{
public:
	FactionalSystemMessage  (Unicode::String const & prosePackage, Vector const & location, float radius, bool notifyImperial, bool notifyRebel);
	FactionalSystemMessage  (Archive::ReadIterator & source);
	~FactionalSystemMessage ();

	Unicode::String const & getProsePackage() const;
	Vector const & getLocation() const;
	float getRadius() const;
	bool getNotifyImperial() const;
	bool getNotifyRebel() const;

private:
	Archive::AutoVariable<Unicode::String> m_prosePackage;
	Archive::AutoVariable<Vector> m_location;
	Archive::AutoVariable<float> m_radius;
	Archive::AutoVariable<bool> m_notifyImperial;
	Archive::AutoVariable<bool> m_notifyRebel;

	FactionalSystemMessage();
	FactionalSystemMessage(const FactionalSystemMessage&);
	FactionalSystemMessage& operator= (const FactionalSystemMessage&);
};

// ----------------------------------------------------------------------

inline Unicode::String const & FactionalSystemMessage::getProsePackage() const
{
	return m_prosePackage.get();
}

// ----------------------------------------------------------------------

inline Vector const & FactionalSystemMessage::getLocation() const
{
	return m_location.get();
}

// ----------------------------------------------------------------------

inline float FactionalSystemMessage::getRadius() const
{
	return m_radius.get();
}

// ----------------------------------------------------------------------

inline bool FactionalSystemMessage::getNotifyImperial() const
{
	return m_notifyImperial.get();
}

// ----------------------------------------------------------------------

inline bool FactionalSystemMessage::getNotifyRebel() const
{
	return m_notifyRebel.get();
}

// ======================================================================

#endif
