// ======================================================================
//
// AppearanceTemplate.h
// Portions copyright 1999 Bootprint Entertainment
// Portions copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_AppearanceTemplate_H
#define INCLUDED_AppearanceTemplate_H

// ======================================================================

class Appearance;
class Camera;
class CrcLowerString;
class CrcString;
class Extent;
class Hardpoint;
class Iff;
class Object;
class Sphere;
class Vector;
class DebugShapeRenderer;

// ======================================================================

class AppearanceTemplate
{
public:

	static void install();

	static AppearanceTemplate *create(const char *newName, Iff *iff);
	static AppearanceTemplate *createDtla(const char *newName, Iff *iff);

public:

	explicit AppearanceTemplate(const char *newName);
	virtual ~AppearanceTemplate();

	virtual Appearance      *createAppearance() const;
	virtual void             drawDebugShapes(DebugShapeRenderer *renderer) const;

	virtual void             preloadAssets () const;
	//-- we use this function to do some preloading but nothing big.  Characters use it in preloading since loading all .MGNs as done by preloadAssets() generally is too heavy.
	virtual void             preloadAssetsLight () const;
	virtual void             garbageCollect () const;

	const CrcLowerString    &getCrcName() const;
	const char              *getName() const;

	const Extent            *getExtent() const;
	void                     setExtent(const Extent *newExent);

	const Extent            *getCollisionExtent() const;
	void                     setCollisionExtent(Extent const *newExtent);


	int                      getHardpointCount() const;
	int                      getHardpointIndex(CrcString const &hardpointName, bool optional = false) const;
	const Hardpoint&         getHardpoint(int index) const;

	const char              *getFloorName() const;

	int                      getReferenceCount() const;
	int                      incrementReference() const;
	int                      decrementReference() const;

	void                     load(Iff &iff);

protected:

	void load_0001(Iff &iff);
	void load_0002(Iff &iff);
	void load_0003(Iff &iff);

	void loadExtents(Iff &iff);
	void loadCollisionExtents(Iff &iff);
	void loadHardpoints(Iff &iff);

	bool loadFloors(Iff &iff);

private:

	typedef std::vector<Hardpoint*> HardpointList;

private:

	AppearanceTemplate();
	AppearanceTemplate(const AppearanceTemplate &);
	AppearanceTemplate &operator =(const AppearanceTemplate &);

private:

	mutable int           m_referenceCount;
	CrcLowerString *const m_crcName;
	const Extent         *m_extent;

	const Extent         *m_collisionExtent;

	HardpointList        *m_hardpoints;

	char *                m_floorName;

	class PreloadManager;
	friend class PreloadManager;
	mutable PreloadManager* m_preloadManager;
};

// ----------------------------------------------------------------------
/**
 *Return the Extent for this AppearanceTemplate.
 */

inline const Extent *AppearanceTemplate::getExtent() const
{
	return m_extent;
}

inline const Extent *AppearanceTemplate::getCollisionExtent() const
{
	return m_collisionExtent;
}

// ----------------------------------------------------------------------

inline int AppearanceTemplate::getReferenceCount() const
{
	return m_referenceCount;
}

// ----------

inline int AppearanceTemplate::incrementReference() const
{
	m_referenceCount++;

	return m_referenceCount;
}

// ----------

inline int AppearanceTemplate::decrementReference() const
{
	m_referenceCount--;

	return m_referenceCount;
}

// ======================================================================

#endif
