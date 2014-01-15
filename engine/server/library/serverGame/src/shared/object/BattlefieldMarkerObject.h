// ======================================================================
//
// BattlefieldMarkerObject.h
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef _INCLUDED_BattlefieldMarkerObject_H
#define _INCLUDED_BattlefieldMarkerObject_H

// ======================================================================

#include "serverGame/TangibleObject.h"
#include "Archive/AutoDeltaMap.h"

// ======================================================================

class ServerBattlefieldMarkerObjectTemplate;

// ======================================================================

class BattlefieldMarkerObject: public TangibleObject
{
	friend class BattlefieldMarkerController;
	
public:
	BattlefieldMarkerObject(ServerBattlefieldMarkerObjectTemplate const *newTemplate);
	virtual ~BattlefieldMarkerObject();

	static BattlefieldMarkerObject *findMarkerByRegionName(std::string const &regionName);

	virtual Controller *createDefaultController();

	void setRegionName(std::string const &newName);
	std::string const &getRegionName() const;
	uint32 getBattlefieldFactionId(NetworkId const &who) const;
	void setBattlefieldParticipant(NetworkId const &who, uint32 factionId);
	void clearBattlefieldParticipants();
	std::map<NetworkId, uint32> const &getBattlefieldParticipants() const;

	void onParticipantSet(NetworkId const &who, uint32 const &, uint32 const &);
	void onParticipantInsert(NetworkId const &who, uint32 const &);
	void onParticipantErase(NetworkId const &who, uint32 const &);
	void onRegionNameChanged(std::string const &oldName, std::string const &newName);

protected:
	virtual void onLoadedFromDatabase();

private:
	BattlefieldMarkerObject();
	BattlefieldMarkerObject(BattlefieldMarkerObject const &);
	BattlefieldMarkerObject &operator=(BattlefieldMarkerObject const &);

	void addMembersToPackages();

private:
	struct RegionNameCallback
	{
		void modified(BattlefieldMarkerObject &target, std::string const &oldValue, std::string const &newValue, bool) const;
	};
//BPM BattlefieldMarkerObject : TangibleObject // Begin persisted members.
	Archive::AutoDeltaVariableCallback<std::string, RegionNameCallback, BattlefieldMarkerObject> m_regionName;
	Archive::AutoDeltaMap<NetworkId, uint32, BattlefieldMarkerObject> m_battlefieldParticipants;
//EPM
};

// ======================================================================

inline void BattlefieldMarkerObject::RegionNameCallback::modified(BattlefieldMarkerObject &target, std::string const &oldValue, std::string const &newValue, bool) const
{
	target.onRegionNameChanged(oldValue, newValue);
}

// ======================================================================

#endif

