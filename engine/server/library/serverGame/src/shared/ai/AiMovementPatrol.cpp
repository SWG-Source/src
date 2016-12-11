// ======================================================================
//
// AiMovementPatrol.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/AiMovementPatrol.h"

#include "serverGame/AiCreatureController.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/MessageToQueue.h"
#include "serverGame/ServerWorld.h"
#include "serverPathfinding/CityPathGraphManager.h"
#include "serverPathfinding/CityPathNode.h"
#include "serverScript/ScriptFunctionTable.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedGame/AiDebugString.h"
#include "UnicodeUtils.h"


using namespace Scripting;


// ======================================================================

AiMovementPatrol::AiMovementPatrol(AICreatureController * controller, std::vector<Location> const & locations, bool random, bool flip, bool repeat, int startPoint) :
	AiMovementMove(controller),
	m_patrolPath(),
	m_random(random),
	m_flip(flip),
	m_repeat(repeat),
	m_patrolPointIndex(startPoint)
{
	int count = locations.size();
	m_patrolPath.reserve(count);
	for (std::vector<Location>::const_iterator i = locations.begin(); i != locations.end(); ++i)
	{
		const Location & point = *i;
		m_patrolPath.push_back(AiLocation(point.getCell(), point.getCoordinates()));
	}

	// this makes sure the path is randomized
	if (random)
		m_patrolPointIndex = -1;

	CHANGE_STATE( AiMovementPatrol::stateWaiting );
}

// ----------

AiMovementPatrol::AiMovementPatrol( AICreatureController * controller, std::vector<Unicode::String> const & locations, bool random, bool flip, bool repeat, int startPoint) :
	AiMovementMove(controller),
	m_patrolPath(),
	m_random(random),
	m_flip(flip),
	m_repeat(repeat),
	m_patrolPointIndex(startPoint)
{
	ServerObject * owner = safe_cast<ServerObject *>(controller->getOwner());
	if (owner != nullptr)
	{
		DEBUG_REPORT_LOG(ConfigServerGame::isAiLoggingEnabled(), ("AiMovementPatrol creating named path for %s\n", owner->getNetworkId().getValueString().c_str()));

		if (!ServerWorld::isPreloadComplete())
		{
			// wait until we know we've loaded all the objects
			if (!locations.empty())
			{
				char buffer[32];
				std::string data;
				for (std::vector<Unicode::String>::const_iterator i = locations.begin(); i != locations.end(); ++i)
				{
					data += Unicode::wideToNarrow(*i) + "|";
				}
				data += random ? '1' : '0';
				data += flip ? '1' : '0';
				data += repeat ? '1' : '0';
				if (startPoint > 0)
				{
					data += "*";
					data += _itoa(startPoint, buffer, 10);
				}
				MessageToQueue::sendRecurringMessageToC(owner->getNetworkId(), "C++WaitForPatrolPreload", data, 5);
				
				DEBUG_REPORT_LOG(ConfigServerGame::isAiLoggingEnabled(), ("AiMovementPatrol delaying named path for %s until preload is done\n", owner->getNetworkId().getValueString().c_str()));
				endBehavior();
				return;
			}
		}
		else
		{
			int count = locations.size();
			m_patrolPath.reserve(count);
			for (std::vector<Unicode::String>::const_iterator i = locations.begin(); i != locations.end(); ++i)
			{
				const Unicode::String & pointName = *i;
				const CityPathNode * node = CityPathGraphManager::getNamedNodeFor(*owner, pointName);
				if (node != nullptr)
				{
					m_patrolPath.push_back(AiLocation(node->getSourceId()));
				}
				else
				{
					m_patrolPath.clear();
					DEBUG_WARNING(true, ("AiMovementPatrol unable to find path node %s for object %s", 
						Unicode::wideToNarrow(pointName).c_str(), 
						owner->getNetworkId().getValueString().c_str()));
					break;
				}
			}
			if (!m_patrolPath.empty())
			{
				DEBUG_REPORT_LOG(ConfigServerGame::isAiLoggingEnabled(),("AiMovementPatrol created named path for %s\n", owner->getNetworkId().getValueString().c_str()));

				// try an find a node on the path that is a previous root node, or isn't 
				// being used in any other path
				std::vector<AiLocation>::iterator i;
				const ServerObject * node = nullptr;
				const ServerObject * root = nullptr;
				for (i = m_patrolPath.begin(); i != m_patrolPath.end(); ++i)
				{
					node = safe_cast<const ServerObject *>(i->getObject());
					if (node != nullptr)
					{
						if (node->isPatrolPathRoot())
						{
							if (root == nullptr || !root->isPatrolPathRoot())
							{
								// if we already have a set up root node, use that
								root = node;
							}
							else
							{
								// we've got two previous root nodes, we can't connect them
								root = nullptr;
								break;
							}
						}
						else if (!node->isPatrolPathNode() && root == nullptr)
						{
							// found a free node
							root = node;
						}
					}
				}
				if (root != nullptr)
				{
					// set up the root node
					if (!root->isPatrolPathRoot())
						const_cast<ServerObject *>(root)->setPatrolPathRoot(*root);

					// connect the nodes to the root node
					for (i = m_patrolPath.begin(); i != m_patrolPath.end(); ++i)
					{
						node = safe_cast<const ServerObject *>(i->getObject());
						if (node != nullptr && node != root)
							const_cast<ServerObject *>(node)->setPatrolPathRoot(*root);
					}
					const_cast<ServerObject *>(root)->addPatrolPathingObject(*owner);
				}
				else
				{
					std::string nodes;
					for (i = m_patrolPath.begin(); i != m_patrolPath.end(); ++i)
					{
						node = safe_cast<const ServerObject *>(i->getObject());
						if (node != nullptr)
							nodes += node->getNetworkId().getValueString() + " ";
					}
					WARNING(true, ("AiMovementPatrol unable to find root node for path: %s", nodes.c_str()));
				}
			}
		}
	}
	else
	{
		WARNING(true, ("AiMovementPatrol::AiMovementPatrol no owner"));
	}

	// this makes sure the path is randomized
	if (random)
		m_patrolPointIndex = -1;

	CHANGE_STATE( AiMovementPatrol::stateWaiting );
}

// ----------

AiMovementPatrol::AiMovementPatrol( AICreatureController * controller, Archive::ReadIterator & source ) : 
	AiMovementMove(controller, source),
	m_patrolPath(),
	m_random(false),
	m_flip(false),
	m_repeat(false),
	m_patrolPointIndex(-1)
{
	int count;
	Archive::get(source, count);
	m_patrolPath.resize(count);
	for (std::vector<AiLocation>::iterator i = m_patrolPath.begin(); i != m_patrolPath.end(); ++i)
		(*i).unpack(source);
		
	Archive::get(source, m_random);
	Archive::get(source, m_flip);
	Archive::get(source, m_repeat);
	Archive::get(source, m_patrolPointIndex);

	SETUP_SYNCRONIZED_STATE( AiMovementPatrol::stateWaiting );
}

// ----------

AiMovementPatrol::~AiMovementPatrol()
{
}

// ----------------------------------------------------------------------

void AiMovementPatrol::pack( Archive::ByteStream & target ) const
{
	AiMovementMove::pack(target);
	
	int count = m_patrolPath.size();
	Archive::put(target, count);
	for (std::vector<AiLocation>::const_iterator i = m_patrolPath.begin(); i != m_patrolPath.end(); ++i)
		(*i).pack(target);

	Archive::put(target, m_random);
	Archive::put(target, m_flip);
	Archive::put(target, m_repeat);
	Archive::put(target, m_patrolPointIndex);
}

// ----------------------------------------------------------------------

void AiMovementPatrol::alter ( float time )
{
	PROFILER_AUTO_BLOCK_DEFINE("AiMovementPatrol::alter");

	AiMovementMove::alter(time);
}

// ----------------------------------------------------------------------

void AiMovementPatrol::getDebugInfo ( std::string & outString ) const
{
	AiMovementMove::getDebugInfo(outString);

	outString += FormattedString<1024>().sprintf("AiMovementPatrol: point index = %d\n", m_patrolPointIndex);
	outString += "AiMovementPatrol: points:\n";
	if (!m_patrolPath.empty())
	{
		for (std::vector<AiLocation>::const_iterator i = m_patrolPath.begin(); i != m_patrolPath.end(); ++i)
			(*i).getDebugInfo(outString);
	}
	else
	{
		outString += "none\n";
	}
}

// ----------------------------------------------------------------------

void AiMovementPatrol::endBehavior()
{
	if (!m_patrolPath.empty() && m_controller != nullptr)
	{
		const ServerObject * owner = safe_cast<const ServerObject *>(m_controller->getOwner());
		if (owner != nullptr)
		{
			for (std::vector<AiLocation>::iterator i = m_patrolPath.begin(); i != m_patrolPath.end(); ++i)
			{
				const ServerObject * node = safe_cast<const ServerObject *>(i->getObject());
				if (node != nullptr && node->isPatrolPathRoot())
				{
					const_cast<ServerObject *>(node)->removePatrolPathingObject(*owner);
					break;
				}
			}		
		}
	}

	AiMovementMove::endBehavior();
}

// ----------------------------------------------------------------------

void AiMovementPatrol::clear ( void )
{
	DEBUG_REPORT_LOG(ConfigServerGame::isAiLoggingEnabled(), ("AiMovementPatrol clearing path for %s\n", m_controller->getOwner()->getNetworkId().getValueString().c_str()));

	AiMovementMove::clear();
	m_patrolPath.clear();
	m_patrolPointIndex = -1;
}

// ----------------------------------------------------------------------

bool AiMovementPatrol::getHibernateOk() const
{
	if (!m_patrolPath.empty())
	{
		const ServerObject * node = safe_cast<const ServerObject *>(m_patrolPath.front().getObject());
		if (node != nullptr)
		{
			return node->getPatrolPathObservers() == 0;
		}
	}

	// we allow patrols to non-named nodes to hibernate
	return true;
}

// ----------------------------------------------------------------------

AiStateResult AiMovementPatrol::stateWaiting ( float time )
{
	PROFILER_AUTO_BLOCK_DEFINE("AiMovementPatrol::stateWaiting");

	if (!m_patrolPath.empty())
		return triggerPathing();
	else
	{
		DEBUG_REPORT_LOG(ConfigServerGame::isAiLoggingEnabled(), ("AiMovementPatrol waiting: no path for %s\n", m_controller->getOwner()->getNetworkId().getValueString().c_str()));

		m_controller->triggerScriptsSimple(TRIG_MOVE_PATH_NOT_FOUND);
		endBehavior();
	}		

	return ASR_Done;
}

// ----------------------------------------------------------------------

AiStateResult AiMovementPatrol::triggerWaiting( void )
{
	CHANGE_STATE( AiMovementPatrol::stateWaiting );
	return ASR_Done;
}

// ----------------------------------------------------------------------

AiStateResult AiMovementPatrol::triggerPathComplete( void )
{
	if (m_controller->isRetreating())
	{
		m_controller->setRetreating(false);
	}

	if (m_patrolPointIndex >= static_cast<int>(m_patrolPath.size()))
	{
		m_controller->triggerScriptsSimple(TRIG_MOVE_PATH_COMPLETE);
		if (!m_repeat)
		{
			DEBUG_REPORT_LOG(ConfigServerGame::isAiLoggingEnabled(), ("AiMovementPatrol path complete for %s\n", m_controller->getOwner()->getNetworkId().getValueString().c_str()));
			endBehavior();
			return ASR_Done;
		}
	}

	return triggerWaiting();
}

// ----------------------------------------------------------------------

bool AiMovementPatrol::setupPathSearch ( void )
{
	// choose the next patrol point we're going to
	int count = m_patrolPath.size();
	if (m_patrolPointIndex < 0 && count > 0)
	{
		m_patrolPointIndex = 0;
		if (m_random)
		{
			std::random_shuffle(m_patrolPath.begin(), m_patrolPath.end());
		}
	}
	if (m_patrolPointIndex >= 0 && m_patrolPointIndex < count)
	{
		m_target = m_patrolPath[m_patrolPointIndex++];
		if (m_patrolPointIndex == count && m_repeat)
		{
			m_patrolPointIndex = -1;
			if (m_flip)
			{
				std::reverse(m_patrolPath.begin(), m_patrolPath.end());
			}
		}
	}

	return AiMovementMove::setupPathSearch();
}

// ----------------------------------------------------------------------

AiMovementType AiMovementPatrol::getType() const
{
	return AMT_patrol;
}

// ----------------------------------------------------------------------

AiMovementPatrol * AiMovementPatrol::asAiMovementPatrol()
{
	return this;
}

#ifdef _DEBUG
// ----------------------------------------------------------------------
void AiMovementPatrol::addDebug(AiDebugString & aiDebugString)
{
	FormattedString<512> fs;
	AiPath const * const path = getPath();

	aiDebugString.addText(fs.sprintf("PATROL\n"), PackedRgb::solidCyan);
	aiDebugString.addText(fs.sprintf("%s\n", m_stateName.c_str()), PackedRgb::solidCyan);
	aiDebugString.addText(fs.sprintf("pathSize(%u)\n", path->size()), PackedRgb::solidCyan);
	aiDebugString.addText(fs.sprintf("patrolPathSize(%u)\n", m_patrolPath.size()), PackedRgb::solidCyan);
	aiDebugString.addText(fs.sprintf("random(%s)\n", m_random ? "yes" : "no"), PackedRgb::solidCyan);
	aiDebugString.addText(fs.sprintf("flip(%s)\n", m_flip ? "yes" : "no"), PackedRgb::solidCyan);
	aiDebugString.addText(fs.sprintf("repeat(%s)\n", m_repeat ? "yes" : "no"), PackedRgb::solidCyan);
	aiDebugString.addText(fs.sprintf("patrolPointIndex(%i)\n", m_patrolPointIndex), PackedRgb::solidCyan);
	aiDebugString.addText(fs.sprintf("stuckCounter(%i)\n", m_controller->getStuckCounter()), PackedRgb::solidCyan);
	aiDebugString.addText(fs.sprintf("avoidanceStuckTime(%.0f)\n", m_avoidanceStuckTime), PackedRgb::solidCyan);
	aiDebugString.addText(fs.sprintf("obstacle(%s)\n", m_obstacleLocation.getObjectId().getValueString().c_str()), PackedRgb::solidCyan);

	//// Render the movement path
	//{
	//	if (!path->empty())
	//	{
	//		AiDebugString::TransformList transformList;
	//		AiPath::const_iterator iterPath = path->begin();
	//	
	//		Transform transform(m_controller->getOwner()->getTransform_o2p());
	//		transform.move_l(Vector(0.0f, 0.1f, 0.0f));
	//		transformList.push_back(transform);
	//	
	//		for (; iterPath != path->end(); ++iterPath)
	//		{
	//			transform.setPosition_p(iterPath->getPosition_p());
	//			transform.move_l(Vector(0.0f, 0.1f, 0.0f));
	//			transformList.push_back(transform);
	//		}
	//	
	//		bool const cyclic = false;
	//		aiDebugString.addPath(transformList, cyclic);
	//	}
	//}

	// Render the patrol path
	{
		if (!m_patrolPath.empty())
		{
			AiDebugString::TransformList transformList;
			std::vector<AiLocation>::const_iterator iterPath = m_patrolPath.begin();
		
			Transform transform(m_controller->getOwner()->getTransform_o2p());
			transform.move_l(Vector(0.0f, 0.1f, 0.0f));
			transformList.push_back(transform);
		
			for (; iterPath != m_patrolPath.end(); ++iterPath)
			{
				transform.setPosition_p(iterPath->getPosition_p());
				transform.move_l(Vector(0.0f, 0.1f, 0.0f));
				transformList.push_back(transform);
			}
		
			bool const cyclic = false;
			aiDebugString.addPath(transformList, cyclic);
		}
	}
}
#endif // _DEBUG

// ======================================================================
