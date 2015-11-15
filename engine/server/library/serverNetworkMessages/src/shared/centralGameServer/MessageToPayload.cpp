// ======================================================================
//
// MessageToPayload.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/MessageToPayload.h"

#include "MessageToPayloadImpl.h"

// ======================================================================

MessageToPayload::MessageToPayload(const NetworkId &networkId, const MessageToId &messageId, const std::string &method, const std::string &packedData, unsigned long callTime, bool guaranteed, DeliveryType deliveryType, NetworkId const & undeliveredCallbackObject, std::string const & undeliveredCallbackMethod, int recurringTime) :
		m_impl(new MessageToPayloadImpl(networkId, messageId, method, packedData, callTime, guaranteed, deliveryType, undeliveredCallbackObject, undeliveredCallbackMethod, recurringTime))
{
}

// ----------------------------------------------------------------------

MessageToPayload::MessageToPayload(const NetworkId &networkId, const MessageToId &messageId, const std::string &method, const std::vector<int8> &packedData, unsigned long callTime, bool guaranteed, DeliveryType deliveryType, NetworkId const & undeliveredCallbackObject, std::string const & undeliveredCallbackMethod, int recurringTime) :
		m_impl(new MessageToPayloadImpl(networkId, messageId, method, packedData, callTime, guaranteed, deliveryType, undeliveredCallbackObject, undeliveredCallbackMethod, recurringTime))
{
}

// ----------------------------------------------------------------------

MessageToPayload::MessageToPayload() :
		m_impl(new MessageToPayloadImpl())
{
}

// ----------------------------------------------------------------------

MessageToPayload::MessageToPayload(MessageToPayload const & rhs) :
		m_impl(rhs.m_impl)
{
	m_impl->increaseRefCount();
}

// ----------------------------------------------------------------------

MessageToPayload::~MessageToPayload()
{
	m_impl->decreaseRefCount();
	if (m_impl->getRefCount()==0)
		delete m_impl;
}

// ----------------------------------------------------------------------

MessageToPayload & MessageToPayload::operator=(MessageToPayload const & rhs)
{
	if (&rhs != this)
	{
		m_impl->decreaseRefCount();
		if (m_impl->getRefCount()==0)
			delete m_impl;
		m_impl=rhs.m_impl;
		m_impl->increaseRefCount();
	}
	return *this;
}

// ----------------------------------------------------------------------

void MessageToPayload::setGuaranteed(bool guaranteed)
{
	// This does not do a copy-on-write if the refcount > 1, because currently
	// we don't expect the code to ever change the guaranteed flag when there
	// is more than one reference to the message.  If it does, it probably
	// indicates incorrect behavior.
	m_impl->setGuaranteed(guaranteed);
}

// ----------------------------------------------------------------------

void MessageToPayload::pack(Archive::ByteStream &target) const
{
	m_impl->pack(target);
}

// ----------------------------------------------------------------------

void MessageToPayload::unpack(Archive::ReadIterator &source)
{
	if (m_impl->getRefCount() > 1)
	{
		// copy-on-write
		m_impl->decreaseRefCount();
		m_impl=new MessageToPayloadImpl();
	}
	m_impl->unpack(source);
}

// ----------------------------------------------------------------------

NetworkId const & MessageToPayload::getUndeliveredCallbackObject() const
{
	return m_impl->getUndeliveredCallbackObject();
}

// ----------------------------------------------------------------------

std::string const & MessageToPayload::getUndeliveredCallbackMethod() const
{
	return m_impl->getUndeliveredCallbackMethod();
}

// ----------------------------------------------------------------------

std::string MessageToPayload::getDataAsString() const
{
	return m_impl->getDataAsString();
}

// ----------------------------------------------------------------------

bool MessageToPayload::operator==(MessageToPayload const & rhs) const
{
	return (m_impl == rhs.m_impl || *m_impl == *(rhs.m_impl));
}

// ----------------------------------------------------------------------

bool MessageToPayload::operator!=(MessageToPayload const & rhs) const
{
	return (m_impl != rhs.m_impl && *m_impl != *(rhs.m_impl));
}

// ----------------------------------------------------------------------

int MessageToPayload::getRecurringTime() const
{
	return m_impl->getRecurringTime();
}

// ----------------------------------------------------------------------

int MessageToPayload::getInstanceCount()
{
	return MessageToPayloadImpl::getInstanceCount();
}

// ----------------------------------------------------------------------

NetworkId const & MessageToPayload::getNetworkId() const
{
	return m_impl->getNetworkId();
}

// ----------------------------------------------------------------------

MessageToId const & MessageToPayload::getMessageId() const
{
	return m_impl->getMessageId();
}

// ----------------------------------------------------------------------

std::string const & MessageToPayload::getMethod() const
{
	return m_impl->getMethod();
}

// ----------------------------------------------------------------------

std::vector<int8> const & MessageToPayload::getPackedDataVector() const
{
	return m_impl->getPackedDataVector();
}

// ----------------------------------------------------------------------

unsigned long MessageToPayload::getCallTime() const
{
	return m_impl->getCallTime();
}

// ----------------------------------------------------------------------

bool MessageToPayload::getGuaranteed() const
{
	return m_impl->getGuaranteed();
}

// ----------------------------------------------------------------------

MessageToPayload::DeliveryType MessageToPayload::getDeliveryType() const
{
	return m_impl->getDeliveryType();
}

// ----------------------------------------------------------------------

uint64 MessageToPayload::getCounterMessageToPayload() const
{
	return m_impl->getCounterMessageToPayload();
}

// ----------------------------------------------------------------------

bool MessageToPayload::getPersisted() const
{
	return m_impl->getPersisted();
}

// ----------------------------------------------------------------------

void MessageToPayload::setPersisted(bool persisted)
{
	m_impl->setPersisted(persisted);
}

// ----------------------------------------------------------------------

void MessageToPayload::addBroadcastCount()
{
	m_impl->addBroadcastCount();
}

// ----------------------------------------------------------------------

void MessageToPayload::addBounceServer(uint32 serverProcessId)
{
	m_impl->addBounceServer(serverProcessId);
}

// ----------------------------------------------------------------------

bool MessageToPayload::hasBounceServer(uint32 serverProcessId) const
{
	return m_impl->hasBounceServer(serverProcessId);
}

// ----------------------------------------------------------------------

void MessageToPayload::clearBounceServers()
{
	m_impl->clearBounceServers();
}

// ----------------------------------------------------------------------

int MessageToPayload::getBroadcastCount() const
{
	return m_impl->getBroadcastCount();
}

// ----------------------------------------------------------------------

int MessageToPayload::getBounceCount() const
{
	return m_impl->getBounceCount();
}

// ======================================================================

void Archive::put(ByteStream & target, const MessageToPayload & source)
{
	source.pack(target);
}

// ----------------------------------------------------------------------

void Archive::get(ReadIterator & source, MessageToPayload & target)
{
	target.unpack(source);
}

// ======================================================================
