// ======================================================================
//
// AiTargetingSystem.cpp
//
// copyright 2005, sony online entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/AiTargetingSystem.h"

#include "serverGame/ConfigServerGame.h"
#include "serverGame/HateList.h"
#include "serverGame/TangibleObject.h"
#include "sharedFoundation/Os.h"

// ======================================================================
//
// AiTargetingSystem
//
// ======================================================================

//-----------------------------------------------------------------------
AiTargetingSystem::AiTargetingSystem(TangibleObject const & owner) 
 : m_owner(owner)
 , m_target()
 , m_verifyTargetTimer(0)
{
}

//-----------------------------------------------------------------------
NetworkId const & AiTargetingSystem::getTarget()
{
	if (!m_owner.isHateListEmpty())
	{
		// If our target is not the primary hate target, see if we need
		// to reset the target to attempt to target the the primary hate target

		if (m_target != m_owner.getHateTarget())
		{
			if (   (m_target == CachedNetworkId::cms_cachedInvalid)
				|| (Os::getRealSystemTime() - m_verifyTargetTimer) > 3)
			{
				m_target = m_owner.getHateTarget();
				m_verifyTargetTimer = Os::getRealSystemTime();
			}
		}

		TangibleObject const * currentTangibleTarget = TangibleObject::asTangibleObject(m_target.getObject());

		// See if the primary hate target is valid
		{
			if (!canAttackTarget(currentTangibleTarget))
			{
				m_target = CachedNetworkId::cms_invalid;
			}
		}

		// If our current target is not valid, we need to find the next valid target
		{
			static HateList::SortedList sortedHateList;

			if (m_target == CachedNetworkId::cms_invalid)
			{
				m_owner.getSortedHateList(sortedHateList);
			}

			HateList::SortedList::const_iterator iterSortedHateList = sortedHateList.begin();

			for (; iterSortedHateList != sortedHateList.end(); ++iterSortedHateList)
			{
				currentTangibleTarget = TangibleObject::asTangibleObject(iterSortedHateList->first.getObject());

				if (canAttackTarget(currentTangibleTarget))
				{
					m_target = CachedNetworkId(*currentTangibleTarget);
					break;
				}
			}

			sortedHateList.clear();
		}
	}
	else
	{
		m_target = CachedNetworkId::cms_invalid;
	}

	return m_target;
}

//-----------------------------------------------------------------------
bool AiTargetingSystem::canAttackTarget(TangibleObject const * target)
{
	bool result = false;

	if (target != nullptr)
	{
		if (   (m_owner.getDistanceBetweenCollisionSpheres_w(*target) <= ConfigServerGame::getMaxCombatRange())
		    && m_owner.checkLOSTo(*target))
		{
			result = true;
		}
	}

	return result;
}

// ======================================================================
