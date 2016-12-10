// ======================================================================
//
// CellProperty.h
// Copyright 2001 Sony Online Entertainment
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_CellProperty_H
#define INCLUDED_CellProperty_H

// ======================================================================

class Appearance;
class BaseClass;
class BaseExtent;
class DebugShapeRenderer;
class Floor;
class Iff;
class IndexedTriangleList;
class Location;
class Object;
class Portal;
class PortalProperty;
class PortalPropertyTemplateCell;
class Texture;
class Vector;
class VectorArgb;

namespace DPVS
{
	class Cell;
};

#include "sharedMath/PackedArgb.h"
#include "sharedObject/Container.h"

// ======================================================================

class CellProperty : public Container
{
	friend class RenderWorld;
	friend class PortalProperty;
	friend class DpvsHelper;

public:

	typedef std::vector<Vector>   VertexList;
	typedef std::vector<Portal *> PortalList;

	typedef void                      (*AddToRenderWorldHookFunction) (Object& object);
	typedef Appearance               *(*PolyAppearanceFactory)(VertexList const & verts, VectorArgb const & color);
	typedef void                      (*RenderHookFunction)();
	typedef DPVS::Cell               *(*CreateDpvsCellHookFunction)(CellProperty *owner);
	typedef void                      (*DestroyDpvsCellHookFunction)(DPVS::Cell *cellToDestroy);
	typedef void                      (*TextureFetch)(Texture const *texture);
	typedef void                      (*TextureRelease)(Texture const *texture);
	typedef void                      (*DeleteVisibleCellProperty)(CellProperty const *cell);

	typedef bool                      (*AccessAllowedHookFunction)(CellProperty const &property);

	struct PortalObjectEntry
	{
		const PortalProperty *portalProperty;
		PortalList           *portalList;
		bool                  attached;
	};

public:

	static void                install();
	static void                setCreateAndDestroyDpvsCellHookFunctions(CreateDpvsCellHookFunction createDpvsCellHookFunction, DestroyDpvsCellHookFunction destroyDpvsCellHookFunction);
	static void                setPortalTransitionsEnabled(bool enabled);
	static void                setTextureHookFunctions(TextureFetch textureFetch, TextureRelease textureRelease);
	static void                setDeleteVisibleCellProperty(DeleteVisibleCellProperty);

	static PropertyId          getClassPropertyId();
	static CellProperty       *getWorldCellProperty();

	static void                addPortalCrossingNotification(Object &object);
	static void                removePortalCrossingNotification(Object &object);

	static void                setAddToRenderWorldHook(AddToRenderWorldHookFunction hook);
	static void                setPortalBarrierFactory(PolyAppearanceFactory factory);
	static void                setForceFieldFactory(PolyAppearanceFactory factory);
	static void                setAccessAllowedHookFunction(AccessAllowedHookFunction hook);

	static void                addToRenderWorld(Object & object);
	static Appearance *        createPortalBarrier(VertexList const & verts, const VectorArgb &color);
	static Appearance *        createForceField(VertexList const & verts, const VectorArgb &color);

	static bool                areAdjacent(const CellProperty *cellProperty1, const CellProperty *cellProperty2);

	static float               getMaximumValidCellSpaceCoordinate();

	static void                releaseWorldCellPropertyEnvironmentTexture();

	static Vector const getPosition_w(Location const & location);

public:

	CellProperty(Object &cellObject);
	~CellProperty();

	bool                  isWorldCell() const;
	virtual bool          isContentItemExposedWith(Object const &item) const;

	virtual int           depersistContents(const Object& item);
	virtual bool          internalItemRemoved(const Object& item);
	
	virtual bool          mayAdd(const Object &item, ContainerErrorCode& error) const;
	virtual bool          remove(Object &item, ContainerErrorCode& error);
	virtual bool          remove(ContainerIterator &pos, ContainerErrorCode& error);
	virtual int           getTypeId() const;
	virtual void          debugPrint(std::string &buffer) const;

	virtual void removeFromWorld();

	const PortalProperty *getPortalProperty() const;
	const char           *getCellName() const;
	const uint32          getCellNameCrc() const;
	int                   getCellIndex() const;
	int                   getPortalCount() const;

	Object               *getAppearanceObject();
	const Object         *getAppearanceObject() const;

	CellProperty         *getDestinationCell(const Vector &startPosition, const Vector &endPosition, float &t, bool passableOnly=false) const;
	CellProperty         *getDestinationCell(const Object *object, int portalId) const;
	bool                  getDestinationCells(const Sphere &sphere, std::vector<CellProperty*> & outCells) const;

	bool                  isAdjacentTo(const CellProperty *cell) const;
	void                  addObject_w(Object &object);

	Floor                *getFloor();
	const Floor          *getFloor() const;

	const BaseExtent     *getCollisionExtent() const;

	const BaseClass      *getPathGraph() const;

	void                  addPreVisibilityTraversalRenderHookFunction(RenderHookFunction renderHookFunction) const;
	void                  removePreVisibilityTraversalRenderHookFunction(RenderHookFunction renderHookFunction) const;
	void                  callPreVisibilityTraversalRenderHookFunctions() const;

	void                  addEnterRenderHookFunction(RenderHookFunction renderHookFunction) const;
	void                  removeEnterRenderHookFunction(RenderHookFunction renderHookFunction) const;
	void                  callEnterRenderHookFunctions() const;

	void                  addPreDrawRenderHookFunction(RenderHookFunction renderHookFunction) const;
	void                  removePreDrawRenderHookFunction(RenderHookFunction renderHookFunction) const;
	void                  callPreDrawRenderHookFunctions() const;

	void                  addExitRenderHookFunction(RenderHookFunction renderHookFunction) const;
	void                  removeExitRenderHookFunction(RenderHookFunction renderHookFunction) const;
	void                  callExitRenderHookFunctions() const;

	void                  drawDebugShapes(DebugShapeRenderer * renderer) const;

	int                     getNumberOfPortalObjects() const;
	PortalObjectEntry const &getPortalObject(int index) const;

	bool                  getAccessAllowed() const;

	DPVS::Cell           *getDpvsCell() const;

	void                  setEnvironmentTexture(Texture const *texture);
	Texture const *       getEnvironmentTexture() const;

	void                  setVisible(bool visible) const;
	bool                  wasVisible() const;

	bool                  isFogEnabled() const;
	void                  setFogEnabled(bool fogEnabled);
	const PackedArgb     &getFogColor() const;
	void                  setFogColor(const PackedArgb &fogColor);
	float                 getFogDensity() const;
	void                  setFogDensity(float fogDensity);

	bool getAppliedInteriorLayout() const;
	void setAppliedInteriorLayout() const;

private:

	typedef std::vector<PortalObjectEntry> PortalObjectList;
	typedef std::vector<RenderHookFunction>  RenderHookFunctionList;

private:

	CellProperty();
	CellProperty(const CellProperty &);
	CellProperty &operator =(const CellProperty &);

	void                  initialize(const PortalProperty &portalProperty, int cellIndex, bool createAppearance);
	void                  attach(const PortalProperty &portalProperty, PortalList *portalList);
	void                  detach(const PortalProperty &portalProperty);
	Portal               *getPortal(int portalIndex);

	void addHookFunction(RenderHookFunctionList *&renderHookFunctionList, RenderHookFunction renderHookFunction) const;
	void removeHookFunction(RenderHookFunctionList *const &renderHookFunctionList, RenderHookFunction renderHookFunction) const;
	void callRenderHookFunctions(RenderHookFunctionList *const &renderHookFunctionList) const;

private:

	static void remove();

private:

	static CellProperty              *ms_worldCellProperty;
	static TextureFetch               ms_textureFetch;
	static TextureRelease             ms_textureRelease;
	static DeleteVisibleCellProperty  ms_deleteVisibleCellProperty;

private:

	const PortalProperty             *m_portalProperty; 
	int                               m_cellIndex;
	mutable Object                   *m_appearanceObject;
	PortalObjectList                 *m_portalObjectList;
	mutable bool                      m_visible;
	Floor                            *m_floor;
	const char                       *m_cellName;
	uint32                            m_cellNameCrc;
	DPVS::Cell                       *m_dpvsCell;
	const Texture                    *m_environmentTexture;
	bool                              m_fogEnabled;
	PackedArgb                        m_fogColor;
	float                             m_fogDensity;
	bool mutable m_appliedInteriorLayout;

	mutable RenderHookFunctionList   *m_preVisibilityTraversalRenderHookFunctionList;
	mutable RenderHookFunctionList   *m_enterRenderHookFunctionList;
	mutable RenderHookFunctionList   *m_preDrawRenderHookFunctionList;
	mutable RenderHookFunctionList   *m_exitRenderHookFunctionList;
};

// ======================================================================

inline const char *CellProperty::getCellName() const
{
	return m_cellName;
}

// ----------------------------------------------------------------------

inline const uint32 CellProperty::getCellNameCrc() const
{
	return m_cellNameCrc;
}

// ----------------------------------------------------------------------

inline DPVS::Cell *CellProperty::getDpvsCell() const
{
	return m_dpvsCell;
}

// ----------------------------------------------------------------------

inline const PortalProperty *CellProperty::getPortalProperty() const
{
	return m_portalProperty;
}

// ----------------------------------------------------------------------

inline int CellProperty::getCellIndex() const
{
	return m_cellIndex;
}

// ----------------------------------------------------------------------

inline Texture const * CellProperty::getEnvironmentTexture() const
{
	return m_environmentTexture;
}

// ----------------------------------------------------------------------

inline void CellProperty::setVisible(bool visible) const
{
	m_visible = visible;
}

// ----------------------------------------------------------------------

inline bool CellProperty::wasVisible() const
{
	return m_visible;
}

// ======================================================================

#endif
