// ======================================================================
//
// Appearance.h
// Portions copyright 1999 Bootprint Entertainment
// Portions copyright 2001, 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_Appearance_H
#define INCLUDED_Appearance_H

// ======================================================================

#include "sharedFoundation/LabelHash.h"
#include "sharedFoundation/Tag.h"
#include "sharedMath/Vector.h"
#include "sharedMath/Transform.h"

class AppearanceTemplate;
class AxialBox;
class BoxExtent;
class CollideParameters;
class CollisionInfo;
class ComponentAppearance;
class CrcLowerString;
class CrcString;
class CustomizationData;
class DebugShapeRenderer;
class DetailAppearance;
class Extent;
class Hardpoint;
class IndexedTriangleList;
class LightningAppearance;
class LightsaberAppearance;
class Object;
class ParticleEffectAppearance;
class SkeletalAppearance2;
class Sphere;
class SwooshAppearance;
class Texture;
class Vector;
typedef uint32 PropertyId;

namespace DPVS
{
	class Object;
};

// ======================================================================

class Appearance
{
	// {Detail,Component,Lightsaber}Appearance need access to Appearance::getDpvsObject()
	friend class DetailAppearance;
	friend class ComponentAppearance;
	friend class LightsaberAppearance;

public:

	static void install();

	static void beginNewFrame ();

	typedef void (*RenderHardpointFunction)(Transform const & transform_h2w, bool renderName, char const * name);
	static void setRenderHardpointFunction(RenderHardpointFunction renderHardpointFunction);

	static bool canRenderEffects();
	static void enableRenderEffects(bool yesno);

	void useRenderEffectsFlag(bool yesno);
	bool usesRenderEffectsFlag() const;

public:

	explicit Appearance(const AppearanceTemplate *newAppearanceTemplate);
	virtual ~Appearance();

	const AppearanceTemplate  *getAppearanceTemplate() const;
	const char *               getAppearanceTemplateName () const;

	virtual bool                isLoaded() const;

	virtual const Sphere       &getSphere() const;

	virtual bool collide(Vector const & start_o, Vector const & end_o, CollideParameters const & collideParameters, CollisionInfo & result) const;
	virtual bool implementsCollide() const;

	virtual const Extent       *getSelectionExtent() const;
	virtual const Extent       *getExtent() const;
	virtual void                setExtent(const Extent *newExent);
	virtual AxialBox const getTangibleExtent() const;

	virtual float               alter(float time);

	virtual void                setAlpha(bool opaqueEnabled, float opaqueAlpha, bool alphaEnabled, float alphaAlpha);

	Object                     *getOwner();
	const Object               *getOwner() const;
	virtual void                setOwner(Object *newOwner);

	void                        setTransform_w(const Transform &t) const;
	const Transform &           getTransform_w() const;

	const char                 *getFloorName() const;

	virtual void                setCustomizationData(CustomizationData *customizationData);
	virtual void                addCustomizationVariables(CustomizationData &customizationData) const;

	virtual void                setKeepAlive(bool keepAlive);
	bool                        isAlive() const;

	virtual bool                findHardpoint(CrcString const &hardpointName, Transform &hardpointTransform) const;
	virtual int                 getHardpointCount() const;
	virtual int                 getHardpointIndex(CrcString const &hardpointName, bool optional = false) const;
	virtual const Hardpoint&    getHardpoint(int index) const;

	virtual void setScale(Vector const & scale);
	virtual Vector const & getScale() const;
	void setUniformScale(float scale);
	float getUniformScale() const;

	virtual void                addToWorld();
	virtual void                removeFromWorld();
	virtual void                preRender() const;
	virtual void                render() const;
	virtual void                objectListCameraRender() const;
	static void                 objectListCameraRenderDescend(Object const & obj);
	
	virtual void                setTexture(Tag tag, const Texture &texture);
	bool getShadowBlobAllowed() const;
	virtual void setShadowBlobAllowed();
	virtual void renderShadowBlob (const Vector& position_o, float radius) const;
	virtual void renderReticle (const Vector& position_o, float radius) const;

#ifdef _DEBUG
	virtual bool                debugRenderingEnabled() const;
	virtual void                drawDebugShapes ( DebugShapeRenderer * R ) const;
	virtual int                 getPolygonCount () const;
	virtual void                debugDump(std::string &result, int indentLevel) const;
#endif

	virtual const IndexedTriangleList* getRadarShape () const;

	void setRenderedThisFrame () const;
	bool getRenderedThisFrame () const;
	bool getRenderedLastFrame () const;
	int getRenderedFrameNumber() const;

	virtual SkeletalAppearance2 *       asSkeletalAppearance2();
	virtual SkeletalAppearance2 const * asSkeletalAppearance2() const;

	virtual ComponentAppearance *       asComponentAppearance();
	virtual ComponentAppearance const * asComponentAppearance() const;

	virtual DetailAppearance * asDetailAppearance();
	virtual DetailAppearance const * asDetailAppearance() const;

	virtual ParticleEffectAppearance * asParticleEffectAppearance();
	virtual ParticleEffectAppearance const * asParticleEffectAppearance() const;

	virtual SwooshAppearance * asSwooshAppearance();
	virtual SwooshAppearance const * asSwooshAppearance() const;

	virtual LightningAppearance * asLightningAppearance();
	virtual LightningAppearance const * asLightningAppearance() const;

	virtual void                onEvent(LabelHash::Id eventId);

protected:

	void extentChanged() const;
	
	virtual bool realIsAlive() const;
	virtual void onScaleModified(Vector const &oldScale, Vector const &newScale);

private:

	// disabled
	Appearance();
	Appearance(const Appearance &);
	Appearance &operator =(const Appearance &);

	virtual DPVS::Object       *getDpvsObject() const;

	///////////////////////////////////////////////////////////

	const AppearanceTemplate *m_appearanceTemplate;

protected:

	const Extent             *m_extent;

private:

	Object                   *m_owner;
	mutable int               m_renderedFrameNumber;
	Vector            m_scale;
	bool                      m_keepAlive;
	bool                      m_useRenderEffectsFlag;
	bool                      m_shadowBlobAllowed;
	mutable Transform         m_appearanceToWorld;
};

// ======================================================================
/**
 * Get the AppearanceTemplate for this Appearance.
 *
 * The AppearanceTemplate may be nullptr.
 *
 * AppearanceTemplates may be shared by multiple Appearances.
 *
 * @return  const pointer to the AppearanceTemplate.
 */

inline const AppearanceTemplate *Appearance::getAppearanceTemplate() const
{
	return m_appearanceTemplate;
}

// ----------------------------------------------------------------------

inline Object *Appearance::getOwner()
{
	return m_owner;
}

// ----------------------------------------------------------------------

inline const Object *Appearance::getOwner() const
{
	return m_owner;
}

// ----------------------------------------------------------------------

inline const Transform &Appearance::getTransform_w() const
{
	return m_appearanceToWorld;
}

// ----------------------------------------------------------------------

inline Vector const &Appearance::getScale() const
{
	return m_scale;
}

// ---------------------------------------------------------------------

inline void Appearance::setUniformScale(float const scale)
{
	setScale(Vector(scale, scale, scale));
}

// ---------------------------------------------------------------------

inline float Appearance::getUniformScale() const
{
	// When requesting a uniform scale, the x-axis is the only one considered.
	return getScale().x;
}

// ---------------------------------------------------------------------

inline int Appearance::getRenderedFrameNumber() const
{
	return m_renderedFrameNumber;
}

// ======================================================================

#endif
