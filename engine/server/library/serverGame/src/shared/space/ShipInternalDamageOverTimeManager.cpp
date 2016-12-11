//======================================================================
//
// ShipInternalDamageOverTimeManager.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ShipInternalDamageOverTimeManager.h"

#include "serverGame/ShipInternalDamageOverTime.h"
#include "serverGame/ShipObject.h"
#include "serverScript/GameScriptObject.h"
#include "serverScript/ScriptFunctionTable.h"
#include "serverScript/ScriptParameters.h"
#include "sharedFoundation/ExitChain.h"
#include <algorithm>
#include <vector>

//======================================================================

namespace ShipInternalDamageOverTimeManagerNamespace
{
	typedef ShipInternalDamageOverTimeManager::IdotVector IdotVector;
	typedef std::pair<ShipInternalDamageOverTime, float> IdotDamage;
	typedef std::vector<IdotDamage> IdotDamageVector;

	IdotVector s_idotVector;
	IdotVector s_temporaryIdotVector;
	IdotDamageVector s_temporaryIdotDamageVector;

	float ms_timeSinceUpdate = 0.0f;

	float const ms_updateTime = 2.0f;

	//----------------------------------------------------------------------

	void notifyIdotRemoval(ShipObject & ship, ShipInternalDamageOverTime const & idot)
	{
		GameScriptObject * const gameScriptObject = ship.getScriptObject();
		
		if (gameScriptObject != nullptr)
		{
			ScriptParams p;
			p.addParam(idot.getChassisSlot());
			p.addParam(idot.getDamageRate());
			p.addParam(idot.getDamageThreshold());
			
			IGNORE_RETURN(gameScriptObject->trigAllScripts(Scripting::TRIG_SHIP_INTERNAL_DAMAGE_OVER_TIME_REMOVED, p));
		}			
	}

	//----------------------------------------------------------------------

	void notifyIdotDamage(ShipObject & ship, ShipInternalDamageOverTime const & idot, float damageApplied)
	{
		GameScriptObject * const gameScriptObject = ship.getScriptObject();
		
		if (gameScriptObject != nullptr)
		{
			ScriptParams p;
			p.addParam(idot.getChassisSlot());
			p.addParam(idot.getDamageRate());
			p.addParam(idot.getDamageThreshold());
			p.addParam(damageApplied);
			
			IGNORE_RETURN(gameScriptObject->trigAllScripts(Scripting::TRIG_SHIP_DAMAGED_BY_INTERNAL_DAMAGE_OVER_TIME, p));
		}
	}
}

using namespace ShipInternalDamageOverTimeManagerNamespace;

//----------------------------------------------------------------------

void ShipInternalDamageOverTimeManager::install()
{
	ExitChain::add(ShipInternalDamageOverTimeManager::remove, "ShipInternalDamageOverTimeManager::remove");
}

//----------------------------------------------------------------------

void ShipInternalDamageOverTimeManager::remove()
{ 
	s_temporaryIdotVector.clear();
	s_idotVector.clear();
}

//----------------------------------------------------------------------

void ShipInternalDamageOverTimeManager::update(float elapsedTime)
{
	ms_timeSinceUpdate += elapsedTime;

	if (ms_timeSinceUpdate < ms_updateTime)
		return;

	bool hasExpired = false;

	{
		s_temporaryIdotDamageVector.clear();

		IdotVector::const_iterator const end = s_idotVector.end();
		for (IdotVector::const_iterator it = s_idotVector.begin(); it != end; ++it)
		{
			ShipInternalDamageOverTime const & idot = *it;
			
			float damageApplied = 0.0f;
			if (idot.isExpired() || !idot.applyDamage(ms_timeSinceUpdate, damageApplied))
			{
				idot.setExpired(true);
				hasExpired = true;
			}

			if (damageApplied != 0.0f)
				s_temporaryIdotDamageVector.push_back(IdotDamage(idot, damageApplied));
		}
	}

	//-- process queued damage notifications
	//-- this is done outside the update loop so that script triggers can be allowed to add/remove idots
	{
		for (IdotDamageVector::const_iterator it = s_temporaryIdotDamageVector.begin(); it != s_temporaryIdotDamageVector.end(); ++it)
		{
			IdotDamage const & idotDamage = *it;
			ShipInternalDamageOverTime const & idot = idotDamage.first;
			float const damageApplied = idotDamage.second;

			ShipObject * const shipObject = idot.getShipObject();

			if (nullptr != shipObject)
				notifyIdotDamage(*shipObject, idot, damageApplied);
		}
	}

	//-- process any expired idots
	//-- this is done outside the update loop so that script triggers can be allowed to add/remove idots
	if (hasExpired)
	{
		IdotVector::iterator const expired_begin = std::remove_if(s_idotVector.begin(), s_idotVector.end(), ShipInternalDamageOverTime::ExpiredRemoveFunctor());
		
		s_temporaryIdotVector.clear();
		s_temporaryIdotVector.insert(s_temporaryIdotVector.end(), expired_begin, s_idotVector.end());
		IGNORE_RETURN(s_idotVector.erase(expired_begin, s_idotVector.end()));
		
		{
			IdotVector::const_iterator const end = s_temporaryIdotVector.end();
			for (IdotVector::const_iterator it = s_temporaryIdotVector.begin(); it != end; ++it)
			{
				ShipInternalDamageOverTime const & expiredIdot = *it;
				ShipObject * const shipObject = expiredIdot.getShipObject();
				
				if (shipObject != nullptr)
					notifyIdotRemoval(*shipObject, expiredIdot);
			}
		}
		
		s_temporaryIdotVector.clear();
	}

	ms_timeSinceUpdate = 0.0f;
}

//----------------------------------------------------------------------

bool ShipInternalDamageOverTimeManager::setEntry(ShipObject const & ship, int chassisSlot, float damageRate, float damageThreshold)
{
	ShipInternalDamageOverTime const idot(ship, chassisSlot, damageRate, damageThreshold);
	
	if (!idot.checkValidity())
		return false;
	
	IdotVector::iterator const lowerBound = std::lower_bound(s_idotVector.begin(), s_idotVector.end(), idot);
	
	//-- there is no lower bound, just stick it at the end
	if (lowerBound == s_idotVector.end())
	{
		s_idotVector.push_back(idot);
	}
	else
	{
		ShipInternalDamageOverTime & lowerBoundIdot = *lowerBound;
		
		//-- the lower bound sorts greater than the new idot, therefore this is a new insertion
		if (idot < lowerBoundIdot)
		{
			IGNORE_RETURN(s_idotVector.insert(lowerBound, idot));
		}
		else
		{
			//-- otherwise we are overriding an existing value
			
			lowerBoundIdot.setDamageRate(damageRate);
			lowerBoundIdot.setDamageThreshold(damageThreshold);
		}
	}

	return true;
}

//----------------------------------------------------------------------

bool ShipInternalDamageOverTimeManager::removeEntry(ShipObject const & ship, int chassisSlot)
{
	ShipInternalDamageOverTime const idot(ship, chassisSlot, 0.0f, 0.0f);

	IdotVector::iterator const lowerBound = std::lower_bound(s_idotVector.begin(), s_idotVector.end(), idot);

	//-- there is no lower bound, the idot is not in the vector
	if (lowerBound == s_idotVector.end())
		return false;

	ShipInternalDamageOverTime const lowerBoundIdot = *lowerBound;

	//-- the lower bound sorts greater than the new idot, therefore this is a new insertion
	if (idot < lowerBoundIdot)
		return false;

	IGNORE_RETURN(s_idotVector.erase(lowerBound));

	ShipObject * const shipObject = lowerBoundIdot.getShipObject();
	if (shipObject != nullptr)
		notifyIdotRemoval(*shipObject, lowerBoundIdot);

	return true;
}

//----------------------------------------------------------------------

ShipInternalDamageOverTime const * const ShipInternalDamageOverTimeManager::findEntry(ShipObject const & ship, int chassisSlot)
{
	ShipInternalDamageOverTime const idot(ship, chassisSlot, 0.0f, 0.0f);

	IdotVector::iterator const lowerBound = std::lower_bound(s_idotVector.begin(), s_idotVector.end(), idot);

	//-- there is no lower bound, the idot is not in the vector
	if (lowerBound == s_idotVector.end())
		return nullptr;

	ShipInternalDamageOverTime & lowerBoundIdot = *lowerBound;

	//-- the lower bound sorts greater than the new idot, therefore this is a new insertion
	if (idot < lowerBoundIdot)
		return nullptr;

	return &lowerBoundIdot;
}

//----------------------------------------------------------------------

ShipInternalDamageOverTimeManager::IdotVector const & ShipInternalDamageOverTimeManager::getActiveShipInternalDamageOverTime()
{
	return s_idotVector;
}

//======================================================================
