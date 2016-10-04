// ======================================================================
//
// SetupSwgServerNetworkMessages.cpp
// copyright 1998 Bootprint Entertainment
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#include "swgServerNetworkMessages/FirstSwgServerNetworkMessages.h"
#include "swgServerNetworkMessages/SetupSwgServerNetworkMessages.h"

#include "StringId.h"
#include "localizationArchive/StringIdArchive.h"
#include "serverNetworkMessages/MessageQueueAlterAttribute.h"
#include "serverNetworkMessages/MessageQueueTeleportObject.h"
#include "serverUtility/LocationData.h"
#include "sharedFoundation/DynamicVariable.h"
#include "sharedFoundation/DynamicVariableLocationData.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedGame/ProsePackage.h"
#include "sharedGame/ProsePackageArchive.h"
#include "sharedMathArchive/VectorArchive.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"
#include "sharedNetworkMessages/MapLocation.h"
#include "sharedNetworkMessages/MapLocationArchive.h"
#include "sharedNetworkMessages/MessageQueueDataTransformWithParent.h"
#include "swgServerNetworkMessages/MessageQueueCombatDamageList.h"
#include "swgServerNetworkMessages/MessageQueueDirectDamage.h"
#include "swgServerNetworkMessages/MessageQueueInstallationHarvest.h"
#include "swgServerNetworkMessages/MessageQueueJediData.h"
#include "swgServerNetworkMessages/MessageQueueJediLocation.h"
#include "swgServerNetworkMessages/MessageQueuePlanetAddTravelPoint.h"
#include "swgServerNetworkMessages/MessageQueueRequestJediBounty.h"
#include "swgServerNetworkMessages/MessageQueueScriptTransferMoney.h"

// ======================================================================

namespace SetupSwgServerNetworkMessagesNamespace
{
	static bool g_installed = false;

	template <typename T> void packGenericMessage (const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		const MessageQueueGenericValueType<T> * const msg = safe_cast<const MessageQueueGenericValueType<T> *>(data);
		if (msg)
		{
			Archive::put (target, msg->getValue ());
		}
	}

	template <typename T> MessageQueue::Data * unpackGenericMessage (Archive::ReadIterator & source)
	{
		T v;
		Archive::get(source, v);
		MessageQueueGenericValueType<T> * result = new MessageQueueGenericValueType<T>(v);
		return result;
	}

	void packString(const MessageQueue::Data *data, Archive::ByteStream &target)
	{
		const MessageQueueGenericValueType<std::string> * msg = safe_cast<const MessageQueueGenericValueType<std::string> *>(data);
		if(msg)
		{
			Archive::put(target, msg->getValue());
		}
	}

	MessageQueue::Data* unpackString(Archive::ReadIterator &source)
	{
		std::string v;
		Archive::get(source, v);
		MessageQueueGenericValueType<std::string> * result = new MessageQueueGenericValueType<std::string>(v);
		return result;
	}

	void packNetworkIdString(const MessageQueue::Data *data, Archive::ByteStream &target)
	{
		const MessageQueueGenericValueType<std::pair<NetworkId, std::string> > * msg = safe_cast<const MessageQueueGenericValueType<std::pair<NetworkId, std::string> > *>(data);
		if(msg)
		{
			Archive::put(target, msg->getValue());
		}
	}
	MessageQueue::Data* unpackNetworkIdString(Archive::ReadIterator &source)
	{
		std::pair<NetworkId, std::string> v;
		Archive::get(source, v);
		MessageQueueGenericValueType<std::pair<NetworkId, std::string> > * result = new MessageQueueGenericValueType<std::pair<NetworkId, std::string> >(v);
		return result;
	}

	void packNetworkIdStringInt(const MessageQueue::Data *data, Archive::ByteStream &target)
	{
		const MessageQueueGenericValueType<std::pair<NetworkId, std::pair<std::string, int> > > * msg = safe_cast<const MessageQueueGenericValueType<std::pair<NetworkId, std::pair<std::string, int> > > *>(data);
		if(msg)
		{
			Archive::put(target, msg->getValue());
		}
	}
	MessageQueue::Data* unpackNetworkIdStringInt(Archive::ReadIterator &source)
	{
		std::pair<NetworkId, std::pair<std::string, int> > v;
		Archive::get(source, v);
		MessageQueueGenericValueType<std::pair<NetworkId, std::pair<std::string, int> > > * result = new MessageQueueGenericValueType<std::pair<NetworkId, std::pair<std::string, int> > >(v);
		return result;
	}
	
	void packNetworkId(const MessageQueue::Data *data, Archive::ByteStream &target)
	{
		const MessageQueueGenericValueType<NetworkId> * msg = safe_cast<const MessageQueueGenericValueType<NetworkId> *>(data);
		if(msg)
		{
			Archive::put(target, msg->getValue());
		}
	}
	MessageQueue::Data* unpackNetworkId(Archive::ReadIterator &source)
	{
		NetworkId v;
		Archive::get(source, v);
		MessageQueueGenericValueType<NetworkId> * result = new MessageQueueGenericValueType<NetworkId>(v);
		return result;
	}
	void packInt(const MessageQueue::Data *data, Archive::ByteStream &target)
	{
		const MessageQueueGenericValueType<int> * msg = safe_cast<const MessageQueueGenericValueType<int> *>(data);
		if(msg)
		{
			Archive::put(target, msg->getValue());
		}
	}
	MessageQueue::Data* unpackInt(Archive::ReadIterator &source)
	{
		int v;
		Archive::get(source, v);
		MessageQueueGenericValueType<int> * result = new MessageQueueGenericValueType<int>(v);
		return result;
	}
	void packFloat(const MessageQueue::Data *data, Archive::ByteStream &target)
	{
		const MessageQueueGenericValueType<float> * msg = safe_cast<const MessageQueueGenericValueType<float> *>(data);
		if(msg)
		{
			Archive::put(target, msg->getValue());
		}
	}
	MessageQueue::Data* unpackFloat(Archive::ReadIterator &source)
	{
		float v;
		Archive::get(source, v);
		MessageQueueGenericValueType<float> * result = new MessageQueueGenericValueType<float>(v);
		return result;
	}

	//----------------------------------------------------------------------

	void packMapLocationTypePair(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		packGenericMessage <std::pair <MapLocation, int> > (data, target);
	}

	//----------------------------------------------------------------------

	MessageQueue::Data* unpackMapLocationTypePair(Archive::ReadIterator &source)
	{
		return unpackGenericMessage <std::pair <MapLocation, int> > (source);
	}

	//----------------------------------------------------------------------

	void packStringId(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		packGenericMessage <StringId> (data, target);
	}

	MessageQueue::Data* unpackStringId(Archive::ReadIterator &source)
	{
		return unpackGenericMessage <StringId> (source);
	}

	void packStringIdProsePackage(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		packGenericMessage <std::pair<StringId, ProsePackage> >(data, target);
	}

	MessageQueue::Data* unpackStringIdProsePackage(Archive::ReadIterator & source)
	{
		return unpackGenericMessage<std::pair<StringId, ProsePackage> > (source);
	}

	void packStringNetworkIdStringIdProsePackage(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		packGenericMessage <std::pair<std::string, std::pair<NetworkId, std::pair<StringId, ProsePackage> > > >(data, target);
	}

	MessageQueue::Data* unpackStringNetworkIdStringIdProsePackage (Archive::ReadIterator & source)
	{
		return unpackGenericMessage <std::pair<std::string, std::pair<NetworkId, std::pair<StringId, ProsePackage> > > >(source);
	}

	void packUnsignedLong(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		const MessageQueueGenericValueType<unsigned long> * msg = safe_cast<const MessageQueueGenericValueType<unsigned long> *>(data);
		if(msg)
		{
			Archive::put(target, msg->getValue());
		}
	}

	MessageQueue::Data * unpackUnsignedLong(Archive::ReadIterator & source)
	{
		unsigned long v;
		Archive::get(source, v);
		return new MessageQueueGenericValueType<unsigned long>(v);
	}

	void packUnicodeString(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		const MessageQueueGenericValueType<Unicode::String> * msg = safe_cast<const MessageQueueGenericValueType<Unicode::String> *>(data);
		if(msg)
		{
			Archive::put(target, msg->getValue());
		}
	}

	MessageQueue::Data * unpackUnicodeString(Archive::ReadIterator & source)
	{
		Unicode::String v;
		Archive::get(source, v);
		return new MessageQueueGenericValueType<Unicode::String>(v);
	}

	void packNothing(const MessageQueue::Data *, Archive::ByteStream &)
	{
	}
	MessageQueue::Data* unpackNothing(Archive::ReadIterator &)
	{
		return nullptr;
	}

	void packVectorNetworkIdVector(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		const MessageQueueGenericValueType<std::pair<Vector, std::pair<NetworkId, Vector> > > * msg = safe_cast<const MessageQueueGenericValueType<std::pair<Vector, std::pair<NetworkId, Vector> > > *>(data);
		if(msg)
		{
			Archive::put(target, msg->getValue());
		}
	}

	MessageQueue::Data * unpackVectorNetworkIdVector(Archive::ReadIterator & source)
	{
		std::pair<Vector, std::pair<NetworkId, Vector> > v;
		Archive::get(source, v);
		return new MessageQueueGenericValueType<std::pair<Vector, std::pair<NetworkId, Vector> > >(v);
	}

	void packIntVector(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		const MessageQueueGenericValueType<std::pair<int, Vector> > * msg = safe_cast<const MessageQueueGenericValueType<std::pair<int, Vector> > *>(data);
		if(msg)
		{
			Archive::put(target, msg->getValue());
		}
	}

	MessageQueue::Data * unpackIntVector(Archive::ReadIterator & source)
	{
		std::pair<int, Vector> v;
		Archive::get(source, v);
		return new MessageQueueGenericValueType<std::pair<int, Vector> >(v);
	}


	void packBool(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		const MessageQueueGenericValueType<bool> * msg = safe_cast<const MessageQueueGenericValueType<bool> *>(data);
		if(msg)
		{
			Archive::put(target, msg->getValue());
		}
	}

	MessageQueue::Data * unpackBool(Archive::ReadIterator & source)
	{
		bool v;
		Archive::get(source, v);
		return new MessageQueueGenericValueType<bool>(v);
	}

	void packStringInt(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		const MessageQueueGenericValueType<std::pair<std::string, int> > * msg = safe_cast<const MessageQueueGenericValueType<std::pair<std::string, int> > *>(data);
		if(msg)
		{
			Archive::put(target, msg->getValue());
		}
	}

	MessageQueue::Data * unpackStringInt(Archive::ReadIterator & source)
	{
		std::pair<std::string, int> v;
		Archive::get(source, v);
		return new MessageQueueGenericValueType<std::pair<std::string, int> >(v);
	}

	void packStringIntArray(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		const MessageQueueGenericValueType<std::pair<std::string, std::vector<int> > > * msg = safe_cast<const MessageQueueGenericValueType<std::pair<std::string, std::vector<int> > > *>(data);
		if(msg)
		{
			Archive::put(target, msg->getValue());
		}
	}

	MessageQueue::Data * unpackStringIntArray(Archive::ReadIterator & source)
	{
		std::pair<std::string, std::vector<int> > v;
		Archive::get(source, v);
		return new MessageQueueGenericValueType<std::pair<std::string, std::vector<int> > >(v);
	}

	void packLocationData(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		const MessageQueueGenericValueType<LocationData> * msg = safe_cast<const MessageQueueGenericValueType<LocationData> *>(data);
		if(msg)
		{
			Archive::put(target, msg->getValue());
		}
	}

	MessageQueue::Data * unpackLocationData(Archive::ReadIterator & source)
	{
		LocationData v;
		Archive::get(source, v);
		return new MessageQueueGenericValueType<LocationData>(v);
	}

	void packBoolNetworkId(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		const MessageQueueGenericValueType<std::pair<bool, NetworkId> > * msg = safe_cast<const MessageQueueGenericValueType<std::pair<bool, NetworkId> > *>(data);
		if(msg)
		{
			Archive::put(target, msg->getValue());
		}
	}

	MessageQueue::Data * unpackBoolNetworkId(Archive::ReadIterator & source)
	{
		std::pair<bool, NetworkId> v;
		Archive::get(source, v);
		return new MessageQueueGenericValueType<std::pair<bool, NetworkId> >(v);
	}

	void packIntNetworkId(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		const MessageQueueGenericValueType<std::pair<int, NetworkId> > * msg = safe_cast<const MessageQueueGenericValueType<std::pair<int, NetworkId> > *>(data);
		if(msg)
		{
			Archive::put(target, msg->getValue());
		}
	}

	MessageQueue::Data * unpackIntNetworkId(Archive::ReadIterator & source)
	{
		std::pair<int, NetworkId> v;
		Archive::get(source, v);
		return new MessageQueueGenericValueType<std::pair<int, NetworkId> >(v);
	}

	void packNetworkIdInt(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		const MessageQueueGenericValueType<std::pair<NetworkId, int> > * msg = safe_cast<const MessageQueueGenericValueType<std::pair<NetworkId, int> > *>(data);
		if(msg)
		{
			Archive::put(target, msg->getValue());
		}
	}

	MessageQueue::Data * unpackNetworkIdInt(Archive::ReadIterator & source)
	{
		std::pair<NetworkId, int> v;
		Archive::get(source, v);
		return new MessageQueueGenericValueType<std::pair<NetworkId, int> >(v);
	}

	void packNetworkIdFloat(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		const MessageQueueGenericValueType<std::pair<NetworkId, float> > * msg = safe_cast<const MessageQueueGenericValueType<std::pair<NetworkId, float> > *>(data);
		if(msg)
		{
			Archive::put(target, msg->getValue());
		}
	}

	MessageQueue::Data * unpackNetworkIdFloat(Archive::ReadIterator & source)
	{
		std::pair<NetworkId, float> v;
		Archive::get(source, v);
		return new MessageQueueGenericValueType<std::pair<NetworkId, float> >(v);
	}
	
	void packNetworkIdNetworkId(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		const MessageQueueGenericValueType<std::pair<NetworkId, NetworkId> > * msg = safe_cast<const MessageQueueGenericValueType<std::pair<NetworkId, NetworkId> > *>(data);
		if(msg)
		{
			Archive::put(target, msg->getValue());
		}
	}

	MessageQueue::Data * unpackNetworkIdNetworkId(Archive::ReadIterator & source)
	{
		std::pair<NetworkId, NetworkId> v;
		Archive::get(source, v);
		return new MessageQueueGenericValueType<std::pair<NetworkId, NetworkId> >(v);
	}

	void packIntArray(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		const MessageQueueGenericValueType<std::vector<int> > * msg = safe_cast<const MessageQueueGenericValueType<std::vector<int> > *>(data);
		if(msg)
		{
			Archive::put(target, msg->getValue());
		}
	}

	MessageQueue::Data * unpackIntArray(Archive::ReadIterator & source)
	{
		std::vector<int> v;
		Archive::get(source, v);
		return new MessageQueueGenericValueType<std::vector<int> >(v);
	}

	void packStringFloat(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		const MessageQueueGenericValueType<std::pair<std::string, float> > * msg = safe_cast<const MessageQueueGenericValueType<std::pair<std::string, float> > *>(data);
		if(msg)
		{
			Archive::put(target, msg->getValue());
		}
	}

	MessageQueue::Data * unpackStringFloat(Archive::ReadIterator & source)
	{
		std::pair<std::string, float> v;
		Archive::get(source, v);
		return new MessageQueueGenericValueType<std::pair<std::string, float> >(v);
	}

	void packStringFloatArray(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		const MessageQueueGenericValueType<std::pair<std::string, std::vector<float> > > * msg = safe_cast<const MessageQueueGenericValueType<std::pair<std::string, std::vector<float> > > *>(data);
		if(msg)
		{
			Archive::put(target, msg->getValue());
		}
	}

	MessageQueue::Data * unpackStringFloatArray(Archive::ReadIterator & source)
	{
		std::pair<std::string, std::vector<float> > v;
		Archive::get(source, v);
		return new MessageQueueGenericValueType<std::pair<std::string, std::vector<float> > >(v);
	}

	void packStringIdArray(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		const MessageQueueGenericValueType<std::vector<StringId> > * msg = safe_cast<const MessageQueueGenericValueType<std::vector<StringId> > *>(data);
		if(msg)
		{
			Archive::put(target, msg->getValue());
		}
	}

	MessageQueue::Data * unpackStringIdArray(Archive::ReadIterator & source)
	{
		std::vector<StringId> v;
		Archive::get(source, v);
		MessageQueueGenericValueType<std::vector<StringId> > * result = new MessageQueueGenericValueType<std::vector<StringId> >(v);
		return result;
	}

	void packAttributeIntNetworkId(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		const MessageQueueGenericValueType<std::pair<Attributes::Enumerator, std::pair<int, NetworkId> > > * msg = safe_cast<const MessageQueueGenericValueType<std::pair<Attributes::Enumerator, std::pair<int, NetworkId> > > *>(data);
		if(msg)
		{
			Archive::put(target, msg->getValue());
		}
	}

	void packUnicodeStringArray(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		const MessageQueueGenericValueType<std::vector<Unicode::String> > * msg = safe_cast<const MessageQueueGenericValueType<std::vector<Unicode::String> > *>(data);
		if(msg)
		{
			Archive::put(target, msg->getValue());
		}
	}

	MessageQueue::Data * unpackUnicodeStringArray(Archive::ReadIterator & source)
	{
		std::vector<Unicode::String> v;
		Archive::get(source, v);
		MessageQueueGenericValueType<std::vector<Unicode::String> > * result = new MessageQueueGenericValueType<std::vector<Unicode::String> >(v);
		return result;
	}

	MessageQueue::Data * unpackAttributeIntNetworkId(Archive::ReadIterator & source)
	{
		std::pair<Attributes::Enumerator, std::pair<int, NetworkId> > v;
		Archive::get(source, v);
		return new MessageQueueGenericValueType<std::pair<Attributes::Enumerator, std::pair<int, NetworkId> > >(v);
	}

	void packAttributeIntFloatFloatFloat(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		const MessageQueueGenericValueType<std::pair<Attributes::Enumerator, std::pair<int, std::pair<float, std::pair<float, float> > > > > * const msg = safe_cast<const MessageQueueGenericValueType<std::pair<Attributes::Enumerator, std::pair<int, std::pair<float, std::pair<float, float> > > > > *>(data);
		if(msg)
		{
			Archive::put(target, msg->getValue());
		}
	}

	MessageQueue::Data * unpackAttributeIntFloatFloatFloat(Archive::ReadIterator & source)
	{
		std::pair<Attributes::Enumerator, std::pair<int, std::pair<float, std::pair<float, float> > > > v;
		Archive::get(source, v);
		return new MessageQueueGenericValueType<std::pair<Attributes::Enumerator, std::pair<int, std::pair<float, std::pair<float, float> > > > >(v);
	}

	void packAttribute(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		const MessageQueueGenericValueType<Attributes::Enumerator> * msg = safe_cast<const MessageQueueGenericValueType<Attributes::Enumerator> *>(data);
		if(msg)
		{
			Archive::put(target, msg->getValue());
		}
	}

	MessageQueue::Data * unpackAttribute(Archive::ReadIterator & source)
	{
		Attributes::Enumerator v;
		Archive::get(source, v);
		return new MessageQueueGenericValueType<Attributes::Enumerator>(v);
	}

	void packNetworkIdArray(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		const MessageQueueGenericValueType<std::vector<NetworkId> > * msg = safe_cast<const MessageQueueGenericValueType<std::vector<NetworkId> > *>(data);
		if(msg)
		{
			Archive::put(target, msg->getValue());
		}
	}

	MessageQueue::Data * unpackNetworkIdArray(Archive::ReadIterator & source)
	{
		std::vector<NetworkId> v;
		Archive::get(source, v);
		MessageQueueGenericValueType<std::vector<NetworkId> > * result = new MessageQueueGenericValueType<std::vector<NetworkId> >(v);
		return result;
	}

	void packNetworkIdNetworkIdNetworkId(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		const MessageQueueGenericValueType<std::pair<NetworkId, std::pair<NetworkId, NetworkId> > > * msg = safe_cast<const MessageQueueGenericValueType<std::pair<NetworkId, std::pair<NetworkId, NetworkId> > > *>(data);
		if(msg)
		{
			Archive::put(target, msg->getValue());
		}
	}

	MessageQueue::Data * packNetworkIdNetworkIdNetworkId(Archive::ReadIterator & source)
	{
		std::pair<NetworkId, std::pair<NetworkId, NetworkId> > v;
		Archive::get(source, v);
		return new MessageQueueGenericValueType<std::pair<NetworkId, std::pair<NetworkId, NetworkId> > >(v);
	}

	void packNetworkIdUint32(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		const MessageQueueGenericValueType<std::pair<NetworkId, uint32> > * msg = safe_cast<const MessageQueueGenericValueType<std::pair<NetworkId, uint32> > *>(data);
		if(msg)
		{
			Archive::put(target, msg->getValue());
		}
	}

	MessageQueue::Data * unpackNetworkIdUint32(Archive::ReadIterator & source)
	{
		std::pair<NetworkId, uint32> v;
		Archive::get(source, v);
		return new MessageQueueGenericValueType<std::pair<NetworkId, uint32> >(v);
	}

	void packULongBool(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		const MessageQueueGenericValueType<std::pair<unsigned long, bool> > * msg = safe_cast<const MessageQueueGenericValueType<std::pair<unsigned long, bool> > *>(data);
		if(msg)
		{
			Archive::put(target, msg->getValue());
		}
	}

	MessageQueue::Data * unpackULongBool(Archive::ReadIterator & source)
	{
		std::pair<unsigned long, bool> v;
		Archive::get(source, v);
		return new MessageQueueGenericValueType<std::pair<unsigned long, bool> >(v);
	}

	void packStringNetworkIdNetworkId(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		const MessageQueueGenericValueType<std::pair<std::string, std::pair<NetworkId, NetworkId> > > * msg = safe_cast<const MessageQueueGenericValueType<std::pair<std::string, std::pair<NetworkId, NetworkId> > > *>(data);
		if(msg)
		{
			Archive::put(target, msg->getValue());
		}
	}

	MessageQueue::Data * unpackStringNetworkIdNetworkId(Archive::ReadIterator & source)
	{
		std::pair<std::string, std::pair<NetworkId, NetworkId> > v;
		Archive::get(source, v);
		return new MessageQueueGenericValueType<std::pair<std::string, std::pair<NetworkId, NetworkId> > >(v);
	}

	void packDynamicVariableLocation(const MessageQueue::Data *data, Archive::ByteStream &target)
	{
		const MessageQueueGenericValueType<std::pair<std::string, DynamicVariableLocationData> > * msg = safe_cast<const MessageQueueGenericValueType<std::pair<std::string, DynamicVariableLocationData> > *>(data);
		if(msg)
		{
			Archive::put(target, msg->getValue());
		}
	}
	MessageQueue::Data* unpackDynamicVariableLocation(Archive::ReadIterator &source)
	{
		std::pair<std::string, DynamicVariableLocationData> v;
		Archive::get(source, v);
		MessageQueueGenericValueType<std::pair<std::string, DynamicVariableLocationData> > * result = new MessageQueueGenericValueType<std::pair<std::string, DynamicVariableLocationData> >(v);
		return result;
	}

	void packDynamicVariableLocationArray(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		const MessageQueueGenericValueType<std::pair<std::string, std::vector<DynamicVariableLocationData> > > * msg = safe_cast<const MessageQueueGenericValueType<std::pair<std::string, std::vector<DynamicVariableLocationData> > > *>(data);
		if(msg)
		{
			Archive::put(target, msg->getValue());
		}
	}

	MessageQueue::Data * unpackDynamicVariableLocationArray(Archive::ReadIterator & source)
	{
		std::pair<std::string, std::vector<DynamicVariableLocationData> > v;
		Archive::get(source, v);
		return new MessageQueueGenericValueType<std::pair<std::string, std::vector<DynamicVariableLocationData> > >(v);
	}
	
	void packStringNetworkId(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		const MessageQueueGenericValueType<std::pair<std::string, NetworkId> > * msg = safe_cast<const MessageQueueGenericValueType<std::pair<std::string, NetworkId> > *>(data);
		if(msg)
		{
			Archive::put(target, msg->getValue());
		}
	}

	MessageQueue::Data * unpackStringNetworkId(Archive::ReadIterator & source)
	{
		std::pair<std::string, NetworkId> v;
		Archive::get(source, v);
		return new MessageQueueGenericValueType<std::pair<std::string, NetworkId> >(v);
	}

	void packStringStringNetworkId(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		const MessageQueueGenericValueType<std::pair<std::string, std::pair<std::string, NetworkId> > > * msg = safe_cast<const MessageQueueGenericValueType<std::pair<std::string, std::pair<std::string, NetworkId> > > *>(data);
		if(msg)
		{
			Archive::put(target, msg->getValue());
		}
	}

	MessageQueue::Data * unpackStringStringNetworkId(Archive::ReadIterator & source)
	{
		std::pair<std::string, std::pair<std::string, NetworkId> > v;
		Archive::get(source, v);
		return new MessageQueueGenericValueType<std::pair<std::string, std::pair<std::string, NetworkId> > >(v);
	}

	void packStringNetworkIdArray(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		const MessageQueueGenericValueType<std::pair<std::string, std::vector<NetworkId> > > * msg = safe_cast<const MessageQueueGenericValueType<std::pair<std::string, std::vector<NetworkId> > > *>(data);
		if(msg)
		{
			Archive::put(target, msg->getValue());
		}
	}

	MessageQueue::Data * unpackStringNetworkIdArray(Archive::ReadIterator & source)
	{
		std::pair<std::string, std::vector<NetworkId> > v;
		Archive::get(source, v);
		return new MessageQueueGenericValueType<std::pair<std::string, std::vector<NetworkId> > >(v);
	}

	void packStringStringId(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		const MessageQueueGenericValueType<std::pair<std::string, StringId> > * msg = safe_cast<const MessageQueueGenericValueType<std::pair<std::string, StringId> > *>(data);
		if(msg)
		{
			Archive::put(target, msg->getValue());
		}
	}

	MessageQueue::Data * unpackStringStringId(Archive::ReadIterator & source)
	{
		std::pair<std::string, StringId> v;
		Archive::get(source, v);
		return new MessageQueueGenericValueType<std::pair<std::string, StringId> >(v);
	}

	void packStringStringIdArray(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		const MessageQueueGenericValueType<std::pair<std::string, std::vector<StringId> > > * msg = safe_cast<const MessageQueueGenericValueType<std::pair<std::string, std::vector<StringId> > > *>(data);
		if(msg)
		{
			Archive::put(target, msg->getValue());
		}
	}

	MessageQueue::Data * unpackStringStringIdArray(Archive::ReadIterator & source)
	{
		std::pair<std::string, std::vector<StringId> > v;
		Archive::get(source, v);
		return new MessageQueueGenericValueType<std::pair<std::string, std::vector<StringId> > >(v);
	}

	void packStringString(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		const MessageQueueGenericValueType<std::pair<std::string, std::string> > * msg = safe_cast<const MessageQueueGenericValueType<std::pair<std::string, std::string> > *>(data);
		if(msg)
		{
			Archive::put(target, msg->getValue());
		}
	}

	MessageQueue::Data * unpackStringString(Archive::ReadIterator & source)
	{
		std::pair<std::string, std::string> v;
		Archive::get(source, v);
		return new MessageQueueGenericValueType<std::pair<std::string, std::string> >(v);
	}

	void packIntStringString(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		const MessageQueueGenericValueType<std::pair<int, std::pair<std::string, std::string> > > * msg = safe_cast<const MessageQueueGenericValueType<std::pair<int, std::pair<std::string, std::string> > > *>(data);
		if(msg)
		{
			Archive::put(target, msg->getValue());
		}
	}

	MessageQueue::Data * unpackIntStringString(Archive::ReadIterator & source)
	{
		std::pair<int, std::pair<std::string, std::string> > v;
		Archive::get(source, v);
		return new MessageQueueGenericValueType<std::pair<int, std::pair<std::string, std::string> > >(v);
	}

	void packStringUnicodeString(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		const MessageQueueGenericValueType<std::pair<std::string, Unicode::String> > * msg = safe_cast<const MessageQueueGenericValueType<std::pair<std::string, Unicode::String> > *>(data);
		if(msg)
		{
			Archive::put(target, msg->getValue());
		}
	}

	MessageQueue::Data * unpackStringUnicodeString(Archive::ReadIterator & source)
	{
		std::pair<std::string, Unicode::String> v;
		Archive::get(source, v);
		return new MessageQueueGenericValueType<std::pair<std::string, Unicode::String> >(v);
	}

	void packStringUnicodeStringArray(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		const MessageQueueGenericValueType<std::pair<std::string, std::vector<Unicode::String> > > * msg = safe_cast<const MessageQueueGenericValueType<std::pair<std::string, std::vector<Unicode::String> > > *>(data);
		if(msg)
		{
			Archive::put(target, msg->getValue());
		}
	}

	MessageQueue::Data * unpackStringUnicodeStringArray(Archive::ReadIterator & source)
	{
		std::pair<std::string, std::vector<Unicode::String> > v;
		Archive::get(source, v);
		return new MessageQueueGenericValueType<std::pair<std::string, std::vector<Unicode::String> > >(v);
	}

	void packJediData(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		MessageQueueJediData::pack(data, target);
	}

	MessageQueue::Data * unpackJediData(Archive::ReadIterator & source)
	{
		return MessageQueueJediData::unpack(source);
	}

	void packJediLocation(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		MessageQueueJediLocation::pack(data, target);
	}

	MessageQueue::Data * unpackJediLocation(Archive::ReadIterator & source)
	{
		return MessageQueueJediLocation::unpack(source);
	}

	void packJediBounties(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		const MessageQueueGenericValueType<std::pair<NetworkId, std::vector<NetworkId> > > * msg = safe_cast<const MessageQueueGenericValueType<std::pair<NetworkId, std::vector<NetworkId> > > *>(data);
		if(msg)
		{
			Archive::put(target, msg->getValue());
		}
	}

	MessageQueue::Data * unpackJediBounties(Archive::ReadIterator & source)
	{
		std::pair<NetworkId, std::vector<NetworkId> > v;
		Archive::get(source, v);
		MessageQueueGenericValueType<std::pair<NetworkId, std::vector<NetworkId> > > * result = new MessageQueueGenericValueType<std::pair<NetworkId, std::vector<NetworkId> > >(v);
		return result;
	}
}

using namespace SetupSwgServerNetworkMessagesNamespace;

SetupSwgServerNetworkMessages::SetupSwgServerNetworkMessages()
{
}

SetupSwgServerNetworkMessages::~SetupSwgServerNetworkMessages()
{
}

void SetupSwgServerNetworkMessages::install ()
{
	SetupSwgServerNetworkMessages setup;
	setup.internalInstall();

	ExitChain::add (SetupSwgServerNetworkMessages::remove, "SetupSwgServerNetworkMessages");
}

// ======================================================================

void SetupSwgServerNetworkMessages::internalInstall ()
{
	DEBUG_FATAL(g_installed, ("SetupSwgServerNetworkMessages::install - is already installed"));

	SetupServerNetworkMessages::internalInstall();

	MessageQueueCombatDamageList::install();
	MessageQueueDirectDamage::install();
	MessageQueueInstallationHarvest::install();
	MessageQueueJediData::install();
	MessageQueueJediLocation::install();
	MessageQueuePlanetAddTravelPoint::install();
	MessageQueueRequestJediBounty::install();
	MessageQueueScriptTransferMoney::install();
	
	ControllerMessageFactory::registerControllerMessageHandler(CM_removeJedi, packNetworkId, unpackNetworkId);
	ControllerMessageFactory::registerControllerMessageHandler(CM_updateJedi, packJediData, unpackJediData);
	ControllerMessageFactory::registerControllerMessageHandler(CM_removeJediBounty, packNetworkIdNetworkId, unpackNetworkIdNetworkId);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setJediOffline, packJediLocation, unpackJediLocation);
	ControllerMessageFactory::registerControllerMessageHandler(CM_updateJediBounties, packJediBounties, unpackJediBounties);
	ControllerMessageFactory::registerControllerMessageHandler(CM_updateJediSpentJediSkillPoints, packNetworkIdInt, unpackNetworkIdInt);
	ControllerMessageFactory::registerControllerMessageHandler(CM_updateJediFaction, packNetworkIdInt, unpackNetworkIdInt);
	ControllerMessageFactory::registerControllerMessageHandler(CM_updateJediScriptData, packNetworkIdStringInt, unpackNetworkIdStringInt);
	ControllerMessageFactory::registerControllerMessageHandler(CM_removeJediScriptData, packNetworkIdString, unpackNetworkIdString);
	ControllerMessageFactory::registerControllerMessageHandler(CM_removeAllJediBounties, packNetworkId, unpackNetworkId);

	g_installed = true;
}

// ----------------------------------------------------------------------

void SetupSwgServerNetworkMessages::remove ( void )
{
	DEBUG_FATAL(!g_installed, ("SetupSwgServerNetworkMessages::remove - not already installed"));
	
	SetupSwgServerNetworkMessages setup;
	setup.internalRemove();
}

void SetupSwgServerNetworkMessages::internalRemove ( void )
{
	SetupServerNetworkMessages::internalRemove();
	g_installed = false;
}

// ======================================================================

