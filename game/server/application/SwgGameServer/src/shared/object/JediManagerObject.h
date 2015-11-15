// ======================================================================
//
// JediManagerObject.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_JediManagerObject_H
#define INCLUDED_JediManagerObject_H

#include "serverGame/UniverseObject.h"
#include "swgSharedUtility/JediConstants.h"

class BountyHunterTargetListMessage;
class ScriptParams;
class ServerJediManagerObjectTemplate;
class Vector;


// ======================================================================

/**
 * Keeps track of what Jedi are online and grants bounty missions for them.
 */
class JediManagerObject : public UniverseObject
{
public:
	         JediManagerObject(const ServerJediManagerObjectTemplate* newTemplate);
	virtual ~JediManagerObject();

	void     addMembersToPackages();

	virtual void         setupUniverse();
	virtual void         onServerUniverseGainedAuthority();
	virtual Controller*  createDefaultController();
	virtual void         conclude();

	void addJedi(const NetworkId & id, const Unicode::String & name, const Vector & location, const std::string & scene, int visibility, int bountyValue, int level, int hoursAlive, int state, int spentJediSkillPoints, int faction);
	void setJediOffline(const NetworkId & id, const Vector & location, const std::string & scene);
	void removeJedi(const NetworkId & id);
	void characterBeingDeleted(const NetworkId & id);
	void updateJedi(const NetworkId & id, int visibility, int bountyValue, int level, int hoursAlive);
	void updateJedi(const NetworkId & id, JediState state);
//	void updateJedi(const NetworkId & id, const std::vector<NetworkId> & bounties) const;
	void updateJediLocation(const NetworkId & id, const Vector & location, const std::string & scene);
	void updateJediSpentJediSkillPoints(const NetworkId & id, int spentJediSkillPoints);
	void updateJediFaction(const NetworkId & id, int faction);
	void updateJediScriptData(const NetworkId & id, const std::string & name, int value);
	void removeJediScriptData(const NetworkId & id, const std::string & name);

	void getJedi(int visibility, int bountyValue, int minLevel, int maxLevel, int hoursAlive, int bounties, int state, ScriptParams & returnParams) const;
	void getJedi(const NetworkId & id, ScriptParams & returnParams) const;
	bool getJediLocation(const NetworkId & id, Vector & location, std::string & scene) const;
	void addJediBounties(const BountyHunterTargetListMessage & msg);
	void requestJediBounty(const NetworkId & targetId, const NetworkId & hunterId, const std::string & successCallback, const std::string & failCallback, const NetworkId & callbackObjectId);
	void removeJediBounty(const NetworkId & targetId, const NetworkId & hunterId);
	void removeAllJediBounties(const NetworkId & targetId);
	const std::vector<NetworkId> & getJediBounties(const NetworkId & targetId) const;
	void getBountyHunterBounties(const NetworkId & hunterId, std::vector<NetworkId> & jedis) const;

	bool hasBountyOnJedi(const NetworkId & targetId, const NetworkId & hunterId) const;
	bool hasBountyOnJedi(NetworkId const & hunterId) const;

	static void queueBountyHunterTargetListFromDB(const BountyHunterTargetListMessage * bhtlm);

protected:
	virtual void initializeFirstTimeObject();
	virtual void endBaselines();

private:
	JediManagerObject(const JediManagerObject&);
	JediManagerObject & operator=(const JediManagerObject&);

	void adjustBountyCount(NetworkId const & hunterId, int adjustment);

private:
	// most of the jedi information are kept in vectors, which makes it very expensive
	// to search, so we're going to keep a map to store the index in the vector where
	// the jedi information is kept; to prevent updating the indices whenever an item is
	// deleted from the vector, we will never delete items from the vector, but we
	// will keep track of places in the vector that has been deleted (i.e. the "holes")
	// so that when a new item is added, we add it to one of the "hole" in the vector
	Archive::AutoDeltaMap<NetworkId, int>                  m_jediId;
	Archive::AutoDeltaVector<int>                          m_holes;

	Archive::AutoDeltaVector<Unicode::String>              m_jediName;
	Archive::AutoDeltaVector<Vector>                       m_jediLocation;
	Archive::AutoDeltaVector<std::string>                  m_jediScene;
	Archive::AutoDeltaVector<int>                          m_jediVisibility;
	Archive::AutoDeltaVector<int>                          m_jediBountyValue;
	Archive::AutoDeltaVector<int>                          m_jediLevel;
	Archive::AutoDeltaVector<std::vector<NetworkId> >      m_jediBounties; // each element of this vector is a vector of bounty hunters that are chasing the jedi
	Archive::AutoDeltaMap<NetworkId, int>                  m_bountyHunters; // map of bounty hunter id --> number of jedis being chased
	Archive::AutoDeltaVector<int>                          m_jediHoursAlive;
	Archive::AutoDeltaVector<int>                          m_jediState;
	// this should actuall be a vector of bool, but it won't compile in VC
	Archive::AutoDeltaVector<int>                          m_jediOnline;
	Archive::AutoDeltaVector<int>                          m_jediSpentJediSkillPoints;
	Archive::AutoDeltaVector<int>                          m_jediFaction;
	Archive::AutoDeltaMap<std::string, int>                m_jediScriptData;

	// this contains all the names that are used to make up the key value in m_jediScriptData
	Archive::AutoDeltaSet<std::string>                     m_jediScriptDataNames;

	// indicates whether the bounty hunter target list has been received from the DB
	Archive::AutoDeltaVariable<bool>                       m_bountyHunterTargetsReceivedFromDB;
};

// ======================================================================

#endif
