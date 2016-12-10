// ======================================================================
//
// AiCreatureCombatProfile.h
//
// copyright 2005, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_AiCreatureCombatProfile_H
#define INCLUDED_AiCreatureCombatProfile_H

class CrcString;
class CreatureObject;
class PersistentCrcString;

// ======================================================================
class AiCreatureCombatProfile
{
public:

	struct Action;

	typedef std::vector<Action *> ActionList;

	AiCreatureCombatProfile();

	static void install();

	std::string const toString() const;

	/**
	 * Returns the combat profile corresponding to the specified profile name.
	 */
	static AiCreatureCombatProfile const * getCombatProfile(CrcString const & profileName);

	/**
	 * Grants all the actions from the action lists to the specified owner.
	 * This is necessary to allow the AI to do the '_' versions of the
	 * actions.
	 */
	void grantActions(CreatureObject & owner) const;

public:

	PersistentCrcString const * m_profileId;
	ActionList m_singleUseActionList;
	ActionList m_delayRepeatActionList;
	ActionList m_instantRepeatActionList;
	time_t m_knockDownRecoveryTime;
};

// ======================================================================

#endif // INCLUDED_AiCreatureCombatProfile_H
