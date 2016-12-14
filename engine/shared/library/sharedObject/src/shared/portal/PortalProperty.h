// ======================================================================
//
// PortalProperty.h
// Copyright 2001 Sony Online Entertainment
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_PortalProperty_H
#define INCLUDED_PortalProperty_H

// ======================================================================

class CellProperty;
class Floor;
class Iff;
class Object;
class Portal;
class PortalPropertyTemplate;
class Vector;

#include "sharedObject/Container.h"
#include "sharedMath/Transform.h"

// ======================================================================

class PortalProperty : public Container
{
	class Notification;

	friend class CellProperty;
	friend class RenderWorld;
	friend class Notification;

public:

	static PropertyId getClassPropertyId();

public:

	typedef std::vector<Portal *>        PortalList;
	typedef std::vector<const char *>    CellNameList;
	typedef std::vector<Vector>          VertexList;

	typedef Object *(*BeginCreateObjectFunction)(int cellIndex);
	typedef void    (*EndCreateObjectFunction)(Object *newObject);

public:

	static void install(BeginCreateObjectFunction beginCreateObjectFunction, EndCreateObjectFunction endCreateObjectFunction);

public:

	PortalProperty(Object &owner, const char *portalPropertyFileName);
	virtual ~PortalProperty();

	virtual void                  initializeFirstTimeObject();
	void                          clientSinglePlayerInitializeFirstTimeObject();

	virtual void                  addToWorld();
	virtual void                  removeFromWorld();

	virtual bool                  isContentItemObservedWith(Object const &item) const;
	virtual bool                  isContentItemExposedWith(Object const &item) const;
	virtual bool                  canContentsBeObservedWith() const;

	virtual bool                  mayAdd(const Object &item, ContainerErrorCode& error) const;
	virtual bool                  remove(Object &item, ContainerErrorCode& error);
	virtual bool                  remove(ContainerIterator &pos, ContainerErrorCode& error);
	virtual int                   getTypeId() const;
	virtual void                  debugPrint(std::string &buffer) const;

	bool                          serverEndBaselines(int crc,std::vector<Object*> &unfixables, bool authoritative);

	const PortalPropertyTemplate &getPortalPropertyTemplate() const;
	int                           getCrc() const;
	const char                   *getPobName() const;
	const char                   *getPobShortName() const;
	int                           getNumberOfCells() const;
	CellProperty                 *getCell(int index);
	const CellProperty           *getCell(int index) const;
	const char                   *getCellAppearanceName(int index) const;

	const CellNameList           &getCellNames() const;
	CellProperty                 *getCell(const char *cellName);
	const CellProperty           *getCell(const char *cellName) const;

	Transform const               getEjectionLocationTransform() const;

	char const                   *getExteriorFloorName() const;

	void                          createAppearance();
	void                          cellLoaded(int cellIndex, Object &cellObject, bool shouldCreateAppearance);

	bool                          findContainingCell(Vector const & buildingPos, Vector & outPos, Object * & outCell);
	void                          queueObjectForFixup(Object &object);
	bool                          fixupObject(Object &object, Transform intendedTransform);
	bool                          hasPassablePortalToParentCell() const;
	CellProperty const * findContainingCell(Vector const & position_l) const;

private:

	PortalProperty();
	PortalProperty(const PortalProperty &);
	PortalProperty &operator =(const PortalProperty &);

private:

	struct FixupRec
	{
		Object    *m_obj;
		Transform  m_transform;
	};
	typedef std::vector<FixupRec>        FixupList;
	typedef std::vector<CellProperty*>   CellList;

private:

	static BeginCreateObjectFunction    ms_beginCreateObjectFunction;
	static EndCreateObjectFunction      ms_endCreateObjectFunction;
	static Notification                 ms_notification;

private:

	const PortalPropertyTemplate  *m_template;
	CellList                      *m_cellList;
	FixupList                     *m_fixupList;
	bool                           m_hasPassablePortalToParentCell;
};

// ======================================================================

#endif
