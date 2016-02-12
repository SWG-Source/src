// ======================================================================
//
// CollisionWorld.cpp
// copyright (c) 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedCollision/FirstSharedCollision.h"
#include "sharedCollision/CollisionNotification.h"

#include "sharedCollision/CollisionProperty.h"
#include "sharedCollision/CollisionWorld.h"

#include "sharedObject/Object.h"

#include "sharedFoundation/Watcher.h"

#include <vector>

// ----------

struct AddRemoveQueueEntry
{
	bool            m_added;
	Watcher<Object> m_object;
};

// ----------

namespace CollisionNotificationNamespace
{

CollisionNotification g_collisionNotification;

std::vector<AddRemoveQueueEntry> g_addRemoveQueue;

};

using namespace CollisionNotificationNamespace;

// ----------------------------------------------------------------------

CollisionNotification::CollisionNotification()
: ObjectNotification()
{
}

CollisionNotification::~CollisionNotification()
{
}

// ----------

CollisionNotification & CollisionNotification::getInstance ( void )
{
	return g_collisionNotification;
}

// ----------

int CollisionNotification::getPriority() const
{
	return 4;
}

// ----------------------------------------------------------------------

void CollisionNotification::purgeQueue ( void )
{
	int count = g_addRemoveQueue.size();

	for(int i = 0; i < count; i++)
	{
		AddRemoveQueueEntry & entry = g_addRemoveQueue[i];

		Object * object = entry.m_object;

		if(object == nullptr) continue;

		if(entry.m_added)
		{
			CollisionWorld::addObject(object);
		}
		else
		{
			CollisionWorld::removeObject(object);
		}
	}

	g_addRemoveQueue.clear();
}

// ----------------------------------------------------------------------

void CollisionNotification::visibilityDataChanged ( Object & object ) const
{
	CollisionWorld::appearanceChanged(&object);
}

// ----------

void CollisionNotification::extentChanged ( Object & object ) const
{
	CollisionWorld::appearanceChanged(&object);
}

// ----------

void CollisionNotification::addToWorld ( Object & object ) const
{
	if(CollisionWorld::isUpdating())
	{
		AddRemoveQueueEntry entry;

		entry.m_added = true;
		entry.m_object = &object;

		g_addRemoveQueue.push_back(entry);
	}
	else
	{
		CollisionWorld::addObject(&object);
	}
}

// ----------

void CollisionNotification::removeFromWorld( Object & object ) const
{
	if(CollisionWorld::isUpdating())
	{
		AddRemoveQueueEntry entry;

		entry.m_added = false;
		entry.m_object = &object;

		g_addRemoveQueue.push_back(entry);
	}
	else
	{
		CollisionWorld::removeObject(&object);
	}
}

// ----------

bool CollisionNotification::positionChanged( Object & object, bool, Vector const & ) const
{
	CollisionWorld::moveObject(&object);

	return true;
}

// ----------

void CollisionNotification::rotationChanged( Object & object, bool ) const
{
	CollisionWorld::moveObject(&object);
}

// ----------

bool CollisionNotification::positionAndRotationChanged ( Object & object, bool, Vector const & ) const
{
	CollisionWorld::moveObject(&object);

	return true;
}

// ----------

void CollisionNotification::cellChanged( Object & object, bool ) const
{
	CollisionWorld::cellChanged(&object);
}

// ----------------------------------------------------------------------
