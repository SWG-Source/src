// ======================================================================
//
// SetupSwgSharedNetworkMessages.cpp
// copyright 1998 Bootprint Entertainment
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#include "swgSharedNetworkMessages/FirstSwgSharedNetworkMessages.h"
#include "swgSharedNetworkMessages/SetupSwgSharedNetworkMessages.h"

#include "StringId.h"
#include "localizationArchive/StringIdArchive.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedGame/MatchMakingCharacterResult.h"
#include "sharedGame/ProsePackage.h"
#include "sharedGame/ProsePackageArchive.h"
#include "sharedMath/Sphere.h"
#include "sharedMathArchive/SphereArchive.h"
#include "sharedMathArchive/VectorArchive.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"
#include "sharedNetworkMessages/MessageQueueCommandTimer.h"
#include "sharedNetworkMessages/MessageQueueGenericValueType.h"
#include "sharedUtility/StartingLocationData.h"
#include "sharedUtility/StartingLocationDataArchive.h"
#include "swgSharedNetworkMessages/MessageQueueCombatAction.h"
#include "swgSharedNetworkMessages/MessageQueueCombatDamage.h"
#include "swgSharedUtility/Behaviors.def"
#include "swgSharedUtility/Locomotions.def"
#include "swgSharedUtility/MentalStates.def"

// ----------------------------------------------------------------------

namespace SetupSwgSharedNetworkMessagesNamespace
{
	static bool g_installed = false;

	void packAttributeMessage(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		const MessageQueueGenericValueType<std::pair<Attributes::Enumerator, Attributes::Value> > * const msg = dynamic_cast<const MessageQueueGenericValueType<std::pair<Attributes::Enumerator, Attributes::Value> > *>(data);
		if(msg)
		{
			Archive::put(target, msg->getValue());
		}
	}

	MessageQueue::Data* unpackAttributeMessage(Archive::ReadIterator & source)
	{
		std::pair<Attributes::Enumerator, Attributes::Value>  v;
		Archive::get(source, v);
		MessageQueueGenericValueType<std::pair<Attributes::Enumerator, Attributes::Value> > * result = new MessageQueueGenericValueType<std::pair<Attributes::Enumerator, Attributes::Value> >(v);
		return result;
	}

	void packMentalStatesMessage(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		const MessageQueueGenericValueType<std::pair<MentalStates::Enumerator, MentalStates::Value> > * const msg = dynamic_cast<const MessageQueueGenericValueType<std::pair<MentalStates::Enumerator, MentalStates::Value> > *>(data);
		if(msg)
		{
			Archive::put(target, msg->getValue());
		}
	}

	MessageQueue::Data* unpackMentalStatesMessage(Archive::ReadIterator & source)
	{
		std::pair<MentalStates::Enumerator, MentalStates::Value>  v;
		Archive::get(source, v);
		MessageQueueGenericValueType<std::pair<MentalStates::Enumerator, MentalStates::Value> > * result = new MessageQueueGenericValueType<std::pair<MentalStates::Enumerator, MentalStates::Value> >(v);
		return result;
	}

	void packMentalStatesTowardsMessage(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		const MessageQueueGenericValueType<std::pair<NetworkId, std::pair<MentalStates::Enumerator, MentalStates::Value> > > * const msg = dynamic_cast<const MessageQueueGenericValueType<std::pair<NetworkId, std::pair<MentalStates::Enumerator, MentalStates::Value> > > *>(data);
		if(msg)
		{
			Archive::put(target, msg->getValue());
		}
	}
	MessageQueue::Data* unpackMentalStatesTowardsMessage(Archive::ReadIterator & source)
	{
		std::pair<NetworkId, std::pair<MentalStates::Enumerator, MentalStates::Value> >  v;
		Archive::get(source, v);
		MessageQueueGenericValueType<std::pair<NetworkId, std::pair<MentalStates::Enumerator, MentalStates::Value> > > * result = new MessageQueueGenericValueType<std::pair<NetworkId, std::pair<MentalStates::Enumerator, MentalStates::Value> > >(v);
		return result;
	}

	void packIntMessage(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		const MessageQueueGenericValueType<int> * const msg = dynamic_cast<const MessageQueueGenericValueType<int> *>(data);
		if(msg)
		{
			Archive::put(target, msg->getValue());
		}
	}

	MessageQueue::Data* unpackIntMessage(Archive::ReadIterator & source)
	{
		int v;
		Archive::get(source, v);
		MessageQueueGenericValueType<int> * result = new MessageQueueGenericValueType<int>(v);
		return result;
	}

	void packUint32PairMessage(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		const MessageQueueGenericValueType<std::pair<uint32, uint32> > * const msg = dynamic_cast<const MessageQueueGenericValueType<std::pair<uint32, uint32> > *>(data);
		if(msg)
		{
			Archive::put(target, msg->getValue());
		}
	}

	MessageQueue::Data* unpackUint32PairMessage(Archive::ReadIterator & source)
	{
		std::pair<uint32, uint32> v;
		Archive::get(source, v);
		MessageQueueGenericValueType<std::pair<uint32, uint32> > * result = new MessageQueueGenericValueType<std::pair<uint32, uint32> >(v);
		return result;
	}

	void packBoolMessage(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		const MessageQueueGenericValueType<bool> * const msg = dynamic_cast<const MessageQueueGenericValueType<bool> *>(data);
		if(msg)
		{
			Archive::put(target, msg->getValue());
		}
	}

	MessageQueue::Data* unpackBoolMessage(Archive::ReadIterator & source)
	{
		bool v;
		Archive::get(source, v);
		MessageQueueGenericValueType<bool> * result = new MessageQueueGenericValueType<bool>(v);
		return result;
	}

	void packMentalStateTowardClampBehaviorMessage(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		const MessageQueueGenericValueType<std::pair<NetworkId, std::pair<MentalStates::Enumerator, std::pair<MentalStates::Value, Behaviors::Enumerator> > > > * const msg = dynamic_cast<const MessageQueueGenericValueType<std::pair<NetworkId, std::pair<MentalStates::Enumerator, std::pair<MentalStates::Value, Behaviors::Enumerator> > > > *>(data);
		if(msg)
		{
			Archive::put(target, msg->getValue());
		}
	}

	MessageQueue::Data* unpackMentalStateTowardClampBehaviorMessage(Archive::ReadIterator & source)
	{
		std::pair<NetworkId, std::pair<MentalStates::Enumerator, std::pair<MentalStates::Value, Behaviors::Enumerator> > >  v;
		Archive::get(source, v);
		MessageQueueGenericValueType<std::pair<NetworkId, std::pair<MentalStates::Enumerator, std::pair<MentalStates::Value, Behaviors::Enumerator> > > > * result = new MessageQueueGenericValueType<std::pair<NetworkId, std::pair<MentalStates::Enumerator, std::pair<MentalStates::Value, Behaviors::Enumerator> > > >(v);
		return result;
	}


	void packMentalStateDecayMessage(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		const MessageQueueGenericValueType<std::pair<MentalStates::Enumerator, float> > * const msg = dynamic_cast<const MessageQueueGenericValueType<std::pair<MentalStates::Enumerator, float> > *>(data);
		if(msg)
		{
			Archive::put(target, msg->getValue());
		}
	}

	MessageQueue::Data* unpackMentalStateDecayMessage(Archive::ReadIterator & source)
	{
		std::pair<MentalStates::Enumerator, float>  v;
		Archive::get(source, v);
		MessageQueueGenericValueType<std::pair<MentalStates::Enumerator, float> > * result = new MessageQueueGenericValueType<std::pair<MentalStates::Enumerator, float> >(v);
		return result;
	}

	void packLocomotionMessage(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		const MessageQueueGenericValueType<Locomotions::Enumerator> * const msg = dynamic_cast<const MessageQueueGenericValueType<Locomotions::Enumerator> *>(data);
		if(msg)
		{
			Archive::put(target, msg->getValue());
		}
	}

	MessageQueue::Data* unpackLocomotionMessage(Archive::ReadIterator & source)
	{
		Locomotions::Enumerator v;
		Archive::get(source, v);
		MessageQueueGenericValueType<Locomotions::Enumerator> * result = new MessageQueueGenericValueType<Locomotions::Enumerator>(v);
		return result;
	}

	void packAddToPlayerSpawnQueueMessage(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		const MessageQueueGenericValueType<std::pair<NetworkId, unsigned long> > * const msg = dynamic_cast<const MessageQueueGenericValueType<std::pair<NetworkId, unsigned long> > *>(data);
		if(msg)
		{
			Archive::put(target, msg->getValue());
		}
	}

	MessageQueue::Data* unpackAddToPlayerSpawnQueueMessage(Archive::ReadIterator & source)
	{
		std::pair<NetworkId, unsigned long>  v;
		Archive::get(source, v);
		MessageQueueGenericValueType<std::pair<NetworkId, unsigned long> > * result = new MessageQueueGenericValueType<std::pair<NetworkId, unsigned long> >(v);
		return result;
	}

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

	void packGenericStringIdMessage(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		const MessageQueueGenericValueType<StringId> * const msg = safe_cast<const MessageQueueGenericValueType<StringId> *>(data);
		if (msg)
			Archive::put (target, msg->getValue ());
	}

	MessageQueue::Data* unpackGenericStringIdMessage(Archive::ReadIterator & source)
	{
		StringId v;
		Archive::get(source, v);
		MessageQueueGenericValueType<StringId> * const result = new MessageQueueGenericValueType<StringId>(v);
		return result;
	}

	void packGenericStringVectorPairMessage(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		const MessageQueueGenericValueType<std::pair<std::string, Vector> > * const msg = dynamic_cast<const MessageQueueGenericValueType<std::pair<std::string, Vector> > *>(data);
		if(msg)
		{
			Archive::put(target, msg->getValue());
		}
	}

	MessageQueue::Data* unpackGenericStringVectorPairMessage(Archive::ReadIterator & source)
	{
		std::pair<std::string, Vector> v;
		Archive::get(source, v);
		MessageQueueGenericValueType<std::pair<std::string, Vector> > * result = new MessageQueueGenericValueType<std::pair<std::string, Vector> >(v);
		return result;
	}

	void packGenericStringIdProsePackageMessage(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		typedef std::pair<std::pair<StringId, ProsePackage>, Unicode::String> Payload;
		const MessageQueueGenericValueType<Payload> * const msg = safe_cast<const MessageQueueGenericValueType<Payload> *>(data);
		if (msg)
			Archive::put (target, msg->getValue ());
	}

	MessageQueue::Data* unpackGenericStringIdProsePackageMessage(Archive::ReadIterator & source)
	{
		typedef std::pair<std::pair<StringId, ProsePackage>, Unicode::String> Payload;
		Payload v;
		Archive::get(source, v);
		MessageQueueGenericValueType<Payload> * const result = new MessageQueueGenericValueType<Payload>(v);
		return result;
	}

	void packNetworkIdUnicodeStringPairMessage(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		const MessageQueueGenericValueType<std::pair<NetworkId, Unicode::String> > * const msg = dynamic_cast<const MessageQueueGenericValueType<std::pair<NetworkId, Unicode::String> > *>(data);
		if(msg)
		{
			Archive::put(target, msg->getValue());
		}
	}

	MessageQueue::Data* unpackNetworkIdUnicodeStringPairMessage(Archive::ReadIterator & source)
	{
		std::pair<NetworkId, Unicode::String> v;
		Archive::get(source, v);
		MessageQueueGenericValueType<std::pair<NetworkId, Unicode::String> > * result = new MessageQueueGenericValueType<std::pair<NetworkId, Unicode::String> >(v);
		return result;
	}

	void packGenericVectorOfUnicodeStrings(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		const MessageQueueGenericValueType<std::vector<Unicode::String> > * const msg = dynamic_cast<const MessageQueueGenericValueType<std::vector<Unicode::String> > *>(data);
		if(msg)
		{
			Archive::put(target, msg->getValue());
		}
	}

	MessageQueue::Data* unpackGenericVectorOfUnicodeStrings(Archive::ReadIterator & source)
	{
		std::vector<Unicode::String> v;
		Archive::get(source, v);
		MessageQueueGenericValueType<std::vector<Unicode::String> > * const result = new MessageQueueGenericValueType<std::vector<Unicode::String> >(v);
		return result;
	}

	//----------------------------------------------------------------------

	void packCharacterMatchResultVectorMessage(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		const MessageQueueGenericValueType<MatchMakingCharacterResult> * const msg = dynamic_cast<const MessageQueueGenericValueType<MatchMakingCharacterResult> *>(data);
		if(msg)
		{
			Archive::put(target, msg->getValue());
		}
	}

	MessageQueue::Data* unpackCharacterMatchResultVectorMessage(Archive::ReadIterator & source)
	{
		MatchMakingCharacterResult v;
		Archive::get(source, v);
		MessageQueueGenericValueType<MatchMakingCharacterResult> * result = new MessageQueueGenericValueType<MatchMakingCharacterResult>(v);
		return result;
	}

	//----------------------------------------------------------------------

	void packStartingLocationSelectionResult(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		typedef std::pair<std::string, bool>   Payload;

		const MessageQueueGenericValueType<Payload> * const msg = safe_cast<const MessageQueueGenericValueType<Payload> *>(data);
		if (msg)
			Archive::put (target, msg->getValue ());

//		packGenericMessage <Payload>(data, target);
	}

	MessageQueue::Data* unpackStartingLocationSelectionResult(Archive::ReadIterator & source)
	{
		typedef std::pair<std::string, bool>   Payload;
		Payload v;
		Archive::get(source, v);
		MessageQueueGenericValueType<Payload> * const result = new MessageQueueGenericValueType<Payload>(v);
		return result;
	}

	//----------------------------------------------------------------------

	void packStartingLocations(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		typedef std::pair<StartingLocationData, bool>   PayloadData;
		typedef std::vector<PayloadData>                Payload;
		const MessageQueueGenericValueType<Payload> * const msg = safe_cast<const MessageQueueGenericValueType<Payload> *>(data);
		if (msg)
			Archive::put (target, msg->getValue ());
	}

	MessageQueue::Data* unpackStartingLocations(Archive::ReadIterator & source)
	{
		typedef std::pair<StartingLocationData, bool>   PayloadData;
		typedef std::vector<PayloadData>                Payload;
		Payload v;
		Archive::get(source, v);
		MessageQueueGenericValueType<Payload> * const result = new MessageQueueGenericValueType<Payload>(v);
		return result;
	}

	//----------------------------------------------------------------------

	void packStringNetworkIdPairVector(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		typedef std::pair<NetworkId, std::string> PayloadData;
		typedef std::vector<PayloadData> Payload;
		const MessageQueueGenericValueType<Payload> * const msg = safe_cast<const MessageQueueGenericValueType<Payload> *>(data);
		if (msg)
			Archive::put (target, msg->getValue ());
	}

	MessageQueue::Data* unpackStringNetworkIdPairVector(Archive::ReadIterator & source)
	{
		typedef std::pair<NetworkId, std::string> PayloadData;
		typedef std::vector<PayloadData> Payload;
		Payload v;
		Archive::get(source, v);
		MessageQueueGenericValueType<Payload> * const result = new MessageQueueGenericValueType<Payload>(v);
		return result;
	}

	//----------------------------------------------------------------------

	void packSphereVectorMessage(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		const MessageQueueGenericValueType<std::vector<Sphere> > * const msg = dynamic_cast<const MessageQueueGenericValueType<std::vector<Sphere> > *>(data);
		if(msg)
		{
			Archive::put(target, msg->getValue());
		}
	}

	MessageQueue::Data* unpackSphereVectorMessage(Archive::ReadIterator & source)
	{
		std::vector<Sphere> v;
		Archive::get(source, v);
		MessageQueueGenericValueType<std::vector<Sphere> > * result = new MessageQueueGenericValueType<std::vector<Sphere> >(v);
		return result;
	}


	//----------------------------------------------------------------------

	void packNetworkIdBoolPair(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		typedef std::pair<NetworkId, bool> Payload;
		const MessageQueueGenericValueType<Payload> * const msg = safe_cast<const MessageQueueGenericValueType<Payload> *>(data);
		if (msg)
		{
			Archive::put (target, msg->getValue ());
		}
	}

	MessageQueue::Data* unpackNetworkIdBoolPair(Archive::ReadIterator & source)
	{
		typedef std::pair<NetworkId, bool> Payload;
		Payload v;
		Archive::get(source, v);
		MessageQueueGenericValueType<Payload> * const result = new MessageQueueGenericValueType<Payload>(v);
		return result;
	}

	//----------------------------------------------------------------------

	void packNetworkIdNetworkIdVector(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		typedef std::pair<NetworkId, std::vector<NetworkId> > Payload;
		const MessageQueueGenericValueType<Payload> * const msg = safe_cast<const MessageQueueGenericValueType<Payload> *>(data);
		if (msg)
		{
			Archive::put(target, msg->getValue().first);
			Archive::put(target, msg->getValue().second);
		}
	}

	MessageQueue::Data* unpackNetworkIdNetworkIdVector(Archive::ReadIterator & source)
	{
		typedef std::pair<NetworkId, std::vector<NetworkId> > Payload;
		Payload payload;
		Archive::get(source, payload.first);
		Archive::get(source, payload.second);
		MessageQueueGenericValueType<Payload> * const result = new MessageQueueGenericValueType<Payload>(payload);
		return result;
	}

	//----------------------------------------------------------------------

	void packNetworkIdVector(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		typedef std::vector<NetworkId> Payload;
		const MessageQueueGenericValueType<Payload> * const msg = safe_cast<const MessageQueueGenericValueType<Payload> *>(data);
		if (msg)
		{
			Archive::put(target, msg->getValue());
		}
	}

	MessageQueue::Data* unpackNetworkIdVector(Archive::ReadIterator & source)
	{
		typedef std::vector<NetworkId> Payload;
		Payload payload;
		Archive::get(source, payload);
		MessageQueueGenericValueType<Payload> * const result = new MessageQueueGenericValueType<Payload>(payload);
		return result;
	}

	//----------------------------------------------------------------------

	void packNothing(const MessageQueue::Data *, Archive::ByteStream &)
	{
	}

	MessageQueue::Data* unpackNothing(Archive::ReadIterator &)
	{
		return nullptr;
	}

	//----------------------------------------------------------------------

	void packCommandTimerMessage( const MessageQueue::Data * data, Archive::ByteStream &target )
	{
		/*
		typedef std::pair< byte, std::vector< float > > Payload;
		
		const MessageQueueGenericValueType< Payload > * const msg = safe_cast< const MessageQueueGenericValueType< Payload > * >( data );
		if ( msg )
		{
			Archive::put( target, msg->getValue() );
		}
		*/
		const MessageQueueCommandTimer * msg = safe_cast< const MessageQueueCommandTimer * >( data );
		
		if ( msg )
		{
			uint32 flags = msg->getFlags();
			
			Archive::put( target, (byte)flags );
			Archive::put( target, msg->getSequenceId() );		
			Archive::put( target, msg->getCommandNameCrc() );

			if ( flags & MessageQueueCommandTimer::toBitValue( MessageQueueCommandTimer::F_cooldown ) )
			{
				Archive::put( target, msg->getCooldownGroup() );
			}
			if ( flags & MessageQueueCommandTimer::toBitValue( MessageQueueCommandTimer::F_cooldown2 ) )
			{
				Archive::put( target, msg->getCooldownGroup2() );
			}

			for ( int i = 0; i < MessageQueueCommandTimer::F_MAX; ++i )
			{
				MessageQueueCommandTimer::Flags flag = static_cast< MessageQueueCommandTimer::Flags >( i );
				
				if ( flags & MessageQueueCommandTimer::toBitValue( flag ) )
				{
					Archive::put( target, msg->getCurrentTime( flag ) );
					Archive::put( target, msg->getMaxTime( flag ) );
				}
			}
		}
		
		
	}

	MessageQueue::Data* unpackCommandTimerMessage(Archive::ReadIterator & source)
	{
		/*
		typedef std::pair< byte, std::vector< float > > Payload;

		Payload v;
		Archive::get(source, v);
		MessageQueueGenericValueType<Payload> * const result = new MessageQueueGenericValueType<Payload>(v);
		return result;
		*/
		
		
		byte flags;
		Archive::get( source, flags );
		
		uint32 sequenceId;
		Archive::get( source, sequenceId );

		uint32 commandNameCrc;
		Archive::get( source, commandNameCrc );

		int cooldownGroup = NULL_COOLDOWN_GROUP;

		if ( flags & MessageQueueCommandTimer::toBitValue( MessageQueueCommandTimer::F_cooldown ) )
		{
			Archive::get( source, cooldownGroup );
		}

		int cooldownGroup2 = NULL_COOLDOWN_GROUP;

		if ( flags & MessageQueueCommandTimer::toBitValue( MessageQueueCommandTimer::F_cooldown2 ) )
		{
			Archive::get( source, cooldownGroup2 );
		}

		MessageQueueCommandTimer *msg = new MessageQueueCommandTimer( sequenceId, cooldownGroup, cooldownGroup2, commandNameCrc );
		
		for ( int i = 0; i < MessageQueueCommandTimer::F_MAX; ++i )
		{
			MessageQueueCommandTimer::Flags flag = static_cast< MessageQueueCommandTimer::Flags >( i );
			
			if ( flags & MessageQueueCommandTimer::toBitValue( flag ) )
			{
				float value;
				Archive::get( source, value );
				msg->setCurrentTime( flag, value );
				
				Archive::get( source, value );
				msg->setMaxTime( flag, value );
			}
		}
		
		return msg;
	}

}

using namespace SetupSwgSharedNetworkMessagesNamespace;

// ----------------------------------------------------------------------

void SetupSwgSharedNetworkMessages::install ()
{
	DEBUG_FATAL(g_installed, ("SetupSwgSharedNetworkMessages::install - is already installed"));

	MessageQueueCombatAction::install();
	MessageQueueCombatDamage::install();

	ControllerMessageFactory::registerControllerMessageHandler(CM_setAttribute,  packAttributeMessage, unpackAttributeMessage);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setMaxAttribute,  packAttributeMessage,  unpackAttributeMessage);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setMentalState,  packMentalStatesMessage,  unpackMentalStatesMessage );
	ControllerMessageFactory::registerControllerMessageHandler(CM_setMaxMentalState,  packMentalStatesMessage,  unpackMentalStatesMessage);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setMentalStateToward,  packMentalStatesTowardsMessage, unpackMentalStatesTowardsMessage);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setCover, packIntMessage, unpackIntMessage);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setMaxHitPoints,  packIntMessage, unpackIntMessage);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setCombatAttitude, packIntMessage, unpackIntMessage);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setAutoAttack,  packBoolMessage, unpackBoolMessage);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setInvulnerable, packBoolMessage, unpackBoolMessage);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setMentalStateTowardClampBehavior,  packMentalStateTowardClampBehaviorMessage, unpackMentalStateTowardClampBehaviorMessage);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setMentalStateDecay,  packMentalStateDecayMessage,  unpackMentalStateDecayMessage);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setLocomotion,  packLocomotionMessage, unpackLocomotionMessage);
	ControllerMessageFactory::registerControllerMessageHandler(CM_addPlayerToSpawnQueue,  packAddToPlayerSpawnQueueMessage, unpackAddToPlayerSpawnQueueMessage);
	ControllerMessageFactory::registerControllerMessageHandler(CM_removePlayerFromSpawnQueue,  packNetworkIdMessage, unpackNetworkIdMessage);
	ControllerMessageFactory::registerControllerMessageHandler(CM_forwardNpcConversationMessage, packGenericStringIdProsePackageMessage, unpackGenericStringIdProsePackageMessage);
	ControllerMessageFactory::registerControllerMessageHandler(CM_musicFlourish, packIntMessage, unpackIntMessage);
	ControllerMessageFactory::registerControllerMessageHandler(CM_npcConversationSelect, packNothing, unpackNothing);
	ControllerMessageFactory::registerControllerMessageHandler(CM_craftingSessionEnded, packBoolMessage, unpackBoolMessage);
	ControllerMessageFactory::registerControllerMessageHandler(CM_biographyRetrieved, packNetworkIdUnicodeStringPairMessage, unpackNetworkIdUnicodeStringPairMessage);
	ControllerMessageFactory::registerControllerMessageHandler(CM_characterMatchRetrieved, packCharacterMatchResultVectorMessage, unpackCharacterMatchResultVectorMessage);
	ControllerMessageFactory::registerControllerMessageHandler(CM_startingLocations, packStartingLocations, unpackStartingLocations);
	ControllerMessageFactory::registerControllerMessageHandler(CM_startingLocationSelectionResult, packStartingLocationSelectionResult, unpackStartingLocationSelectionResult);
	ControllerMessageFactory::registerControllerMessageHandler(CM_disconnect, packNothing, unpackNothing);
	ControllerMessageFactory::registerControllerMessageHandler(CM_emergencyDismountForRider, packNothing, unpackNothing);
	ControllerMessageFactory::registerControllerMessageHandler(CM_detachRiderForMount, packNothing, unpackNothing);
	ControllerMessageFactory::registerControllerMessageHandler(CM_formDataForEdit, packNetworkIdUnicodeStringPairMessage, unpackNetworkIdUnicodeStringPairMessage);
	ControllerMessageFactory::registerControllerMessageHandler(CM_serverAsteroidDebugData, packSphereVectorMessage, unpackSphereVectorMessage);
	ControllerMessageFactory::registerControllerMessageHandler(CM_commPlayer, packNetworkIdUnicodeStringPairMessage, unpackNetworkIdUnicodeStringPairMessage);
	ControllerMessageFactory::registerControllerMessageHandler(CM_clientLookAtTargetComponent, packIntMessage, unpackIntMessage, true);
	ControllerMessageFactory::registerControllerMessageHandler(CM_aboutToHyperspace, packGenericStringVectorPairMessage, unpackGenericStringVectorPairMessage);
	ControllerMessageFactory::registerControllerMessageHandler(CM_playerTransitioningOut, packNothing, unpackNothing);
	ControllerMessageFactory::registerControllerMessageHandler(CM_playerTransitioningIn, packNothing, unpackNothing);
	ControllerMessageFactory::registerControllerMessageHandler(CM_addIgnoreIntersect, packNetworkIdMessage, unpackNetworkIdMessage);
	ControllerMessageFactory::registerControllerMessageHandler(CM_removeIgnoreIntersect, packNetworkIdMessage, unpackNetworkIdMessage);
	ControllerMessageFactory::registerControllerMessageHandler(CM_spaceTerminalRequest, packNetworkIdMessage, unpackNetworkIdMessage, true);
	ControllerMessageFactory::registerControllerMessageHandler(CM_spaceTerminalResponse, packStringNetworkIdPairVector, unpackStringNetworkIdPairVector);
	ControllerMessageFactory::registerControllerMessageHandler(CM_hyperspaceOrientShipToPointAndLockPlayerInput, packGenericStringVectorPairMessage, unpackGenericStringVectorPairMessage);
	ControllerMessageFactory::registerControllerMessageHandler(CM_lockPlayerShipInputOnClient, packNothing, unpackNothing);
	ControllerMessageFactory::registerControllerMessageHandler(CM_unlockPlayerShipInputOnClient, packNothing, unpackNothing);
	ControllerMessageFactory::registerControllerMessageHandler(CM_inviteOtherGroupMembersToLaunchIntoSpace, packNetworkIdMessage, unpackNetworkIdMessage, true);
	ControllerMessageFactory::registerControllerMessageHandler(CM_askGroupMemberToLaunchIntoSpace, packNetworkIdMessage, unpackNetworkIdMessage);
	ControllerMessageFactory::registerControllerMessageHandler(CM_groupMemberInvitationToLaunchIntoSpaceResponse, packNetworkIdBoolPair, unpackNetworkIdBoolPair, true);
	ControllerMessageFactory::registerControllerMessageHandler(CM_relayGroupMemberInvitationToLaunchAnswer, packNetworkIdBoolPair, unpackNetworkIdBoolPair);
	ControllerMessageFactory::registerControllerMessageHandler(CM_groupOpenLotteryWindowOnClient, packNetworkIdMessage, unpackNetworkIdMessage);
	ControllerMessageFactory::registerControllerMessageHandler(CM_groupCloseLotteryWindowOnClient, packNetworkIdMessage, unpackNetworkIdMessage);
	ControllerMessageFactory::registerControllerMessageHandler(CM_groupLotteryWindowHeartbeat, packNetworkIdMessage, unpackNetworkIdMessage);
	ControllerMessageFactory::registerControllerMessageHandler(CM_groupLotteryWindowCloseResults, packNetworkIdNetworkIdVector, unpackNetworkIdNetworkIdVector, true);
	ControllerMessageFactory::registerControllerMessageHandler(CM_commandTimer, packCommandTimerMessage, unpackCommandTimerMessage );
	ControllerMessageFactory::registerControllerMessageHandler(CM_requestActivateQuest, packIntMessage, unpackIntMessage);
	ControllerMessageFactory::registerControllerMessageHandler(CM_requestCompleteQuest, packIntMessage, unpackIntMessage);
	ControllerMessageFactory::registerControllerMessageHandler(CM_staticLootItemData, packGenericVectorOfUnicodeStrings, unpackGenericVectorOfUnicodeStrings);
	ControllerMessageFactory::registerControllerMessageHandler(CM_showLootBox, packNetworkIdVector, unpackNetworkIdVector);
	ControllerMessageFactory::registerControllerMessageHandler(CM_forceActivateQuest, packIntMessage, unpackIntMessage);

	g_installed = true;
	ExitChain::add (SetupSwgSharedNetworkMessages::remove, "SetupSwgSharedNetworkMessages");
}
// ----------------------------------------------------------------------

void SetupSwgSharedNetworkMessages::remove ( void )
{
	DEBUG_FATAL(!g_installed, ("SetupSwgSharedNetworkMessages::remove - not already installed"));
	g_installed = false;
}
// ----------------------------------------------------------------------
