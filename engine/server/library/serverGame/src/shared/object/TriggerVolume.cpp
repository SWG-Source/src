// ======================================================================
//
// TriggerVolume.cpp
//
// Copyright 2000-04 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/ServerObject.h"
#include "serverGame/ServerWorld.h"
#include "serverGame/TriggerVolume.h"
#include "serverScript/GameScriptObject.h"
#include "serverScript/ScriptDictionary.h"
#include "serverScript/ScriptParameters.h"
#include "serverScript/ScriptMessage.h"
#include "sharedDebug/Profiler.h"
#include "sharedObject/NetworkIdManager.h"
#include <algorithm>
#include <unordered_map>

// ======================================================================

namespace TriggerVolumeNamespace
{

	// ----------------------------------------------------------------------

	class TriggerScriptParam
	{
	public:
		void add(NetworkId const & source, NetworkId const & target, const char * name, bool isEnter);
		void remove(NetworkId const & id);
		void clear();

		std::deque<bool> const &  getIsEnter() const;
		std::vector<const char *> const & getNames() const;
		std::vector<NetworkId> const & getSources() const;
		std::vector<NetworkId> const & getTargets() const;

	private:
		std::deque<bool>          m_isEnter;
		std::vector<const char *> m_names;
		std::vector<NetworkId>    m_sources;
		std::vector<NetworkId>    m_targets;

		// optimization to track how many time a NetworkId appears
		// in sources and targets, so that we can avoid unnecessarily
		// searching the entire sources and targets vectors
		std::unordered_map<NetworkId, int> m_sourcesCount;
		std::unordered_map<NetworkId, int> m_targetsCount;
	};

	// ----------------------------------------------------------------------

	void TriggerScriptParam::add(NetworkId const & source, NetworkId const & target, const char * name, bool isEnter)
	{
		m_isEnter.push_back(isEnter);
		m_names.push_back(name);
		m_sources.push_back(source);
		m_targets.push_back(target);

		std::unordered_map<NetworkId, int>::iterator i = m_sourcesCount.find(source);
		if (i == m_sourcesCount.end())
			m_sourcesCount[source] = 1;
		else
			++(i->second);

		i = m_targetsCount.find(target);
		if (i == m_targetsCount.end())
			m_targetsCount[target] = 1;
		else
			++(i->second);
	}

	// ----------------------------------------------------------------------

	void TriggerScriptParam::remove(NetworkId const & id)
	{
		// remove all entries from sources
		// and targets for the specified id
		std::unordered_map<NetworkId, int>::iterator const iterCountSource = m_sourcesCount.find(id);
		if (iterCountSource != m_sourcesCount.end())
		{
			int countSource = iterCountSource->second;
			for (std::vector<NetworkId>::iterator s = m_sources.begin(); ((s != m_sources.end()) && (countSource > 0));)
			{
				if (*s == id)
				{
					size_t const index = std::distance(m_sources.begin(), s);

					// remove corresponding item from m_isEnter
					std::deque<bool>::iterator enterIter = m_isEnter.begin();
					std::advance(enterIter, index);
					IGNORE_RETURN(m_isEnter.erase(enterIter));
					
					// remove corresponding item from m_names
					std::vector<const char *>::iterator namesIter = m_names.begin();
					std::advance(namesIter, index);
					IGNORE_RETURN(m_names.erase(namesIter));

					// remove corresponding item from m_targets
					std::vector<NetworkId>::iterator targetsIter = m_targets.begin();
					std::advance(targetsIter, index);

					std::unordered_map<NetworkId, int>::iterator const iterCountTarget = m_targetsCount.find(*targetsIter);
					if (iterCountTarget != m_targetsCount.end())
					{
						if (iterCountTarget->second <= 1)
							m_targetsCount.erase(iterCountTarget);
						else
							--(iterCountTarget->second);
					}

					IGNORE_RETURN(m_targets.erase(targetsIter));

					// remove item from m_sources
					s =  m_sources.erase(s);
					--countSource;
				}
				else
				{
					++s;
				}
			}

			m_sourcesCount.erase(iterCountSource);
		}

		std::unordered_map<NetworkId, int>::iterator const iterCountTarget = m_targetsCount.find(id);
		if (iterCountTarget != m_targetsCount.end())
		{
			int countTarget = iterCountTarget->second;
			for (std::vector<NetworkId>::iterator s = m_targets.begin(); ((s != m_targets.end()) && (countTarget > 0));)
			{
				if (*s == id)
				{
					size_t const index = std::distance(m_targets.begin(), s);

					// remove corresponding item from m_isEnter
					std::deque<bool>::iterator enterIter = m_isEnter.begin();
					std::advance(enterIter, index);
					IGNORE_RETURN(m_isEnter.erase(enterIter));
					
					// remove corresponding item from m_names
					std::vector<const char *>::iterator namesIter = m_names.begin();
					std::advance(namesIter, index);
					IGNORE_RETURN(m_names.erase(namesIter));

					// remove corresponding item from m_sources
					std::vector<NetworkId>::iterator sourcesIter = m_sources.begin();
					std::advance(sourcesIter, index);

					std::unordered_map<NetworkId, int>::iterator const iterCountSource = m_sourcesCount.find(*sourcesIter);
					if (iterCountSource != m_sourcesCount.end())
					{
						if (iterCountSource->second <= 1)
							m_sourcesCount.erase(iterCountSource);
						else
							--(iterCountSource->second);
					}

					IGNORE_RETURN(m_sources.erase(sourcesIter));

					// remove item from m_targets
					s = m_targets.erase(s);
					--countTarget;
				}
				else
				{
					++s;
				}
			}

			m_targetsCount.erase(iterCountTarget);
		}
	}

	// ----------------------------------------------------------------------

	void TriggerScriptParam::clear()
	{
		m_isEnter.clear();
		m_names.clear();
		m_sources.clear();
		m_targets.clear();
		m_sourcesCount.clear();
		m_targetsCount.clear();
	}

	// ----------------------------------------------------------------------

	std::deque<bool> const & TriggerScriptParam::getIsEnter() const
	{
		return m_isEnter;
	}

	// ----------------------------------------------------------------------

	std::vector<const char *> const & TriggerScriptParam::getNames() const
	{
		return m_names;
	}

	// ----------------------------------------------------------------------

	std::vector<NetworkId> const & TriggerScriptParam::getSources() const
	{
		return m_sources;
	}

	// ----------------------------------------------------------------------

	std::vector<NetworkId> const & TriggerScriptParam::getTargets() const
	{
		return m_targets;
	}

	// ----------------------------------------------------------------------

	// don't update s_triggers1 and s_triggers2 directly; use s_triggers instead
	TriggerScriptParam s_triggers1;
	TriggerScriptParam s_triggers2;
	TriggerScriptParam * s_triggers = &s_triggers1;
}

using namespace TriggerVolumeNamespace;

// ======================================================================

TriggerVolume::TriggerVolume(ServerObject &newOwner, float radius, std::string const &newName, bool isPromiscuous) :
	m_radius(radius),
	m_contents(),
	m_eventSources(),
	m_name(newName),
	m_owner(newOwner),
	m_spatialSubdivisionHandle(0),
	m_promiscuous(isPromiscuous)
{
}

// ----------------------------------------------------------------------

TriggerVolume::~TriggerVolume()
{
	// remove all entries from s_triggers.sources
	// and s_triggers.targets corresponding to m_owner
	s_triggers->remove(m_owner.getNetworkId());

	// tell all objects in our trigger volume that we are going away
	for (ContentsSet::const_iterator i = m_contents.begin(); i != m_contents.end(); ++i)
		(*i)->onRemovedFromTriggerVolume(*this);

	m_spatialSubdivisionHandle = 0;
}

// ----------------------------------------------------------------------

bool TriggerVolume::intersectsExtent(ServerObject const &object) const
{
	Vector const &position_w = m_owner.getPosition_w();
	return Sphere(position_w, m_radius).intersectsSphere(object.getSphereExtent());
}

// ----------------------------------------------------------------------

void TriggerVolume::addEventSource(NetworkId const &source)
{
	IGNORE_RETURN(m_eventSources.insert(source));
	ServerObject * const object = ServerWorld::findObjectByNetworkId(source);
	if (object && intersectsExtent(*object))
		s_triggers->add(object->getNetworkId(), m_owner.getNetworkId(), m_name.c_str(), true);
}

// ----------------------------------------------------------------------

void TriggerVolume::addObject(ServerObject &object)
{
	if (intersectsExtent(object))
		onEnter(object);
}

// ----------------------------------------------------------------------

void TriggerVolume::onEnter(ServerObject &object)
{
	std::pair<ContentsSet::iterator, bool> const result = m_contents.insert(&object);
	if (result.second)
	{
		virtualOnEnter(object);
		object.onAddedToTriggerVolume(*this);
	}
}

// ----------------------------------------------------------------------

void TriggerVolume::onExit(ServerObject &object)
{
	if (m_contents.erase(&object) > 0)
	{
		virtualOnExit(object);
		object.onRemovedFromTriggerVolume(*this);
	}
}

// ----------------------------------------------------------------------

void TriggerVolume::virtualOnEnter(ServerObject &object)
{
	if (   m_owner.isAuthoritative()
	    && &object != &m_owner
	    && !isNetworkTriggerVolume()
	    && (   !ConfigServerGame::getEnforcePlayerTriggerVolumesOnly()
	        || object.isPlayerControlled()
	        || m_owner.isPlayerControlled()
	        || object.isInterested(m_owner)
	        || m_owner.isInterested(object))
	    && (   m_promiscuous
	        || m_eventSources.find(object.getNetworkId()) != m_eventSources.end()))
	{
		s_triggers->add(object.getNetworkId(), m_owner.getNetworkId(), m_name.c_str(), true);
	}
	m_owner.onTriggerEnter(object, *this);
}

// ----------------------------------------------------------------------

void TriggerVolume::virtualOnExit(ServerObject &object)
{
	if (   m_owner.isAuthoritative()
	    && &object != &m_owner
	    && !isNetworkTriggerVolume()
	    && (   !ConfigServerGame::getEnforcePlayerTriggerVolumesOnly()
	        || object.isPlayerControlled()
	        || m_owner.isPlayerControlled()
	        || object.isInterested(m_owner)
	        || m_owner.isInterested(object))
	    && (   m_promiscuous
	        || m_eventSources.find(object.getNetworkId()) != m_eventSources.end()))
	{
		s_triggers->add(object.getNetworkId(), m_owner.getNetworkId(), m_name.c_str(), false);
	}
	m_owner.onTriggerExit(object, *this);
}

// ----------------------------------------------------------------------

void TriggerVolume::moveObject(ServerObject &object, Vector const &start, Vector const &end)
{
	static const int notInTrigger     = 0x0000;
	static const int startedInTrigger = 0x0001;
	static const int endedInTrigger   = 0x0002;

	// update the sphere position
	Vector const &position_w = m_owner.getPosition_w();
	Sphere const &objectExtentSphere = object.getSphereExtent();

	Vector intersectionPoint;
	if (end.magnitudeBetweenSquared(start) > Vector::NORMALIZE_THRESHOLD)
		intersectionPoint = position_w.findClosestPointOnLineSegment(start, end);
	else
		intersectionPoint = end;

	if (position_w.magnitudeBetween(intersectionPoint) - m_radius < objectExtentSphere.getRadius())
	{
		// there was an intersection along the path. Determine start and end sphere intersection status
		int flags = notInTrigger;

		Sphere const triggerSphere(position_w, m_radius);

		if (triggerSphere.intersectsSphere(Sphere(end, objectExtentSphere.getRadius())))
			flags |= endedInTrigger;
		
		if (triggerSphere.intersectsSphere(Sphere(start, objectExtentSphere.getRadius())))
			flags |= startedInTrigger;

		switch (flags)
		{
			case (startedInTrigger | endedInTrigger):
				onEnter(object); // started and ended in the trigger volume
				break;
			case startedInTrigger:
				onExit(object);	// it started in the trigger but ended outside of it
				break;
			case endedInTrigger:
				onEnter(object); // started outside the trigger, but ended inside of it
				break;
			case notInTrigger:
				// The object crossed the trigger volume but did not start or end inside
				// it, so if we didn't move more than the triggerVolumeWarpDistance,
				// consider us as having entered and left the trigger volume.
				if (start.magnitudeBetweenSquared(end) < sqr(ConfigServerGame::getTriggerVolumeWarpDistance()))
				{
					onEnter(object);
					onExit(object);
				}
				break;
			default:
				break;
		}
	}
	else
	{
		onExit(object);
	}
}

// ----------------------------------------------------------------------
/**
	@brief move the trigger volume relative to an object to determine if
	there was an interaction.
*/
void TriggerVolume::moveTriggerVolume(ServerObject &object, Vector const &start, Vector const &end)
{
	PROFILER_AUTO_BLOCK_DEFINE("TriggerVolume::moveTriggerVolume");
	static const int notInTrigger     = 0x0000;
	static const int startedInTrigger = 0x0001;
	static const int endedInTrigger   = 0x0002;

	Sphere const &objectExtentSphere = object.getSphereExtent();

	Vector intersectionPoint;
	if (end.magnitudeBetweenSquared(start) > Vector::NORMALIZE_THRESHOLD)
		intersectionPoint = objectExtentSphere.getCenter().findClosestPointOnLineSegment(start, end);
	else
		intersectionPoint = end;
	
	if (objectExtentSphere.getCenter().magnitudeBetween(intersectionPoint) - objectExtentSphere.getRadius() < m_radius)
	{
		// there was an intersection along the path. Determine start and end sphere intersection
		// status
		int flags = notInTrigger;

		if (objectExtentSphere.intersectsSphere(Sphere(end, m_radius)))
			flags |= endedInTrigger;
		
		if (objectExtentSphere.intersectsSphere(Sphere(start, m_radius)))
			flags |= startedInTrigger;

		switch (flags)
		{
			case (startedInTrigger | endedInTrigger):
				onEnter(object); // started and ended in the trigger volume
				break;
			case startedInTrigger:
				onExit(object);	// it started in the trigger but ended outside of it
				break;
			case endedInTrigger:
				onEnter(object); // started outside the trigger, but ended inside of it
				break;
			case notInTrigger:
				onExit(object); // handle warping out
				break;			
			default:
				break;
		}
	}
}

// ----------------------------------------------------------------------

void TriggerVolume::removeEventSource(NetworkId const &source)
{
	IGNORE_RETURN(m_eventSources.erase(source));
	ServerObject * const object = ServerWorld::findObjectByNetworkId(source);
	if (object && intersectsExtent(*object))
		onExit(*object);
}

// ----------------------------------------------------------------------

void TriggerVolume::removeObject(ServerObject &object)
{
	onExit(object); // doesn't hurt if it isn't already in the volume
}

// ----------------------------------------------------------------------

void TriggerVolume::runScriptTriggers()
{
	if (!s_triggers->getIsEnter().empty())
	{
		ScriptParams s;
		s.addParam(const_cast<std::deque<bool> &>(s_triggers->getIsEnter()), "is_enter_trigger");
		s.addParam(const_cast<std::vector<const char *> &>(s_triggers->getNames()), "trigger_volume_names");
		s.addParam(const_cast<std::vector<NetworkId> &>(s_triggers->getSources()), "source_objects");
		s.addParam(const_cast<std::vector<NetworkId> &>(s_triggers->getTargets()), "target_objects");

		// set s_triggers to use the other TriggerScriptParam so that
		// any TriggerVolume update that occurs because of the call
		// into script below will get processed the next time
		// TriggerVolume::runScriptTriggers() gets called;
		// hang on to the current TriggerScriptParam so that we can
		// clear it out after the call into script below 
		TriggerScriptParam * current = s_triggers;
		s_triggers = ((s_triggers == &s_triggers1) ? &s_triggers2 : &s_triggers1);

		// invoke the script for trigger enters
		GameScriptObject::runOneScript("base_class", "dispatchTriggerVolumeEvents", "b[s[O[O[", s);

		// triggers have been processed
		// MUST be done after calling GameScriptObject::runOneScript() since
		// ScriptParams just save off the pointers to the vectors; the vectors
		// don't actually get packed until GameScriptObject::runOneScript()
		// is called
		current->clear();
	}
}

// ----------------------------------------------------------------------

void TriggerVolume::setPromiscuous(bool isPromiscuous)
{
	m_promiscuous = isPromiscuous;
}

// ----------------------------------------------------------------------

void TriggerVolume::setSpatialSubdivisionHandle(SpatialSubdivisionHandle *newHandle)
{
	m_spatialSubdivisionHandle = newHandle;
}

// ----------------------------------------------------------------------

bool TriggerVolume::isNetworkTriggerVolume() const
{
	return false;
}

// ----------------------------------------------------------------------

bool TriggerVolume::isPortalTriggerVolume() const
{
	return false;
}

// ----------------------------------------------------------------------

void TriggerVolume::objectMoved(ServerObject &object)
{
	if (intersectsExtent(object))
		onEnter(object);
	else
		onExit(object);
}

// ----------------------------------------------------------------------

void TriggerVolume::objectDestroyed(ServerObject &object)
{
	// NOTE: do not reference the ServerObject because
	// it is being destroyed and is not guaranteed to be
	// in a valid state
	IGNORE_RETURN(m_contents.erase(&object));
}

// ----------------------------------------------------------------------

void TriggerVolume::contentsChanged(std::vector<ServerObject *> const &newSortedContents)
{
	static std::vector<ServerObject *> lostObjects;
	static std::vector<ServerObject *> gainedObjects;

	IGNORE_RETURN(std::set_difference(
		m_contents.begin(), m_contents.end(),
		newSortedContents.begin(), newSortedContents.end(),
		std::back_insert_iterator<std::vector<ServerObject *> >(lostObjects)));
	IGNORE_RETURN(std::set_difference(
		newSortedContents.begin(), newSortedContents.end(),
		m_contents.begin(), m_contents.end(),
		std::back_insert_iterator<std::vector<ServerObject *> >(gainedObjects)));

	m_contents.clear();
	m_contents.insert(newSortedContents.begin(), newSortedContents.end());

	for (std::vector<ServerObject *>::const_iterator i = lostObjects.begin(); i != lostObjects.end(); ++i)
	{
		virtualOnExit(**i);
		(*i)->onRemovedFromTriggerVolume(*this);
	}
	for (std::vector<ServerObject *>::const_iterator j = gainedObjects.begin(); j != gainedObjects.end(); ++j)
	{
		virtualOnEnter(**j);
		(*j)->onAddedToTriggerVolume(*this);
	}

	lostObjects.clear();
	gainedObjects.clear();
}

// ======================================================================

