// ======================================================================
//
// MessageToPayloadImpl.h
// copyright (c) 2001 Sony Online Entertainment
//const
// ======================================================================

#ifndef INCLUDED_MessageToPayloadImpl_H
#define INCLUDED_MessageToPayloadImpl_H

// ======================================================================

#include "serverNetworkMessages/MessageToPayload.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedFoundation/NetworkIdArchive.h"

#include <string>
#include <vector>

// ======================================================================

/** 
 * Data associated with a MessageTo, used by several different network messages.
 */
class MessageToPayloadImpl
{
public:
	MessageToPayloadImpl(const NetworkId &networkId, const MessageToId &messageId, const std::string &method, const	std::string &packedData, unsigned long callTime, bool guaranteed, MessageToPayload::DeliveryType deliveryType, NetworkId const & undeliveredCallbackObject, std::string const & undeliveredCallbackMethod, int recurringTime);
	MessageToPayloadImpl(const NetworkId &networkId, const MessageToId &messageId, const std::string &method, const	std::vector<int8> &packedData, unsigned long callTime, bool guaranteed, MessageToPayload::DeliveryType deliveryType, NetworkId const & undeliveredCallbackObject, std::string const & undeliveredCallbackMethod, int recurringTime);
	MessageToPayloadImpl();
	~MessageToPayloadImpl();

	bool operator==(MessageToPayloadImpl const & rhs) const;
	bool operator!=(MessageToPayloadImpl const & rhs) const;

	void pack(Archive::ByteStream &target) const;
	void unpack(Archive::ReadIterator &source);

	void setGuaranteed(bool guaranteed);
	void setPersisted(bool persisted);
	void addBroadcastCount();
	void addBounceServer(uint32 serverProcessId);
	bool hasBounceServer(uint32 serverProcessId) const;
	void clearBounceServers();
	
	NetworkId const &getNetworkId() const;
	MessageToId const &getMessageId() const;
	std::string const &getMethod() const;
	std::vector<int8> const &getPackedDataVector() const;
	std::string getDataAsString() const;
	unsigned long getCallTime() const;
	bool getGuaranteed() const;
	bool getPersisted() const;
	MessageToPayload::DeliveryType getDeliveryType() const;
	NetworkId const & getUndeliveredCallbackObject() const;
	std::string const & getUndeliveredCallbackMethod() const;
	int getRecurringTime() const;
	uint64 getCounterMessageToPayload() const;
	int getBroadcastCount() const;
	int getBounceCount() const;
	static int getInstanceCount();
	void increaseRefCount();
	void decreaseRefCount();
	int getRefCount() const;

private:
	NetworkId m_networkId;
	MessageToId m_messageId;
	std::string m_method;
	std::vector<int8> m_packedData;
	unsigned long m_callTime;
	bool m_guaranteed;
	bool m_persisted;
	MessageToPayload::DeliveryType m_deliveryType;
	NetworkId m_undeliveredCallbackObject;
	std::string m_undeliveredCallbackMethod;
	int m_recurringTime;
	int m_broadcastCount; // home many times this message has been broadcasted to find the recipient
	int m_bounceCount; // how many times this message has been forwarded to a new server
	std::set<uint32> m_bounceServers; // the servers which have forwarded this message, to catch circular forwarding
	int m_refCount;

	// there is no need to pack this var, it is used to "mark" each MessageToPayloadImpl
	// on a per game server execution basis, and is used as the second criterion
	// when prioritizing messageTo that originated on the game server and has
	// the same calltime, so that the messageTo will execute in the order that
	// they were originated
	uint64 m_counterMessageToPayload;

	// used to assign m_counterMessageToPayload
	static uint64 ms_counterMessageToPayload;

	// used to count how many messages there are at one time
	static int ms_instanceCount;

private:
	MessageToPayloadImpl & operator=(MessageToPayloadImpl const & rhs); // disable
	MessageToPayloadImpl(MessageToPayloadImpl const & rhs); // disable
};

// ----------------------------------------------------------------------

inline NetworkId const &MessageToPayloadImpl::getNetworkId() const
{
	return m_networkId;
}

// ----------------------------------------------------------------------

inline MessageToId const &MessageToPayloadImpl::getMessageId() const
{
	return m_messageId;
}

// ----------------------------------------------------------------------

inline std::string const &MessageToPayloadImpl::getMethod() const
{
	return m_method;
}

// ----------------------------------------------------------------------

inline std::vector<int8> const &MessageToPayloadImpl::getPackedDataVector() const
{
	return m_packedData;
}

// ----------------------------------------------------------------------

inline unsigned long MessageToPayloadImpl::getCallTime() const
{
	return m_callTime;
}

// ----------------------------------------------------------------------

inline bool MessageToPayloadImpl::getGuaranteed() const
{
	return m_guaranteed;
}

// ----------------------------------------------------------------------

inline bool MessageToPayloadImpl::getPersisted() const
{
	return m_persisted;
}

// ----------------------------------------------------------------------

inline MessageToPayload::DeliveryType MessageToPayloadImpl::getDeliveryType() const
{
	return m_deliveryType;
}

// ----------------------------------------------------------------------

inline uint64 MessageToPayloadImpl::getCounterMessageToPayload() const
{
	return m_counterMessageToPayload;
}

// ----------------------------------------------------------------------

inline int MessageToPayloadImpl::getBroadcastCount() const
{
	return m_broadcastCount;
}

// ----------------------------------------------------------------------

inline int MessageToPayloadImpl::getBounceCount() const
{
	return m_bounceCount;
}

// ======================================================================

namespace Archive
{
	void put(ByteStream & target, const MessageToPayloadImpl & source);
	void get(ReadIterator & source, MessageToPayloadImpl & target);
}

// ======================================================================

#endif
