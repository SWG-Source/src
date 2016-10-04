// ======================================================================
//
// ServerController.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ServerController.h"

#include "LocalizationManager.h"
#include "UnicodeUtils.h"
#include "serverGame/CellObject.h"
#include "serverGame/Client.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/ContainerInterface.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/GameServer.h"
#include "serverGame/ServerCreatureObjectTemplate.h"
#include "serverGame/ServerMessageForwarding.h"
#include "serverGame/ServerObject.h"
#include "serverGame/ServerWorld.h"
#include "serverNetworkMessages/MessageQueueScriptTrigger.h"
#include "serverNetworkMessages/MessageQueueTeleportObject.h"
#include "serverScript/GameScriptObject.h"
#include "serverScript/ScriptParameters.h"
#include "serverUtility/ChatLogManager.h"
#include "sharedCollision/CollisionProperty.h"
#include "sharedCollision/CollisionWorld.h"
#include "sharedDebug/Profiler.h"
#include "sharedFoundation/DynamicVariableList.h"
#include "sharedFoundation/DynamicVariableLocationData.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedFoundation/Timer.h"
#include "sharedGame/GameObjectTypes.h"
#include "sharedLog/Log.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "sharedNetworkMessages/MessageQueueCombatSpam.h"
#include "sharedNetworkMessages/MessageQueueDataTransform.h"
#include "sharedNetworkMessages/MessageQueueDataTransformWithParent.h"
#include "sharedNetworkMessages/MessageQueueNetworkIdAndTransform.h"
#include "sharedNetworkMessages/MessageQueueSocial.h"
#include "sharedNetworkMessages/MessageQueueSpatialChat.h"
#include "sharedNetworkMessages/MessageQueueString.h"
#include "sharedNetworkMessages/ObjectChannelMessages.h"
#include "sharedNetworkMessages/UpdateTransformMessage.h"
#include "sharedNetworkMessages/UpdateTransformWithParentMessage.h"
#include "sharedObject/AlterResult.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/NetworkIdManager.h"
#include "swgServerNetworkMessages/MessageQueueScriptTransferMoney.h"

//----------------------------------------------------------------------

CellProperty const * getCell(Object const * obj)
{
	if(obj == nullptr) return nullptr;

	return obj->getCellProperty();
}

//----------------------------------------------------------------------

// ----------
// This isn't being used yet...

Vector		movePointBetweenCells ( Object const * oldCellObject, Object const * newCellObject, Vector position )
{
	if(oldCellObject == newCellObject) return position;

	// ----------

	Transform toWorld = Transform::identity;
	Transform toCell = Transform::identity;

	if(oldCellObject != nullptr)
	{
		CellProperty const * oldCell = getCell(oldCellObject);

		if(oldCell)
		{
			toWorld = oldCell->getOwner().getTransform_o2w();
		}
	}

	if(newCellObject != nullptr)
	{
		CellProperty const * newCell = getCell(newCellObject);

		if(newCell)
		{
			toCell = newCell->getOwner().getTransform_o2w();
		}
	}

	// ----------

	Vector worldPos = toWorld.rotateTranslate_l2p(position);

	Vector newCellPos = toCell.rotateTranslate_p2l(worldPos);

	return newCellPos;
}

// ----------------------------------------------------------------------
// This isn't being used yet...

Transform	moveTransformBetweenCells ( Object const * oldCellObject, Object const * newCellObject, Transform const & inTransform )
{
	if(oldCellObject == newCellObject) return inTransform;

	// ----------

	Transform toWorld = Transform::identity;
	Transform toCell = Transform::identity;

	if(oldCellObject != nullptr)
	{
		CellProperty const * oldCell = getCell(oldCellObject);

		if(oldCell)
		{
			toWorld = oldCell->getOwner().getTransform_o2w();
		}
	}

	if(newCellObject != nullptr)
	{
		CellProperty const * newCell = getCell(newCellObject);

		if(newCell)
		{
			toCell = newCell->getOwner().getTransform_o2w();
		}
	}

	// ----------

	// Out transform = toCell * toWorld * inTransform

	Transform temp;

	temp.multiply(toCell,toWorld);

	Transform out;

	out.multiply(temp,inTransform);

	return out;
}


//-----------------------------------------------------------------------

ServerController::MessageQueueNotification::MessageQueueNotification() :
	Notification(),
	m_serverController(0)
{
}

//-----------------------------------------------------------------------

ServerController::MessageQueueNotification::~MessageQueueNotification()
{
	m_serverController = 0;
}

//-----------------------------------------------------------------------

void ServerController::MessageQueueNotification::setController(ServerController* serverController)
{
	m_serverController = serverController;
}

//-----------------------------------------------------------------------

void ServerController::MessageQueueNotification::onChanged() const
{
	NOT_NULL(m_serverController);
	m_serverController->onAppendMessage();
}

// ----------------------------------------------------------------------

ServerController::ServerController(Object *newOwner) :
		NetworkController         (newOwner),
		m_sendReliableTransformThisFrame(true),
		m_moveSequenceNumber      (0),
		m_currentVelocity         (Vector::zero),
		m_oldVelocity             (Vector::zero),
		m_goalTransform           (Transform::identity),
		m_goalCellObject          (),
		m_bProcessingMessages     (false),
		m_bHasGoal                (false),
		m_bAtGoal                 (false),
		m_bTeleport               (false),
		m_alteredSinceConclude    (false),
		m_notification            ()
{
	m_notification.setController (this);
	setMessageQueueNotification (&m_notification);
}

// ----------------------------------------------------------------------

ServerController::~ServerController(void)
{
}

// ----------------------------------------------------------------------

void ServerController::endBaselines()
{
}

// ----------------------------------------------------------------------

void ServerController::setAuthoritative(bool newAuthoritative)
{
	if (!newAuthoritative && m_bHasGoal && m_goalCellObject)
	{
		m_goalCellObject = nullptr;
		m_bHasGoal = false;
		m_bAtGoal = true;
	}

	NetworkController::setAuthoritative(newAuthoritative);
}	// ServerController::setAuthoritative

// ----------------------------------------------------------------------

void ServerController::setOwner(Object *newOwner)
{
	NOT_NULL(safe_cast<ServerObject*>(newOwner));
	Controller::setOwner(newOwner);
}

// ----------------------------------------------------------------------

int ServerController::elevatorMove( int nFloors )
{
	CollisionProperty * collision = getOwner()->getCollisionProperty();

	if(collision)
	{
		Transform elevatorTransform;

		int movedFloors = collision->elevatorMove(nFloors,elevatorTransform);

		if(movedFloors)
		{
			ServerObject * cellObject = dynamic_cast<ServerObject *>(&(getOwner()->getParentCell()->getOwner()));
			teleport(elevatorTransform,cellObject);
		}

		return movedFloors;
	}
	else
	{
		return 0;
	}
}

// ----------------------------------------------------------------------

float ServerController::realAlter(float time)
{
	PROFILER_AUTO_BLOCK_DEFINE("ServerController::realAlter");

	int message;
	real value;
	MessageQueue::Data* data;
	uint32 flags;

	ServerObject *target = static_cast<ServerObject *>(getOwner());

	// ----------

	m_bProcessingMessages = true;

	int i;
	int const messageCount = getNumberOfMessages();
	for (i = 0; i < messageCount; ++i)
	{
		getMessage(i, &message, &value, &data, &flags);
		if (message)
		{
			// if the message is for an authoritative/proxy, make sure we are
			// the right type
			bool processMessage = false;
			if (flags & GameControllerMessageFlags::DEST_AUTH_SERVER)
			{
				if (target->isAuthoritative())
					processMessage = true;
				else
				{
					if(ConfigServerGame::getEnableDebugControllerMessageSpam())
					{
						if(messageCount > 1000)
						{
							if(message == CM_scriptTrigger)
							{
								const MessageQueueScriptTrigger * const triggerMessage = dynamic_cast<const MessageQueueScriptTrigger *>(data);
								if(triggerMessage)
								{
									const Scripting::TrigId trigId = static_cast<const Scripting::TrigId>(triggerMessage->getTriggerId());
									LOG("ObjControllerMessageSTORM:CM_scriptTrigger", ("(%s:%s) received a CM_scriptTrigger. The object is not authoritative and there are over 1000 messages pending in the queue! script TrigId=%d", target->getObjectTemplateName(), target->getNetworkId().getValueString().c_str(), trigId));
									if (!target->isPlayerControlled() && target->asCreatureObject())
										target->unload();
								}							
							}
						}
					}
				}
			}
			if (flags & GameControllerMessageFlags::DEST_PROXY_SERVER)
			{
				if (!target->isAuthoritative())
					processMessage = true;
			}
			if (flags & GameControllerMessageFlags::DEST_SERVER)
			{
				processMessage = true;
			}
			if (processMessage)
			{
				handleMessage(message, value, data, flags);
				//@todo clear if flags are not SEND messageQueue->clearMessage(i);
			}
		}
	}

	if(i > 0)
	{
		safe_cast<ServerObject *>(getOwner())->addObjectToConcludeList();
	}

	m_bProcessingMessages = false;

	moveToGoal();

	// ----------

	m_alteredSinceConclude = true;

	// No reason to force an alter next frame here.  Controller ensures
	// we get an alter() if there are any messages in the message queue.
	float alterResult = NetworkController::realAlter(time);
	float defaultAlterTime = safe_cast<ServerObject *>(getOwner())->getDefaultAlterTime();
	if (defaultAlterTime == AlterResult::cms_alterQuickly)
		defaultAlterTime = Random::randomReal(0.25f, 0.45f);
	AlterResult::incorporateExactAlterResult(alterResult, defaultAlterTime);
	return alterResult;
}

// ----------------------------------------------------------------------

void ServerController::teleport(Transform const &goal, ServerObject *goalObject)
{
	setGoal(goal, goalObject, true);
}

// ----------------------------------------------------------------------

void ServerController::setGoal ( Transform const & newGoal, ServerObject * goalCellObject, bool teleport )
{
	m_goalTransform = newGoal;
	m_goalCellObject = goalCellObject;

	m_bHasGoal = true;
	m_bAtGoal = false;
	m_bTeleport = teleport;

	// If setGoal isn't called while we're processing our message queue,
	// set the creature to the goal position immediately instead of
	// waiting until after all messages have been handled.

	if(!m_bProcessingMessages)
	{
		moveToGoal();
	}
}

// ----------------------------------------------------------------------

bool changeCells(ServerObject &object, ServerObject *newCellObject)
{
	bool result = false;

	if (!object.isInWorld())
	{
		if (!newCellObject)
		{
			Container::ContainerErrorCode tmp = Container::CEC_Success;
			result = ContainerInterface::transferItemToWorld(
				object,
				ContainerInterface::getTopmostContainer(object)->getTransform_o2p(),
				0,
				tmp);
		}
		else
		{
			Container::ContainerErrorCode tmp = Container::CEC_Success;
			result = ContainerInterface::transferItemToCell(
				*newCellObject,
				object,
				Transform::identity,
				0,
				tmp);
		}

		return result;
	}

  Object * const oldCellObject = object.getAttachedTo();

	if (oldCellObject == newCellObject)
		return true;

	// ----------

	if (newCellObject == nullptr)
	{
		// Object was in a cell and is moving to the world, transfer from cell container to world

		Transform const objectTransform = object.getTransform_o2p();
		Transform const &newTransform = oldCellObject->getTransform_o2w().rotateTranslate_l2p(objectTransform);

		Container::ContainerErrorCode tmp = Container::CEC_Success;
		result = ContainerInterface::transferItemToWorld(object, newTransform, nullptr, tmp);

		if (!result)
		{
			DEBUG_REPORT_LOG(true, ("Object %s (id=%s) transfer to world was denied via ContainerInterface::transferItemToWorld()\n", object.getObjectTemplateName(), object.getNetworkId().getValueString().c_str()));

			// The failed transfer may have mucked up the transform, so restore it now.
			object.setTransform_o2p(objectTransform);
		}
	}
	else
	{
		// Object is moving to a different cell

		CellProperty * const pCell = ContainerInterface::getCell(*newCellObject);

		DEBUG_REPORT_LOG(pCell == nullptr, ("changeCells - Received transform with parent to a non-cell object.  This should only happen from the DB\n"));

		if (pCell)
		{
			Transform objectTransform = object.getTransform_o2p();
			Container::ContainerErrorCode tmp = Container::CEC_Success;
			result = ContainerInterface::transferItemToCell(*newCellObject, object, nullptr, tmp);

			if (!result)
			{
				DEBUG_REPORT_LOG(true, ("Object %s (id=%s) transfer to cell (id=%s) was denied via ContainerInterface::transferItemToCell()\n", object.getObjectTemplateName(), object.getNetworkId().getValueString().c_str(), newCellObject->getNetworkId().getValueString().c_str()));

				// The failed transfer may have mucked up the transform, so restore it now.
				object.setTransform_o2p(objectTransform);
			}
		}
	}

	return result;
}

// ----------------------------------------------------------------------

bool ServerController::setObjectCell(ServerObject *pCellObject)
{
	ServerObject * const object = NON_NULL(static_cast<ServerObject *>(getOwner()));

	return changeCells(*object, pCellObject);
}

// ----------------------------------------------------------------------

void ServerController::moveToGoal( void )
{
	if((m_bAtGoal) || (!m_bHasGoal))
	{
		return;
	}

	//@todo - This really needs to try and move the object to the goal first, and then
	// if doing so doesn't put the object in the right cell, teleport the object to
	// the correct cell. Same sort of thing as RemoteCreatureController.
	ServerObject *owner = static_cast<ServerObject *>(getOwner());
	if (m_goalCellObject)
	{
		if (!m_goalCellObject->isAuthoritative() && owner->isAuthoritative())
		{
			GenericValueTypeMessage<std::pair<NetworkId, NetworkId> > const rssMessage(
				"RequestSameServer",
				std::make_pair(
					ContainerInterface::getTopmostContainer(*owner)->getNetworkId(),
					ContainerInterface::getTopmostContainer(*m_goalCellObject)->getNetworkId()));
			GameServer::getInstance().sendToPlanetServer(rssMessage);
			return;
		}
	}

	if (!setObjectCell(m_goalCellObject))
	{
		m_bHasGoal = false;
		m_bAtGoal = true;
		m_bTeleport = false;
		onMoveFailed();
		return;
	}

	Transform oldTransform = owner->getTransform_o2p();

	if(oldTransform != m_goalTransform)
	{
		owner->setTransform_o2p(m_goalTransform);
	}

	if(m_bTeleport)
	{
		CollisionWorld::objectWarped(getOwner());
	}

	m_bHasGoal = false;
	m_bAtGoal = true;
	m_bTeleport = false;
}

//-----------------------------------------------------------------------

void ServerController::handleNetUpdateTransform(const MessageQueueDataTransform& message)
{
	if (!getServerOwner()->isInitialized())
	{
		getServerOwner()->setTransform_o2p(message.getTransform());
		return;
	}

	setGoal( message.getTransform(), nullptr );
}

//-----------------------------------------------------------------------

void ServerController::handleNetUpdateTransformWithParent(const MessageQueueDataTransformWithParent& message)
{
	if (!getServerOwner()->isInitialized())
	{
		getServerOwner()->setTransform_o2p(message.getTransform());
		return;
	}

	const NetworkId &cellNetworkId = message.getParent();

	Object *cellObject = NetworkIdManager::getObjectById(cellNetworkId);
	ServerObject * serverCellObject = safe_cast<ServerObject*>(cellObject);

	// ----------
	//@todo remove after we have depersisting to interiors
	//We don't want to depersist into non existant cell objects

	if(!cellObject)
	{
		WARNING_STRICT_FATAL(true, ("WARNING: db or authoritative player says to depersist %s into a cell %s but we cannot because cell does not exist.", getOwner()->getNetworkId().getValueString().c_str(), cellNetworkId.getValueString().c_str()));

		//@todo big demo hack.  If depersisting into a cell, set the transform to start lcoation
		Transform start = Transform::identity;
		start.setPosition_p(ConfigServerGame::getStartingPosition());
		setGoal( start, nullptr );

		return;
	}
	setGoal( message.getTransform(), serverCellObject );
}

//-----------------------------------------------------------------------

void ServerController::handleMessage (const int message, const float value, const MessageQueue::Data* const data, const uint32 flags)
{
	ServerObject* const owner = static_cast<ServerObject*>(getOwner());

	switch(message)
    {
	case CM_spatialChatSend:
		{
			const MessageQueueSpatialChat * const spatialChat = dynamic_cast<const MessageQueueSpatialChat *>(data);
			NOT_NULL (spatialChat);

			if (spatialChat->getSourceId () != getOwner ()->getNetworkId ())
			{
				WARNING (true, ("Received a CM_spatialChatSend message on object %s from object %s",
					owner->getNetworkId ().getValueString ().c_str (), spatialChat->getSourceId ().getValueString ().c_str ()));
				break;
			}

			owner->speakText (*spatialChat);
		}
		break;

	case CM_spatialChatReceive:
		{
			const MessageQueueSpatialChat * const spatialChat = dynamic_cast<const MessageQueueSpatialChat *>(data);
			NOT_NULL (spatialChat);
			ServerObject * const sourceObject = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(spatialChat->getSourceId()));
			if (sourceObject)
				owner->hearText(*sourceObject, *spatialChat, ChatLogManager::getNextMessageIndex());
		}
		break;

	case CM_socialSend:
		{
			const MessageQueueSocial * const socialMsg = NON_NULL (dynamic_cast<const MessageQueueSocial *>(data));

			if (socialMsg->getSourceId () != owner->getNetworkId ())
			{
				WARNING (true, ("Received a CM_spatialChatSend message on object %s from object %s",
					owner->getNetworkId ().getValueString ().c_str (), socialMsg->getSourceId ().getValueString ().c_str ()));
				break;
			}

			owner->performSocial (*socialMsg);
		}
		break;

	case CM_socialReceive:
		{
			const MessageQueueSocial * const socialMsg = NON_NULL (dynamic_cast<const MessageQueueSocial *>(data));
			owner->seeSocial (*socialMsg);
		}
		break;

	case CM_scriptTrigger:
		{
			const MessageQueueScriptTrigger * const triggerMessage = dynamic_cast<const MessageQueueScriptTrigger *>(data);
			if(triggerMessage)
			{
				ServerObject * object = getServerOwner();
				if(object)
				{
					GameScriptObject * scriptObject = object->getScriptObject();
					if(scriptObject)
					{
						const Archive::ByteStream & paramData = triggerMessage->getScriptParamData();
						const Scripting::TrigId trigId = static_cast<const Scripting::TrigId>(triggerMessage->getTriggerId());
						ScriptParams p;
						Archive::ReadIterator ri = paramData.begin();
						Archive::get(ri, p);
						scriptObject->trigAllScripts(trigId, p);
					}
				}

			}
		}
		break;
	case CM_attachScript:
		{
			if(data)
			{
				const MessageQueueString * const msg = static_cast<const MessageQueueString *>(data);
				owner->getScriptObject()->attachScript(msg->getString(), true);
			}
		}
		break;
	case CM_detachScript:
		{
			if(data)
			{
				const MessageQueueString * const msg = static_cast<const MessageQueueString *>(data);
				owner->getScriptObject()->detachScript(msg->getString());
			}
		}
		break;

	case CM_scriptTransferMoney:
	{
		const MessageQueueScriptTransferMoney * const msg = safe_cast<const MessageQueueScriptTransferMoney *>(data);
		if(msg)
		{
			ServerObject * owner = safe_cast<ServerObject *>(getOwner());
			if(owner)
				switch (msg->getTypeId())
				{
					case MessageQueueScriptTransferMoney::TT_bankTransfer:
						owner->scriptTransferBankCreditsTo(msg->getTarget(), msg->getAmount(), msg->getReplyTo(),
														   msg->getSuccessCallback(), msg->getFailCallback(), msg->getPackedDictionary());
						break;
					case MessageQueueScriptTransferMoney::TT_cashTransfer:
						owner->scriptTransferCashTo(msg->getTarget(), msg->getAmount(), msg->getReplyTo(),
													msg->getSuccessCallback(), msg->getFailCallback(), msg->getPackedDictionary());
						break;
					case MessageQueueScriptTransferMoney::TT_bankWithdrawal:
						owner->scriptWithdrawCashFromBank(msg->getAmount(), msg->getReplyTo(),
														  msg->getSuccessCallback(), msg->getFailCallback(), msg->getPackedDictionary());
						break;
					case MessageQueueScriptTransferMoney::TT_bankDeposit:
						owner->scriptDepositCashToBank(msg->getAmount(), msg->getReplyTo(),
													   msg->getSuccessCallback(), msg->getFailCallback(), msg->getPackedDictionary());
						break;
					case MessageQueueScriptTransferMoney::TT_bankTransferToNamedAccount:
						owner->scriptTransferBankCreditsTo(msg->getNamedAccount(), msg->getAmount(), msg->getReplyTo(),
														   msg->getSuccessCallback(), msg->getFailCallback(), msg->getPackedDictionary());
						break;
					case MessageQueueScriptTransferMoney::TT_bankTransferFromNamedAccount:
						owner->scriptTransferBankCreditsFrom(msg->getNamedAccount(), msg->getAmount(), msg->getReplyTo(),
															 msg->getSuccessCallback(), msg->getFailCallback(), msg->getPackedDictionary());
						break;
					default:
						DEBUG_FATAL(true,("MessageQueueScriptTransferMoney transaction type %i was not recognized.  Object is %s",
										  msg->getTypeId(), owner->getNetworkId().getValueString().c_str()));
				}
		}
	}
	case CM_persistObject:
		{
			owner->persist();
		}
		break;
	case CM_setCacheVersion:
		{
			const MessageQueueGenericValueType<int> * const msg = safe_cast<const MessageQueueGenericValueType<int> *>(data);
			if(msg)
			{
				owner->setCacheVersion(msg->getValue());
			}
		}
		break;
	case CM_transferAuthority:
		{
			const MessageQueueGenericValueType<std::pair<uint32, bool> > * const msg = safe_cast<const MessageQueueGenericValueType<std::pair<uint32, bool> > *>(data);
			if(msg)
			{
				owner->transferAuthority(msg->getValue().first, msg->getValue().second, false, false);
			}
		}
		break;
	case CM_setObjectName:
		{
			const MessageQueueGenericValueType<Unicode::String> * const msg = safe_cast<const MessageQueueGenericValueType<Unicode::String> *>(data);
			if(msg)
			{
				owner->setObjectName(msg->getValue());
			}
		}
		break;
	case CM_setObjectNameStringId:
		{
			const MessageQueueGenericValueType<StringId> * const msg = safe_cast<const MessageQueueGenericValueType<StringId> *>(data);
			if(msg)
			{
				owner->setObjectNameStringId(msg->getValue());
			}
		}
		break;
	case CM_teleportObject:
		{
			const MessageQueueTeleportObject * const msg = safe_cast<const MessageQueueTeleportObject *>(data);
			if(msg)
			{
				owner->teleportObject(msg->getPosition_w(), msg->getTargetContainerId(), msg->getTargetCellName(), msg->getPosition_p(), msg->getScriptCallback());
			}
		}
		break;
	case CM_updatePositionOnPlanetServer:
		{
			const MessageQueueGenericValueType<bool> * const msg = safe_cast<const MessageQueueGenericValueType<bool> *>(data);
			if(msg)
			{
				owner->updatePositionOnPlanetServer(msg->getValue());
			}
		}
		break;
	case CM_setPlayerControlled:
		{
			const MessageQueueGenericValueType<bool> * const msg = safe_cast<const MessageQueueGenericValueType<bool> *>(data);
			if(msg)
			{
				owner->setPlayerControlled(msg->getValue());
			}
		}
		break;
	case CM_customize:
		{
			const MessageQueueGenericValueType<std::pair<std::string, int> > * const msg = safe_cast<const MessageQueueGenericValueType<std::pair<std::string, int> > *>(data);
			if(msg)
			{
				owner->customize(msg->getValue().first, msg->getValue().second);
			}
		}
		break;
	case CM_permanentlyDestroy:
		{
			const MessageQueueGenericValueType<int> * const msg = safe_cast<const MessageQueueGenericValueType<int> *>(data);
			if (msg)
			{
				owner->permanentlyDestroy(msg->getValue());
			}
		}
		break;
	case CM_setIntDynamicVariable:
		{
			const MessageQueueGenericValueType<std::pair<std::string, int> >* const msg = safe_cast<const MessageQueueGenericValueType<std::pair<std::string, int> > *>(data);
			if(msg)
			{
				owner->setObjVarItem(msg->getValue().first, msg->getValue().second);
			}
		}
		break;
	case CM_setIntArrayDynamicVariable:
		{
			const MessageQueueGenericValueType<std::pair<std::string, std::vector<int> > >* const msg = safe_cast<const MessageQueueGenericValueType<std::pair<std::string, std::vector<int> > > *>(data);
			if(msg)
			{
				owner->setObjVarItem(msg->getValue().first, msg->getValue().second);
			}
		}
		break;
	case CM_setFloatDynamicVariable:
		{
			const MessageQueueGenericValueType<std::pair<std::string, float> >* const msg = safe_cast<const MessageQueueGenericValueType<std::pair<std::string, float> > *>(data);
			if(msg)
			{
				owner->setObjVarItem(msg->getValue().first, msg->getValue().second);
			}
		}
		break;
	case CM_setFloatArrayDynamicVariable:
		{
			const MessageQueueGenericValueType<std::pair<std::string, std::vector<float> > >* const msg = safe_cast<const MessageQueueGenericValueType<std::pair<std::string, std::vector<float> > > *>(data);
			if(msg)
			{
				owner->setObjVarItem(msg->getValue().first, msg->getValue().second);
			}
		}
		break;
	case CM_setStringDynamicVariable:
		{
			const MessageQueueGenericValueType<std::pair<std::string, Unicode::String> >* const msg = safe_cast<const MessageQueueGenericValueType<std::pair<std::string, Unicode::String> > *>(data);
			if(msg)
			{
				owner->setObjVarItem(msg->getValue().first, msg->getValue().second);
			}
		}
		break;
	case CM_setStringArrayDynamicVariable:
		{
			const MessageQueueGenericValueType<std::pair<std::string, std::vector<Unicode::String> > >* const msg = safe_cast<const MessageQueueGenericValueType<std::pair<std::string, std::vector<Unicode::String> > > *>(data);
			if(msg)
			{
				owner->setObjVarItem(msg->getValue().first, msg->getValue().second);
			}
		}
		break;
	case CM_setObjIdDynamicVariable:
		{
			const MessageQueueGenericValueType<std::pair<std::string, NetworkId> >* const msg = safe_cast<const MessageQueueGenericValueType<std::pair<std::string, NetworkId> > *>(data);
			if(msg)
			{
				owner->setObjVarItem(msg->getValue().first, msg->getValue().second);
			}
		}
		break;
	case CM_setObjIdArrayDynamicVariable:
		{
			const MessageQueueGenericValueType<std::pair<std::string, std::vector<NetworkId> > >* const msg = safe_cast<const MessageQueueGenericValueType<std::pair<std::string, std::vector<NetworkId> > > *>(data);
			if(msg)
			{
				owner->setObjVarItem(msg->getValue().first, msg->getValue().second);
			}
		}
		break;
	case CM_setLocationDynamicVariable:
		{
			const MessageQueueGenericValueType<std::pair<std::string, DynamicVariableLocationData> >* const msg = safe_cast<const MessageQueueGenericValueType<std::pair<std::string, DynamicVariableLocationData> > *>(data);
			if(msg)
			{
				owner->setObjVarItem(msg->getValue().first, msg->getValue().second);
			}
		}
		break;
	case CM_setLocationArrayDynamicVariable:
		{
			const MessageQueueGenericValueType<std::pair<std::string, std::vector<DynamicVariableLocationData> > >* const msg = safe_cast<const MessageQueueGenericValueType<std::pair<std::string, std::vector<DynamicVariableLocationData> > > *>(data);
			if(msg)
			{
				owner->setObjVarItem(msg->getValue().first, msg->getValue().second);
			}
		}
		break;
	case CM_setStringIdDynamicVariable:
		{
			const MessageQueueGenericValueType<std::pair<std::string, StringId> >* const msg = safe_cast<const MessageQueueGenericValueType<std::pair<std::string, StringId> > *>(data);
			if(msg)
			{
				owner->setObjVarItem(msg->getValue().first, msg->getValue().second);
			}
		}
		break;
	case CM_setStringIdArrayDynamicVariable:
		{
			const MessageQueueGenericValueType<std::pair<std::string, std::vector<StringId> > >* const msg = safe_cast<const MessageQueueGenericValueType<std::pair<std::string, std::vector<StringId> > > *>(data);
			if(msg)
			{
				owner->setObjVarItem(msg->getValue().first, msg->getValue().second);
			}
		}
		break;
	case CM_setTransformDynamicVariable:
		{
			MessageQueueGenericValueType<std::pair<std::string, Transform> > const * msg = safe_cast<MessageQueueGenericValueType<std::pair<std::string, Transform> > const *>(data);
			if (msg)
				owner->setObjVarItem(msg->getValue().first, msg->getValue().second);
		}
		break;
	case CM_setTransformArrayDynamicVariable:
		{
			MessageQueueGenericValueType<std::pair<std::string, std::vector<Transform> > > const * msg = safe_cast<MessageQueueGenericValueType<std::pair<std::string, std::vector<Transform> > > const *>(data);
			if (msg)
				owner->setObjVarItem(msg->getValue().first, msg->getValue().second);
		}
		break;
	case CM_setVectorDynamicVariable:
		{
			MessageQueueGenericValueType<std::pair<std::string, Vector> > const * msg = safe_cast<MessageQueueGenericValueType<std::pair<std::string, Vector> > const *>(data);
			if (msg)
				owner->setObjVarItem(msg->getValue().first, msg->getValue().second);
		}
		break;
	case CM_setVectorArrayDynamicVariable:
		{
			MessageQueueGenericValueType<std::pair<std::string, std::vector<Vector> > > const * msg = safe_cast<MessageQueueGenericValueType<std::pair<std::string, std::vector<Vector> > > const *>(data);
			if (msg)
				owner->setObjVarItem(msg->getValue().first, msg->getValue().second);
		}
		break;
	case CM_copyDynamicVariable:
		{
			const MessageQueueGenericValueType<std::pair<std::string, NetworkId> > * const msg = safe_cast<const MessageQueueGenericValueType<std::pair<std::string, NetworkId> > *>(data);
			if(msg)
			{
				ServerObject* fromObject = dynamic_cast<ServerObject*>(NetworkIdManager::getObjectById(msg->getValue().second));
				if(fromObject)
				{
					owner->copyObjVars(msg->getValue().first, *fromObject, msg->getValue().first);
				}
			}
		}
		break;
	case CM_copyDynamicVariableToSubList:
		{
			const MessageQueueGenericValueType<std::pair<std::string, std::pair<std::string, NetworkId> > > * const msg = safe_cast<const MessageQueueGenericValueType<std::pair<std::string, std::pair<std::string, NetworkId> > > *>(data);
			if(msg)
			{
				ServerObject* fromObject = dynamic_cast<ServerObject*>(NetworkIdManager::getObjectById(msg->getValue().second.second));
				if(fromObject)
				{
					owner->copyObjVars(msg->getValue().first, *fromObject, msg->getValue().second.first);
				}
			}
		}
		break;
	case CM_addNestedObjVarList:
		{
			const MessageQueueGenericValueType<std::string>* const msg = safe_cast<const MessageQueueGenericValueType<std::string> *>(data);
			if(msg)
			{
				//owner->addNestedObjVarList(msg->getValue()); // This operation is now a no-op
			}
		}
		break;
	case CM_removeObjVarItem:
		{
			const MessageQueueGenericValueType<std::string>* const msg = safe_cast<const MessageQueueGenericValueType<std::string> *>(data);
			if(msg)
			{
				owner->removeObjVarItem(msg->getValue());
			}
		}
		break;
	case CM_eraseObjVarListEntries:
		{
			const MessageQueueGenericValueType<std::string>* const msg = safe_cast<const MessageQueueGenericValueType<std::string> *>(data);
			if(msg)
			{
				owner->eraseObjVarListEntries(msg->getValue());
			}
		}
		break;
	case CM_setObjVarItemVar:
		{
		}
		break;

	case CM_disconnect:
		owner->disconnect();
		break;

	case CM_updateObserversCount:
		{
			const MessageQueueGenericValueType<int>* const msg = safe_cast<const MessageQueueGenericValueType<int> *>(data);
			if(msg)
			{
				owner->updateObserversCount(msg->getValue());
			}
		}
		break;

	case CM_cancelMessageTo:
		{
			const MessageQueueGenericValueType<std::string>* const msg = safe_cast<const MessageQueueGenericValueType<std::string> *>(data);
			if(msg)
			{
				NetworkId const possibleNetworkId = NetworkId(msg->getValue());
				if (possibleNetworkId.getValueString() == msg->getValue())
					owner->cancelMessageToByMessageId(possibleNetworkId);
				else
					owner->cancelMessageTo(msg->getValue());
			}
		}
		break;

	case CM_combatSpam:
		{
			const MessageQueueCombatSpam * const msg = safe_cast<const MessageQueueCombatSpam *>(data);
			if (msg)
			{
				owner->seeCombatSpam(*msg);
			}
		}
		break;

	default:
		NetworkController::handleMessage(message, value, data, flags);
		break;
    }
}

//-----------------------------------------------------------------------

void ServerController::conclude()
{
	PROFILER_AUTO_BLOCK_DEFINE("ServerController::conclude");

	{
		PROFILER_AUTO_BLOCK_DEFINE("updateTransform");
		updateTransform();
	}

	ServerObject * const netObject = NON_NULL(getOwner())->asServerObject();
	netObject->sendDeltas();

	if (m_alteredSinceConclude)
	{
		PROFILER_AUTO_BLOCK_DEFINE("NetworkController::conclude");
		NetworkController::conclude();
		m_alteredSinceConclude = false;
	}
}

// ----------------------------------------------------------------------

void ServerController::onClientReady()
{
}

// ----------------------------------------------------------------------

void ServerController::onClientLost()
{
}

//-----------------------------------------------------------------------

void ServerController::onAppendMessage()
{
	ServerObject *owner = static_cast<ServerObject *>(getOwner());
	if(owner)
		owner->addObjectToConcludeList();
}

// ----------------------------------------------------------------------

/**
 * Send a controller message to wherever it is supposed to go.
 *
 * @param msg		the message to send
 */
void ServerController::sendControllerMessage(ObjControllerMessage const &msg)
{
	uint32 const destFlags = (msg.getFlags() & GameControllerMessageFlags::DESTINATIONS);
	bool const reliable = ((msg.getFlags() & GameControllerMessageFlags::RELIABLE) > 0);

	ServerObject * const owner = static_cast<ServerObject *>(getOwner());

	// Note: in the case where we are sending to both auth and proxy clients,
	// sendToAuthClients is NOT set, because the sendToProxyClients handling
	// also covers the auth client, and can do so potentially with fewer messages.

	bool const sendToAuthClients = (   owner->isAuthoritative()
	                                && (destFlags & GameControllerMessageFlags::DEST_AUTH_CLIENT)
	                                && !(destFlags & GameControllerMessageFlags::DEST_PROXY_CLIENT));
	bool const sendToProxyClients = (destFlags & GameControllerMessageFlags::DEST_PROXY_CLIENT);
	bool const sendToAuthServer = (   !owner->isAuthoritative()
	                               && (   (destFlags & GameControllerMessageFlags::DEST_AUTH_SERVER)
	                                   || (   (destFlags & GameControllerMessageFlags::DEST_AUTH_CLIENT)
	                                       && !(destFlags & GameControllerMessageFlags::DEST_PROXY_CLIENT))));
	bool const sendToProxyServers = (   owner->isAuthoritative()
	                                 && (destFlags & (GameControllerMessageFlags::DEST_PROXY_CLIENT|GameControllerMessageFlags::DEST_PROXY_SERVER)));

	if (sendToAuthClients)
	{
		std::set<Client const *> authClients;
		owner->getAuthClients(authClients);

		// need to do special handling for CM_combatSpam controller
		// message to determine if the client should get the message
		bool isCombatSpamMessage = false;
		NetworkId combatSpamAttacker, combatSpamDefender;
		Vector combatSpamAttackerPosition_w, combatSpamDefenderPosition_w;
		if (!authClients.empty())
		{
			if (msg.getMessage() == CM_combatSpam)
			{
				MessageQueueCombatSpam const * mqcs = static_cast<MessageQueueCombatSpam const *>(msg.getData());

				if (mqcs)
				{
					isCombatSpamMessage = true;
					combatSpamAttacker = mqcs->m_attacker;
					combatSpamAttackerPosition_w = mqcs->m_attackerPosition_w;
					combatSpamDefender = mqcs->m_defender;
					combatSpamDefenderPosition_w = mqcs->m_defenderPosition_w;
				}
			}
		}

		for (std::set<Client const *>::const_iterator i = authClients.begin(); i != authClients.end(); ++i)
		{
			if (isCombatSpamMessage && !(*i)->shouldReceiveCombatSpam(combatSpamAttacker, combatSpamAttackerPosition_w, combatSpamDefender, combatSpamDefenderPosition_w))
				continue;

			(*i)->send(msg, reliable);
		}
	}

	if (sendToProxyClients)
	{
		// NOTE: This will not behave correctly if a DEST_PROXY_CLIENT message is sent from a non-authoritative object!
		// Currently we do not need this behavior, but if we do at some point, this will need to be changed.
		bool const sendToAuthoritativeClient = (destFlags & GameControllerMessageFlags::DEST_AUTH_CLIENT) != 0;
		owner->sendToClientsInUpdateRange(msg, reliable, sendToAuthoritativeClient);
	}

	if (sendToAuthServer || sendToProxyServers)
	{
		std::vector<uint32> serverList;
		if (sendToAuthServer)
			serverList.push_back(owner->getAuthServerProcessId());
		if (sendToProxyServers)
		{
			ProxyList const &proxyList = owner->getExposedProxyList();
			for (ProxyList::const_iterator i = proxyList.begin(); i != proxyList.end(); ++i)
				serverList.push_back(*i);
		}
		if (!serverList.empty())
		{
			ServerMessageForwarding::begin(serverList);

			ServerMessageForwarding::send(msg);

			ServerMessageForwarding::end();
		}
	}
}

// ----------------------------------------------------------------------

/**
 * Sets an object's name.
 *
 * @param name		the new name
 */
void ServerController::setObjectName(const Unicode::String &name)
{
	ServerObject* owner = static_cast<ServerObject *>(getOwner());

	if (!owner->isAuthoritative())
	{
		// @todo: what to do here
		DEBUG_WARNING(true, ("setObjectName on non-authoritative object!\n"));
		return;
	}

	owner->setObjectName(name);
}

//-----------------------------------------------------------------------

void ServerController::updateTransform()
{
	// Note: only the auth server sends updates from here.  Non-creatures use DEST_PROXY_CLIENT controller messages
	// to get the messages to all clients, and CreatureControllers receiving transforms from other servers forward
	// to local clients, since creatures don't use controller messages to the clients.
	ServerObject * const owner = static_cast<ServerObject *>(getOwner());

	if (!owner->getTransformChanged() && m_oldVelocity == getCurrentVelocity())
		return;

	DEBUG_FATAL(owner->getCellProperty() && !owner->getKill(), ("A cell object is trying to update its transform! Cells aren't EVER supposed to do that!"));

	if (owner->getClient())
		owner->getClient()->resetIdleTime();

	owner->setTransformChanged(false);
	m_oldVelocity = getCurrentVelocity();

	if (!owner->isAuthoritative())
		return;

	// bail out if we're not in the world --- no reason to send updates.
	if (!owner->isInWorld())
		return;

	synchronizeTransform();
}

//-----------------------------------------------------------------------

void ServerController::synchronizeTransform()
{
	ServerObject * const owner = safe_cast<ServerObject *>(getOwner());
	NOT_NULL(owner);

	m_moveSequenceNumber = owner->getAndIncrementMoveSequenceNumber();

	Transform const & tr = getOwner()->getTransform_o2p();

	bool isCreature = owner->getObjectTemplate()->getId() == ServerCreatureObjectTemplate::ServerCreatureObjectTemplate_tag;
	ServerObject *attachedTo = safe_cast<ServerObject*>(owner->getAttachedTo());

	int flags = GameControllerMessageFlags::SEND | GameControllerMessageFlags::DEST_PROXY_SERVER;

	if (m_sendReliableTransformThisFrame || owner->alwaysSendReliableTransform())
	{
		flags |= GameControllerMessageFlags::RELIABLE;
		m_sendReliableTransformThisFrame = false;
	}

	// If we're contained by something other than what we're attached to, then transforms are
	// not relevant to clients since they will be pulling their positions purely from their parent.
	bool sendPositionUpdateToClients = ContainerInterface::getContainedByObject(*owner) == attachedTo;

	// We only send the controller message to clients if the position update is useful to clients
	// and it is also not a creature, as creatures have it sent to clients in a different message.
	if (sendPositionUpdateToClients && !isCreature)
		flags |= GameControllerMessageFlags::DEST_PROXY_CLIENT;

	float lookAtYaw = 0.f;
	bool useLookAtYaw = false;
	if(isCreature)
	{
		CreatureObject *creatureObject = safe_cast<CreatureObject *>(owner);
		lookAtYaw = creatureObject->getLookAtYaw();
		useLookAtYaw = creatureObject->getUseLookAtYaw();		
	}
	if (attachedTo)
	{
		MessageQueueDataTransformWithParent message(0, m_moveSequenceNumber, attachedTo->getNetworkId(), tr, getCurrentVelocity ().magnitude (), lookAtYaw, useLookAtYaw);

		ObjControllerMessage c(owner->getNetworkId(),
							   CM_netUpdateTransformWithParent,
							   0.0f,
							   flags,
							   &message);

		sendControllerMessage(c);
		if (sendPositionUpdateToClients && isCreature)
		{
			//Special smaller packet for updating creature movement.
			UpdateTransformWithParentMessage utm(owner->getNetworkId(), m_moveSequenceNumber, attachedTo->getNetworkId(), tr, static_cast<int8>(getCurrentVelocity().magnitude()), lookAtYaw, useLookAtYaw);
			owner->sendToClientsInUpdateRange(utm, flags & GameControllerMessageFlags::RELIABLE, false);
		}
	}
	else
	{
		MessageQueueDataTransform message(0, m_moveSequenceNumber, tr, getCurrentVelocity ().magnitude (), lookAtYaw, useLookAtYaw);

		ObjControllerMessage c(owner->getNetworkId(),
						   CM_netUpdateTransform,
						   0.0f,
						   flags,
						   &message);

		sendControllerMessage(c);
		if (sendPositionUpdateToClients && isCreature)
		{
			//Special smaller packet for updating creature movement.
			UpdateTransformMessage utm(owner->getNetworkId(), m_moveSequenceNumber, tr, static_cast<int8>(getCurrentVelocity().magnitude()), lookAtYaw, useLookAtYaw);
			owner->sendToClientsInUpdateRange(utm, flags & GameControllerMessageFlags::RELIABLE, false);
		}
	}
}

// ----------------------------------------------------------------------

int ServerController::getMoveSequenceNumber() const
{
	return m_moveSequenceNumber;
}

// ----------------------------------------------------------------------

void ServerController::onMoveFailed()
{
}

//-----------------------------------------------------------------------
