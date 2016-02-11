//========================================================================
//
// JediManagerController.cpp
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "FirstSwgGameServer.h"
#include "SwgGameServer/JediManagerController.h"
#include "sharedNetworkMessages/MessageQueueGenericValueType.h"
#include "SwgGameServer/JediManagerObject.h"
#include "swgServerNetworkMessages/MessageQueueJediData.h"
#include "swgServerNetworkMessages/MessageQueueJediLocation.h"
#include "swgServerNetworkMessages/MessageQueueRequestJediBounty.h"


//-----------------------------------------------------------------------

JediManagerController::JediManagerController(JediManagerObject * newOwner) :
	UniverseController(newOwner)
{
}

//-----------------------------------------------------------------------

JediManagerController::~JediManagerController()
{
}

//-----------------------------------------------------------------------

void JediManagerController::handleMessage (const int message, const float value, const MessageQueue::Data* const data, const uint32 flags)
{
	JediManagerObject * owner = dynamic_cast<JediManagerObject *>(getOwner());
	NOT_NULL(owner);
	
	switch (message)
	{
	case CM_removeJedi:
		{
			const MessageQueueGenericValueType<NetworkId> * const msg = safe_cast<const MessageQueueGenericValueType<NetworkId> *>(data);
			if (msg != nullptr)
			{
				owner->removeJedi(msg->getValue());
			}
		}
		break;
	case CM_addJedi:
		{
			const MessageQueueJediData * const msg = safe_cast<const MessageQueueJediData *>(data);
			if (msg != nullptr)
			{
				owner->addJedi(msg->getId(), 
				               msg->getName(),
							   msg->getLocation(),
							   msg->getScene(),
				               msg->getVisibility(),
							   msg->getBountyValue(),
							   msg->getLevel(),
				               msg->getHoursAlive(),
							   msg->getState(),
							   msg->getSpentJediSkillPoints(),
							   msg->getFaction()
				              );
			}
		}
		break;
	case CM_updateJedi:
		{
			const MessageQueueJediData * const msg = safe_cast<const MessageQueueJediData *>(data);
			if (msg != nullptr)
			{
				owner->updateJedi(msg->getId(), 
				                  msg->getVisibility(),
								  msg->getBountyValue(),
								  msg->getLevel(),
				                  msg->getHoursAlive()
				                 );
			}
		}
		break;
	case CM_updateJediState:
		{
			const MessageQueueGenericValueType<std::pair<NetworkId, int> > * const msg = safe_cast<const MessageQueueGenericValueType<std::pair<NetworkId, int> > *>(data);
			if (msg != nullptr)
			{
				owner->updateJedi(msg->getValue().first,
				                  static_cast<JediState>(msg->getValue().second)
				                 );
			}
		}
		break;
	case CM_updateJediBounties:
		{
/*
			const MessageQueueGenericValueType<std::pair<NetworkId, std::vector<NetworkId> > > * const msg = safe_cast<const MessageQueueGenericValueType<std::pair<NetworkId, std::vector<NetworkId> > > *>(data);
			if (msg != nullptr)
			{
				owner->updateJedi(msg->getValue().first,
				                  msg->getValue().second
				                 );
			}
*/
		}
		break;
	case CM_updateJediLocation:
		{
			const MessageQueueJediLocation * const msg = safe_cast<const MessageQueueJediLocation *>(data);
			if (msg != nullptr)
			{
				owner->updateJediLocation(msg->getId(), 
				                          msg->getLocation(),
										  msg->getScene()
				                         );
			}
		}
		break;
	case CM_setJediOffline:
		{
			const MessageQueueJediLocation * const msg = safe_cast<const MessageQueueJediLocation *>(data);
			if (msg != nullptr)
			{
				owner->setJediOffline(msg->getId(), 
				                      msg->getLocation(),
									  msg->getScene()
				                     );
			}
		}
		break;
	case CM_requestJediBounty:
		{
			const MessageQueueRequestJediBounty * const msg = safe_cast<const MessageQueueRequestJediBounty *>(data);
			if (msg != nullptr)
			{
				owner->requestJediBounty(msg->getTargetId(), 
				                         msg->getHunterId(),
				                         msg->getSuccessCallback(),
				                         msg->getFailCallback(),
										 msg->getCallbackObjectId()
				                        );
			}
		}
		break;
	case CM_removeJediBounty:
		{
			const MessageQueueGenericValueType<std::pair<NetworkId, NetworkId> > * const msg = safe_cast<const MessageQueueGenericValueType<std::pair<NetworkId, NetworkId> > *>(data);
			if (msg != nullptr)
			{
				owner->removeJediBounty(msg->getValue().first, msg->getValue().second);
			}	
		}
		break;
	case CM_removeAllJediBounties:
		{
			const MessageQueueGenericValueType<NetworkId> * const msg = safe_cast<const MessageQueueGenericValueType<NetworkId> *>(data);
			if (msg != nullptr)
			{
				owner->removeAllJediBounties(msg->getValue());
			}	
		}
		break;
	case CM_updateJediSpentJediSkillPoints:
		{
			const MessageQueueGenericValueType<std::pair<NetworkId, int> > * const msg = safe_cast<const MessageQueueGenericValueType<std::pair<NetworkId, int> > *>(data);
			if (msg != nullptr)
			{
				owner->updateJediSpentJediSkillPoints(msg->getValue().first, msg->getValue().second);
			}
		}
		break;
	case CM_updateJediFaction:
		{
			const MessageQueueGenericValueType<std::pair<NetworkId, int> > * const msg = safe_cast<const MessageQueueGenericValueType<std::pair<NetworkId, int> > *>(data);
			if (msg != nullptr)
			{
				owner->updateJediFaction(msg->getValue().first, msg->getValue().second);
			}
		}
		break;
	case CM_updateJediScriptData:
		{
			const MessageQueueGenericValueType<std::pair<NetworkId, std::pair<std::string, int> > > * const msg = safe_cast<const MessageQueueGenericValueType<std::pair<NetworkId, std::pair<std::string, int> > > *>(data);
			if (msg != nullptr)
			{
				owner->updateJediScriptData(msg->getValue().first, msg->getValue().second.first, msg->getValue().second.second);
			}
		}
		break;
	case CM_removeJediScriptData:
		{
			const MessageQueueGenericValueType<std::pair<NetworkId, std::string> > * const msg = safe_cast<const MessageQueueGenericValueType<std::pair<NetworkId, std::string> > *>(data);
			if (msg != nullptr)
			{
				owner->removeJediScriptData(msg->getValue().first, msg->getValue().second);
			}
		}
		break;
	default:
		UniverseController::handleMessage(message, value, data, flags);
		break;
	}
}

//-----------------------------------------------------------------------
