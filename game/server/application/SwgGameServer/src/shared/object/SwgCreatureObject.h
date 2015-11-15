//========================================================================
//
// SwgCreatureObject.h
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef INCLUDED_SwgCreatureObject_H
#define INCLUDED_SwgCreatureObject_H

#include "serverGame/CreatureObject.h"


class ServerCreatureObjectTemplate;


//----------------------------------------------------------------------

class SwgCreatureObject : public CreatureObject
{
public:
	explicit  SwgCreatureObject(const ServerCreatureObjectTemplate * newTemplate);
	virtual  ~SwgCreatureObject();

	virtual void handleCMessageTo (const MessageToPayload &message);

	bool         isJedi(void) const;
	const int    getSpentJediSkillPoints() const;

	virtual Controller* createDefaultController();
	virtual float       alter(float time);
	virtual void        onRemovingFromWorld();
	virtual void        onPermanentlyDestroyed();

	virtual bool  hasBounty(const CreatureObject & target) const;
	virtual bool  hasBounty() const;
	virtual std::vector<NetworkId> const & getJediBountiesOnMe() const;
	int           getBountyValue() const;

	virtual const bool  grantSkill(const SkillObject & newSkill);
	virtual void        revokeSkill(const SkillObject & oldSkill);

	virtual void        setPvpFaction(Pvp::FactionId factionId);

protected:
	virtual void endBaselines();
	virtual void onAddedToWorld();
	virtual void levelChanged() const;

private:
	SwgCreatureObject();
	SwgCreatureObject(const SwgCreatureObject& rhs);
	SwgCreatureObject&	operator=(const SwgCreatureObject& rhs);
};


//----------------------------------------------------------------------


#endif	// INCLUDED_SwgCreatureObject_H
