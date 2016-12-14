// ======================================================================
//
// CellObject.h
// Copyright 2001 Sony Online Entertainment
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_CellObject_H
#define INCLUDED_CellObject_H

// ======================================================================

#include "serverGame/ServerObject.h"
#include "serverGame/CellPermissions.h"
#include "Archive/AutoDeltaVariableObserver.h"
#include "Archive/AutoDeltaSetObserver.h"

// ======================================================================

class BuildingObject;
class CreatureObject;

// ======================================================================

class CellObject : public ServerObject
{
public:

	static CellObject * getCellObject(NetworkId const & networkId);
	static CellObject * asCellObject(Object * object);
	static CellObject const * asCellObject(Object const * objec);

public:

	explicit CellObject(const ServerObjectTemplate *newTemplate);
	virtual ~CellObject();

	virtual CellObject *asCellObject();
	virtual CellObject const *asCellObject() const;

	static void removeDefaultTemplate(void);
	virtual Controller * createDefaultController();
	void setCell(int cellNumber);
	int getCell() const;
	void setIsPublic(bool isPublic);
	bool getIsPublic() const;
	void addAllowed(std::string const &name);
	void removeAllowed(std::string const &name);
	void removeAllAllowed();
	void addBanned(std::string const &name);
	void removeBanned(std::string const &name);
	void removeAllBanned();
	CellPermissions::PermissionList const &getAllowed() const;
	CellPermissions::PermissionList const &getBanned() const;
	Unicode::String const & getCellLabel() const;
	void setCellLabel(Unicode::String const & label);
	Vector const & getLabelLocationOffset() const;
	void setLabelLocationOffset(Vector const & offset);

	bool getClosestPathNodePos( const ServerObject & object, Vector & outPos ) const;

	ServerObject *         getOwner();
	ServerObject const *   getOwner() const;
	BuildingObject *       getOwnerBuilding();
	BuildingObject const * getOwnerBuilding() const;
	ShipObject *           getOwnerShip();
	ShipObject const *     getOwnerShip() const;
	bool                   isAllowed(CreatureObject const &who) const;
	virtual bool           isVisibleOnClient              (const Client & client) const;
	void addMembersToPackages();
	virtual void           getAttributes                  (std::vector<std::pair<std::string, Unicode::String> > &data) const;

	virtual const NetworkId &     getOwnerId() const;
	virtual bool                  isOwner(NetworkId const &id, Client const *client) const;
	virtual void                  setOwnerId(const NetworkId &id);

	virtual void sendObjectSpecificBaselinesToClient(Client const &client) const;

//Container triggers for notifiying the building
	virtual bool                          onContainerAboutToLoseItem(ServerObject * destination, ServerObject& item, ServerObject* transferer);
	virtual int                           onContainerAboutToGainItem(ServerObject& item, ServerObject* transferer);
	virtual void                          onContainerLostItem(ServerObject * destination, ServerObject& item, ServerObject* transferer);
	virtual void                          onContainerGainItem(ServerObject& item, ServerObject* source, ServerObject* transferer);
	virtual void               onAllContentsLoaded();
	
protected:
	virtual void endBaselines();
	virtual void onLoadedFromDatabase();

	virtual const SharedObjectTemplate *  getDefaultSharedTemplate(void) const;

private:

	CellObject();
	CellObject(const CellObject &);
	CellObject &operator =(const CellObject &);

	typedef Archive::AutoDeltaVariableObserver<bool, CellPermissions::UpdateObserver, CellObject>                         ObservedBool;
	typedef Archive::AutoDeltaSetObserver<CellPermissions::PermissionObject, CellPermissions::UpdateObserver, CellObject> ObservedPermissionObjectSet;

private:

	static const SharedObjectTemplate * m_defaultSharedTemplate;	// template to use if no shared template is given
	
	Archive::AutoDeltaVariable<int>  m_cellNumber;
	ObservedBool                     m_isPublic;
	ObservedPermissionObjectSet      m_allowed;
	ObservedPermissionObjectSet      m_banned;
	Archive::AutoDeltaVariable<Unicode::String> m_cellLabel;
	Archive::AutoDeltaVariable<Vector> m_labelLocationOffset;
};

// ======================================================================

inline void CellObject::setCell(int cellNumber)
{
	m_cellNumber = cellNumber;
}

// ----------------------------------------------------------------------

inline int CellObject::getCell() const
{
	return m_cellNumber.get();
}

// ----------------------------------------------------------------------

inline bool CellObject::getIsPublic() const
{
	return m_isPublic.get();
}

// ----------------------------------------------------------------------

inline void CellObject::setIsPublic(bool isPublic)
{
	m_isPublic.set(isPublic);
}

// ----------------------------------------------------------------------

inline CellPermissions::PermissionList const &CellObject::getAllowed() const
{
	return m_allowed.get();
}

// ----------------------------------------------------------------------

inline CellPermissions::PermissionList const &CellObject::getBanned() const
{
	return m_banned.get();
}

// ----------------------------------------------------------------------

inline Unicode::String const & CellObject::getCellLabel() const
{
	return m_cellLabel.get();
}

// ----------------------------------------------------------------------

inline void CellObject::setCellLabel(Unicode::String const & label)
{
	m_cellLabel.set(label);
}

// ----------------------------------------------------------------------

inline Vector const & CellObject::getLabelLocationOffset() const
{
	return m_labelLocationOffset.get();
}

// ----------------------------------------------------------------------

inline void CellObject::setLabelLocationOffset(Vector const & offset)
{
	m_labelLocationOffset.set(offset);
}

// ======================================================================

#endif

