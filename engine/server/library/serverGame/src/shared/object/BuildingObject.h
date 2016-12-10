// ======================================================================
//
// BuildingObject.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef BuildingObject_H
#define BuildingObject_H

// ======================================================================

#include "Archive/AutoDeltaSetObserver.h"
#include "serverGame/TangibleObject.h"
#include "serverGame/CellPermissions.h"
#include "sharedTerrain/TerrainGenerator.h"

// ======================================================================

class ServerBuildingObjectTemplate;
class CreatureObject;

// ======================================================================

/** 
  *  A BuildingObject is the base class for all large structures that players will be placing.
  */

class BuildingObject : public TangibleObject
{
public:

	explicit BuildingObject(const ServerBuildingObjectTemplate* newTemplate);
	virtual ~BuildingObject();

	virtual BuildingObject *       asBuildingObject();
	virtual BuildingObject const * asBuildingObject() const;


	virtual float       alter(float time);
	
	static void         removeDefaultTemplate(void);

	virtual Controller* createDefaultController(void);
	void                addMembersToPackages   ();
	virtual void        getAttributes          (std::vector<std::pair<std::string, Unicode::String> > &data) const;

	int                 getMaintenanceCost() const;
	void                setMaintenanceCost(int newConst);
	real                getTimeLastChecked() const;
	void                setTimeLastChecked(real newTime);

	int                 getCityId() const;
	void                setCityId(int cityId);
	bool                getIsPublic() const;
	void                setIsPublic(bool isPublic);
	void                addBanned(std::string const &name);
	void                removeBanned(std::string const &name);
	void                removeAllBanned();
	void                addAllowed(std::string const &name);
	void                removeAllowed(std::string const &name);
	void                removeAllAllowed();
	bool                isAllowed(CreatureObject const &who) const;
	bool                isPlayerPlaced() const;
	bool                isSpaceDungeon() const;
	bool                playersWithAlignedEnemyFlagsAllowed() const;
	virtual bool        isVisibleOnClient(const Client & client) const;
	CellPermissions::PermissionList const &getAllowed() const;
	CellPermissions::PermissionList const &getBanned() const;
	void                expelObject(ServerObject &who);
	bool                getContentsLoaded() const;
	void                handleCMessageTo(const MessageToPayload &message);
	virtual void        onPermanentlyDestroyed();
	virtual void        onAllContentsLoaded();
	void                onCellContentsLoaded();
	virtual void        onAddedToWorld();
	void                gainedPlayer(ServerObject const &player);
	void                lostPlayer(ServerObject const &player);
	virtual void        changeTeleportDestination(Vector & position, float & yaw) const;
	void                loadContents(bool immediate = false);
	bool                unloadContents();

	struct BuildingObjectPointerHash
	{
		size_t operator()(const BuildingObject * const ptr) const
		{
			return (reinterpret_cast<const size_t>(ptr) >> 4);
		};
	};

	typedef std::unordered_set<const BuildingObject *, BuildingObjectPointerHash> AllBuildingsSet;
	static const AllBuildingsSet & getAllBuildings();

protected:

	virtual void                          initializeFirstTimeObject();
	virtual void                          endBaselines();
	virtual void                          onLoadedFromDatabase();
	virtual const SharedObjectTemplate *  getDefaultSharedTemplate(void) const;

private:
	BuildingObject();
	BuildingObject(const BuildingObject& rhs);
	BuildingObject&	operator=(const BuildingObject& rhs);

	typedef Archive::AutoDeltaVariableObserver<bool, CellPermissions::UpdateObserver, BuildingObject>                         ObservedBool;
	typedef Archive::AutoDeltaSetObserver<CellPermissions::PermissionObject, CellPermissions::UpdateObserver, BuildingObject> ObservedPermissionObjectSet;

private:
	
	static const SharedObjectTemplate * m_defaultSharedTemplate;	// template to use if no shared template is given

	// BPM BuildingObject : TangibleObject // Begin persisted members.
	Archive::AutoDeltaVariable<int>     m_maintenanceCost;				///< The weekly cost (in credits) of maintaining this Building.
	Archive::AutoDeltaVariable<real>    m_timeLastChecked;				///< The in-game time this Building was last accessed by the player.
	Archive::AutoDeltaVariable<int>     m_cityId;                 ///< The player city id this building is assigned to
	ObservedBool                        m_isPublic;
	ObservedPermissionObjectSet         m_allowed;
	ObservedPermissionObjectSet         m_banned;
	Archive::AutoDeltaVariable<bool>    m_contentsLoaded;
	// EPM 

	bool                     m_checkCreatePortalVolume;
	unsigned long            m_unloadTime;
};

// ======================================================================

inline int BuildingObject::getMaintenanceCost() const
{
	return m_maintenanceCost.get();
}

// ----------------------------------------------------------------------

inline void BuildingObject::setMaintenanceCost(int newConst)
{
	m_maintenanceCost = newConst;
}

// ----------------------------------------------------------------------

inline real BuildingObject::getTimeLastChecked() const
{
	return m_timeLastChecked.get();
}

// ----------------------------------------------------------------------

inline void BuildingObject::setTimeLastChecked(real newTime)
{
	m_timeLastChecked = newTime;
}

// ----------------------------------------------------------------------

inline bool BuildingObject::getIsPublic() const
{
	return m_isPublic.get();
}

// ----------------------------------------------------------------------

inline CellPermissions::PermissionList const &BuildingObject::getAllowed() const
{
	return m_allowed.get();
}

// ----------------------------------------------------------------------

inline CellPermissions::PermissionList const &BuildingObject::getBanned() const
{
	return m_banned.get();
}

// ----------------------------------------------------------------------

inline int BuildingObject::getCityId() const
{
	return m_cityId.get();
}

// ----------------------------------------------------------------------

inline bool BuildingObject::getContentsLoaded() const
{
	return m_contentsLoaded.get();
}

// ======================================================================

#endif // BuildingObject_H

