// ======================================================================
//
// SetupSharedNetworkMessages.cpp
// copyright 1998 Bootprint Entertainment
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/SetupSharedNetworkMessages.h"

#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedNetworkMessages/BaselinesMessage.h"
#include "sharedNetworkMessages/DeltasMessage.h"
#include "sharedNetworkMessages/SceneChannelMessages.h"
#include "sharedMathArchive/TransformArchive.h"
#include "sharedNetworkMessages/ObjectChannelMessages.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"
#include "sharedUtility/ValueDictionary.h"
#include "sharedUtility/ValueDictionaryArchive.h"

#include "sharedNetworkMessages/BuffBuilderChangeMessage.h"
#include "sharedNetworkMessages/BuffBuilderStartMessage.h"
#include "sharedNetworkMessages/DroidCommandProgrammingMessage.h"
#include "sharedNetworkMessages/EnvironmentalHitData.h"
#include "sharedNetworkMessages/ImageDesignChangeMessage.h"
#include "sharedNetworkMessages/ImageDesignStartMessage.h"
#include "sharedNetworkMessages/IncubatorCommitMessage.h"
#include "sharedNetworkMessages/IncubatorStartMessage.h"
#include "sharedNetworkMessages/MessageQueueChangeRoleIconChoice.h"
#include "sharedNetworkMessages/MessageQueueCombatSpam.h"
#include "sharedNetworkMessages/MessageQueueCommandQueueEnqueue.h"
#include "sharedNetworkMessages/MessageQueueCommandQueueRemove.h"
#include "sharedNetworkMessages/MessageQueueCreateSaga.h"
#include "sharedNetworkMessages/MessageQueueCraftCustomization.h"
#include "sharedNetworkMessages/MessageQueueCraftEmptySlot.h"
#include "sharedNetworkMessages/MessageQueueCraftExperiment.h"
#include "sharedNetworkMessages/MessageQueueCraftFillSlot.h"
#include "sharedNetworkMessages/MessageQueueCraftIngredients.h"
#include "sharedNetworkMessages/MessageQueueCraftRequestSession.h"
#include "sharedNetworkMessages/MessageQueueCraftSelectSchematic.h"
#include "sharedNetworkMessages/MessageQueueCyberneticsChangeRequest.h"
#include "sharedNetworkMessages/MessageQueueCyberneticsOpen.h"
#include "sharedNetworkMessages/MessageQueueDataTransform.h"
#include "sharedNetworkMessages/MessageQueueDataTransformWithParent.h"
#include "sharedNetworkMessages/MessageQueueDraftSchematics.h"
#include "sharedNetworkMessages/MessageQueueDraftSlots.h"
#include "sharedNetworkMessages/MessageQueueDraftSlotsQueryResponse.h"
#include "sharedNetworkMessages/MessageQueueGeneric.h"
#include "sharedNetworkMessages/MessageQueueGenericIntResponse.h"
#include "sharedNetworkMessages/MessageQueueGenericResponse.h"
#include "sharedNetworkMessages/MessageQueueGenericString.h"
#include "sharedNetworkMessages/MessageQueueHarvesterResourceData.h"
#include "sharedNetworkMessages/MessageQueueMissionCreateRequest.h"
#include "sharedNetworkMessages/MessageQueueMissionCreateRequestData.h"
#include "sharedNetworkMessages/MessageQueueMissionDetailsRequest.h"
#include "sharedNetworkMessages/MessageQueueMissionDetailsResponse.h"
#include "sharedNetworkMessages/MessageQueueMissionDetailsResponseData.h"
#include "sharedNetworkMessages/MessageQueueMissionGenericRequest.h"
#include "sharedNetworkMessages/MessageQueueMissionGenericResponse.h"
#include "sharedNetworkMessages/MessageQueueMissionListRequest.h"
#include "sharedNetworkMessages/MessageQueueMissionListResponse.h"
#include "sharedNetworkMessages/MessageQueueNetworkId.h"
#include "sharedNetworkMessages/MessageQueueNetworkIdAndTransform.h"
#include "sharedNetworkMessages/MessageQueueNetworkIdPair.h"
#include "sharedNetworkMessages/MessageQueueNpcConversationMessage.h"
#include "sharedNetworkMessages/MessageQueueObjectMenuRequest.h"
#include "sharedNetworkMessages/MessageQueueOpponentInfo.h"
#include "sharedNetworkMessages/MessageQueuePosture.h"
#include "sharedNetworkMessages/MessageQueuePushCreature.h"
#include "sharedNetworkMessages/MessageQueueQuestTaskCounterMessage.h"
#include "sharedNetworkMessages/MessageQueueQuestTaskLocationMessage.h"
#include "sharedNetworkMessages/MessageQueueQuestTaskTimerMessage.h"
#include "sharedNetworkMessages/MessageQueueResourceEmptyHopper.h"
#include "sharedNetworkMessages/MessageQueueResourceWeights.h"
#include "sharedNetworkMessages/MessageQueueSecureTrade.h"
#include "sharedNetworkMessages/MessageQueueSelectCurrentWorkingSkill.h"
#include "sharedNetworkMessages/MessageQueueSelectProfessionTemplate.h"
#include "sharedNetworkMessages/MessageQueueShipHit.h"
#include "sharedNetworkMessages/MessageQueueShowCombatText.h"
#include "sharedNetworkMessages/MessageQueueShowFlyText.h"
#include "sharedNetworkMessages/MessageQueueSitOnObject.h"
#include "sharedNetworkMessages/MessageQueueSlowDownEffect.h"
#include "sharedNetworkMessages/MessageQueueSocial.h"
#include "sharedNetworkMessages/MessageQueueSpaceMiningSellResource.h"
#include "sharedNetworkMessages/MessageQueueSpatialChat.h"
#include "sharedNetworkMessages/MessageQueueStartNpcConversation.h"
#include "sharedNetworkMessages/MessageQueueStopNpcConversation.h"
#include "sharedNetworkMessages/MessageQueueString.h"
#include "sharedNetworkMessages/MessageQueueStringList.h"
#include "sharedNetworkMessages/MessageQueueTeleportAck.h"
#include "sharedNetworkMessages/MessageQueueUpdateShipOnCollision.h"
#include "sharedNetworkMessages/NebulaLightningHitData.h"
#include "sharedNetworkMessages/ShipDamageMessage.h"
#include "sharedNetworkMessages/ShipDamageMessageArchive.h"
#include "sharedNetworkMessages/UpdateTransformMessage.h"
#include "sharedNetworkMessages/UpdateTransformWithParentMessage.h"

// ----------------------------------------------------------------------
namespace SetupSharedNetworkMessagesNamespace
{
	static bool g_installed = false;

	void packGenericUint32Message(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		MessageQueueGenericValueType<uint32> const * const msg = safe_cast<MessageQueueGenericValueType<uint32> const *>(data);
		if (msg)
			Archive::put(target, msg->getValue());
	}

	MessageQueue::Data* unpackGenericUint32Message(Archive::ReadIterator & source)
	{
		uint32 v;
		Archive::get(source, v);
		MessageQueueGenericValueType<uint32> * const result = new MessageQueueGenericValueType<uint32>(v);
		return result;
	}

	void packGenericUint32FloatMessage(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		MessageQueueGenericValueType<std::pair<uint32, float> > const * const msg = safe_cast<MessageQueueGenericValueType<std::pair<uint32, float> > const *>(data);
		if (msg)
			Archive::put(target, msg->getValue());
	}
	
	MessageQueue::Data* unpackGenericUint32FloatMessage(Archive::ReadIterator & source)
	{
		std::pair<uint32, float> v;
		Archive::get(source, v);
		return new MessageQueueGenericValueType<std::pair<uint32, float> >(v);
	}

	void packNothing(MessageQueue::Data const *, Archive::ByteStream &)
	{
	}

	MessageQueue::Data* unpackNothing(Archive::ReadIterator &)
	{
		return nullptr;
	}

	void packGenericShipDamageMessage(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		if (data)
		{
			const MessageQueueGenericValueType<ShipDamageMessage> * const shipDamage = safe_cast<const MessageQueueGenericValueType<ShipDamageMessage> *>(data);
			Archive::put(target, shipDamage->getValue());
		}
	}
	
	MessageQueue::Data* unpackGenericShipDamageMessage(Archive::ReadIterator & source)
	{
		ShipDamageMessage shipDamage;
		Archive::get(source, shipDamage);
		MessageQueueGenericValueType<ShipDamageMessage> * const result = new MessageQueueGenericValueType<ShipDamageMessage>(shipDamage);
		return result;
	}

	//----------------------------------------------------------------------

	void packGenericNebulaLightningHitData(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		if (data)
		{
			MessageQueueGenericValueType<NebulaLightningHitData> const * const message = safe_cast<const MessageQueueGenericValueType<NebulaLightningHitData> *>(data);
			Archive::put(target, message->getValue());
		}
	}

	//----------------------------------------------------------------------

	MessageQueue::Data* unpackGenericNebulaLightningHitData(Archive::ReadIterator & source)
	{
		NebulaLightningHitData messageData;
		Archive::get(source, messageData);
		return new MessageQueueGenericValueType<NebulaLightningHitData>(messageData);
	}

	//----------------------------------------------------------------------

	void packGenericEnvironmentalHitData(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		if (data)
		{
			MessageQueueGenericValueType<EnvironmentalHitData> const * const message = safe_cast<const MessageQueueGenericValueType<EnvironmentalHitData> *>(data);
			Archive::put(target, message->getValue());
		}
	}

	//----------------------------------------------------------------------		
		
	MessageQueue::Data * unpackGenericEnvironmentalHitData(Archive::ReadIterator & source)
	{
		EnvironmentalHitData messageData;
		Archive::get(source, messageData);
		return new MessageQueueGenericValueType<EnvironmentalHitData>(messageData);
	}

	//----------------------------------------------------------------------

	void packGenericNetworkIdStringMessage(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		MessageQueueGenericValueType<std::pair<NetworkId, std::string> > const * const msg = safe_cast<MessageQueueGenericValueType<std::pair<NetworkId, std::string> > const *>(data);
		if (msg)
			Archive::put(target, msg->getValue());
	}
	
	//----------------------------------------------------------------------

	MessageQueue::Data* unpackGenericNetworkIdStringMessage(Archive::ReadIterator & source)
	{
		std::pair<NetworkId, std::string> v;
		Archive::get(source, v);
		return new MessageQueueGenericValueType<std::pair<NetworkId, std::string> >(v);
	}

	//----------------------------------------------------------------------

	void packGenericValueDictionaryMessage(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		MessageQueueGenericValueType<ValueDictionary> const * const msg = safe_cast<MessageQueueGenericValueType<ValueDictionary> const *>(data);
		if (msg)
			Archive::put(target, msg->getValue());
	}

	//----------------------------------------------------------------------

	MessageQueue::Data* unpackGenericValueDictionaryMessage(Archive::ReadIterator & source)
	{
		ValueDictionary v;
		Archive::get(source, v);
		return new MessageQueueGenericValueType<ValueDictionary>(v);
	}
	//----------------------------------------------------------------------

	void packInt(const MessageQueue::Data *data, Archive::ByteStream &target)
	{
		const MessageQueueGenericValueType<int> * msg = safe_cast<const MessageQueueGenericValueType<int> *>(data);
		if(msg)
		{
			Archive::put(target, msg->getValue());
		}
	}

	//----------------------------------------------------------------------

	MessageQueue::Data* unpackInt(Archive::ReadIterator &source)
	{
		int v;
		Archive::get(source, v);
		MessageQueueGenericValueType<int> * result = new MessageQueueGenericValueType<int>(v);
		return result;
	}

	//----------------------------------------------------------------------

	void packGenericStringStringMessage(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		MessageQueueGenericValueType<std::pair<std::string, std::string> > const * const msg = safe_cast<MessageQueueGenericValueType<std::pair<std::string, std::string> > const *>(data);
		if (msg)
			Archive::put(target, msg->getValue());
	}

	//----------------------------------------------------------------------

	MessageQueue::Data* unpackGenericStringStringMessage(Archive::ReadIterator & source)
	{
		std::pair<std::string, std::string> v;
		Archive::get(source, v);
		return new MessageQueueGenericValueType<std::pair<std::string, std::string> >(v);
	}

	//----------------------------------------------------------------------

	void packNetworkIdMessage(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		const MessageQueueGenericValueType<NetworkId> * const msg = dynamic_cast<const MessageQueueGenericValueType<NetworkId> *>(data);
		if(msg)
		{
			Archive::put(target, msg->getValue());
		}
	}

	MessageQueue::Data* unpackNetworkIdMessage(Archive::ReadIterator & source)
	{
		NetworkId v;
		Archive::get(source, v);
		MessageQueueGenericValueType<NetworkId> * result = new MessageQueueGenericValueType<NetworkId>(v);
		return result;
	}
}

using namespace SetupSharedNetworkMessagesNamespace;

// ----------------------------------------------------------------------

void SetupSharedNetworkMessages::install ()
{
	DEBUG_FATAL(g_installed, ("SetupSharedNetworkMessages::install - is already installed"));

	ControllerMessageFactory::install ();

	BaselinesMessage::install();
	DeltasMessage::install();
	DroidCommandProgrammingMessage::install();
	BuffBuilderChangeMessage::install();
	BuffBuilderStartMessage::install();
	ImageDesignChangeMessage::install();
	ImageDesignStartMessage::install();
	IncubatorCommitMessage::install();
	IncubatorStartMessage::install();
	ObjControllerMessage::install();
	SceneCreateObjectByName::install();
	SceneCreateObjectByCrc::install();
	SceneDestroyObject::install();
	SceneEndBaselines::install();
	UpdateTransformMessage::install();
	UpdateTransformWithParentMessage::install();

	MessageQueueSelectProfessionTemplate::install();
	MessageQueueSelectCurrentWorkingSkill::install();
	MessageQueueChangeRoleIconChoice::install();
	MessageQueueCombatSpam::install();
	MessageQueueCommandQueueEnqueue::install();
	MessageQueueCommandQueueRemove::install();
	MessageQueueCreateSaga::install();
	MessageQueueCraftCustomization::install();
	MessageQueueCraftEmptySlot::install();
	MessageQueueCraftExperiment::install();
	MessageQueueCraftFillSlot::install();
	MessageQueueCraftIngredients::install();
	MessageQueueCraftRequestSession::install();
	MessageQueueCraftSelectSchematic::install();
	MessageQueueCyberneticsChangeRequest::install();
	MessageQueueCyberneticsOpen::install();
	MessageQueueDataTransform::install();
	MessageQueueDataTransformWithParent::install();
	MessageQueueDraftSchematics::install();
	MessageQueueDraftSlots::install();
	MessageQueueDraftSlotsQueryResponse::install();
	MessageQueueGeneric::install();
	MessageQueueGenericIntResponse::install();
	MessageQueueGenericResponse::install();
	MessageQueueGenericString::install();
	MessageQueueHarvesterResourceData::install();
	MessageQueueMissionCreateRequest::install();
	MessageQueueMissionDetailsRequest::install();
	MessageQueueMissionDetailsResponse::install();
	MessageQueueMissionGenericRequest::install();
	MessageQueueMissionGenericResponse::install();
	MessageQueueMissionListRequest::install();
	MessageQueueMissionListResponse::install();
	MessageQueueNetworkId::install();
	MessageQueueNetworkIdAndTransform::install();
	MessageQueueNetworkIdPair::install();
	MessageQueueNpcConversationMessage::install();
	MessageQueueObjectMenuRequest::install();
	MessageQueueOpponentInfo::install();
	MessageQueuePosture::install();
	MessageQueuePushCreature::install();
	MessageQueueResourceEmptyHopper::install();
	MessageQueueResourceWeights::install();
	MessageQueueSecureTrade::install();
	MessageQueueShipHit::install();
	MessageQueueShowFlyText::install();
	MessageQueueShowCombatText::install();
	MessageQueueSitOnObject::install();
	MessageQueueSlowDownEffect::install();
	MessageQueueSocial::install();
	MessageQueueSpatialChat::install();
	MessageQueueStartNpcConversation::install();
	MessageQueueStopNpcConversation::install();
	MessageQueueString::install();
	MessageQueueStringList::install();
	MessageQueueTeleportAck::install();
	MessageQueueUpdateShipOnCollision::install();
	MessageQueueQuestTaskCounterMessage::install();
	MessageQueueQuestTaskLocationMessage::install();
	MessageQueueQuestTaskTimerMessage::install();
	MessageQueueSpaceMiningSellResource::install();

	ControllerMessageFactory::registerControllerMessageHandler(CM_modData, packGenericUint32FloatMessage, unpackGenericUint32FloatMessage);
	ControllerMessageFactory::registerControllerMessageHandler(CM_cancelMod, packGenericUint32Message, unpackGenericUint32Message);

	ControllerMessageFactory::registerControllerMessageHandler(CM_objectMenuDirty, packNothing, unpackNothing);
	ControllerMessageFactory::registerControllerMessageHandler(CM_attributesDirty, packNothing, unpackNothing);
	ControllerMessageFactory::registerControllerMessageHandler(CM_imageDesignerCancel, ImageDesignChangeMessage::pack, ImageDesignChangeMessage::unpack, true);

	ControllerMessageFactory::registerControllerMessageHandler(CM_buffBuilderCancel, BuffBuilderChangeMessage::pack, BuffBuilderChangeMessage::unpack, true);

	ControllerMessageFactory::registerControllerMessageHandler(CM_incubatorCancel, IncubatorCommitMessage::pack, IncubatorCommitMessage::unpack, true);

	ControllerMessageFactory::registerControllerMessageHandler(CM_shipDamageMessage, packGenericShipDamageMessage, unpackGenericShipDamageMessage);

	ControllerMessageFactory::registerControllerMessageHandler(CM_lightningHitShip, packGenericNebulaLightningHitData, unpackGenericNebulaLightningHitData);

	ControllerMessageFactory::registerControllerMessageHandler(CM_environmentHitShip, packGenericEnvironmentalHitData, unpackGenericEnvironmentalHitData);

	ControllerMessageFactory::registerControllerMessageHandler(CM_uiPlayEffect, packGenericNetworkIdStringMessage, unpackGenericNetworkIdStringMessage);

	ControllerMessageFactory::registerControllerMessageHandler(CM_removeSlowDownEffect, packNothing, unpackNothing);

	ControllerMessageFactory::registerControllerMessageHandler(CM_shipStopFiring, packInt, unpackInt, true);

	ControllerMessageFactory::registerControllerMessageHandler(CM_spaceMiningSaleOpen, packGenericNetworkIdStringMessage, unpackGenericNetworkIdStringMessage);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setCurrentQuest, packGenericUint32Message, unpackGenericUint32Message, true);

	ControllerMessageFactory::registerControllerMessageHandler(CM_jump, packNothing, unpackNothing);

	ControllerMessageFactory::registerControllerMessageHandler(CM_openCustomizationWindow, packGenericNetworkIdStringMessage, unpackGenericNetworkIdStringMessage);
	ControllerMessageFactory::registerControllerMessageHandler(CM_customizeFinished, packGenericNetworkIdStringMessage, unpackGenericNetworkIdStringMessage, true);
	
	ControllerMessageFactory::registerControllerMessageHandler(CM_clientMinigameOpen, packGenericValueDictionaryMessage, unpackGenericValueDictionaryMessage);
	ControllerMessageFactory::registerControllerMessageHandler(CM_clientMinigameClose, packGenericValueDictionaryMessage, unpackGenericValueDictionaryMessage);
	ControllerMessageFactory::registerControllerMessageHandler(CM_clientMinigameResult, packGenericValueDictionaryMessage, unpackGenericValueDictionaryMessage, true);

	ControllerMessageFactory::registerControllerMessageHandler(CM_openRatingWindow, packGenericStringStringMessage, unpackGenericStringStringMessage);
	ControllerMessageFactory::registerControllerMessageHandler(CM_ratingFinished, packInt, unpackInt, true);
	ControllerMessageFactory::registerControllerMessageHandler(CM_abandonPlayerQuest, packNetworkIdMessage, unpackNetworkIdMessage, true);
	ControllerMessageFactory::registerControllerMessageHandler(CM_openRecipe, packNetworkIdMessage, unpackNetworkIdMessage, true);

	g_installed = true;
	ExitChain::add (SetupSharedNetworkMessages::remove, "SetupSharedNetworkMessages");
}
// ----------------------------------------------------------------------

void SetupSharedNetworkMessages::remove ( void )
{
	DEBUG_FATAL(!g_installed, ("SetupSharedNetworkMessages::remove - not already installed"));
	BaselinesMessage::remove();
	ObjControllerMessage::remove();
	SceneCreateObjectByName::remove();
	SceneCreateObjectByCrc::remove();
	SceneDestroyObject::remove();
	SceneEndBaselines::remove();
	UpdateTransformMessage::remove();
	UpdateTransformWithParentMessage::remove();

	//-- @todo this should be moved to SetupSharedNetworkMessages
	ControllerMessageFactory::remove ();

	g_installed = false;
}

// ----------------------------------------------------------------------
