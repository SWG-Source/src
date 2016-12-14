// ======================================================================
//
// MessageToPayload.h
// copyright (c) 2001 Sony Online Entertainment
//const
// ======================================================================

#ifndef INCLUDED_MessageToPayload_H
#define INCLUDED_MessageToPayload_H

// ======================================================================

class NetworkId;
typedef NetworkId MessageToId;
class MessageToPayload;
class MessageToPayloadImpl;

namespace Archive
{
	class ByteStream;
	class ReadIterator;
	
	void put(ByteStream & target, const MessageToPayload & source);
	void get(ReadIterator & source, MessageToPayload & target);
}

// ======================================================================

/** 
 * Data associated with a MessageTo
 *
 * Implemented as a pointer to a refcounted class, because the game server
 * copies these often.
 */
class MessageToPayload
{
public:
	enum DeliveryType
	{
		DT_c,
		DT_java,
		DT_none
	};
	
public:
	MessageToPayload(const NetworkId &networkId, const MessageToId &messageId, const std::string &method, const	std::string &packedData, unsigned long callTime, bool guaranteed, DeliveryType deliveryType, NetworkId const & undeliveredCallbackObject, std::string const & undeliveredCallbackMethod, int recurringTime);
	MessageToPayload(const NetworkId &networkId, const MessageToId &messageId, const std::string &method, const	std::vector<int8> &packedData, unsigned long callTime, bool guaranteed, DeliveryType deliveryType, NetworkId const & undeliveredCallbackObject, std::string const & undeliveredCallbackMethod, int recurringTime);
	MessageToPayload(MessageToPayload const & rhs);
	MessageToPayload();
	~MessageToPayload();
	MessageToPayload & operator=(MessageToPayload const & rhs);

	bool operator==(MessageToPayload const & rhs) const;
	bool operator!=(MessageToPayload const & rhs) const;

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
	DeliveryType getDeliveryType() const;
	NetworkId const & getUndeliveredCallbackObject() const;
	std::string const & getUndeliveredCallbackMethod() const;
	int getRecurringTime() const;
	uint64 getCounterMessageToPayload() const;
	bool getPersisted() const;
	int getBroadcastCount() const;
	int getBounceCount() const;
	
	static int getInstanceCount();

private:
	MessageToPayloadImpl * m_impl;
};

// ======================================================================

#endif
