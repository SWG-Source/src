// ======================================================================
//
// ShipClientUpdateTracker.cpp
//
// Copyright 2004 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ShipClientUpdateTracker.h"

#include "serverGame/Client.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/ConnectionServerConnection.h"
#include "serverGame/ContainerInterface.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/ObserveTracker.h"
#include "serverGame/PlayerShipController.h"
#include "serverGame/ShipObject.h"
#include "serverNetworkMessages/GameConnectionServerMessages.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedMathArchive/TransformArchive.h"
#include "sharedNetworkMessages/ShipUpdateTransformMessage.h"
#include "sharedUtility/PackedRotationRate.h"
#include "sharedUtility/PackedTransform.h"
#include "sharedUtility/PackedVelocity.h"
#include <algorithm>

#ifdef _DEBUG
#include "sharedGame/AiDebugString.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#endif

// ======================================================================

namespace ShipClientUpdateTrackerNamespace
{

	// ----------------------------------------------------------------------

	class ClientShipEntry
	{
	public:
		ClientShipEntry(unsigned int priority, ShipObject const &ship) :
			m_priority(priority),
			m_ship(ship)
		{
		}

		void setPriority(unsigned int priority)
		{
			m_priority = priority;
		}

		unsigned int getPriority() const
		{
			return m_priority;
		}

		ShipObject *getShip() const
		{
			return safe_cast<ShipObject *>(m_ship.getObject());
		}

		NetworkId const &getShipNetworkId() const
		{
			return m_ship;
		}

	private:
		ClientShipEntry();

	private:
		unsigned int m_priority;
		CachedNetworkId m_ship;
	};

	// ----------------------------------------------------------------------

	typedef std::vector<ClientShipEntry *> ClientShipUpdateQueue;

	struct ClientShipUpdateInfo
	{
		ClientShipUpdateInfo() :
			queuedShipIds(),
			updateQueue(),
			unusedTime(0.0f),
			lastSentPriority(0)
		{
		}

		void reprioritize()
		{
			lastSentPriority = 0;
			if (!updateQueue.empty())
			{
				unsigned int priorityOffset = (*updateQueue.begin())->getPriority();
				for (ClientShipUpdateQueue::const_iterator i = updateQueue.begin(); i != updateQueue.end(); ++i)
					(*i)->setPriority((*i)->getPriority()-priorityOffset);
			}
		}

		unsigned int getShipUpdatePriority(Client const &client, ShipObject const &ship)
		{
			unsigned int const priorityValue = ShipClientUpdateTracker::getShipUpdatePriorityValue(client, ship);
			unsigned int newPriority = lastSentPriority+priorityValue;
			if (newPriority > lastSentPriority)
				return newPriority;
			reprioritize();
			return priorityValue;
		}

		std::set<NetworkId> queuedShipIds;
		ClientShipUpdateQueue updateQueue;
		float unusedTime;
		unsigned int lastSentPriority;
	};

	typedef std::map<Client *, ClientShipUpdateInfo> ClientShipUpdateInfoMap;

	bool s_installed = false;
	ClientShipUpdateInfoMap s_clientShipUpdateInfoMap;

	// ----------------------------------------------------------------------

	bool compareClientShipEntryPriorities(ClientShipEntry const *lhs, ClientShipEntry const *rhs)
	{
		return rhs->getPriority() < lhs->getPriority();
	}

	// ----------------------------------------------------------------------

	void remove()
	{
		FATAL(!s_installed, ("ShipClientUpdateTracker not installed"));
		for (ClientShipUpdateInfoMap::const_iterator i = s_clientShipUpdateInfoMap.begin(); i != s_clientShipUpdateInfoMap.end(); ++i)
			std::for_each((*i).second.updateQueue.begin(), (*i).second.updateQueue.end(), PointerDeleter());
		s_clientShipUpdateInfoMap.clear();
	}

	// ----------------------------------------------------------------------

}
using namespace ShipClientUpdateTrackerNamespace;

// ======================================================================

void ShipClientUpdateTracker::install() // static
{
	FATAL(s_installed, ("ShipClientUpdateTracker already installed"));
	s_installed = true;
	ExitChain::add(&ShipClientUpdateTrackerNamespace::remove, "ShipClientUpdateTrackerNamespace::remove");
}

// ----------------------------------------------------------------------

void ShipClientUpdateTracker::update(float elapsedTime) // static
{
	typedef std::map<ConnectionServerConnection *, std::map<ShipObject *, std::vector<NetworkId> > > ShipDistributionList;
	ShipDistributionList shipDistributionList;

	// Run through all clients, gathering where updates need to be sent this frame.
	ClientShipUpdateInfoMap::iterator i = s_clientShipUpdateInfoMap.begin();
	while (i != s_clientShipUpdateInfoMap.end())
	{
		Client * const client = (*i).first;
		ClientShipUpdateInfo &updateInfo = (*i).second;
		ClientShipUpdateQueue &updateQueue = updateInfo.updateQueue;
		static std::vector<ClientShipEntry *> reenqueues;

		// Run through the update queue for this client, setting up to send
		// updates as needed. When an update is sent, move the entries to a
		// list to be reenqueued after all sending is done, so we don't get
		// multiple sends for a single ship in a frame.

		int const updatesPerSecond = ConfigServerGame::getShipUpdatesPerSecond();
		float const deltaTime = elapsedTime+updateInfo.unusedTime;
		int updateCount = static_cast<int>(updatesPerSecond*deltaTime);
		updateInfo.unusedTime = deltaTime-static_cast<float>(updateCount)/updatesPerSecond;

		while (updateCount && !updateQueue.empty())
		{
			--updateCount;

			ClientShipEntry * const top = updateQueue.front();
			std::pop_heap(updateQueue.begin(), updateQueue.end(), compareClientShipEntryPriorities);
			updateQueue.pop_back();

			ShipObject * const ship = top->getShip();

			if (   ship
			    && (client != ship->getClient() || ship->hasCondition(static_cast<int>(TangibleObject::C_docking)))
			    && ship->isInWorld()
			    && ObserveTracker::isObserving(*client, *ship))
			{
				if (ship->isAuthoritative())
				{
					ServerObject const * const characterObject = client->getCharacterObject();
					if (characterObject)
					{
						#ifdef _DEBUG
						if (ConfigServerGame::getShipClientUpdateDebugEnabled())
						{
							if (characterObject->getObjVars().hasItem("ship_client_update_debug"))
							{
								char buf[32];
								IGNORE_RETURN(snprintf(buf, sizeof(buf)-1, "%u\n", ShipClientUpdateTracker::getShipUpdatePriorityValue(*client, *ship)));
								buf[sizeof(buf)-1] = '\0';
								AiDebugString a;
								a.addText(buf, PackedRgb::solidCyan);
								client->send(GenericValueTypeMessage<std::pair<NetworkId, std::string> >("AiDebugString", std::make_pair(ship->getNetworkId(), a.toString())), false);
							}
						}
						#endif

						// Don't send position updates to players in ships that are teleporting
						ShipObject const * const clientShip = ShipObject::getContainingShipObject(characterObject);
						if (!clientShip || !safe_cast<PlayerShipController const *>(clientShip->getController())->isTeleporting() || !clientShip->getPilot())
							shipDistributionList[client->getConnection()][ship].push_back(characterObject->getNetworkId());
					}
				}
				reenqueues.push_back(top);
				updateInfo.lastSentPriority = top->getPriority();
			}
			else
			{
				IGNORE_RETURN(updateInfo.queuedShipIds.erase(top->getShipNetworkId()));
				delete top;
			}
		}

		// Reenqueue any ships for which updates are being sent this frame.
		for (std::vector<ClientShipEntry *>::const_iterator j = reenqueues.begin(); j != reenqueues.end(); ++j)
		{
			(*j)->setPriority(updateInfo.getShipUpdatePriority(*client, *NON_NULL((*j)->getShip())));
			updateQueue.push_back(*j);
			std::push_heap(updateQueue.begin(), updateQueue.end(), compareClientShipEntryPriorities);
		}
		reenqueues.clear();

		if (updateQueue.empty())
			s_clientShipUpdateInfoMap.erase(i++);
		else
			++i;
	}

	// Push out updates now that we've determined what needs to be pushed this frame.

	// The first implementation of this is here to show what we are logically
	// doing, which is building a GameClientMessage with an embedded
	// ShipUpdateTransformMessage and sending it to the connection server,
	// for everything in the distribution list.

	// The second implementation effectively does the same thing, but does
	// about 4-5x less copying of the data, and the copy to each connection
	// server's network buffer in a single chunk.  This could be further
	// improved by making a packet interface which directly builds into
	// the network buffer, but this was much less intrusive for optimizing
	// this particular case.

	for (ShipDistributionList::const_iterator j = shipDistributionList.begin(); j != shipDistributionList.end(); ++j)
	{
		for (std::map<ShipObject *, std::vector<NetworkId> >::const_iterator k = (*j).second.begin(); k != (*j).second.end(); ++k)
		{
			ShipObject * const ship = (*k).first;
			ShipController const * const controller = safe_cast<ShipController const *>(ship->getController());

			static Archive::ByteStream bs;

			static ConstCharCrcString const s_gcmname("GameClientMessage");
			static ConstCharCrcString const s_sutmname("ShipUpdateTransformMessage");
			static unsigned int const s_sutmByteStreamLength =
				  2 // member count
				+ 4 // crc
				+ 2 // shipId
				+ PackedTransform::cs_packedSize
				+ PackedVelocity::cs_packedSize
				+ PackedRotationRate::cs_packedSize*3
				+ 4 // syncstamp
				;

			Archive::put(bs, static_cast<unsigned short>(4/*gcmMemberCount*/));
			Archive::put(bs, s_gcmname.getCrc());
			Archive::put(bs, (*k).second);
			Archive::put(bs, false);
			Archive::put(bs, s_sutmByteStreamLength);
			Archive::put(bs, static_cast<unsigned short>(5/*sutmMemberCount*/));
			Archive::put(bs, s_sutmname.getCrc());
			Archive::put(bs, ship->getShipId());
			PackedTransform const packedTransform(controller->getTransform());
			Archive::put(bs, packedTransform);
			PackedVelocity const packedVelocity(controller->getVelocity());
			Archive::put(bs, packedVelocity);
			PackedRotationRate const packedYawRate(controller->getYawRate());
			Archive::put(bs, packedYawRate);
			PackedRotationRate const packedPitchRate(controller->getPitchRate());
			Archive::put(bs, packedPitchRate);
			PackedRotationRate const packedRollRate(controller->getRollRate());
			Archive::put(bs, packedRollRate);
			Archive::put(bs, (*j).first->getSyncStampLong());

			(*j).first->Connection::send(bs, false);
			bs.clear();
		}
	}
}

// ----------------------------------------------------------------------

void ShipClientUpdateTracker::queueForUpdate(Client &client, ShipObject const &ship) // static
{
	if (ship.getClient() != &client || ship.hasCondition(static_cast<int>(TangibleObject::C_docking)))
	{
		ClientShipUpdateInfo &updateInfo = s_clientShipUpdateInfoMap[&client];
		std::pair<std::set<NetworkId>::iterator, bool> result = updateInfo.queuedShipIds.insert(ship.getNetworkId());
		if (result.second)
		{
			updateInfo.updateQueue.push_back(new ClientShipEntry(updateInfo.getShipUpdatePriority(client, ship), ship));
			std::push_heap(updateInfo.updateQueue.begin(), updateInfo.updateQueue.end(), compareClientShipEntryPriorities);
		}
	}
} //lint !e1764 // used in a nonconst manner, even if lint doesn't believe so

// ----------------------------------------------------------------------

void ShipClientUpdateTracker::onClientDestroyed(Client &client) // static
{
	ClientShipUpdateInfoMap::iterator i = s_clientShipUpdateInfoMap.find(&client);
	if (i != s_clientShipUpdateInfoMap.end())
	{
		ClientShipUpdateQueue &updateQueue = (*i).second.updateQueue;
		std::for_each(updateQueue.begin(), updateQueue.end(), PointerDeleter());
		s_clientShipUpdateInfoMap.erase(i);
	}
} //lint !e1764 // used in a nonconst manner, even if lint doesn't believe so

// ----------------------------------------------------------------------

unsigned int ShipClientUpdateTracker::getShipUpdatePriorityValue(Client const &client, ShipObject const &ship) // static
{
	CreatureObject const * const characterObject = safe_cast<CreatureObject const *>(client.getCharacterObject());
	if (characterObject && characterObject->getLookAtTarget() != ship.getNetworkId())
	{
		Object const * const clientTopmost = NON_NULL(ContainerInterface::getTopmostContainer(*characterObject));
		Vector const shipOffset = ship.getPosition_p()-clientTopmost->getPosition_p();
		float const distance = shipOffset.magnitude();
		float const angle = acosf(clientTopmost->getObjectFrameK_p().dot(shipOffset/distance));
		float const anglePriorityAdjust = 1.f+((clamp(PI_OVER_4, angle, PI)-PI_OVER_4)/(PI-PI_OVER_4))*4.f; // 1 when roughly in view frustum, up to 5 directly behind
		return static_cast<unsigned int>(clamp(100.f, distance, 8192.f)*anglePriorityAdjust);
	}
	return 100;
}

// ======================================================================

