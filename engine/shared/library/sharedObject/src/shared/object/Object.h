// ======================================================================
//
// Object.h
// copyright 1998 Bootprint Entertainment
// Copyright 2000-2001 Sony Online Entertainment
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_Object_H
#define INCLUDED_Object_H

// ======================================================================

class AlterScheduler;
class Appearance;
class AxialBox;
class Camera;
class CellProperty;
class ClientObject;
class CollisionProperty;
class Controller;
class Container;
class ContainedByProperty;
class Dynamics;
class Iff;
class ObjectNotification;
class ObjectTemplate;
class PortalProperty;
class Property;
class ScheduleData;
class ServerObject;
class SlottedContainer;
class SpatialSubdivisionHandle;
class Sphere;
class MemoryBlockManager;
class VolumeContainer;
typedef uint32 PropertyId;

namespace DPVS
{
	class Object;
};

#include "sharedFoundation/Tag.h"
#include "sharedFoundation/Watcher.h"
#include "sharedMath/Transform.h"
#include "sharedMath/Vector.h"
#include "sharedFoundation/NetworkId.h"

// ======================================================================

#ifdef _DEBUG

#define OBJECT_SUPPORTS_DESTROYED_FLAG   1
#define OBJECT_SUPPORTS_IS_ALTERING_FLAG 1

#else

#define OBJECT_SUPPORTS_DESTROYED_FLAG   1
#define OBJECT_SUPPORTS_IS_ALTERING_FLAG 1

#endif

// ======================================================================

// Base object class for all objects in the 3d system.
//
// Suffixes:
//    A == appearance
//    O == object
//    P == parent
//    W == world

class Object
{
	friend class AlterScheduler;

public:

	enum InitializeFlag
	{
		IF_clientOnly
	};

	enum DetachFlags
	{
		DF_none,		// Do not attach to another cell.
		DF_parent,		// Attach to the parent's cell.
		DF_world		// Place the object into the world cell.
	};

	typedef std::vector<DPVS::Object *>  DpvsObjects;
	typedef uint64                          ScheduleTime;

	class NotificationList;
	class NotificationListManager;

public:

	typedef void (*InWorldAddDpvsObjectHookFunction)(Object *object, DPVS::Object *dpvsObject);
	typedef void (*InWorldRemoveDpvsObjectHookFunction)(DPVS::Object *dpvsObject);
	typedef void (*LeakedDpvsObjectHookFunction)(DPVS::Object *dpvsObject);

	static void install(bool objectsAlterChildrenAndContents);
	static void setDpvsObjectHookFunctions(InWorldAddDpvsObjectHookFunction alreadyInWorldAddDpvsObjectHookFunction, InWorldRemoveDpvsObjectHookFunction inWorldRemoveDpvsObjectHookFunction, LeakedDpvsObjectHookFunction leakedDpvsObjectHookFunction);
	static void setDisallowObjectDelete(bool disallowObjectDelete);
	static bool shouldObjectsAlterChildrenAndContents();

public:

	Object();
	Object(const ObjectTemplate *objectTemplate, InitializeFlag clientOnly);
	Object(const ObjectTemplate *objectTemplate, const NetworkId &networkId);
	virtual ~Object();

	virtual bool          isInitialized() const;

	bool                  isInWorld() const;
	virtual void          addToWorld();
	virtual void          removeFromWorld();

	bool                  hasNotification(const ObjectNotification &notification) const;

	// If allowWhenInWorld is true, the notification can be added to the object even if the object is already
	// in the world. This could be a bad thing (since a notification usually expects to receive/ matched
	// addToWorld/removeFromWorld pairs), but in certain cases it's required. Be careful when setting
	// allowWhenInWorld to true.

	void                  addNotification(const ObjectNotification &notification, bool allowWhenInWorld = false);
	void                  removeNotification(const ObjectNotification &notification, bool allowWhenInWorld = false);

	WatchedByList        &getWatchedByList() const;

	virtual float         alter(float time);
	virtual void          conclude();

	bool                  getKill() const;
	virtual void          kill();

	virtual void          setRegionOfInfluenceEnabled(bool enabled) const;

	virtual ContainedByProperty* getContainedByProperty();
	virtual const ContainedByProperty* getContainedByProperty() const;

	//@todo these next 2 would be better implemented as notifications or messages.
	virtual void          containedByModified(const NetworkId & oldValue, const NetworkId & newValue,bool);
	virtual void          arrangementModified(int oldValue, int newValue,bool);

	const ObjectTemplate *getObjectTemplate() const;
	const char           *getObjectTemplateName() const;
	Tag                   getObjectType() const;

	const char           *getDebugName() const;
	void                  setDebugName(const char *debugName);
	std::string const     getDebugInformation(bool includeParent = false) const;

	bool                  isActive() const;
	virtual void          setActive(bool active);

	bool                  isAuthoritative() const;
	void                  setAuthoritative(bool authoritative);

	const NetworkId      &getNetworkId() const;
	void                  setNetworkId(const NetworkId &networkId);

	CellProperty         *getParentCell() const;
	virtual void          setParentCell(CellProperty *cellProperty);
	bool                  isInWorldCell() const;

	const Appearance     *getAppearance() const;
	Appearance           *getAppearance();
	Appearance           *stealAppearance();
	const Vector         &getAppearanceSphereCenter() const;
	const Vector          getAppearanceSphereCenter_w() const;
	real                  getAppearanceSphereRadius() const;
	void                  setAppearance(Appearance *appearance);
	void                  setAppearanceByName(char const *path);
	char const *          getAppearanceTemplateName() const;

	Sphere const getCollisionSphereExtent_o() const;
	Sphere const getCollisionSphereExtent_w() const;
	float getDistanceBetweenCollisionSpheres_w(Object const & object) const;

	void                  addDpvsObject(DPVS::Object *dpvsObject);
	void                  removeDpvsObject(DPVS::Object *dpvsObject);
	const DpvsObjects   *getDpvsObjects() const;

	const Controller     *getController() const;
	Controller           *getController();
	Controller           *stealController();
	void                  setController(Controller *controller);

	const Dynamics       *getDynamics() const;
	Dynamics             *getDynamics();
	Dynamics             *stealDynamics();
	void                  setDynamics(Dynamics *dynamics);

	void                  attachToObject_p(Object *object, bool asChildObject);
	void                  attachToObject_w(Object *object, bool asChildObject);
	void                  detachFromObject(DetachFlags detachFlags);

	bool                  isChildObject() const;
	bool                  hasChildObjects() const;
	Object               *getParent();
	const Object         *getParent() const;
	Object               *getAttachedTo();
	const Object         *getAttachedTo() const;
	Object               *getRootParent();
	const Object         *getRootParent() const;
	int                   getNumberOfAttachedObjects() const;
	Object *              getAttachedObject(int attachedObjectIndex);
	Object const *        getAttachedObject(int attachedObjectIndex) const;
	int                   getNumberOfChildObjects() const;
	Object               *getChildObject(int childObjectIndex);
	const Object         *getChildObject(int childObjectIndex) const;
	void                  addChildObject_o(Object *childObject);
	void                  addChildObject_p(Object *childObject);
	void                  removeChildObject(Object *childObjectToRemove, DetachFlags detachFlags) const;

	void                  addProperty(Property& property);
	void                  addProperty(Property& property, bool allowWhileInWorld);
	const Property       *getProperty(const PropertyId& id) const;
	Property             *getProperty(const PropertyId& id); 
	void                  removeProperty(const PropertyId& id);

	const Vector         &getScale() const;
	void                  setScale(const Vector &scale);
	// recursively scales this and all of it's children
	// parent space locations of the children are _not_ scaled
	void setRecursiveScale(Vector const & scale);

	const Transform      &getTransform_o2p() const;
	DLLEXPORT Transform const & getTransform_o2w() const;
	const Transform       getTransform_o2c() const;
	const Transform      &getTransform_p2w() const;
	const Vector          findPosition_w() const;
	const Vector          getPosition_w() const;
	const Vector          getPosition_c() const;
	const Vector          getPosition_p() const;
	void                  setPosition_p(const Vector &position_p);
	void                  setPosition_w(const Vector &position_w);
	void                  setTransform_o2p(const Transform &objectToParentTransform);
	void                  setTransform_o2w(const Transform &objectToWorldTransform);
	void                  setTransformIJK_o2p(const Vector &i, const Vector &j, const Vector &k);
	void                  setTransformKJ_o2p(const Vector &k, const Vector &j);

	const Transform      &getTransform_a2w() const;
	void                  setTransform_a2w(const Transform &appearanceToWorldTransform) const;

	void                  move_p(const Vector &vectorInParentSpace);
	void                  move_o(const Vector &vectorInObjectSpace);

	void                  yaw_o(real radians);
	void                  pitch_o(real radians);
	void                  roll_o(real radians);
	void                  resetRotate_o2p();
	void                  resetRotateTranslate_o2p();
	void                  lookAt_p(const Vector &position_p, const Vector& j_o);
	void                  lookAt_p(const Vector &position_p);
	void                  lookAt_o(const Vector &position_o, const Vector& j_o);
	void                  lookAt_o(const Vector &position_o);

	const Vector          getObjectFrameI_p() const;
	const Vector          getObjectFrameJ_p() const;
	const Vector          getObjectFrameK_p() const;
	const Vector          getObjectFrameI_w() const;
	const Vector          getObjectFrameJ_w() const;
	const Vector          getObjectFrameK_w() const;

	const Vector          getParentFrameI_o() const;
	const Vector          getParentFrameJ_o() const;
	const Vector          getParentFrameK_o() const;

	const Vector          rotate_o2p(const Vector &vectorToTransform) const;
	const Vector          rotateTranslate_o2p(const Vector &vectorToTransform) const;
	const Vector          rotate_p2o(const Vector &vectorToTransform) const;
	const Vector          rotateTranslate_p2o(const Vector &vectorToTransform) const;
	const Vector          rotate_p2w(const Vector &vectorToTransform) const;
	const Vector          rotateTranslate_p2w(const Vector &vectorToTransform) const;
	const Vector          rotate_w2p(const Vector &vectorToTransform) const;
	const Vector          rotateTranslate_w2p(const Vector &vectorToTransform) const;

	const Vector          rotate_o2w(const Vector &vectorToTransform) const;
	const Vector          rotateTranslate_o2w(const Vector &vectorToTransform) const;
	const Sphere          rotateTranslate_o2w(const Sphere &sphereToTransform) const;
	const Vector          rotate_w2o(const Vector &vectorToTransform) const;
	const Vector          rotateTranslate_w2o(const Vector &vectorToTransform) const;
	const Sphere          rotateTranslate_w2o(const Sphere &sphereToTransform) const;

	virtual void          scheduleForAlter();

	Container *               getContainerProperty();
	Container const *         getContainerProperty() const;
	SlottedContainer *        getSlottedContainerProperty();
	SlottedContainer const *  getSlottedContainerProperty() const;
	VolumeContainer *         getVolumeContainerProperty();
	VolumeContainer const *   getVolumeContainerProperty() const;
	CellProperty *            getCellProperty();
	CellProperty const *      getCellProperty() const;
	PortalProperty *          getPortalProperty();
	PortalProperty const *    getPortalProperty() const;

	CollisionProperty *       getCollisionProperty();
	CollisionProperty const * getCollisionProperty() const;

	SpatialSubdivisionHandle* getSpatialSubdivisionHandle ();
	void setSpatialSubdivisionHandle (SpatialSubdivisionHandle* spatialSubdivisionHandle);

	virtual ClientObject *       asClientObject();
	virtual ClientObject const * asClientObject() const;
	virtual ServerObject *       asServerObject();
	virtual ServerObject const * asServerObject() const;

#if OBJECT_SUPPORTS_DESTROYED_FLAG
	bool                         isDestroyed() const;
#endif

#if OBJECT_SUPPORTS_IS_ALTERING_FLAG
	bool                         isAltering() const;
#endif

	void extentChanged();
	void pobFixupComplete();
	AxialBox const getTangibleExtent() const;

	bool getUseAlterScheduler() const;
	void setUseAlterScheduler(bool const useAlterScheduler);

	virtual void getObjectInfo(std::map<std::string, std::map<std::string, Unicode::String> > & propertyMap) const;

	bool getShouldBakeIntoMesh() const;
	void setShouldBakeIntoMesh(bool bake);

	void setAlternateAppearance(const char * path);
	void useDefaultAppearance();
	void useAlternateAppearance();
	bool isUsingAlternateAppearance() const;

protected:

	// scheduleForAlter() strategies available for use by derived classes.
	void scheduleForAlter_scheduleTopmostWorldParent();

	void nullWatchers();

	virtual void  positionChanged(bool dueToParentChange, const Vector &oldPosition);
	virtual void  rotationChanged(bool dueToParentChange);
	virtual void  positionAndRotationChanged(bool dueToParentChange, const Vector &oldPosition);

private:

	enum
	{
		ROTATIONS_BETWEEN_REORTHONORMALIZE = 255
	};

private:

	Object(const Object &);
	Object &operator =(const Object &);

private:

	void  reorthonormalize();
	void  cellChanged(bool dueToParentChange);

	void setObjectToWorldDirty(bool objectToWorldDirty) const;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// Begin AlterScheduler Interface
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	// ScheduleData creation.
	bool  hasScheduleData() const;
	void  createScheduleDataIfNecessary(ScheduleTime initialMostRecentAlterTime);

	// Most recent alter time management.
	ScheduleTime  getMostRecentAlterTime() const;
	void          setMostRecentAlterTime(ScheduleTime  mostRecentAlterTime);

	// Container management.
	bool  isInAlterNextFrameList() const;
	void  insertIntoAlterNextFrameList(Object *afterThisObject);
	void  removeFromAlterNextFrameList();

	int   getAlterSchedulePhase() const;
	void  setAlterSchedulePhase(int schedulePhaseIndex);

	bool  isInAlterNowList() const;
	void  insertIntoAlterNowList(Object *afterThisObject);
	void  removeFromAlterNowList();

	bool  isInConcludeList() const;
	void  insertIntoConcludeList(Object *afterThisObject);
	void  removeFromConcludeList();

	void *getScheduleTimeMapIterator();

	// Container traversal.
	Object *getNextFromAlterNowList();
	Object *getPreviousFromAlterNowList();

	Object *getNextFromAlterNextFrameList();
	Object *getPreviousFromAlterNextFrameList();

	Object *getNextFromConcludeList();
	Object *getPreviousFromConcludeList();

#if OBJECT_SUPPORTS_IS_ALTERING_FLAG
	void    setIsAltering(bool isAltering);
#endif

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// End AlterScheduler Interface
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

public:
	typedef std::vector<Property *> PropertyList;

protected:
	PropertyList * const m_propertyList;

private:

	bool m_inWorld:1;
	bool m_active:1;
	bool m_kill:1;
	bool m_authoritative:1;
	bool m_childObject:1;
	bool mutable m_objectToWorldDirty:1;
#if OBJECT_SUPPORTS_DESTROYED_FLAG
	bool m_isDestroyed:1;
#endif

#if OBJECT_SUPPORTS_IS_ALTERING_FLAG
	bool m_isAltering:1;
#endif

	const ObjectTemplate     *m_objectTemplate;
	const NotificationList   *m_notificationList;
	char                     *m_debugName;

	NetworkId                 m_networkId;

	Appearance               *m_appearance;
	Controller               *m_controller;
	Dynamics                 *m_dynamics;

	Object                   *m_attachedToObject;
	typedef std::vector<Object *> AttachedObjects;
	AttachedObjects          *m_attachedObjects;
	DpvsObjects              *m_dpvsObjects;

	int                       m_rotations;
	Vector                    m_scale;
	Transform                 m_objectToParent;
	Transform                *m_objectToWorld;

	mutable WatchedByList     m_watchedByList;

	Container                *m_containerProperty;
	CollisionProperty        *m_collisionProperty;

	SpatialSubdivisionHandle *m_spatialSubdivisionHandle;

	bool                      m_useAlterScheduler;
	ScheduleData             *m_scheduleData;

	bool					  m_shouldBakeIntoMesh;

	Appearance				 *m_defaultAppearance;
	Appearance				 *m_alternateAppearance;

protected:

	ContainedByProperty      *m_containedBy;
};

// ======================================================================
/**
 * Returns the object's template.
 */
inline const ObjectTemplate *Object::getObjectTemplate() const
{
	return m_objectTemplate;
}

// ----------------------------------------------------------------------
// Get the object's name
//
// Return value:
//
//   Const pointer to the object's name
//
// Remarks:
//
//   This return may return nullptr.

inline const char *Object::getDebugName() const
{
	return m_debugName;
}

// ----------------------------------------------------------------------
/**
 * Get the scale for this object.
 *
 * @return Const reference to the scale vector for this object's appearance
 */

inline const Vector &Object::getScale() const
{
	return m_scale;
}

// ----------------------------------------------------------------------
/**
 * Get the controller for an object.
 *
 * The return value may be nullptr.
 *
 * @return Const pointer to the controller
 */

inline const Controller *Object::getController() const
{
	return m_controller;
}

// ----------------------------------------------------------------------
/**
 * Get the controller for an object.
 *
 * The return value may be nullptr.
 *
 * @return Non-const pointer to the controller
 */

inline Controller *Object::getController()
{
	return m_controller;
}

// ----------------------------------------------------------------------
/**
 * Get the dynamics for an object.
 *
 * The return value may be nullptr.
 *
 * @return Const pointer to the dynamics
 */

inline const Dynamics *Object::getDynamics() const
{
	return m_dynamics;
}

// ----------------------------------------------------------------------
/**
 * Get the dynamics for an object.
 *
 * The return value may be nullptr.
 *
 * @return Non-const pointer to the dynamics
 */

inline Dynamics *Object::getDynamics()
{
	return m_dynamics;
}

// ----------------------------------------------------------------------
/**
 * Get the const appearance for this object.
 *
 * @return Const Pointer to the appearance for this object
 */

inline const Appearance *Object::getAppearance() const
{
	return m_appearance;
}

// ----------------------------------------------------------------------
/**
 * Get the appearance for this object.
 *
 * @return Pointer to the appearance for this object
 */

inline Appearance *Object::getAppearance()
{
	return m_appearance;
}

// ----------------------------------------------------------------------
/**
 * Get the parent object for this object.
 *
 * This routine will return nullptr if the object is not a child object.
 */

inline Object *Object::getParent()
{
	return m_childObject ? m_attachedToObject : nullptr;
}

// ----------------------------------------------------------------------
/**
 * Get the parent object for this object.
 *
 * This routine will return nullptr if the object is not a child object.
 */

inline const Object *Object::getParent() const
{
	return m_childObject ? m_attachedToObject : nullptr;
}

// ----------------------------------------------------------------------
/**
 * Get the object this object is attached to.
 *
 * The object may be a child of the attached object.
 */

inline Object *Object::getAttachedTo()
{
	return m_attachedToObject;
}

// ----------------------------------------------------------------------
/**
 * Get the parent object for this object.
 *
 * The object may be a child of the attached object.
 */

inline const Object *Object::getAttachedTo() const
{
	return m_attachedToObject;
}

// ----------------------------------------------------------------------
/**
 * Get the object-to-parent transformation for this object.
 *
 * @return Const reference to the object-to-parent transform for this object
 */

inline const Transform &Object::getTransform_o2p() const
{
	return m_objectToParent;
}

// ----------------------------------------------------------------------
/**
 * Get the parent-to-world transformation for this object.
 *
 * @return Const reference to the parent-to-world transform for this object
 */

inline const Transform &Object::getTransform_p2w() const
{
	if (m_attachedToObject)
		return m_attachedToObject->getTransform_o2w();

	return Transform::identity;
}

// ----------------------------------------------------------------------
/**
 * Get the position of this object in its parent space.
 *
 * This routine returns a temporary.
 */

inline const Vector Object::getPosition_p() const
{
	return m_objectToParent.getPosition_p();
}

// ----------------------------------------------------------------------
/**
 * Get the position of this object in world space.
 *
 * This routine returns a temporary.
 */

inline const Vector Object::getPosition_w() const
{
	return getTransform_o2w().getPosition_p();
}

// ----------------------------------------------------------------------
/**
 * Get the position of the object in cell space.
 *
 * This routine returns a temporary.
 */

inline const Vector Object::getPosition_c() const
{
	return getTransform_o2c().getPosition_p();
}

// ----------------------------------------------------------------------
/**
 * Set the position of this object in its parent space.
 *
 * @param newPositionInParentSpace  New position in the parent's space
 */

inline void Object::setPosition_p(const Vector &newPositionInParentSpace)
{
	const Vector oldPosition = getPosition_p();
	m_objectToParent.setPosition_p(newPositionInParentSpace);
	positionChanged(false, oldPosition);
}

// ----------------------------------------------------------------------
/**
 * Set the position of this object in world space.
 *
 * @param newPositionInWorldSpace  New position in the world space
 */

inline void Object::setPosition_w(const Vector &newPositionInWorldSpace)
{
	if (m_attachedToObject)
		setPosition_p(m_attachedToObject->rotateTranslate_w2o(newPositionInWorldSpace));
	else
		setPosition_p(newPositionInWorldSpace);
}

// ----------------------------------------------------------------------
/**
 * Set the object-to-parent transform for this object.
 *
 * @param newObjectToParentTransform  New object-to-parent transform
 */

inline void Object::setTransform_o2p(const Transform &newObjectToParentTransform)
{
	const Vector oldPosition = getPosition_p();
	m_objectToParent = newObjectToParentTransform;
	positionAndRotationChanged(false, oldPosition);
}

// ----------------------------------------------------------------------
/**
 * Set the object-to-parent transform for this object.
 *
 * @param i  Unit vector along the X axis
 * @param j  Unit vector along the Y axis
 * @param k  Unit vector along the Z axis
 */

inline void Object::setTransformIJK_o2p(const Vector &i, const Vector &j, const Vector &k)
{
	m_objectToParent.setLocalFrameIJK_p(i, j, k);
	rotationChanged(false);
}

// ----------------------------------------------------------------------
/**
 * Set the object-to-parent transform for this object.
 *
 * @param k  Unit vector along the Z axis
 * @param j  Unit vector along the Y axis
 */

inline void Object::setTransformKJ_o2p(const Vector &k, const Vector &j)
{
	m_objectToParent.setLocalFrameKJ_p(k, j);
	rotationChanged(false);
}

// ----------------------------------------------------------------------
/**
 * Reorthonormalize the object-to-world transform.
 *
 * Repeated rotations will introduce numerical error into the transform,
 * which will cause the upper 3x3 matrix to become non-orthonormal.  If
 * enough error is introduced, weird errors will begin to occur when using
 * the transform.
 *
 * This routine attempts to reduce the numerical error by reorthonormalizing
 * the upper 3x3 matrix.
 */

inline void Object::reorthonormalize()
{
	m_objectToParent.reorthonormalize();
	m_rotations = 0;
}

// ----------------------------------------------------------------------
/**
 * Yaw the object around its Y axis.
 *
 * This routine will rotate the object around its Y axis by the specified
 * number of radians.
 *
 * Positive rotations are clockwise when viewed from the positive side of
 * the axis being rotated about looking towards the origin.
 *
 * @param radians  Radians to yaw the object.
 * @see Object::pitch_o(), Object::roll_o(),
 */

inline void Object::yaw_o(real radians)
{
	if (radians != 0.0f)
	{
		m_objectToParent.yaw_l(radians);
		if (++m_rotations >= ROTATIONS_BETWEEN_REORTHONORMALIZE)
			reorthonormalize();
		rotationChanged(false);
	}
}

// ----------------------------------------------------------------------
/**
 * Pitch the object around its X axis.
 *
 * This routine will rotate the object around its X axis by the specified
 * number of radians.
 *
 * Positive rotations are clockwise when viewed from the positive side of
 * the axis being rotated about looking towards the origin.
 *
 * @param radians  Radians to pitch the object.
 * @see Object::yaw_o(), Object::roll_o(),
 */

inline void Object::pitch_o(real radians)
{
	if (radians != 0.0f)
	{
		m_objectToParent.pitch_l(radians);
		if (++m_rotations >= ROTATIONS_BETWEEN_REORTHONORMALIZE)
			reorthonormalize();
		rotationChanged(false);
	}
}

// ----------------------------------------------------------------------
/**
 * Roll the object around its Z axis.
 *
 * This routine will rotate the object around its Z axis by the specified
 * number of radians.
 *
 * Positive rotations are clockwise when viewed from the positive side of
 * the axis being rotated about looking towards the origin.
 *
 * @param radians  Radians to roll the object.
 * @see Object::yaw_o(), Object::pitch_o(),
 */

inline void Object::roll_o(real radians)
{
	if (radians != 0.0f)
	{
		m_objectToParent.roll_l(radians);
		if (++m_rotations >= ROTATIONS_BETWEEN_REORTHONORMALIZE)
			reorthonormalize();
		rotationChanged(false);
	}
}

// ----------------------------------------------------------------------
/**
 * Reset the objectToParent transform's rotations.
 *
 * This does NOT affect the object's position.  This will make the object
 * have the same orientation as its parent.
 */

inline void Object::resetRotate_o2p()
{
	m_objectToParent.resetRotate_l2p();
	m_rotations = 0;
	rotationChanged(false);
}

// ----------------------------------------------------------------------
/**
 * Reset the objectToParent transform.
 *
 * This will make the object have the same orientation and position as its parent.
 */

inline void Object::resetRotateTranslate_o2p()
{
	const Vector oldPosition = getPosition_p();
	m_objectToParent.resetRotateTranslate_l2p();
	m_rotations = 0;
	positionAndRotationChanged(false, oldPosition);
}

// ----------------------------------------------------------------------
/**
 * Get the parent-space vector pointing along the X axis of this frame of reference.
 *
 * This routine returns a temporary.
 *
 * @return The vector pointing along the X axis of the frame in parent space
 */

inline const Vector Object::getObjectFrameI_p() const
{
	return m_objectToParent.getLocalFrameI_p();
}

// ----------------------------------------------------------------------
/**
 * Get the parent-space vector pointing along the Y axis of this frame of reference.
 *
 * This routine returns a temporary.
 *
 * @return The vector pointing along the Y axis of the frame in parent space
 */

inline const Vector Object::getObjectFrameJ_p() const
{
	return m_objectToParent.getLocalFrameJ_p();
}

// ----------------------------------------------------------------------
/**
 * Get the parent-space vector pointing along the Z axis of this frame of reference.
 *
 * This routine returns a temporary.
 *
 * @return The vector pointing along the Z axis of the frame in parent space
 */

inline const Vector Object::getObjectFrameK_p() const
{
	return m_objectToParent.getLocalFrameK_p();
}

// ----------------------------------------------------------------------
/**
 * Get the parent-space vector pointing along the X axis of this frame of reference.
 *
 * This routine returns a temporary.
 *
 * @return The vector pointing along the X axis of the frame in parent space
 */

inline const Vector Object::getObjectFrameI_w() const
{
	return getTransform_o2w().getLocalFrameI_p();
}

// ----------------------------------------------------------------------
/**
 * Get the parent-space vector pointing along the Y axis of this frame of reference.
 *
 * This routine returns a temporary.
 *
 * @return The vector pointing along the Y axis of the frame in parent space
 */

inline const Vector Object::getObjectFrameJ_w() const
{
	return getTransform_o2w().getLocalFrameJ_p();
}

// ----------------------------------------------------------------------
/**
 * Get the parent-space vector pointing along the Z axis of this frame of reference.
 *
 * This routine returns a temporary.
 *
 * @return The vector pointing along the Z axis of the frame in parent space
 */

inline const Vector Object::getObjectFrameK_w() const
{
	return getTransform_o2w().getLocalFrameK_p();
}

// ----------------------------------------------------------------------
/**
 * Get the object-space vector pointing along the X axis of the parent of reference.
 *
 * This routine returns a temporary.
 *
 * The vector returned is in local space.
 *
 * @return The vector pointing along the X axis of the parent's frame in local space
 */

inline const Vector Object::getParentFrameI_o() const
{
	return m_objectToParent.getParentFrameI_l();
}

// ----------------------------------------------------------------------
/**
 * Get the object-space vector pointing along the Y axis of the parent of reference.
 *
 * This routine returns a temporary.
 *
 * The vector returned is in local space.
 *
 * @return The vector pointing along the Y axis of the parent's frame in local space
 */

inline const Vector Object::getParentFrameJ_o() const
{
	return m_objectToParent.getParentFrameJ_l();
}

// ----------------------------------------------------------------------
/**
 * Get the object-space vector pointing along the Z axis of the parent of reference.
 *
 * This routine returns a temporary.
 *
 * The vector returned is in local space.
 *
 * @return The vector pointing along the Z axis of the parent's frame in local space
 */

inline const Vector Object::getParentFrameK_o() const
{
	return m_objectToParent.getParentFrameK_l();
}

// ----------------------------------------------------------------------
/**
 * Rotate vector from the object's frame to the parent frame.
 *
 * @param vector  Vector to rotate from object space into parent space
 * @return The source object-space vector rotated into parent space
 */

inline const Vector Object::rotate_o2p(const Vector &vector) const
{
	return m_objectToParent.rotate_l2p(vector);
}

// ----------------------------------------------------------------------
/**
 * Calculate vector from the object's frame to the parent frame.
 *
 * @param vector  Vector to rotate and translate from object space into parent space
 * @return The source object-space vector rotated and translated into parent space
 */

inline const Vector Object::rotateTranslate_o2p(const Vector &vector) const
{
	return m_objectToParent.rotateTranslate_l2p(vector);
}

// ----------------------------------------------------------------------
/**
 * Rotate vector from the parent frame to the object's frame.
 *
 * @param vector  Vector to rotate from parent space into object space
 * @return The source parent-space vector rotated into object space
 */

inline const Vector Object::rotate_p2o(const Vector &vector) const
{
	return m_objectToParent.rotate_p2l(vector);
}

// ----------------------------------------------------------------------
/**
 * Calculate vector from the parent frame to the object's frame.
 *
 * @param vector  Vector to rotate and translate from parent space into object space
 * @return The source parent-space vector rotated into object space
 */

inline const Vector Object::rotateTranslate_p2o(const Vector &vector) const
{
	return m_objectToParent.rotateTranslate_p2l(vector);
}

// ----------------------------------------------------------------------
/**
 * Rotate vector from the object's frame to the world frame.
 *
 * @param vector  Vector to rotate from object space into world space
 * @return The source object-space vector rotated into world space
 */

inline const Vector Object::rotate_o2w(const Vector &vector) const
{
	return getTransform_o2w().rotate_l2p(vector);
}

// ----------------------------------------------------------------------
/**
 * Calculate vector from the object's frame to the world frame.
 *
 * @param vector  Vector to rotate and translate from object space into world space
 * @return The source object-space vector rotated and translated into world space
 */

inline const Vector Object::rotateTranslate_o2w(const Vector &vector) const
{
	return getTransform_o2w().rotateTranslate_l2p(vector);
}

// ----------------------------------------------------------------------
/**
 * Rotate vector from the world frame to the object's frame.
 *
 * @param vector  Vector to rotate from world space into object space
 * @return The source world-space vector rotated into object space
 */

inline const Vector Object::rotate_w2o(const Vector &vector) const
{
	return getTransform_o2w().rotate_p2l(vector);
}

// ----------------------------------------------------------------------
/**
 * Calculate vector from the world frame to the object's frame.
 *
 * @param vector  Vector to rotate and translate from world space into object space
 * @return The source world-space vector rotated into object space
 */

inline const Vector Object::rotateTranslate_w2o(const Vector &vector) const
{
	return getTransform_o2w().rotateTranslate_p2l(vector);
}

// ----------------------------------------------------------------------
/**
 * Calculate vector from the parent frame to the world frame.
 *
 * @param vector  Vector to rotate and translate from parent space into world space
 * @return The source parent-space vector rotated into world space
 */

inline const Vector Object::rotateTranslate_p2w(const Vector &vector) const
{
	if (m_attachedToObject)
		return m_attachedToObject->rotateTranslate_o2w(vector);

	return vector;
}

// ----------------------------------------------------------------------
/**
 * Calculate vector from the parent frame to the world frame.
 *
 * @param vector  Vector to rotate from parent space into world space
 * @return The source parent-space vector rotated into world space
 */

inline const Vector Object::rotate_p2w(const Vector &vector) const
{
	if (m_attachedToObject)
		return m_attachedToObject->rotate_o2w(vector);

	return vector;
}

// ----------------------------------------------------------------------
/**
 * Calculate vector from the world frame to the parent frame.
 *
 * @param vector  Vector to rotate from world space into parent space
 * @return The source world-space vector rotated into parent space
 */

inline const Vector Object::rotate_w2p(const Vector &vector) const
{
	if (m_attachedToObject)
		return m_attachedToObject->rotate_w2o(vector);

	return vector;
}

// ----------------------------------------------------------------------
/**
 * Calculate vector from the world frame to the parent frame.
 *
 * @param vector  Vector to rotate from world space into parent space
 * @return The source world-space vector rotated into parent space
 */

inline const Vector Object::rotateTranslate_w2p(const Vector &vector) const
{
	if (m_attachedToObject)
		return m_attachedToObject->rotateTranslate_w2o(vector);

	return vector;
}

// ----------------------------------------------------------------------
/**
 * Move the object in it's own local space.
 *
 * This routine moves the object according to its current frame of reference.
 * Therefore, moving along the Z axis will move the object forward in the direction
 * in which it is pointed.
 *
 * @param vectorInObjectSpace  Offset to move in local space
 * @see Object::move_p()
 */

inline void Object::move_o(const Vector &vectorInObjectSpace)
{
	move_p(rotate_o2p(vectorInObjectSpace));
}

// ----------------------------------------------------------------------
/**
 * @internal
 */

inline WatchedByList &Object::getWatchedByList() const
{
	return m_watchedByList;
}

// ----------------------------------------------------------------------

inline const Object::DpvsObjects *Object::getDpvsObjects() const
{
	return m_dpvsObjects;
}

// ----------------------------------------------------------------------

inline Container * Object::getContainerProperty()
{
	return m_containerProperty;
}

// ----------------------------------------------------------------------

inline Container const * Object::getContainerProperty() const
{
	return m_containerProperty;
}

//-----------------------------------------------------------------------

inline ContainedByProperty * Object::getContainedByProperty()
{
	return m_containedBy;
}

//-----------------------------------------------------------------------

inline ContainedByProperty const * Object::getContainedByProperty() const
{
	return m_containedBy;
}

// ----------------------------------------------------------------------

inline CollisionProperty * Object::getCollisionProperty()
{
	return m_collisionProperty;
}

// ----------------------------------------------------------------------

inline CollisionProperty const * Object::getCollisionProperty() const
{
	return m_collisionProperty;
}

// ----------------------------------------------------------------------
/**
 * Determine if this object is a child object.
 *
 * If the object is a child object, it will have a parent object.
 *
 * @return True if the object is a child object, otherwise false.
 */

inline bool Object::isChildObject() const
{
	return m_childObject;
}

// ----------------------------------------------------------------------

inline bool Object::getKill() const
{
	return m_kill;
}

// ----------------------------------------------------------------------
/**
 * Return the object is authoritative.
 */

inline bool Object::isAuthoritative() const
{
	return m_authoritative;
}

// ----------------------------------------------------------------------
/**
 * Specify whether the object is authoritative.
 */

inline void Object::setAuthoritative(bool newAuthoritative)
{
	m_authoritative = newAuthoritative;
}

// ----------------------------------------------------------------------
/**
 * Check if the object is active.
 *
 * @return True if the object is active, otherwise false
 */

inline bool Object::isActive() const
{
	return m_active;
}

// ----------------------------------------------------------------------
/**
 * Returns whether the object is in the world or not.
 */

inline bool Object::isInWorld() const
{
	return m_inWorld;
}

// ----------------------------------------------------------------------

inline const NetworkId &Object::getNetworkId() const
{
	return m_networkId;
}

inline bool Object::getShouldBakeIntoMesh() const
{
	return m_shouldBakeIntoMesh;
}

inline void Object::setShouldBakeIntoMesh(bool bake)
{
	m_shouldBakeIntoMesh = bake;
}

inline bool Object::isUsingAlternateAppearance() const
{
	return m_appearance == m_alternateAppearance;
}

// ======================================================================

#endif
