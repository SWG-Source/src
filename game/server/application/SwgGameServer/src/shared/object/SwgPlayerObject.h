//========================================================================
//
// SwgPlayerObject.h
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef INCLUDED_SwgPlayerObject_H
#define INCLUDED_SwgPlayerObject_H

#include "serverGame/PlayerObject.h"
#include "swgSharedUtility/JediConstants.h"

class ServerPlayerObjectTemplate;


//----------------------------------------------------------------------

class SwgPlayerObject : public PlayerObject
{
public:
	explicit     SwgPlayerObject(const ServerPlayerObjectTemplate * newTemplate);
	virtual     ~SwgPlayerObject();

	// Jedi functions
	virtual bool isJedi(void) const;
	JediState    getJediState() const;
	void         setJediState(JediState state);
	void         updateJediLocationTime(float time);
	void         setJediVisibility(int visibility);
	int          getJediVisibility(void) const;
//	void         addJediBounty(const NetworkId & hunter);
//	void         removeJediBounty(const NetworkId & hunter);
//	void         setJediBounties(const std::vector<NetworkId> & bounties);
//	bool         getJediBounties(std::vector<NetworkId> & bounties);

protected:
	virtual void endBaselines();
	virtual void virtualOnSetAuthority();

private:
	void addMembersToPackages();

//	bool         getJediBounties(std::vector<NetworkId> & bounties);

private:
	SwgPlayerObject();
	SwgPlayerObject(const SwgPlayerObject & rhs);
	SwgPlayerObject & operator=(const SwgPlayerObject & rhs);

private:
	float m_updateJediLocationTime;
	Archive::AutoDeltaVariable<int> m_jediState;
};


//----------------------------------------------------------------------

inline JediState SwgPlayerObject::getJediState() const
{
	return static_cast<JediState>(m_jediState.get());
}


#endif	// INCLUDED_SwgPlayerObject_H
