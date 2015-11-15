//========================================================================
//
// MessageQueueResourceSetInstalledEfficiency.h
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================


#ifndef INCLUDED_MessageQueueResourceSetInstalledEfficiency_H
#define INCLUDED_MessageQueueResourceSetInstalledEfficiency_H

#include "sharedFoundation/MessageQueue.h"

/**
 * Sent by:  Resource Pool
 * Sent to:  Harvester
 * Action:  Informs the harvester what its efficiency is, based on its
 * installed location
 */

class MessageQueueResourceSetInstalledEfficiency : public MessageQueue::Data
{
public:
	MessageQueueResourceSetInstalledEfficiency(float installedEfficiency, float tickCount);
	virtual ~MessageQueueResourceSetInstalledEfficiency();
	
	MessageQueueResourceSetInstalledEfficiency&	operator=	(const MessageQueueResourceSetInstalledEfficiency & source);
	MessageQueueResourceSetInstalledEfficiency(const MessageQueueResourceSetInstalledEfficiency & source);

	float getInstalledEfficiency() const;
	float getTickCount() const;
	
private:
	float m_installedEfficiency;
	float m_tickCount;
};

// ======================================================================

inline float MessageQueueResourceSetInstalledEfficiency::getInstalledEfficiency() const
{
	return m_installedEfficiency;
}

// ----------------------------------------------------------------------

inline float MessageQueueResourceSetInstalledEfficiency::getTickCount() const
{
	return m_tickCount;
}

// ======================================================================

#endif
