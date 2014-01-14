// ======================================================================
//
// DebugYawedFloorNotification.cpp
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#include "sharedObject/FirstSharedObject.h"
#include "sharedObject/DebugYawedFloorNotification.h"

#include "sharedCollision/CollisionProperty.h"
#include "sharedObject/Object.h"

#include <string>

// ======================================================================

DebugYawedFloorNotification DebugYawedFloorNotification::ms_instance;

// ======================================================================
// STATIC PUBLIC DebugYawedFloorNotification
// ======================================================================

DebugYawedFloorNotification& DebugYawedFloorNotification::getInstance ()
{
	return ms_instance;
}

// ======================================================================
// PUBLIC DebugYawedFloorNotification
// ======================================================================

DebugYawedFloorNotification::DebugYawedFloorNotification () :
	ObjectNotification ()
{
}

//-------------------------------------------------------------------
	
DebugYawedFloorNotification::~DebugYawedFloorNotification ()
{
}

//-------------------------------------------------------------------

void DebugYawedFloorNotification::addToWorld (Object& object) const
{
	UNREF (object);

#ifdef _DEBUG
	if (object.isChildObject ())
		return;

	const CollisionProperty* const collisionProperty = object.getCollisionProperty ();
	if (!collisionProperty)
		return;

	if (collisionProperty->getFloor () && object.getObjectFrameJ_w ().dot (Vector::unitY) < 0.99f)
	{
		const Vector position_w = object.getPosition_w ();
		DEBUG_WARNING (true, ("DebugYawedFloorNotification: object %s [%s] has a floor but is not oriented straight up at <%1.2f, %1.2f>", object.getNetworkId ().getValueString ().c_str (), object.getObjectTemplateName (), position_w.x, position_w.z));
	}
#endif
}

// ----------------------------------------------------------------------

void DebugYawedFloorNotification::rotationChanged(Object &object, bool dueToParentChange) const
{
	if (dueToParentChange)
		return;

	addToWorld (object);
}

// ----------------------------------------------------------------------

bool DebugYawedFloorNotification::positionAndRotationChanged (Object& object, bool dueToParentChange, const Vector& /*oldPosition*/) const
{
	rotationChanged (object, dueToParentChange);

	return true;
}

// ======================================================================
