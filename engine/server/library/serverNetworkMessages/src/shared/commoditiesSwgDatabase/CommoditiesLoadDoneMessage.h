#ifndef CommoditiesLoadDoneMessage_H
#define CommoditiesLoadDoneMessage_H

#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"
#include "Unicode.h"

class CommoditiesLoadDoneMessage : public GameNetworkMessage
{

public:
	explicit CommoditiesLoadDoneMessage(
		const int            auctionLocationsCount,
		const int            marketAuctionsCount,
		const int            marketAuctionAttributesCount,
		const int            marketAuctionBidsCount
	);

	CommoditiesLoadDoneMessage(Archive::ReadIterator & source);
	~CommoditiesLoadDoneMessage();

	const int              getAuctionLocationsCount()	const {return m_auctionLocationsCount.get();}
	const int              getMarketAuctionsCount()		const {return m_marketAuctionsCount.get();}
	const int              getMarketAuctionAttributesCount() const {return m_marketAuctionAttributesCount.get();}
	const int              getMarketAuctionBidsCount()	const {return m_marketAuctionBidsCount.get();}
		
protected:

private:
	Archive::AutoVariable<int>	m_auctionLocationsCount;
	Archive::AutoVariable<int>	m_marketAuctionsCount;
	Archive::AutoVariable<int>	m_marketAuctionAttributesCount;
	Archive::AutoVariable<int>	m_marketAuctionBidsCount;
};

#endif
