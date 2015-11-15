// ======================================================================
//
// BatchBaselinesMessage.h
// copyright (c) 2005 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_BatchBaselinesMessage_H
#define INCLUDED_BatchBaselinesMessage_H

// ======================================================================

#include "Archive/AutoDeltaByteStream.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/Tag.h"
#include "sharedFoundation/NetworkId.h"

// ======================================================================

class BatchBaselinesMessageData
{
public:
	NetworkId m_networkId;
	Tag m_objectType;
	signed char m_packageId; // For some reason, GCC can't pick the right version of "unpack" unless "signed char" is used instead of "char"
	Archive::ByteStream m_package;

public:
	BatchBaselinesMessageData(NetworkId const & networkId, Tag objectType, char packageId, Archive::ByteStream const & package);
	BatchBaselinesMessageData();
};

// ======================================================================

/**
 * A single network message that consolidates a large batch of baselines
 * Avoids the overhead of constructing and sending a BaselinesMessage
 * for every baseline.
 */
class BatchBaselinesMessage : public GameNetworkMessage
{
public:
	BatchBaselinesMessage(std::vector<BatchBaselinesMessageData> const & data);
	explicit BatchBaselinesMessage(Archive::ReadIterator & source);
	~BatchBaselinesMessage();

	std::vector<BatchBaselinesMessageData> const & getData() const;

private:
	Archive::AutoVariable<std::vector<BatchBaselinesMessageData> > m_data;
	// Archive::AutoArray<BatchBaselinesMessageData> m_data;
};

// ======================================================================

namespace Archive
{
	void put(ByteStream & target, BatchBaselinesMessageData const & source);
	void get(ReadIterator & source, BatchBaselinesMessageData & target);
}

// ======================================================================

#endif
