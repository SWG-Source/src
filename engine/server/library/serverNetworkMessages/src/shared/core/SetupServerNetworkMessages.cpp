// ======================================================================
//
// SetupServerNetworkMessages.cpp
// copyright 1998 Bootprint Entertainment
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/SetupServerNetworkMessages.h"

#include "StringId.h"
#include "localizationArchive/StringIdArchive.h"
#include "serverGame/CityInfo.h"
#include "serverNetworkMessages/AiMovementMessage.h"
#include "serverNetworkMessages/AiCreatureStateMessage.h"
#include "serverNetworkMessages/GroupMemberParam.h"
#include "serverNetworkMessages/GroupMemberParamArchive.h"
#include "serverNetworkMessages/MessageQueueAlterAttribute.h"
#include "serverNetworkMessages/MessageQueueCommandQueueEnqueueFwd.h"
#include "serverNetworkMessages/MessageQueueCommandQueueForceExecuteCommandFwd.h"
#include "serverNetworkMessages/MessageQueueGrantOfflineXp.h"
#include "serverNetworkMessages/MessageQueuePvpCommand.h"
#include "serverNetworkMessages/MessageQueueScriptTrigger.h"
#include "serverNetworkMessages/MessageQueueSetState.h"
#include "serverNetworkMessages/MessageQueueTeleportObject.h"
#include "serverUtility/LocationData.h"
#include "sharedFoundation/DynamicVariable.h"
#include "sharedFoundation/DynamicVariableLocationData.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedGame/AttribMod.h"
#include "sharedGame/AttribModArchive.h"
#include "sharedGame/LfgCharacterData.h"
#include "sharedUtility/Location.h"
#include "sharedGame/ProsePackage.h"
#include "sharedGame/ProsePackageArchive.h"
#include "sharedMathArchive/VectorArchive.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"
#include "sharedNetworkMessages/MapLocation.h"
#include "sharedNetworkMessages/MapLocationArchive.h"
#include "sharedNetworkMessages/MessageQueueDataTransformWithParent.h"
#include "swgSharedUtility/Attributes.def"

// ======================================================================

namespace SetupServerNetworkMessagesNamespace
{
	static bool g_installed = false;

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

	void packLocation(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		const MessageQueueGenericValueType<Location> * msg = safe_cast<const MessageQueueGenericValueType<Location> *>(data);
		if(msg)
		{
			Archive::put(target, msg->getValue());
		}
	}

	MessageQueue::Data * unpackLocation(Archive::ReadIterator & source)
	{
		Location location;
		Archive::get(source, location);
		MessageQueueGenericValueType<Location> * result = new MessageQueueGenericValueType<Location>(location);
		return result;
	}

	void packGroupMemberParam(const MessageQueue::Data *data, Archive::ByteStream &target)
	{
		const MessageQueueGenericValueType<GroupMemberParam> * msg = safe_cast<const MessageQueueGenericValueType<GroupMemberParam> *>(data);
		if(msg)
		{
			Archive::put(target, msg->getValue());
		}
	}
	MessageQueue::Data* unpackGroupMemberParam(Archive::ReadIterator &source)
	{
		GroupMemberParam v;
		Archive::get(source, v);
		MessageQueueGenericValueType<GroupMemberParam> * result = new MessageQueueGenericValueType<GroupMemberParam>(v);
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
		const MessageQueueGenericValueType<std::pair<MapLocation, std::pair<int, int> > > * const msg = safe_cast<const MessageQueueGenericValueType<std::pair<MapLocation, std::pair<int, int> > > *>(data);
		if (msg)
		{
			Archive::put (target, msg->getValue ());
		}
	}

	//----------------------------------------------------------------------

	MessageQueue::Data* unpackMapLocationTypePair(Archive::ReadIterator &source)
	{
		std::pair <MapLocation, std::pair<int, int> > v;
		Archive::get(source, v);
		MessageQueueGenericValueType<std::pair<MapLocation, std::pair<int, int> > > * result = new MessageQueueGenericValueType<std::pair<MapLocation, std::pair<int, int> > >(v);
		return result;
	}

	//----------------------------------------------------------------------

	void packStringId(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		const MessageQueueGenericValueType<StringId> * const msg = safe_cast<const MessageQueueGenericValueType<StringId> *>(data);
		if (msg)
		{
			Archive::put (target, msg->getValue ());
		}
	}

	MessageQueue::Data* unpackStringId(Archive::ReadIterator &source)
	{
		StringId v;
		Archive::get(source, v);
		MessageQueueGenericValueType<StringId> * result = new MessageQueueGenericValueType<StringId>(v);
		return result;
	}

	void packStringIdProsePackage(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		const MessageQueueGenericValueType<std::pair<StringId, ProsePackage> > * const msg = safe_cast<const MessageQueueGenericValueType<std::pair<StringId, ProsePackage> > *>(data);
		if (msg)
		{
			Archive::put (target, msg->getValue ());
		}
	}

	MessageQueue::Data* unpackStringIdProsePackage(Archive::ReadIterator & source)
	{
		std::pair<StringId, ProsePackage> v;
		Archive::get(source, v);
		MessageQueueGenericValueType<std::pair<StringId, ProsePackage> > * result = new MessageQueueGenericValueType<std::pair<StringId, ProsePackage> >(v);
		return result;
	}

	void packStringNetworkIdStringIdProsePackage(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		const MessageQueueGenericValueType<std::pair<std::string, std::pair<NetworkId, std::pair<StringId, ProsePackage> > > > * const msg = safe_cast<const MessageQueueGenericValueType<std::pair<std::string, std::pair<NetworkId, std::pair<StringId, ProsePackage> > > > *>(data);
		if (msg)
		{
			Archive::put (target, msg->getValue ());
		}
	}

	MessageQueue::Data* unpackStringNetworkIdStringIdProsePackage (Archive::ReadIterator & source)
	{
		std::pair<std::string, std::pair<NetworkId, std::pair<StringId, ProsePackage> > > v;
		Archive::get(source, v);
		MessageQueueGenericValueType<std::pair<std::string, std::pair<NetworkId, std::pair<StringId, ProsePackage> > > > * result = new MessageQueueGenericValueType<std::pair<std::string, std::pair<NetworkId, std::pair<StringId, ProsePackage> > > >(v);
		return result;
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

	void packStringVector(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		const MessageQueueGenericValueType<std::pair<std::string, Vector> > * msg = safe_cast<const MessageQueueGenericValueType<std::pair<std::string, Vector> > *>(data);
		if(msg)
		{
			Archive::put(target, msg->getValue());
		}
	}

	MessageQueue::Data * unpackStringVector(Archive::ReadIterator & source)
	{
		std::pair<std::string, Vector> v;
		Archive::get(source, v);
		return new MessageQueueGenericValueType<std::pair<std::string, Vector> >(v);
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

	void packStringInt64(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		const MessageQueueGenericValueType<std::pair<std::string, int64> > * msg = safe_cast<const MessageQueueGenericValueType<std::pair<std::string, int64> > *>(data);
		if(msg)
		{
			Archive::put(target, msg->getValue());
		}
	}

	MessageQueue::Data * unpackStringInt64(Archive::ReadIterator & source)
	{
		std::pair<std::string, int64> v;
		Archive::get(source, v);
		return new MessageQueueGenericValueType<std::pair<std::string, int64> >(v);
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

	void packUint32Bool(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		const MessageQueueGenericValueType<std::pair<uint32, bool> > * msg = safe_cast<const MessageQueueGenericValueType<std::pair<uint32, bool> > *>(data);
		if(msg)
		{
			Archive::put(target, msg->getValue());
		}
	}

	MessageQueue::Data * unpackUint32Bool(Archive::ReadIterator & source)
	{
		std::pair<uint32, bool> v;
		Archive::get(source, v);
		return new MessageQueueGenericValueType<std::pair<uint32, bool> >(v);
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

	void packUnicodeStringNetworkId(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		const MessageQueueGenericValueType<std::pair<Unicode::String, NetworkId> > * msg = safe_cast<const MessageQueueGenericValueType<std::pair<Unicode::String, NetworkId> > *>(data);
		if(msg)
		{
			Archive::put(target, msg->getValue());
		}
	}

	MessageQueue::Data * unpackUnicodeStringNetworkId(Archive::ReadIterator & source)
	{
		std::pair<Unicode::String, NetworkId> v;
		Archive::get(source, v);
		return new MessageQueueGenericValueType<std::pair<Unicode::String, NetworkId> >(v);
	}

	void packStringNetworkIdInt(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		const MessageQueueGenericValueType<std::pair<std::string, std::pair<NetworkId, int> > > * msg = safe_cast<const MessageQueueGenericValueType<std::pair<std::string, std::pair<NetworkId, int> > > *>(data);
		if(msg)
		{
			Archive::put(target, msg->getValue());
		}
	}

	MessageQueue::Data * unpackStringNetworkIdInt(Archive::ReadIterator & source)
	{
		std::pair<std::string, std::pair<NetworkId, int> > v;
		Archive::get(source, v);
		return new MessageQueueGenericValueType<std::pair<std::string, std::pair<NetworkId, int> > >(v);
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

	void packIntString(const MessageQueue::Data *data, Archive::ByteStream &target)
	{
		MessageQueueGenericValueType<std::pair<int, std::string> > const *msg = safe_cast<MessageQueueGenericValueType<std::pair<int, std::string> > const *>(data);
		if (msg)
		{
			Archive::put(target, msg->getValue());
		}
	}

	MessageQueue::Data *unpackIntString(Archive::ReadIterator &source)
	{
		std::pair<int, std::string> v;
		Archive::get(source, v);
		return new MessageQueueGenericValueType<std::pair<int, std::string> >(v);
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

	void packStringNetworkIdUnicodeStringString(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		const MessageQueueGenericValueType<std::pair<std::pair<std::string, NetworkId>, std::pair<Unicode::String, std::string> > > * msg = safe_cast<const MessageQueueGenericValueType<std::pair<std::pair<std::string, NetworkId>, std::pair<Unicode::String, std::string> > > *>(data);
		if(msg)
		{
			Archive::put(target, msg->getValue());
		}
	}

	MessageQueue::Data * unpackStringNetworkIdUnicodeStringString(Archive::ReadIterator & source)
	{
		std::pair<std::pair<std::string, NetworkId>, std::pair<Unicode::String, std::string> > v;
		Archive::get(source, v);
		return new MessageQueueGenericValueType<std::pair<std::pair<std::string, NetworkId>, std::pair<Unicode::String, std::string> > >(v);
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

	void packIntInt(MessageQueue::Data const *data, Archive::ByteStream &target)
	{
		MessageQueueGenericValueType<std::pair<int, int> > const *msg = safe_cast<MessageQueueGenericValueType<std::pair<int, int> > const *>(data);
		if (msg)
			Archive::put(target, msg->getValue());
	}

	MessageQueue::Data *unpackIntInt(Archive::ReadIterator &source)
	{
		std::pair<int, int> v;
		Archive::get(source, v);
		return new MessageQueueGenericValueType<std::pair<int, int> >(v);
	}

	void packIntUint32Bool(MessageQueue::Data const *data, Archive::ByteStream &target)
	{
		MessageQueueGenericValueType<std::pair<int, std::pair<uint32, bool> > > const *msg = safe_cast<MessageQueueGenericValueType<std::pair<int, std::pair<uint32, bool> > > const *>(data);
		if (msg)
			Archive::put(target, msg->getValue());
	}

	MessageQueue::Data *unpackIntUint32Bool(Archive::ReadIterator &source)
	{
		std::pair<int, std::pair<uint32, bool> > v;
		Archive::get(source, v);
		return new MessageQueueGenericValueType<std::pair<int, std::pair<uint32, bool> > >(v);
	}

	void packIntIntInt(MessageQueue::Data const *data, Archive::ByteStream &target)
	{
		MessageQueueGenericValueType<std::pair<std::pair<int, int>, int> > const *msg = safe_cast<MessageQueueGenericValueType<std::pair<std::pair<int, int>, int> > const *>(data);
		if (msg)
			Archive::put(target, msg->getValue());
	}

	MessageQueue::Data *unpackIntIntInt(Archive::ReadIterator &source)
	{
		std::pair<std::pair<int, int>, int> v;
		Archive::get(source, v);
		return new MessageQueueGenericValueType<std::pair<std::pair<int, int>, int> >(v);
	}

	void packIntIntIntInt(MessageQueue::Data const *data, Archive::ByteStream &target)
	{
		MessageQueueGenericValueType<std::pair<std::pair<int, int>, std::pair<int, int> > > const *msg = safe_cast<MessageQueueGenericValueType<std::pair<std::pair<int, int>, std::pair<int, int> > > const *>(data);
		if (msg)
			Archive::put(target, msg->getValue());
	}

	MessageQueue::Data *unpackIntIntIntInt(Archive::ReadIterator &source)
	{
		std::pair<std::pair<int, int>, std::pair<int, int> > v;
		Archive::get(source, v);
		return new MessageQueueGenericValueType<std::pair<std::pair<int, int>, std::pair<int, int> > >(v);
	}

	void packInt32Int32(MessageQueue::Data const *data, Archive::ByteStream &target)
	{
		MessageQueueGenericValueType<std::pair<int32, int32> > const *msg = safe_cast<MessageQueueGenericValueType<std::pair<int32, int32> > const *>(data);
		if (msg)
			Archive::put(target, msg->getValue());
	}

	MessageQueue::Data *unpackInt32Int32(Archive::ReadIterator &source)
	{
		std::pair<int32, int32> v;
		Archive::get(source, v);
		return new MessageQueueGenericValueType<std::pair<int32, int32> >(v);
	}

	void packIntFloat(MessageQueue::Data const *data, Archive::ByteStream &target)
	{
		MessageQueueGenericValueType<std::pair<int, float> > const *msg = safe_cast<MessageQueueGenericValueType<std::pair<int, float> > const *>(data);
		if (msg)
			Archive::put(target, msg->getValue());
	}

	MessageQueue::Data *unpackIntFloat(Archive::ReadIterator &source)
	{
		std::pair<int, float> v;
		Archive::get(source, v);
		return new MessageQueueGenericValueType<std::pair<int, float> >(v);
	}

	void packIntFloatFloat(MessageQueue::Data const *data, Archive::ByteStream &target)
	{
		MessageQueueGenericValueType<std::pair<int, std::pair<float, float> > > const *msg = safe_cast<MessageQueueGenericValueType<std::pair<int, std::pair<float, float> > > const *>(data);
		if (msg)
			Archive::put(target, msg->getValue());
	}

	MessageQueue::Data *unpackIntFloatFloat(Archive::ReadIterator &source)
	{
		std::pair<int, std::pair<float, float> > v;
		Archive::get(source, v);
		return new MessageQueueGenericValueType<std::pair<int, std::pair<float, float> > >(v);
	}

	void packIntFloatFloatFloat(MessageQueue::Data const *data, Archive::ByteStream &target)
	{
		MessageQueueGenericValueType<std::pair<int, std::pair<float, std::pair<float, float> > > > const *msg = safe_cast<MessageQueueGenericValueType<std::pair<int, std::pair<float, std::pair<float, float> > > > const *>(data);
		if (msg)
			Archive::put(target, msg->getValue());
	}

	MessageQueue::Data *unpackIntFloatFloatFloat(Archive::ReadIterator &source)
	{
		std::pair<int, std::pair<float, std::pair<float, float> > > v;
		Archive::get(source, v);
		return new MessageQueueGenericValueType<std::pair<int, std::pair<float, std::pair<float, float> > > >(v);
	}

	void packIntFloatFloatFloatNetworkId(MessageQueue::Data const *data, Archive::ByteStream &target)
	{
		MessageQueueGenericValueType<std::pair<int, std::pair<float, std::pair<float, std::pair< float, NetworkId > > > > > const *msg = safe_cast<MessageQueueGenericValueType<std::pair<int, std::pair<float, std::pair<float, std::pair< float, NetworkId > > > > > const *>(data);
		if (msg)
			Archive::put(target, msg->getValue());
	}

	MessageQueue::Data *unpackIntFloatFloatFloatNetworkId(Archive::ReadIterator &source)
	{
		std::pair<int, std::pair<float, std::pair<float, std::pair< float, NetworkId > > > > v;
		Archive::get(source, v);
		return new MessageQueueGenericValueType<std::pair<int, std::pair<float, std::pair<float, std::pair< float, NetworkId > > > > >(v);
	}

	void packIntStringIntInt(MessageQueue::Data const *data, Archive::ByteStream &target)
	{
		MessageQueueGenericValueType<std::pair<int, std::pair<std::string, std::pair<int, int> > > > const *msg = safe_cast<MessageQueueGenericValueType<std::pair<int, std::pair<std::string, std::pair<int, int> > > > const *>(data);
		if (msg)
			Archive::put(target, msg->getValue());
	}

	MessageQueue::Data *unpackIntStringIntInt(Archive::ReadIterator &source)
	{
		std::pair<int, std::pair<std::string, std::pair<int, int> > > v;
		Archive::get(source, v);
		return new MessageQueueGenericValueType<std::pair<int, std::pair<std::string, std::pair<int, int> > > >(v);
	}

	void packIntVectorIntBool(MessageQueue::Data const *data, Archive::ByteStream &target)
	{
		MessageQueueGenericValueType<std::pair<int, std::pair<Vector, std::pair<int, bool> > > > const *msg = safe_cast<MessageQueueGenericValueType<std::pair<int, std::pair<Vector, std::pair<int, bool> > > > const *>(data);
		if (msg)
			Archive::put(target, msg->getValue());
	}

	MessageQueue::Data *unpackIntVectorIntBool(Archive::ReadIterator &source)
	{
		std::pair<int, std::pair<Vector, std::pair<int, bool> > > v;
		Archive::get(source, v);
		return new MessageQueueGenericValueType<std::pair<int, std::pair<Vector, std::pair<int, bool> > > >(v);
	}

	void packIntVectorVectorNetworkIdNetworkId(MessageQueue::Data const *data, Archive::ByteStream &target)
	{
		MessageQueueGenericValueType<std::pair<int, std::pair<Vector, std::pair<Vector, std::pair<NetworkId, NetworkId> > > > > const *msg = safe_cast<MessageQueueGenericValueType<std::pair<int, std::pair<Vector, std::pair<Vector, std::pair<NetworkId, NetworkId> > > > > const *>(data);
		if (msg)
			Archive::put(target, msg->getValue());
	}

	MessageQueue::Data *unpackIntVectorVectorNetworkIdNetworkId(Archive::ReadIterator &source)
	{
		std::pair<int, std::pair<Vector, std::pair<Vector, std::pair<NetworkId, NetworkId> > > > v;
		Archive::get(source, v);
		return new MessageQueueGenericValueType<std::pair<int, std::pair<Vector, std::pair<Vector, std::pair<NetworkId, NetworkId> > > > >(v);
	}

	void packAttribMod(MessageQueue::Data const *data, Archive::ByteStream &target)
	{
		MessageQueueGenericValueType<AttribMod::AttribMod> const *msg = safe_cast<MessageQueueGenericValueType<AttribMod::AttribMod> const *>(data);
		if (msg)
			Archive::put(target, msg->getValue());
	}

	MessageQueue::Data *unpackAttribMod(Archive::ReadIterator &source)
	{
		AttribMod::AttribMod v;
		Archive::get(source, v);
		return new MessageQueueGenericValueType<AttribMod::AttribMod>(v);
	}

	void packIntIntNetworkId(MessageQueue::Data const *data, Archive::ByteStream &target)
	{
		MessageQueueGenericValueType<std::pair<std::pair<int, int>, NetworkId> > const * msg = safe_cast<MessageQueueGenericValueType<std::pair<std::pair<int, int>, NetworkId> > const *>(data);
		if (msg)
			Archive::put(target, msg->getValue());
	}

	MessageQueue::Data *unpackIntIntNetworkId(Archive::ReadIterator &source)
	{
		std::pair<std::pair<int, int>, NetworkId> v;
		Archive::get(source, v);
		return new MessageQueueGenericValueType<std::pair<std::pair<int, int>, NetworkId> >(v);
	}

	void packNetworkIdIntFloat(const MessageQueue::Data * data, Archive::ByteStream & target)
	{
		const MessageQueueGenericValueType<std::pair<NetworkId, std::pair<int, float> > > * msg = safe_cast<const MessageQueueGenericValueType<std::pair<NetworkId, std::pair<int, float> > > *>(data);
		if(msg)
		{
			Archive::put(target, msg->getValue());
		}
	}

	MessageQueue::Data * unpackNetworkIdIntFloat(Archive::ReadIterator & source)
	{
		std::pair<NetworkId, std::pair<int, float> > v;
		Archive::get(source, v);
		return new MessageQueueGenericValueType<std::pair<NetworkId, std::pair<int, float> > >(v);
	}

	void packNetworkIdLfgCharacterData(const MessageQueue::Data *data, Archive::ByteStream &target)
	{
		const MessageQueueGenericValueType<std::pair<NetworkId, LfgCharacterData> > * msg = safe_cast<const MessageQueueGenericValueType<std::pair<NetworkId, LfgCharacterData> > *>(data);
		if(msg)
		{
			Archive::put(target, msg->getValue());
		}
	}
	MessageQueue::Data* unpackNetworkIdLfgCharacterData(Archive::ReadIterator &source)
	{
		std::pair<NetworkId, LfgCharacterData> v;
		Archive::get(source, v);
		return new MessageQueueGenericValueType<std::pair<NetworkId, LfgCharacterData> >(v);
	}

	void packNetworkIdBitArray(const MessageQueue::Data *data, Archive::ByteStream &target)
	{
		const MessageQueueGenericValueType<std::pair<NetworkId, BitArray> > * msg = safe_cast<const MessageQueueGenericValueType<std::pair<NetworkId, BitArray> > *>(data);
		if(msg)
		{
			Archive::put(target, msg->getValue());
		}
	}
	MessageQueue::Data* unpackNetworkIdBitArray(Archive::ReadIterator &source)
	{
		std::pair<NetworkId, BitArray> v;
		Archive::get(source, v);
		return new MessageQueueGenericValueType<std::pair<NetworkId, BitArray> >(v);
	}

	void packIntNetworkIdStringInt(const MessageQueue::Data *data, Archive::ByteStream &target)
	{
		const MessageQueueGenericValueType<std::pair<std::pair<int, NetworkId>, std::pair<std::string, int> > > * msg = safe_cast<const MessageQueueGenericValueType<std::pair<std::pair<int, NetworkId>, std::pair<std::string, int> > > *>(data);
		if(msg)
		{
			Archive::put(target, msg->getValue());
		}
	}
	MessageQueue::Data* unpackIntNetworkIdStringInt(Archive::ReadIterator &source)
	{
		std::pair<std::pair<int, NetworkId>, std::pair<std::string, int> > v;
		Archive::get(source, v);
		return new MessageQueueGenericValueType<std::pair<std::pair<int, NetworkId>, std::pair<std::string, int> > >(v);
	}

	void packIntNetworkIdStringStringInt(const MessageQueue::Data *data, Archive::ByteStream &target)
	{
		const MessageQueueGenericValueType<std::pair<std::pair<int, NetworkId>, std::pair<std::string, std::pair<std::string, int> > > > * msg = safe_cast<const MessageQueueGenericValueType<std::pair<std::pair<int, NetworkId>, std::pair<std::string, std::pair<std::string, int> > > > *>(data);
		if(msg)
		{
			Archive::put(target, msg->getValue());
		}
	}
	MessageQueue::Data* unpackIntNetworkIdStringStringInt(Archive::ReadIterator &source)
	{
		std::pair<std::pair<int, NetworkId>, std::pair<std::string, std::pair<std::string, int> > > v;
		Archive::get(source, v);
		return new MessageQueueGenericValueType<std::pair<std::pair<int, NetworkId>, std::pair<std::string, std::pair<std::string, int> > > >(v);
	}

	void packIntNetworkIdNetworkId(const MessageQueue::Data *data, Archive::ByteStream &target)
	{
		const MessageQueueGenericValueType<std::pair<std::pair<int, NetworkId>, NetworkId> > * msg = safe_cast<const MessageQueueGenericValueType<std::pair<std::pair<int, NetworkId>, NetworkId> > *>(data);
		if(msg)
		{
			Archive::put(target, msg->getValue());
		}
	}
	MessageQueue::Data* unpackIntNetworkIdNetworkId(Archive::ReadIterator &source)
	{
		std::pair<std::pair<int, NetworkId>, NetworkId> v;
		Archive::get(source, v);
		return new MessageQueueGenericValueType<std::pair<std::pair<int, NetworkId>, NetworkId> >(v);
	}

	void packIntNetworkIdIntNetworkId(const MessageQueue::Data *data, Archive::ByteStream &target)
	{
		const MessageQueueGenericValueType<std::pair<std::pair<int, NetworkId>, std::pair<int, NetworkId> > > * msg = safe_cast<const MessageQueueGenericValueType<std::pair<std::pair<int, NetworkId>, std::pair<int, NetworkId> > > *>(data);
		if(msg)
		{
			Archive::put(target, msg->getValue());
		}
	}
	MessageQueue::Data* unpackIntNetworkIdIntNetworkId(Archive::ReadIterator &source)
	{
		std::pair<std::pair<int, NetworkId>, std::pair<int, NetworkId> > v;
		Archive::get(source, v);
		return new MessageQueueGenericValueType<std::pair<std::pair<int, NetworkId>, std::pair<int, NetworkId> > >(v);
	}

	void packMapStringInt64(const MessageQueue::Data *data, Archive::ByteStream &target)
	{
		const MessageQueueGenericValueType<std::map<std::string, int64> > * msg = safe_cast<const MessageQueueGenericValueType<std::map<std::string, int64> > *>(data);
		if(msg)
		{
			Archive::put(target, msg->getValue());
		}
	}
	MessageQueue::Data* unpackMapStringInt64(Archive::ReadIterator &source)
	{
		std::map<std::string, int64> v;
		Archive::get(source, v);
		return new MessageQueueGenericValueType<std::map<std::string, int64> >(v);
	}

	void packIntCityInfo(const MessageQueue::Data *data, Archive::ByteStream &target)
	{
		const MessageQueueGenericValueType<std::pair<int, CityInfo> > * msg = safe_cast<const MessageQueueGenericValueType<std::pair<int, CityInfo> > *>(data);
		if(msg)
		{
			Archive::put(target, msg->getValue());
		}
	}
	MessageQueue::Data* unpackIntCityInfo(Archive::ReadIterator &source)
	{
		std::pair<int, CityInfo> v;
		Archive::get(source, v);
		return new MessageQueueGenericValueType<std::pair<int, CityInfo> >(v);
	}

	void packIntNetworkIdStringNetworkIdInt(const MessageQueue::Data *data, Archive::ByteStream &target)
	{
		const MessageQueueGenericValueType<std::pair<std::pair<int, NetworkId>, std::pair<std::string, std::pair<NetworkId, int> > > > * msg = safe_cast<const MessageQueueGenericValueType<std::pair<std::pair<int, NetworkId>, std::pair<std::string, std::pair<NetworkId, int> > > > *>(data);
		if(msg)
		{
			Archive::put(target, msg->getValue());
		}
	}
	MessageQueue::Data* unpackIntNetworkIdStringNetworkIdInt(Archive::ReadIterator &source)
	{
		std::pair<std::pair<int, NetworkId>, std::pair<std::string, std::pair<NetworkId, int> > > v;
		Archive::get(source, v);
		return new MessageQueueGenericValueType<std::pair<std::pair<int, NetworkId>, std::pair<std::string, std::pair<NetworkId, int> > > >(v);
	}

	void packIntNetworkIdIntBool(const MessageQueue::Data *data, Archive::ByteStream &target)
	{
		const MessageQueueGenericValueType<std::pair<std::pair<int, NetworkId>, std::pair<int, bool> > > * msg = safe_cast<const MessageQueueGenericValueType<std::pair<std::pair<int, NetworkId>, std::pair<int, bool> > > *>(data);
		if(msg)
		{
			Archive::put(target, msg->getValue());
		}
	}
	MessageQueue::Data* unpackIntNetworkIdIntBool(Archive::ReadIterator &source)
	{
		std::pair<std::pair<int, NetworkId>, std::pair<int, bool> > v;
		Archive::get(source, v);
		return new MessageQueueGenericValueType<std::pair<std::pair<int, NetworkId>, std::pair<int, bool> > >(v);
	}

	void packIntStringIntBool(const MessageQueue::Data *data, Archive::ByteStream &target)
	{
		const MessageQueueGenericValueType<std::pair<std::pair<int, std::string>, std::pair<int, bool> > > * msg = safe_cast<const MessageQueueGenericValueType<std::pair<std::pair<int, std::string>, std::pair<int, bool> > > *>(data);
		if(msg)
		{
			Archive::put(target, msg->getValue());
		}
	}
	MessageQueue::Data* unpackIntStringIntBool(Archive::ReadIterator &source)
	{
		std::pair<std::pair<int, std::string>, std::pair<int, bool> > v;
		Archive::get(source, v);
		return new MessageQueueGenericValueType<std::pair<std::pair<int, std::string>, std::pair<int, bool> > >(v);
	}
}

using namespace SetupServerNetworkMessagesNamespace;

SetupServerNetworkMessages::SetupServerNetworkMessages ()
{
}

SetupServerNetworkMessages::~SetupServerNetworkMessages ()
{
}

void SetupServerNetworkMessages::install ()
{
	SetupServerNetworkMessages setup;
	setup.internalInstall();

	ExitChain::add (SetupServerNetworkMessages::remove, "SetupServerNetworkMessages");
}

// ======================================================================

void SetupServerNetworkMessages::internalInstall ()
{
	DEBUG_FATAL(g_installed, ("SetupServerNetworkMessages::install - is already installed"));

	AiMovementMessage::install();
	AiCreatureStateMessage::install();
	MessageQueueAlterAttribute::install();
	MessageQueueCommandQueueEnqueueFwd::install();
	MessageQueueCommandQueueForceExecuteCommandFwd::install();
	MessageQueueGrantOfflineXp::install();
	MessageQueuePvpCommand::install();
	MessageQueueScriptTrigger::install();
	MessageQueueSetState::install();
	MessageQueueTeleportObject::install();
	
	ControllerMessageFactory::registerControllerMessageHandler(CM_groupSetName, packString, unpackString);
	ControllerMessageFactory::registerControllerMessageHandler(CM_groupAddMember, packGroupMemberParam, unpackGroupMemberParam);
	ControllerMessageFactory::registerControllerMessageHandler(CM_groupRemoveMember, packNetworkId, unpackNetworkId);
	ControllerMessageFactory::registerControllerMessageHandler(CM_groupDisband, packNothing, unpackNothing);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setGroupInviter, packStringNetworkIdNetworkId, unpackStringNetworkIdNetworkId);
	ControllerMessageFactory::registerControllerMessageHandler(CM_groupMakeLeader, packNetworkId, unpackNetworkId);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setPerformanceType, packInt, unpackInt);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setCacheVersion, packInt, unpackInt);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setPerformanceListenTarget, packNetworkId, unpackNetworkId);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setPerformanceWatchTarget, packNetworkId, unpackNetworkId);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setPerformanceStartTime, packInt, unpackInt);
	ControllerMessageFactory::registerControllerMessageHandler(CM_addNpcConversationResponse, packStringIdProsePackage, unpackStringIdProsePackage);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setObjectNameStringId, packStringId, unpackStringId);
	ControllerMessageFactory::registerControllerMessageHandler(CM_removeNpcConversationResponse, packStringIdProsePackage, unpackStringIdProsePackage);
	ControllerMessageFactory::registerControllerMessageHandler(CM_sendNpcConversationResponses, packNothing, unpackNothing);
	ControllerMessageFactory::registerControllerMessageHandler(CM_persistObject, packNothing, unpackNothing);
	ControllerMessageFactory::registerControllerMessageHandler(CM_playerResponseToNpcConversation, packStringNetworkIdStringIdProsePackage, unpackStringNetworkIdStringIdProsePackage);
	ControllerMessageFactory::registerControllerMessageHandler(CM_transferAuthority, packUint32Bool, unpackUint32Bool);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setObjectName, packUnicodeString, unpackUnicodeString);
	ControllerMessageFactory::registerControllerMessageHandler(CM_updatePositionOnPlanetServer, packBool, unpackBool);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setPlayerControlled, packBool, unpackBool);
	ControllerMessageFactory::registerControllerMessageHandler(CM_customize, packStringInt, unpackStringInt);
	ControllerMessageFactory::registerControllerMessageHandler(CM_permanentlyDestroy, packInt, unpackInt);
	ControllerMessageFactory::registerControllerMessageHandler(CM_addLocationTarget, packLocationData, unpackLocationData);
	ControllerMessageFactory::registerControllerMessageHandler(CM_removeLocationTarget, packString, unpackString);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setInCombat, packBool, unpackBool);
	ControllerMessageFactory::registerControllerMessageHandler(CM_addAim, packNothing, unpackNothing);
	ControllerMessageFactory::registerControllerMessageHandler(CM_clearAims, packNothing, unpackNothing);
	ControllerMessageFactory::registerControllerMessageHandler(CM_alterHitPoints, packIntNetworkId, unpackIntNetworkId);
	ControllerMessageFactory::registerControllerMessageHandler(CM_addResource, packNetworkIdFloat, unpackNetworkIdFloat);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setCombatTarget, packNetworkId, unpackNetworkId);
	ControllerMessageFactory::registerControllerMessageHandler(CM_removeCombatTarget, packNetworkId, unpackNetworkId);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setCraftedId, packNetworkId, unpackNetworkId);
	ControllerMessageFactory::registerControllerMessageHandler(CM_startCraftingSession, packNetworkId, unpackNetworkId);
	ControllerMessageFactory::registerControllerMessageHandler(CM_stopCraftingSession, packNothing, unpackNothing);
	ControllerMessageFactory::registerControllerMessageHandler(CM_addIngredientToHopper, packNetworkIdNetworkId, unpackNetworkIdNetworkId);
	ControllerMessageFactory::registerControllerMessageHandler(CM_removeIngredientFromHopper, packNetworkId, unpackNetworkId);
	ControllerMessageFactory::registerControllerMessageHandler(CM_addObjectToOutputSlot, packNetworkIdNetworkId, unpackNetworkIdNetworkId);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setCraftingManufactureSchematic, packNetworkId, unpackNetworkId);
	ControllerMessageFactory::registerControllerMessageHandler(CM_clearCraftingManufactureSchematic, packNothing, unpackNothing);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setCraftingPrototype, packNetworkId, unpackNetworkId);
	ControllerMessageFactory::registerControllerMessageHandler(CM_clearCraftingPrototype, packNothing, unpackNothing);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setVisibleComponents, packIntArray, unpackIntArray);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setIsShield, packBool, unpackBool);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setAmmo, packNetworkId, unpackNetworkId);
	ControllerMessageFactory::registerControllerMessageHandler(CM_addAllowed, packString, unpackString);
	ControllerMessageFactory::registerControllerMessageHandler(CM_removeAllowed, packString, unpackString);
	ControllerMessageFactory::registerControllerMessageHandler(CM_addBanned, packString, unpackString);
	ControllerMessageFactory::registerControllerMessageHandler(CM_removeBanned, packString, unpackString);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setDefaultWeapon, packString, unpackString);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setModValue, packStringInt, unpackStringInt);
	ControllerMessageFactory::registerControllerMessageHandler(CM_grantCommand, packString, unpackString);
	ControllerMessageFactory::registerControllerMessageHandler(CM_grantExperiencePoints, packStringInt, unpackStringInt);
	ControllerMessageFactory::registerControllerMessageHandler(CM_grantSkill, packString, unpackString);
	ControllerMessageFactory::registerControllerMessageHandler(CM_revokeCommand, packString, unpackString);
	ControllerMessageFactory::registerControllerMessageHandler(CM_revokeSkill, packString, unpackString);
	ControllerMessageFactory::registerControllerMessageHandler(CM_addAttributeModifier, packAttribMod, unpackAttribMod);
	ControllerMessageFactory::registerControllerMessageHandler(CM_removeAttributeModifiers, packAttribute, unpackAttribute);
	ControllerMessageFactory::registerControllerMessageHandler(CM_removeAllAttributeModifiers, packNothing, unpackNothing);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setCombatTargets, packNetworkIdArray, unpackNetworkIdArray);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setCurrentWeapon, packNetworkId, unpackNetworkId);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setGroup, packBoolNetworkId, unpackBoolNetworkId);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setMood, packUnsignedLong, unpackUnsignedLong);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setIncapacitated, packBoolNetworkId, unpackBoolNetworkId);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setSayMode, packUnsignedLong, unpackUnsignedLong);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setAnimationMood, packString, unpackString);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setScaleFactor, packFloat, unpackFloat);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setShockWounds, packInt, unpackInt);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setLookAtTarget, packNetworkId, unpackNetworkId);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setIntendedTarget, packNetworkId, unpackNetworkId);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setMovementStationary, packNothing, unpackNothing);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setMovementWalk, packNothing, unpackNothing);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setMovementRun, packNothing, unpackNothing);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setSlopeModAngle, packFloat, unpackFloat);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setSlopeModPercent, packFloat, unpackFloat);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setWaterModPercent, packFloat, unpackFloat);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setMovementPercent, packFloat, unpackFloat);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setTurnPercent, packFloat, unpackFloat);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setAccelPercent, packFloat, unpackFloat);
	ControllerMessageFactory::registerControllerMessageHandler(CM_factoryAddObject, packNetworkId, unpackNetworkId);
	ControllerMessageFactory::registerControllerMessageHandler(CM_addSchematic, packNetworkIdNetworkId, unpackNetworkIdNetworkId);
	ControllerMessageFactory::registerControllerMessageHandler(CM_manufactureInstallationCreateObject, packNothing, unpackNothing);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setBattlefieldParticipant, packNetworkIdUint32, unpackNetworkIdUint32);
	ControllerMessageFactory::registerControllerMessageHandler(CM_clearBattlefieldParticipants, packNothing, unpackNothing);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setIntDynamicVariable, packStringInt, unpackStringInt);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setIntArrayDynamicVariable, packStringIntArray, unpackStringIntArray);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setFloatDynamicVariable, packStringFloat, unpackStringFloat);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setFloatArrayDynamicVariable, packStringFloatArray, unpackStringFloatArray);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setStringDynamicVariable, packStringUnicodeString, unpackStringUnicodeString);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setStringArrayDynamicVariable, packStringUnicodeStringArray, unpackStringUnicodeStringArray);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setObjIdDynamicVariable, packStringNetworkId, unpackStringNetworkId);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setObjIdArrayDynamicVariable, packStringNetworkIdArray, unpackStringNetworkIdArray);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setLocationDynamicVariable, packDynamicVariableLocation, unpackDynamicVariableLocation);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setLocationArrayDynamicVariable, packDynamicVariableLocationArray, unpackDynamicVariableLocationArray);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setStringIdDynamicVariable, packStringStringId, unpackStringStringId);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setStringIdArrayDynamicVariable, packStringStringIdArray, unpackStringStringIdArray);
	ControllerMessageFactory::registerControllerMessageHandler(CM_copyDynamicVariable, packStringNetworkId, unpackStringNetworkId);
	ControllerMessageFactory::registerControllerMessageHandler(CM_copyDynamicVariableToSubList, packStringStringNetworkId, unpackStringStringNetworkId);
	ControllerMessageFactory::registerControllerMessageHandler(CM_addNestedObjVarList, packString, unpackString);
	ControllerMessageFactory::registerControllerMessageHandler(CM_removeObjVarItem, packString, unpackString);
	ControllerMessageFactory::registerControllerMessageHandler(CM_eraseObjVarListEntries, packString, unpackString);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setObjVarItemVar, packString, unpackString);
	ControllerMessageFactory::registerControllerMessageHandler(CM_guildCreate, packIntStringString, unpackIntStringString);
	ControllerMessageFactory::registerControllerMessageHandler(CM_guildRemoveMember, packIntNetworkId, unpackIntNetworkId);
	ControllerMessageFactory::registerControllerMessageHandler(CM_guildAddCreatorMember, packIntNetworkIdStringStringInt, unpackIntNetworkIdStringStringInt);
	ControllerMessageFactory::registerControllerMessageHandler(CM_guildAddSponsorMember, packIntNetworkIdStringStringInt, unpackIntNetworkIdStringStringInt);
	ControllerMessageFactory::registerControllerMessageHandler(CM_guildSetMemberPermission, packIntIntNetworkId, unpackIntIntNetworkId);
	ControllerMessageFactory::registerControllerMessageHandler(CM_guildSetMemberTitle, packStringNetworkIdInt, unpackStringNetworkIdInt);
	ControllerMessageFactory::registerControllerMessageHandler(CM_guildSetMemberAllegiance, packIntNetworkIdNetworkId, unpackIntNetworkIdNetworkId);
	ControllerMessageFactory::registerControllerMessageHandler(CM_guildSetMemberPermissionAndAllegiance, packIntNetworkIdIntNetworkId, unpackIntNetworkIdIntNetworkId);
	ControllerMessageFactory::registerControllerMessageHandler(CM_guildSetMemberNameAndPermission, packIntNetworkIdStringInt, unpackIntNetworkIdStringInt);
	ControllerMessageFactory::registerControllerMessageHandler(CM_guildSetMemberProfessionInfo, packIntNetworkIdStringInt, unpackIntNetworkIdStringInt);
	ControllerMessageFactory::registerControllerMessageHandler(CM_guildAddMemberRank, packStringNetworkIdInt, unpackStringNetworkIdInt);
	ControllerMessageFactory::registerControllerMessageHandler(CM_guildRemoveMemberRank, packStringNetworkIdInt, unpackStringNetworkIdInt);
	ControllerMessageFactory::registerControllerMessageHandler(CM_guildRemoveEnemy, packIntInt, unpackIntInt);
	ControllerMessageFactory::registerControllerMessageHandler(CM_guildSetEnemy, packIntInt, unpackIntInt);
	ControllerMessageFactory::registerControllerMessageHandler(CM_guildDisband, packInt, unpackInt);
	ControllerMessageFactory::registerControllerMessageHandler(CM_guildSetLeader, packIntNetworkId, unpackIntNetworkId);
	ControllerMessageFactory::registerControllerMessageHandler(CM_guildSetGuildElectionEndTime, packIntIntInt, unpackIntIntInt);
	ControllerMessageFactory::registerControllerMessageHandler(CM_guildSetGuildFaction, packIntInt, unpackIntInt);
	ControllerMessageFactory::registerControllerMessageHandler(CM_guildSetGuildGcwDefenderRegion, packIntString, unpackIntString);
	ControllerMessageFactory::registerControllerMessageHandler(CM_guildSetName, packIntString, unpackIntString);
	ControllerMessageFactory::registerControllerMessageHandler(CM_guildSetAbbrev, packIntString, unpackIntString);
	ControllerMessageFactory::registerControllerMessageHandler(CM_initDecay, packNothing, unpackNothing);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setDecay, packFloat, unpackFloat);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setVolume, packInt, unpackInt);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setMasterId, packNetworkId, unpackNetworkId);
	ControllerMessageFactory::registerControllerMessageHandler(CM_planetRemoveTravelPoint, packString, unpackString);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setCreatorId, packNetworkId, unpackNetworkId);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setCreatorName, packUnicodeString, unpackUnicodeString);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setCreatorXp, packInt, unpackInt);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setCreatorXpType, packInt, unpackInt);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setCraftedType, packInt, unpackInt);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setForcePower, packInt, unpackInt);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setMaxForcePower, packInt, unpackInt);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setWeather, packIntVector, unpackIntVector);
	ControllerMessageFactory::registerControllerMessageHandler(CM_teleportNextFrame, MessageQueueDataTransformWithParent::pack, MessageQueueDataTransformWithParent::unpack);
	ControllerMessageFactory::registerControllerMessageHandler(CM_serverAddPlanetMapLocation,    packMapLocationTypePair, unpackMapLocationTypePair);
	ControllerMessageFactory::registerControllerMessageHandler(CM_serverRemovePlanetMapLocation, packNetworkId,           unpackNetworkId);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setCondition, packInt, unpackInt);
	ControllerMessageFactory::registerControllerMessageHandler(CM_clearCondition, packInt, unpackInt);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setHouse, packNetworkId, unpackNetworkId);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setGroupMemberDifficulty, packNetworkIdInt, unpackNetworkIdInt);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setDifficulty, packInt, unpackInt);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setLevel, packInt, unpackInt);
	ControllerMessageFactory::registerControllerMessageHandler(CM_recalculateLevel, packInt, unpackInt);
	ControllerMessageFactory::registerControllerMessageHandler(CM_cityCreate, packIntCityInfo, unpackIntCityInfo);
	ControllerMessageFactory::registerControllerMessageHandler(CM_citySetName, packIntString, unpackIntString);
	ControllerMessageFactory::registerControllerMessageHandler(CM_citySetCityHall, packIntNetworkId, unpackIntNetworkId);
	ControllerMessageFactory::registerControllerMessageHandler(CM_citySetLocation, packIntStringIntInt, unpackIntStringIntInt);
	ControllerMessageFactory::registerControllerMessageHandler(CM_citySetRadius, packIntInt, unpackIntInt);
	ControllerMessageFactory::registerControllerMessageHandler(CM_citySetFaction, packIntUint32Bool, unpackIntUint32Bool);
	ControllerMessageFactory::registerControllerMessageHandler(CM_citySetGcwDefenderRegion, packIntStringIntBool, unpackIntStringIntBool);
	ControllerMessageFactory::registerControllerMessageHandler(CM_citySetCreationTime, packIntInt, unpackIntInt);
	ControllerMessageFactory::registerControllerMessageHandler(CM_citySetLeader, packIntNetworkId, unpackIntNetworkId);
	ControllerMessageFactory::registerControllerMessageHandler(CM_citySetIncomeTax, packIntInt, unpackIntInt);
	ControllerMessageFactory::registerControllerMessageHandler(CM_citySetPropertyTax, packIntInt, unpackIntInt);
	ControllerMessageFactory::registerControllerMessageHandler(CM_citySetSalesTax, packIntInt, unpackIntInt);
	ControllerMessageFactory::registerControllerMessageHandler(CM_citySetTravelInfo, packIntVectorIntBool, unpackIntVectorIntBool);
	ControllerMessageFactory::registerControllerMessageHandler(CM_citySetCloneInfo, packIntVectorVectorNetworkIdNetworkId, unpackIntVectorVectorNetworkIdNetworkId);
	ControllerMessageFactory::registerControllerMessageHandler(CM_cityRemove, packInt, unpackInt);
	ControllerMessageFactory::registerControllerMessageHandler(CM_citySetCitizen, packIntNetworkIdStringNetworkIdInt, unpackIntNetworkIdStringNetworkIdInt);
	ControllerMessageFactory::registerControllerMessageHandler(CM_citySetCitizenProfessionInfo, packIntNetworkIdStringInt, unpackIntNetworkIdStringInt);
	ControllerMessageFactory::registerControllerMessageHandler(CM_citySetCitizenAddRank, packStringNetworkIdInt, unpackStringNetworkIdInt);
	ControllerMessageFactory::registerControllerMessageHandler(CM_citySetCitizenRemoveRank, packStringNetworkIdInt, unpackStringNetworkIdInt);
	ControllerMessageFactory::registerControllerMessageHandler(CM_citySetCitizenTitle, packStringNetworkIdInt, unpackStringNetworkIdInt);
	ControllerMessageFactory::registerControllerMessageHandler(CM_cityRemoveCitizen, packIntNetworkId, unpackIntNetworkId);
	ControllerMessageFactory::registerControllerMessageHandler(CM_citySetStructure, packIntNetworkIdIntBool, unpackIntNetworkIdIntBool);
	ControllerMessageFactory::registerControllerMessageHandler(CM_cityRemoveStructure, packIntNetworkId, unpackIntNetworkId);
	ControllerMessageFactory::registerControllerMessageHandler(CM_adjustPgcRating, packStringNetworkIdInt, unpackStringNetworkIdInt);
	ControllerMessageFactory::registerControllerMessageHandler(CM_adjustPgcRatingData, packIntNetworkIdStringInt, unpackIntNetworkIdStringInt);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setPgcRatingData, packIntNetworkIdStringInt, unpackIntNetworkIdStringInt);
	ControllerMessageFactory::registerControllerMessageHandler(CM_fixPgcChroniclerName, packStringNetworkId, unpackStringNetworkId);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setBuildingIsPublic, packBool, unpackBool);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setBuildingCityId, packInt, unpackInt);
	ControllerMessageFactory::registerControllerMessageHandler(CM_creatureSetBaseWalkSpeed, packNothing, unpackNothing);
	ControllerMessageFactory::registerControllerMessageHandler(CM_creatureSetBaseRunSpeed, packNothing, unpackNothing);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setDecayTime, packFloat, unpackFloat);
	ControllerMessageFactory::registerControllerMessageHandler(CM_clearDecayTime, packNothing, unpackNothing);
	ControllerMessageFactory::registerControllerMessageHandler(CM_aiSetMovementRun, packNothing, unpackNothing);
	ControllerMessageFactory::registerControllerMessageHandler(CM_aiSetMovementWalk, packNothing, unpackNothing);
	ControllerMessageFactory::registerControllerMessageHandler(CM_applyWound, packIntIntNetworkId, unpackIntIntNetworkId);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setWound, packIntInt, unpackIntInt);
	ControllerMessageFactory::registerControllerMessageHandler(CM_makeDead, packNetworkIdNetworkId, unpackNetworkIdNetworkId);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setGroupShipForMember, packNetworkIdNetworkId, unpackNetworkIdNetworkId);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setGroupShipFormationSlotForMember, packNetworkIdInt, unpackNetworkIdInt);
	ControllerMessageFactory::registerControllerMessageHandler(CM_addGroupPOBShipAndOwner, packNetworkIdNetworkId, unpackNetworkIdNetworkId);
	ControllerMessageFactory::registerControllerMessageHandler(CM_removeGroupPOBShip, packNetworkId, unpackNetworkId);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setBioLink, packNetworkId, unpackNetworkId);
	ControllerMessageFactory::registerControllerMessageHandler(CM_addHate, packNetworkIdFloat, unpackNetworkIdFloat);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setHate, packNetworkIdFloat, unpackNetworkIdFloat);
	ControllerMessageFactory::registerControllerMessageHandler(CM_removeHateTarget, packNetworkId, unpackNetworkId);
	ControllerMessageFactory::registerControllerMessageHandler(CM_clearHateList, packNothing, unpackNothing);
	ControllerMessageFactory::registerControllerMessageHandler(CM_groupMakeLootMaster, packNetworkId, unpackNetworkId);
	ControllerMessageFactory::registerControllerMessageHandler(CM_groupSetLootRule, packInt, unpackInt);
	ControllerMessageFactory::registerControllerMessageHandler(CM_aiSetCreatureName, packString, unpackString);
	ControllerMessageFactory::registerControllerMessageHandler(CM_aiSetHomeLocation, packLocation, unpackLocation);
	ControllerMessageFactory::registerControllerMessageHandler(CM_aiSetFrozen, packBool, unpackBool);
	ControllerMessageFactory::registerControllerMessageHandler(CM_aiSetRetreating, packBool, unpackBool);
	ControllerMessageFactory::registerControllerMessageHandler(CM_aiSetLogging, packBool, unpackBool);
	ControllerMessageFactory::registerControllerMessageHandler(CM_aiEquipPrimaryWeapon, packNothing, unpackNothing);
	ControllerMessageFactory::registerControllerMessageHandler(CM_aiEquipSecondaryWeapon, packNothing, unpackNothing);
	ControllerMessageFactory::registerControllerMessageHandler(CM_aiUnEquipWeapons, packNothing, unpackNothing);
	ControllerMessageFactory::registerControllerMessageHandler(CM_aiMarkCombatStartLocation, packNothing, unpackNothing);
	ControllerMessageFactory::registerControllerMessageHandler(CM_resetHateTimer, packNothing, unpackNothing);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setRegenRate, packIntFloat, unpackIntFloat);
	ControllerMessageFactory::registerControllerMessageHandler(CM_updateObserversCount, packInt, unpackInt);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setVisible, packBool, unpackBool);
	ControllerMessageFactory::registerControllerMessageHandler(CM_hideFromClient, packBool, unpackBool);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setHibernationDelay, packFloat, unpackFloat);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setAttackableOverride, packBool, unpackBool);
	ControllerMessageFactory::registerControllerMessageHandler(CM_modifyCurrentGcwPoints, packIntInt, unpackIntInt);
	ControllerMessageFactory::registerControllerMessageHandler(CM_modifyCurrentGcwRating, packIntInt, unpackIntInt);
	ControllerMessageFactory::registerControllerMessageHandler(CM_modifyCurrentPvpKills, packIntInt, unpackIntInt);
	ControllerMessageFactory::registerControllerMessageHandler(CM_modifyLifetimeGcwPoints, packInt, unpackInt);
	ControllerMessageFactory::registerControllerMessageHandler(CM_modifyMaxGcwImperialRating, packInt, unpackInt);
	ControllerMessageFactory::registerControllerMessageHandler(CM_modifyMaxGcwRebelRating, packInt, unpackInt);
	ControllerMessageFactory::registerControllerMessageHandler(CM_modifyLifetimePvpKills, packInt, unpackInt);
	ControllerMessageFactory::registerControllerMessageHandler(CM_modifyNextGcwRatingCalcTime, packInt, unpackInt);
	ControllerMessageFactory::registerControllerMessageHandler(CM_cancelMessageTo, packString, unpackString);
	ControllerMessageFactory::registerControllerMessageHandler(CM_clearSessionActivity, packNothing, unpackNothing);
	ControllerMessageFactory::registerControllerMessageHandler(CM_addSessionActivity, packUnsignedLong, unpackUnsignedLong);
	ControllerMessageFactory::registerControllerMessageHandler(CM_incrementKillMeter, packInt, unpackInt);
	ControllerMessageFactory::registerControllerMessageHandler(CM_addPassiveReveal, packIntIntNetworkId, unpackIntIntNetworkId);
	ControllerMessageFactory::registerControllerMessageHandler(CM_removePassiveReveal, packNetworkId, unpackNetworkId);
	ControllerMessageFactory::registerControllerMessageHandler(CM_removeAllPassiveReveal, packNothing, unpackNothing);
	ControllerMessageFactory::registerControllerMessageHandler(CM_modifyHologramType, packInt, unpackInt);
	ControllerMessageFactory::registerControllerMessageHandler(CM_modifyVisibleOnMapAndRadar, packBool, unpackBool);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setCoverVisibility, packBool, unpackBool);
	ControllerMessageFactory::registerControllerMessageHandler(CM_modifyCollectionSlotValue, packStringInt64, unpackStringInt64);
	ControllerMessageFactory::registerControllerMessageHandler(CM_adjustLotCount, packInt, unpackInt);
	ControllerMessageFactory::registerControllerMessageHandler(CM_squelch, packStringNetworkIdInt, unpackStringNetworkIdInt);
	ControllerMessageFactory::registerControllerMessageHandler(CM_unsquelch, packNothing, unpackNothing);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setPriviledgedTitle, packInt, unpackInt);
	ControllerMessageFactory::registerControllerMessageHandler(CM_collectionRequestServerFirst, packStringNetworkIdUnicodeStringString, unpackStringNetworkIdUnicodeStringString);
	ControllerMessageFactory::registerControllerMessageHandler(CM_collectionRevokeServerFirst, packString, unpackString);
	ControllerMessageFactory::registerControllerMessageHandler(CM_addHateOverTime, packNetworkIdIntFloat, unpackNetworkIdIntFloat);
	ControllerMessageFactory::registerControllerMessageHandler(CM_removeAllAttributeAndSkillmodModifiers, packNothing, unpackNothing);
	ControllerMessageFactory::registerControllerMessageHandler(CM_addBuff, packIntFloatFloatFloatNetworkId, unpackIntFloatFloatFloatNetworkId);
	ControllerMessageFactory::registerControllerMessageHandler(CM_removeBuff, packInt, unpackInt);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setConnectedCharacterLfgData, packNetworkIdLfgCharacterData, unpackNetworkIdLfgCharacterData);
	ControllerMessageFactory::registerControllerMessageHandler(CM_removeConnectedCharacterData, packNetworkId, unpackNetworkId);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setConnectedCharacterGroupData, packNetworkIdNetworkId, unpackNetworkIdNetworkId);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setConnectedCharacterPlanetData, packStringNetworkId, unpackStringNetworkId);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setConnectedCharacterRegionData, packStringNetworkId, unpackStringNetworkId);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setConnectedCharacterPlayerCityData, packStringNetworkId, unpackStringNetworkId);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setConnectedCharacterBiographyData, packUnicodeStringNetworkId, unpackUnicodeStringNetworkId);
	ControllerMessageFactory::registerControllerMessageHandler(CM_removeConnectedCharacterBiographyData, packNetworkId, unpackNetworkId);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setGroupMemberProfession, packNetworkIdUint32, unpackNetworkIdUint32);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setConnectedCharacterLevelData, packNetworkIdInt, unpackNetworkIdInt);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setConnectedCharacterProfessionData, packNetworkIdInt, unpackNetworkIdInt);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setConnectedCharacterSearchableByCtsSourceGalaxyData, packNetworkIdInt, unpackNetworkIdInt);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setConnectedCharacterDisplayLocationInSearchResultsData, packNetworkIdInt, unpackNetworkIdInt);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setConnectedCharacterAnonymousData, packNetworkIdInt, unpackNetworkIdInt);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setConnectedCharacterInterestsData, packNetworkIdBitArray, unpackNetworkIdBitArray);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setGroupPickupTimer, packInt32Int32, unpackInt32Int32);
	ControllerMessageFactory::registerControllerMessageHandler(CM_setGroupPickupLocation, packStringVector, unpackStringVector);
	ControllerMessageFactory::registerControllerMessageHandler(CM_addUniverseEvent, packString, unpackString);
	ControllerMessageFactory::registerControllerMessageHandler(CM_removeUniverseEvent, packString, unpackString);
	ControllerMessageFactory::registerControllerMessageHandler(CM_forceHateTarget, packNetworkId, unpackNetworkId);
	ControllerMessageFactory::registerControllerMessageHandler(CM_addUserToAccessList, packIntNetworkId, unpackIntNetworkId);
	ControllerMessageFactory::registerControllerMessageHandler(CM_removeUserFromAccessList, packIntNetworkId, unpackIntNetworkId);
	ControllerMessageFactory::registerControllerMessageHandler(CM_clearUserAccessList, packBool, unpackBool);
	ControllerMessageFactory::registerControllerMessageHandler(CM_guildUpdateGuildWarKillTrackingData, packIntIntIntInt, unpackIntIntIntInt);
	ControllerMessageFactory::registerControllerMessageHandler(CM_adjustGcwImperialScore, packMapStringInt64, unpackMapStringInt64);
	ControllerMessageFactory::registerControllerMessageHandler(CM_adjustGcwRebelScore, packMapStringInt64, unpackMapStringInt64);

	g_installed = true;
}

// ----------------------------------------------------------------------

void SetupServerNetworkMessages::remove ( void )
{
	DEBUG_FATAL(!g_installed, ("SetupServerNetworkMessages::remove - not already installed"));

	SetupServerNetworkMessages setup;
	setup.internalRemove();
}

void SetupServerNetworkMessages::internalRemove ( void )
{
	g_installed = false;
}

// ======================================================================

