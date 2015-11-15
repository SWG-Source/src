// ======================================================================
//
// TransferAccountDataArchive.cpp
// copyright (c) 2001-2004 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/TransferAccountDataArchive.h"
#include "serverNetworkMessages/TransferAccountData.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "Archive/Archive.h"

namespace Archive
{
	void get(ReadIterator & source, TransferAccountData & target)
	{
		get(source, target.m_destinationStationId);
		get(source, target.m_sourceStationId);
		get(source, target.m_track);
		get(source, target.m_transactionId);
		get(source, target.m_destinationHasAvatars);
		get(source, target.m_sourceAvatarData);
		get(source, target.m_startGalaxy);
	}

	void put(ByteStream & target, const TransferAccountData & source)
	{
		put(target, source.getDestinationStationId());
		put(target, source.getSourceStationId());
		put(target, source.getTrack());
		put(target, source.getTransactionId());
		put(target, source.getDestinationHasAvatars());
		put(target, source.getSourceAvatarData());
		put(target, source.getStartGalaxy());
	}
}
