// ======================================================================
//
// Appearance.cpp
// Portions copyright 1999 Bootprint Entertainment
// Portions copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedObject/FirstSharedObject.h"
#include "sharedObject/Appearance.h"

#include "sharedCollision/CollideParameters.h"
#include "sharedCollision/ExtentList.h"
#include "sharedCollision/Extent.h"
#include "sharedCollision/FloorManager.h"
#include "sharedCollision/Floor.h"
#include "sharedCollision/CollisionProperty.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedMath/AxialBox.h"
#include "sharedMath/DebugShapeRenderer.h"
#include "sharedObject/AlterResult.h"
#include "sharedObject/AppearanceTemplate.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/CustomizationData.h"
#include "sharedObject/CustomizationDataProperty.h"
#include "sharedObject/Hardpoint.h"
#include "sharedObject/Object.h"

// ======================================================================

namespace AppearanceNamespace
{
#ifdef _DEBUG
	bool ms_drawAppearanceFrames;
#endif

	int ms_frameNumber;
	bool ms_renderHardpoints;
	bool ms_renderHardpointNames;
	Appearance::RenderHardpointFunction ms_renderHardpointFunction;

	bool s_renderEnabled = true;
	
	//----------------------------------------------------------------------

}

using namespace AppearanceNamespace;

// ======================================================================

void Appearance::install()
{
#ifdef _DEBUG
	DebugFlags::registerFlag(ms_drawAppearanceFrames, "SharedObject", "drawAppearanceFrames");
	DebugFlags::registerFlag(ms_renderHardpoints, "SharedObject", "renderHardpoints");
	DebugFlags::registerFlag(ms_renderHardpointNames, "SharedObject", "renderHardpointNames");
#endif
}

// ----------------------------------------------------------------------

void Appearance::beginNewFrame()
{
	++ms_frameNumber;
}

// ----------------------------------------------------------------------

void Appearance::setRenderHardpointFunction(RenderHardpointFunction renderHardpointFunction)
{
	ms_renderHardpointFunction = renderHardpointFunction;
}

// ======================================================================
// Construct a new Appearance

Appearance::Appearance(const AppearanceTemplate *newAppearanceTemplate) :
	m_appearanceTemplate(AppearanceTemplateList::fetch(newAppearanceTemplate)),
	m_extent(m_appearanceTemplate ? ExtentList::fetch(m_appearanceTemplate->getExtent()) : nullptr),
	m_owner(nullptr),
	m_renderedFrameNumber(0),
	m_scale(Vector::xyz111),
	m_keepAlive(false),
	m_useRenderEffectsFlag(false),
	m_shadowBlobAllowed(false)
{
}

// ----------------------------------------------------------------------
/**
 * Destroy an Appearance.
 * 
 * This routine will release the AppearanceTemplate used by this Appearance.
 */

Appearance::~Appearance()
{
	ExtentList::release(m_extent);
	m_extent = nullptr;

	if (m_appearanceTemplate)
	{
		AppearanceTemplateList::release(m_appearanceTemplate);
		m_appearanceTemplate = nullptr;
	}

	m_owner = nullptr;
}

// ----------------------------------------------------------------------
/**
 * Check if the data for this appearance has been loaded.
 */

bool Appearance::isLoaded() const
{
	return true;
}

// ----------------------------------------------------------------------
/**
 * Alter the appearance specifying the amount of time that has passed.
 * 
 * This routine can be used to modify the appearance before it's rendered
 */

float Appearance::alter(float deltaTime)
{
	UNREF(deltaTime);
	return (isAlive() ? AlterResult::cms_keepNoAlter : AlterResult::cms_kill);
}

// ----------------------------------------------------------------------
/**
 * Get an Extent to be used for selection of this object in tools
 * 
 * This routine may be overloaded in derived classes.
 */

const Extent *Appearance::getSelectionExtent() const
{
	return getExtent();
}

// ----------------------------------------------------------------------
/**
 * Get the Extent of this Appearance.
 * 
 * This routine may be overloaded in derived classes.
 */

const Extent *Appearance::getExtent() const
{
	return m_extent;
}

// ----------------------------------------------------------------------
/**
 * Set a new Extent for this Appearance.
 * 
 * The current extent will be released.
 * 
 * @param newExtent  New extent for this Appearance
 */

void Appearance::setExtent(const Extent *extent)
{
	ExtentList::release(m_extent);
	m_extent = extent;

	extentChanged();
}

// ----------------------------------------------------------------------
/**
 * Get the tangible axial box, if applicable, for systems such as targeting.
 * Derived appearances can choose how much of the appearance contributes.
 */

AxialBox const Appearance::getTangibleExtent() const
{
	return AxialBox();
}

// ----------------------------------------------------------------------
/**
 * Return the bounding sphere for the Appearance.
 * 
 * This routine may be overloaded in derived classes. By default it
 * gets the spheere from its extents. If the extent doesn't exist, it
 * returns the zero sphere
 *
 * @return The bounding sphere in object space.
 */

const Sphere &Appearance::getSphere() const
{
	const Extent *const extent = getExtent();
	return extent ? extent->getSphere() : Sphere::zero;
}

// ----------------------------------------------------------------------

void Appearance::addToWorld()
{
	DPVS::Object *dpvsObject = getDpvsObject();
	if (dpvsObject)
	{
		NOT_NULL(m_owner);
		m_owner->addDpvsObject(dpvsObject);
	}
}

// ----------------------------------------------------------------------

void Appearance::removeFromWorld()
{
	DPVS::Object *dpvsObject = getDpvsObject();
	if (dpvsObject)
	{
		Object *owner = getOwner();
		if (owner)
		{
			owner->removeDpvsObject(dpvsObject);
		}
	}
}

// ----------------------------------------------------------------------

void Appearance::preRender() const
{
}

// ----------------------------------------------------------------------

void Appearance::render() const
{
#ifdef _DEBUG
	const Object *owner = getOwner();
	if (owner)
	{
		DebugShapeRenderer * const renderer = DebugShapeRenderer::create(owner);
		drawDebugShapes(renderer);
		delete renderer;

		if (ms_renderHardpoints && ms_renderHardpointFunction)
		{
			for (int i = 0; i < getHardpointCount(); ++i)
			{
				Hardpoint const & hardpoint = getHardpoint(i);

				Transform transform_h2w;
				transform_h2w.multiply(getTransform_w(), hardpoint.getTransform());

				ms_renderHardpointFunction(transform_h2w, ms_renderHardpointNames, hardpoint.getName().getString());
			}
		}
	}
#endif
}

//----------------------------------------------------------------------

void Appearance::objectListCameraRenderDescend(Object const & obj)
{
	//-- don't descend through cells
	if (nullptr != obj.getCellProperty())
		return;

	int const childCount = obj.getNumberOfChildObjects();
	for (int i = 0; i < childCount; ++i)
	{
		// Get the child object.
		Object const *const childObject = obj.getChildObject(i);
		if (!childObject)
			continue;
		
		if (!childObject->isActive())
			continue;
		
		// Setup child object appearance to world transform.
		const_cast<Object*>(childObject)->setTransform_a2w(childObject->getTransform_o2w());
		
		// Render its appearance.
		const Appearance *const childAppearance = childObject->getAppearance();
		if (childAppearance)
			childAppearance->objectListCameraRender();
		else
			objectListCameraRenderDescend(*childObject);
	}
}

// ----------------------------------------------------------------------

void Appearance::objectListCameraRender() const
{
	if (!usesRenderEffectsFlag() || canRenderEffects())
	{
		//-- Render this appearance.
		render();

		setRenderedThisFrame();

		//-- Render the owner's child objects.
		Object const *const owner = getOwner();
		if (!owner)
			return;

		int const childCount = owner->getNumberOfChildObjects();
		for (int i = 0; i < childCount; ++i)
		{
			// Get the child object.
			Object const *const childObject = owner->getChildObject(i);
			if (!childObject)
				continue;

			if (!childObject->isActive())
				continue;

			const Appearance *const childAppearance = childObject->getAppearance();
			if (childAppearance)
			{
				// Setup child object appearance to world transform.
				childAppearance->setTransform_w(childObject->getTransform_o2w());

				// Render its appearance.
				childAppearance->objectListCameraRender();
			}
			else
				objectListCameraRenderDescend(*childObject);
		}
	}
}

// ----------------------------------------------------------------------

void Appearance::setTexture(Tag /*tag*/, const Texture & /*texture*/)
{
}

// ----------------------------------------------------------------------
/**
 * Cast a ray through object space and return the result of the collision (if any) to result
 * 
 * @param  start_o  Object space start position of the ray
 * @param  end_o    Object space end position of the ray
 * @param  collideParameters Struct which contains information about the type and quality of collision
 * @param  result   Struct which contains information about the collision
 * @return          Whether or not a collision occurred
 */

bool Appearance::collide(Vector const & /*start_o*/, Vector const & /*end_o*/, CollideParameters const & /*collideParameters*/, CollisionInfo & /*result*/) const
{
	return false;
}

// ----------------------------------------------------------------------

bool Appearance::implementsCollide() const
{
	return false;
}

// ----------------------------------------------------------------------
/**
 * Set the new owning object associated with this appearance.
 *
 * There is a 1-1 correpsondence between Object instances and Appearance
 * instances.  This function is called to set the Object instance
 * associated with this Appearance instance.
 *
 * This implementation checks the owner Object instance for a
 * CustomizationDataProperty.  If there is such a property, this
 * function will invoke Appearance::setCustomizationData() with the 
 * appropriate value.  If the property doesn't exist, this function
 * will invoke Appearance::setCustomizationData() with nullptr.  Note if
 * the caller sets the CustomizationDataProperty for an Object after
 * associating the Object instance with the appearance, the caller is
 * responsible for calling Appearance::setCustomizationData().
 *
 * Implementers overriding this function must ensure they chain back
 * to this function as it performs necessary state modification.
 *
 * -TRF- I'm not sure why this is virtual.  I think this probably needs
 * to be made non virtual.  Currently it is mandatory for descendants
 * that override this function to chain back to this function to handle
 * the CustomizationData setup.
 */

void Appearance::setOwner(Object* owner)
{
	//-- keep track of owner object.
	m_owner = owner;

	// DBE - This was added to mimic the behavior of when the appearance-to-world 
	// transform was stored in Object instead of Appearance.
	if (owner)
	{
		Appearance *ownerAppearance = owner->getAppearance();
		if (!ownerAppearance)
		{
			setTransform_w(owner->getTransform_o2w());
		}
		else if (ownerAppearance!=this)
		{
			setTransform_w(ownerAppearance->getTransform_w());
		}
	}

	//-- retrieve customization data for new owner
	CustomizationData *customizationData = 0;

	if (owner)
	{
		// get the property
		CustomizationDataProperty *const property = safe_cast<CustomizationDataProperty*>(owner->getProperty(CustomizationDataProperty::getClassPropertyId()));
		if (property)
		{
			// retrieve the customization data from the property
			customizationData = property->fetchCustomizationData();
		}
	}

	//-- set the CustomizationData
	setCustomizationData(customizationData);

	//-- release local reference
	if (customizationData)
		customizationData->release();

	//-- clear the rendered frame number
	m_renderedFrameNumber = 0;
}

// ----------------------------------------------------------------------

void Appearance::setAlpha(bool const /* opaqueEnabled */, float const /* opaqueAlpha*/, bool const /* alphaEnabled */, float const /* alphaAlpha */)
{
}

// ----------------------------------------------------------------------

void Appearance::setTransform_w(const Transform &t) const
{
	m_appearanceToWorld=t;
}

// ----------------------------------------------------------------------

const char * Appearance::getFloorName () const
{
	if (m_appearanceTemplate)
		return m_appearanceTemplate->getFloorName();

	return nullptr;
}

// ----------------------------------------------------------------------
/**
 * Set the CustomizationData associated with this Appearance instance.
 *
 * This function's primary purpose is to be overridden.  It allows
 * derived Appearance classes to do something with the CustomizationData
 * when the CustomizationData instance is set or changed.  The CustomizationData
 * instance changes when the owning Object is initially set or changed 
 * to a new Object instance.
 *
 * Derived classes are not required to chain call this function.
 *
 * Callers need only invoke this function directly if CustomizationData
 * is initialized for the owning Object after Appearance::setOwner() is
 * called.  The only feasible time this should happen is when the Object
 * and Appearance instances are created and associated prior to creation
 * of CustomizationData.
 *
 * @param customizationData  the new CustomizationData instance associated
 *                           with this Appearance instance.
 */

void Appearance::setCustomizationData(CustomizationData * /* customizationData */)
{
}

// ----------------------------------------------------------------------
/**
 * Add all CustomizationData variables influencing this Appearance instance
 * to the given CustomizationData instance.
 *
 * This is primarily useful as a mechanism for tools.  The game should already
 * know which customization variables it has enabled for objects via the
 * ObjectTemplate system.
 *
 * Derived classes do not need to chain down to this function.
 *
 * Implementers: there are no guarantees as to the order of calls between 
 * setCustomizationData() and this function.  In other words, the implementation 
 * should use the given CustomizationData arg to add variables.
 *
 * @param customizationData  the CustomizationData instance to which new
 *                           variables will be added.  Assumed to be empty
 *                           on entry.
 */

void Appearance::addCustomizationVariables(CustomizationData & /* customizationData */) const
{
}

// ----------------------------------------------------------------------

void Appearance::setKeepAlive(bool const keepAlive)
{
	m_keepAlive = keepAlive;
}

// ----------------------------------------------------------------------

bool Appearance::isAlive() const
{
	return m_keepAlive ? true : realIsAlive();
}

// ----------------------------------------------------------------------

bool Appearance::realIsAlive() const
{
	return true;
}

// ----------------------------------------------------------------------
/**
 * Find and return the transform for the given hardpoint.
 *
 * The transform is returned in local Object (Appearance) space.  It is a
 * hardpoint-to-Object transform.
 *
 * Note in the general case, it is not safe to look up an index for
 * a hardpoint and then use the index.  For skeletal appearances, indices
 * change when wearables are applied.  Name lookup is a must when
 * possibly dealing with skeletal appearances.
 *
 * @param hardpointName       the name of the hardpoint to look up.
 * @param hardpointTransform  the result is returned in this parameter
 *                            upon successful completion.
 *
 * @return  true if the hardpoint with the given name was found; false
 *          otherwise.
 */

bool Appearance::findHardpoint(CrcString const &hardpointName, Transform &hardpointTransform) const
{
	// There are no hardpoints if there is no appearance template.
	if (!m_appearanceTemplate)
		return false;

	//-- The default implementation is to lookup the hardpoint via the appearance template.
	const int hardpointIndex = getHardpointIndex(hardpointName, true);
	if (hardpointIndex < 0)
		return false;
	else
	{
		//-- Handle the appearance scale.
		Transform  scaleTransform(Transform::IF_none);
		scaleTransform.setToScale(m_scale);

		//-- Get the hardpoint transform from the hardpoint object.
		const Hardpoint &hardpoint = getHardpoint(hardpointIndex);
		hardpointTransform.multiply(scaleTransform, hardpoint.getTransform());

		return true;
	}
}

// ----------------------------------------------------------------------
/**
 * Notify this Appearance of a change in scale.
 *
 * Implementers of derived classes should chain down to this function to
 * ensure that the internal scale variable is set.
 *
 * @param scale  the new scale factor, where a scale of (1,1,1) is equivalent to not scaling.
 */

void Appearance::setScale(const Vector &scale)
{
	Vector const oldValue = m_scale;
	m_scale = scale;

	onScaleModified(oldValue, m_scale);
}

// ----------------------------------------------------------------------

const char * Appearance::getAppearanceTemplateName () const
{
	if (m_appearanceTemplate)
		return m_appearanceTemplate->getName ();
	else
		return 0;
}

// ----------------------------------------------------------------------

DPVS::Object *Appearance::getDpvsObject() const
{
	return nullptr;
}

// ----------------------------------------------------------------------

#ifdef _DEBUG

bool Appearance::debugRenderingEnabled() const
{
	return true;
}

#endif

// ----------------------------------------------------------------------

#ifdef _DEBUG

void Appearance::drawDebugShapes(DebugShapeRenderer * renderer) const
{
	if (renderer && debugRenderingEnabled())
	{
		if(m_appearanceTemplate)
			m_appearanceTemplate->drawDebugShapes(renderer);

		if (ms_drawAppearanceFrames)
			renderer->drawFrame(1.0f);

		const Object *owner = getOwner();
		if (owner)
		{
			CollisionProperty const * collision = owner->getCollisionProperty();
			if(collision)
				collision->drawDebugShapes(renderer);

			Object const * parent = owner->getParent();
			if (parent)
			{
				CellProperty const * cellProperty = parent->getCellProperty();
				if (cellProperty)
					cellProperty->drawDebugShapes(renderer);
			}
		}
	}
}

#endif

// ----------------------------------------------------------------------

#ifdef _DEBUG

int Appearance::getPolygonCount () const
{
	return 0;
}

#endif

// ----------------------------------------------------------------------

#ifdef _DEBUG

void Appearance::debugDump(std::string & /*result*/, const int /*indentLevel*/) const
{
}

#endif

// ----------------------------------------------------------------------

const IndexedTriangleList* Appearance::getRadarShape () const
{
	return 0;
}

// ----------------------------------------------------------------------

bool Appearance::getShadowBlobAllowed() const
{
	return m_shadowBlobAllowed;
}

// ----------------------------------------------------------------------

void Appearance::setShadowBlobAllowed()
{
	m_shadowBlobAllowed = true;
}

// ----------------------------------------------------------------------

void Appearance::renderShadowBlob (const Vector& /*position_o*/, const float /*radius*/) const
{
}

// ----------------------------------------------------------------------

void Appearance::renderReticle (const Vector& /*position_o*/, const float /*radius*/) const
{
}

// ----------------------------------------------------------------------

void Appearance::setRenderedThisFrame () const
{
	m_renderedFrameNumber = ms_frameNumber;
}

// ----------------------------------------------------------------------

bool Appearance::getRenderedThisFrame () const
{
	return m_renderedFrameNumber == ms_frameNumber;
}

//----------------------------------------------------------------------

bool Appearance::getRenderedLastFrame () const
{
	return m_renderedFrameNumber == (ms_frameNumber - 1);
}

// ----------------------------------------------------------------------

SkeletalAppearance2 * Appearance::asSkeletalAppearance2()
{
	return 0;
}

// ----------------------------------------------------------------------

SkeletalAppearance2 const * Appearance::asSkeletalAppearance2() const
{
	return 0;
}

// ----------------------------------------------------------------------

ComponentAppearance * Appearance::asComponentAppearance()
{
	return nullptr;
}

// ----------------------------------------------------------------------

ComponentAppearance const * Appearance::asComponentAppearance() const
{
	return nullptr;
}

// ----------------------------------------------------------------------

DetailAppearance * Appearance::asDetailAppearance()
{
	return 0;
}

// ----------------------------------------------------------------------

DetailAppearance const * Appearance::asDetailAppearance() const
{
	return 0;
}

// ----------------------------------------------------------------------

void Appearance::onEvent(LabelHash::Id /* eventId */)
{
	// Default: handle no events.
}

// ----------------------------------------------------------------------

int Appearance::getHardpointCount() const
{
	return m_appearanceTemplate != nullptr ? m_appearanceTemplate->getHardpointCount() : 0;
}

// ----------------------------------------------------------------------

int Appearance::getHardpointIndex(CrcString const &hardpointName, bool optional) const
{
	return m_appearanceTemplate != nullptr ? m_appearanceTemplate->getHardpointIndex(hardpointName, optional) : -1;
}

// ----------------------------------------------------------------------

const Hardpoint & Appearance::getHardpoint(int index) const
{
	NOT_NULL(m_appearanceTemplate);
	return m_appearanceTemplate->getHardpoint(index);
}

// ======================================================================
// class Appearance: PROTECTED
// ======================================================================

void Appearance::extentChanged() const
{
	const Object *owner = getOwner();
	if (owner)
	{
		const_cast<Object *>(owner)->extentChanged();
	}
}

// ----------------------------------------------------------------------

void Appearance::onScaleModified(Vector const &oldScale, Vector const &newScale)
{
	// Do nothing.
	UNREF(oldScale);
	UNREF(newScale);
}

// ----------------------------------------------------------------------

bool Appearance::canRenderEffects()
{
	return s_renderEnabled;
}

// ----------------------------------------------------------------------

void Appearance::enableRenderEffects(bool const yesno)
{
	s_renderEnabled = yesno;
}

// ----------------------------------------------------------------------

void Appearance::useRenderEffectsFlag(bool const yesno)
{
	m_useRenderEffectsFlag = yesno;
}

// ----------------------------------------------------------------------

bool Appearance::usesRenderEffectsFlag() const
{
	return m_useRenderEffectsFlag;
}

// ----------------------------------------------------------------------

ParticleEffectAppearance * Appearance::asParticleEffectAppearance()
{
	return nullptr;
}

// ----------------------------------------------------------------------

ParticleEffectAppearance const * Appearance::asParticleEffectAppearance() const
{
	return nullptr;
}

// ----------------------------------------------------------------------

SwooshAppearance * Appearance::asSwooshAppearance()
{
	return nullptr;
}

// ----------------------------------------------------------------------

SwooshAppearance const * Appearance::asSwooshAppearance() const
{
	return nullptr;
}

// ----------------------------------------------------------------------

LightningAppearance * Appearance::asLightningAppearance()
{
	return nullptr;
}

// ----------------------------------------------------------------------

LightningAppearance const * Appearance::asLightningAppearance() const
{
	return nullptr;
}

// ======================================================================
