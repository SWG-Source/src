// ======================================================================
//
// MessageToPayloadImpl.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "MessageToPayloadImpl.h"

uint64 MessageToPayloadImpl::ms_counterMessageToPayload = 0;
int MessageToPayloadImpl::ms_instanceCount = 0;

// ======================================================================

MessageToPayloadImpl::MessageToPayloadImpl(const NetworkId &networkId, const MessageToId &messageId, const std::string &method, const std::string &packedData, unsigned long callTime, bool guaranteed, MessageToPayload::DeliveryType deliveryType, NetworkId const & undeliveredCallbackObject, std::string const & undeliveredCallbackMethod, int recurringTime) :
		m_networkId(networkId),
		m_messageId(messageId),
		m_method(method),
		m_packedData(packedData.begin(), packedData.end()),
		m_callTime(callTime),
		m_guaranteed(guaranteed),
		m_persisted(false),
		m_deliveryType(deliveryType),
		m_undeliveredCallbackObject(undeliveredCallbackObject),
		m_undeliveredCallbackMethod(undeliveredCallbackMethod),
		m_recurringTime(recurringTime),
		m_broadcastCount(0),
		m_bounceCount(0),
		m_bounceServers(),
		m_refCount(1),
		m_counterMessageToPayload(++ms_counterMessageToPayload)
{
	DEBUG_FATAL(m_guaranteed && m_recurringTime!=0,("Attempted to create MessageToPayloadImpl for message %s, which was guaranteed and had a recurringTime.  Recurring MessageTos may not be guaranteed."));
	++ms_instanceCount;
}

// ----------------------------------------------------------------------

MessageToPayloadImpl::MessageToPayloadImpl(const NetworkId &networkId, const MessageToId &messageId, const std::string &method, const std::vector<int8> &packedData, unsigned long callTime, bool guaranteed, MessageToPayload::DeliveryType deliveryType, NetworkId const & undeliveredCallbackObject, std::string const & undeliveredCallbackMethod, int recurringTime) :
		m_networkId(networkId),
		m_messageId(messageId),
		m_method(method),
		m_packedData(packedData),
		m_callTime(callTime),
		m_guaranteed(guaranteed),
		m_persisted(false),
		m_deliveryType(deliveryType),
		m_undeliveredCallbackObject(undeliveredCallbackObject),
		m_undeliveredCallbackMethod(undeliveredCallbackMethod),
		m_recurringTime(recurringTime),
		m_broadcastCount(0),
		m_bounceCount(0),
		m_bounceServers(),
		m_refCount(1),
		m_counterMessageToPayload(++ms_counterMessageToPayload)
{
	DEBUG_FATAL(m_guaranteed && m_recurringTime!=0,("Attempted to create MessageToPayloadImpl for message %s, which was guaranteed and had a recurringTime.  Recurring MessageTos may not be guaranteed."));
	++ms_instanceCount;
}

// ----------------------------------------------------------------------

MessageToPayloadImpl::MessageToPayloadImpl() :
		m_networkId(),
		m_messageId(),
		m_method(),
		m_packedData(),
		m_callTime(0),
		m_guaranteed(false),
		m_persisted(false),
		m_deliveryType(MessageToPayload::DT_none),
		m_undeliveredCallbackObject(NetworkId::cms_invalid),
		m_undeliveredCallbackMethod(),
		m_recurringTime(0),
		m_broadcastCount(0),
		m_bounceCount(0),
		m_bounceServers(),
		m_refCount(1),
		m_counterMessageToPayload(++ms_counterMessageToPayload)
{
	++ms_instanceCount;
}

// ----------------------------------------------------------------------

MessageToPayloadImpl::MessageToPayloadImpl(MessageToPayloadImpl const & rhs) :
		m_networkId(rhs.m_networkId),
		m_messageId(rhs.m_messageId),
		m_method(rhs.m_method),
		m_packedData(rhs.m_packedData),
		m_callTime(rhs.m_callTime),
		m_guaranteed(rhs.m_guaranteed),
		m_persisted(rhs.m_persisted),
		m_deliveryType(rhs.m_deliveryType),
		m_undeliveredCallbackObject(rhs.m_undeliveredCallbackObject),
		m_undeliveredCallbackMethod(rhs.m_undeliveredCallbackMethod),
		m_recurringTime(rhs.m_recurringTime),
		m_broadcastCount(0),
		m_bounceCount(0),
		m_bounceServers(),
		m_refCount(1),
		m_counterMessageToPayload(rhs.m_counterMessageToPayload)
{
	++ms_instanceCount;
}

// ----------------------------------------------------------------------

MessageToPayloadImpl::~MessageToPayloadImpl()
{
	--ms_instanceCount;
}

// ----------------------------------------------------------------------

void MessageToPayloadImpl::setGuaranteed(bool guaranteed)
{
	WARNING_DEBUG_FATAL(m_refCount > 1,("Programmer bug:  attempted to set data on a MessageToPayloadImpl that had a refcount > 1.  This would overwrite data that is referenced in multiple places, which is not safe."));
	m_guaranteed = guaranteed;
}

// ----------------------------------------------------------------------

void MessageToPayloadImpl::setPersisted(bool persisted)
{
	WARNING_DEBUG_FATAL(m_refCount > 1,("Programmer bug:  attempted to set data on a MessageToPayloadImpl that had a refcount > 1.  This would overwrite data that is referenced in multiple places, which is not safe."));
	m_persisted = persisted;
}

// ----------------------------------------------------------------------

void MessageToPayloadImpl::pack(Archive::ByteStream &target) const
{
	Archive::put(target, m_networkId);
	Archive::put(target, m_messageId);
	Archive::put(target, m_method);
	Archive::put(target, m_packedData);
	Archive::put(target, m_callTime);
	Archive::put(target, m_guaranteed);
	Archive::put(target, m_persisted);
	int temp = static_cast<int>(m_deliveryType);
	Archive::put(target, temp);
	Archive::put(target, m_undeliveredCallbackObject);
	Archive::put(target, m_undeliveredCallbackMethod);
	Archive::put(target, m_recurringTime);
	Archive::put(target, m_broadcastCount);
	Archive::put(target, m_bounceCount);
	Archive::put(target, m_bounceServers);
}

// ----------------------------------------------------------------------

void MessageToPayloadImpl::unpack(Archive::ReadIterator &source)
{
	WARNING_DEBUG_FATAL(m_refCount > 1,("Programmer bug:  attempted to unpack a MessageToPayloadImpl that had a refcount > 1.  This would overwrite data that is referenced in multiple places, which is not safe."));
	
	Archive::get(source, m_networkId);
	Archive::get(source, m_messageId);
	Archive::get(source, m_method);
	Archive::get(source, m_packedData);
	Archive::get(source, m_callTime);
	Archive::get(source, m_guaranteed);
	Archive::get(source, m_persisted);
	int temp;
	Archive::get(source, temp);
	m_deliveryType = static_cast<MessageToPayload::DeliveryType>(temp);
	Archive::get(source, m_undeliveredCallbackObject);
	Archive::get(source, m_undeliveredCallbackMethod);
	Archive::get(source, m_recurringTime);
	Archive::get(source, m_broadcastCount);
	Archive::get(source, m_bounceCount);
	Archive::get(source, m_bounceServers);
}

// ----------------------------------------------------------------------

NetworkId const & MessageToPayloadImpl::getUndeliveredCallbackObject() const
{
	return m_undeliveredCallbackObject;
}

// ----------------------------------------------------------------------

std::string const & MessageToPayloadImpl::getUndeliveredCallbackMethod() const
{
	return m_undeliveredCallbackMethod;
}

// ----------------------------------------------------------------------

std::string MessageToPayloadImpl::getDataAsString() const
{
	return std::string(m_packedData.begin(), m_packedData.end());
}

// ----------------------------------------------------------------------

bool MessageToPayloadImpl::operator==(MessageToPayloadImpl const & rhs) const
{
	return (m_messageId == rhs.m_messageId);
}

// ----------------------------------------------------------------------

bool MessageToPayloadImpl::operator!=(MessageToPayloadImpl const & rhs) const
{
	return (m_messageId != rhs.m_messageId);
}

// ----------------------------------------------------------------------

int MessageToPayloadImpl::getRecurringTime() const
{
	return m_recurringTime;
}

// ----------------------------------------------------------------------

int MessageToPayloadImpl::getInstanceCount()
{
	return ms_instanceCount;
}

// ----------------------------------------------------------------------

void MessageToPayloadImpl::increaseRefCount()
{
	++m_refCount;
}

// ----------------------------------------------------------------------

void MessageToPayloadImpl::decreaseRefCount()
{
	--m_refCount;
}

// ----------------------------------------------------------------------

int MessageToPayloadImpl::getRefCount() const
{
	return m_refCount;
}

// ----------------------------------------------------------------------

void MessageToPayloadImpl::addBroadcastCount()
{
	m_broadcastCount++;
}

// ----------------------------------------------------------------------

void MessageToPayloadImpl::addBounceServer(uint32 serverProcessId)
{
	WARNING_DEBUG_FATAL(m_refCount > 1,("Programmer bug:  attempted to change a MessageToPayloadImpl that had a refcount > 1.  MessageToPayloadImpls do not have copy-on-write, so this is not safe."));
	m_bounceServers.insert(serverProcessId);
	m_bounceCount++;
}

// ----------------------------------------------------------------------

bool MessageToPayloadImpl::hasBounceServer(uint32 serverProcessId) const
{
	return (m_bounceServers.count(serverProcessId) > 0);
}

// ----------------------------------------------------------------------

void MessageToPayloadImpl::clearBounceServers()
{
	m_bounceServers.clear();
}

// ======================================================================

void Archive::put(ByteStream & target, const MessageToPayloadImpl & source)
{
	source.pack(target);
}

// ----------------------------------------------------------------------

void Archive::get(ReadIterator & source, MessageToPayloadImpl & target)
{
	target.unpack(source);
}

// ======================================================================
