#include "serverGame/FirstServerGame.h"
#include "serverGame/AiCombatPulseQueue.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/MessageToQueue.h"
#include "serverGame/TangibleObject.h"
#include "serverScript/GameScriptObject.h"
#include "serverScript/ScriptParameters.h"
#include "sharedFoundation/Clock.h"
#include "sharedFoundation/Watcher.h"
#include "sharedLog/Log.h"

#include <map>

namespace AiCombatPulseQueueNamespace
{
	int s_numberPerFrame;
	unsigned int s_maxWaitTimeMs;

	typedef std::pair<Watcher<TangibleObject>, unsigned long>  ScheduledAiMapEntry;
	typedef std::map<Watcher<TangibleObject>, unsigned long>   ScheduledAiMap;
	typedef std::pair<unsigned long, ScheduledAiMap::iterator>     TimeOrderedAiMultiMapEntry;
	typedef std::multimap<unsigned long, ScheduledAiMap::iterator> TimeOrderedAiMultiMap;

	ScheduledAiMap s_scheduledAi;
	TimeOrderedAiMultiMap s_timeOrderedAi;
}

using namespace AiCombatPulseQueueNamespace;

//------------------------------------------------------------------------------------------

void AiCombatPulseQueue::install()
{
	s_numberPerFrame = ConfigServerGame::getAiPulseQueuePerFrame();
	s_maxWaitTimeMs = ConfigServerGame::getAiPulseQueueMaxWaitTimeMs();
}

//------------------------------------------------------------------------------------------

void AiCombatPulseQueue::remove()
{
}

//------------------------------------------------------------------------------------------

void AiCombatPulseQueue::schedule(TangibleObject * const object, int waitTimeMs, unsigned long currentFrameTimeMs)
{
	if (object == nullptr)
		return;

	unsigned long desiredTime = Clock::getFrameStartTimeMs() + currentFrameTimeMs + waitTimeMs;

	// entry is inserted as <object pointer, time we want an onCombatLoop callback>
	std::pair<ScheduledAiMap::iterator, bool> insertReturn = s_scheduledAi.insert(ScheduledAiMapEntry(Watcher<TangibleObject>(object), desiredTime));

	// if the object already has a call back schedule and our desired time is after the current time, replace the current time
	if (!insertReturn.second && desiredTime > (insertReturn.first)->second)
		(insertReturn.first)->second = desiredTime;
}

//------------------------------------------------------------------------------------------

void AiCombatPulseQueue::setAiPerFrame(int amount)
{
	// this ai per frame only comes into effect after all ai that have waited > maxWaitTime have been processed
	// so it is possible that there may be more Ai per frame calculated
	s_numberPerFrame = amount;
}

//------------------------------------------------------------------------------------------

void AiCombatPulseQueue::setAiMaxWaitTimeMs(unsigned int amount)
{
	// you are guaranteed that any Ai that have been waiting over this amount are processed at the end of the server frame
	s_maxWaitTimeMs = amount;
}

//------------------------------------------------------------------------------------------

void AiCombatPulseQueue::alter(real time)
{
	int count = 0;
	unsigned long timeMs = Clock::getFrameStartTimeMs() + static_cast<unsigned long>(time * 1000.0f);

	s_timeOrderedAi.clear();

	// fill in the time ordered ai multimap to traverse across
	for (ScheduledAiMap::iterator i = s_scheduledAi.begin(); i != s_scheduledAi.end(); ++i)
		s_timeOrderedAi.insert(TimeOrderedAiMultiMapEntry(i->second, i));

	for (TimeOrderedAiMultiMap::iterator j = s_timeOrderedAi.begin(); j != s_timeOrderedAi.end(); ++j)
	{
		// if the entry has a time request later than the current time,
		// or we have done our max number per frame and the entry is less than s_maxWaitTimeMs old, then stop
		if (j->first > timeMs || (count > s_numberPerFrame && (timeMs - j->first) > s_maxWaitTimeMs))
			break;

		TangibleObject * const object = (j->second)->first;

		if (object != nullptr && object->isInCombat())
		{
			NetworkId const & id = object->getNetworkId();

			if (id != NetworkId::cms_invalid)
			{
				++count;

				GameScriptObject * const gameScriptObject = GameScriptObject::asGameScriptObject(object);

				if (gameScriptObject != nullptr)
				{
					ScriptParams scriptParams;
					IGNORE_RETURN(gameScriptObject->trigAllScripts(Scripting::TRIG_AI_COMBAT_FRAME, scriptParams));
				}
				else
				{
					WARNING(true, ("AiCombatPulseQueue::alter() Unable to get the ScriptObject for object(%s)", object->getDebugInformation().c_str()));
				}
			}
		}

		s_scheduledAi.erase(j->second);
	}

	LOGC(ConfigServerGame::isAiLoggingEnabled() && count > s_numberPerFrame, "debug_ai", ("AiCombatPulseQueue::alter() processed %i ai combat loops, max number set at %i", count, s_numberPerFrame));
}

//------------------------------------------------------------------------------------------

