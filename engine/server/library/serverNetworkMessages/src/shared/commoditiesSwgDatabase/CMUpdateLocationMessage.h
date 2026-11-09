#ifndef CMUPDATELOCATIONMESSAGE_H
#define CMUPDATELOCATIONMESSAGE_H

//#include "AuctionBase.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"
//#include "Unicode.h"

class CMUpdateLocationMessage : public GameNetworkMessage
{

public:
	explicit CMUpdateLocationMessage(
		const NetworkId &    locationId,
		const NetworkId &    ownerId,
		const std::string &  locationString,
		int                  salesTax,
		const NetworkId &    bankId,
		int64_t              emptyDate,
		int64_t              lastAccessDate,
		int64_t              inactiveDate,
		int                  status,
		bool                 searchEnabled,
		int                  entranceCharge
	);

	CMUpdateLocationMessage(Archive::ReadIterator & source);
	~CMUpdateLocationMessage();

	const NetworkId &      GetLocationId()        const {return m_locationId.get();}
	const NetworkId &      GetOwnerId()           const {return m_ownerId.get();}
	const std::string &    GetLocationString()    const {return m_locationString.get();}
	int                    GetSalesTax()          const {return m_salesTax.get();}
	const NetworkId &      GetBankId()            const {return m_bankId.get();}
	int64_t                GetEmptyDate()         const {return m_emptyDate.get();}
	int64_t                GetLastAccessDate()    const {return m_lastAccessDate.get();}
	int64_t                GetInactiveDate()      const {return m_inactiveDate.get();}
	int                    GetStatus()            const {return m_status.get();}
	int                    GetEntranceCharge()    const {return m_entranceCharge.get();}
	bool                   GetSearchEnabled()     const {return m_searchEnabled.get();}
protected:

private:
	Archive::AutoVariable<NetworkId>    m_locationId;
	Archive::AutoVariable<NetworkId>    m_ownerId;
	Archive::AutoVariable<std::string>  m_locationString;
	Archive::AutoVariable<int>          m_salesTax;
	Archive::AutoVariable<NetworkId>    m_bankId;
	Archive::AutoVariable<int64_t>          m_emptyDate;
	Archive::AutoVariable<int64_t>          m_lastAccessDate;
	Archive::AutoVariable<int64_t>          m_inactiveDate;
	Archive::AutoVariable<int>          m_status;
	Archive::AutoVariable<bool>         m_searchEnabled;
	Archive::AutoVariable<int>          m_entranceCharge;
};


#endif
