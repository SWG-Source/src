#ifndef CMCREATEAUCTIONMESSAGE_H
#define CMCREATEAUCTIONMESSAGE_H

//#include "AuctionBase.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"
#include "Unicode.h"

class CMCreateAuctionMessage : public GameNetworkMessage
{

public:
	explicit CMCreateAuctionMessage(
		const NetworkId &      creatorId,
		int                     minimumBid,
		int                     auctionTimer,
		int                     buyNowPrice,
		int                     userDescriptionLength,
		const Unicode::String & userDescription,
		std::vector<std::pair<std::string, Unicode::String> > const & attributes,
		const NetworkId &       locationId,
		const NetworkId &       itemId,
		int                     itemType,
		int                     itemTemplateId,
		int                     expireTimer,
		int                     itemNameLength,
		const Unicode::String & itemName,
		const NetworkId &       ownerId,
		int                     flags,
		int                     itemSize
	);

	CMCreateAuctionMessage(Archive::ReadIterator & source);
	~CMCreateAuctionMessage();

	const NetworkId &      GetCreatorId()       const {return m_creatorId.get();}
	int                    GetMinimumBid()      const {return m_minimumBid.get();}
	int                    GetAuctionTimer()    const {return m_auctionTimer.get();}
	int                    GetBuyNowPrice()     const {return m_buyNowPrice.get();}
	int                    GetUserDescLength()  const {return m_userDescriptionLength.get();}
	const Unicode::String &GetUserDescription() const {return m_userDescription.get();}
	std::vector<std::pair<std::string, Unicode::String> > const & GetAttributes() const {return m_attributes.get();}
	const NetworkId &      GetLocationId()      const {return m_locationId.get();}
	const NetworkId &      GetItemId()          const {return m_itemId.get();}
	int                    GetItemType()        const {return m_itemType.get();}
	int                    GetItemTemplateId()  const {return m_itemTemplateId.get();}
	int                    GetExpireTimer()     const {return m_expireTimer.get();}
	int                    GetItemNameLength()  const {return m_itemNameLength.get();}
	const Unicode::String &GetItemName()        const {return m_itemName.get();}
	const NetworkId &      GetOwnerId()         const {return m_ownerId.get();}
	int                    GetFlags()           const {return m_flags.get();}
	int                    GetItemSize()        const {return m_itemSize.get();}
		
protected:

private:
	Archive::AutoVariable<NetworkId>       m_creatorId;
	Archive::AutoVariable<int>             m_minimumBid;
	Archive::AutoVariable<int>             m_auctionTimer;
	Archive::AutoVariable<int>             m_buyNowPrice;
	Archive::AutoVariable<int>             m_userDescriptionLength;
	Archive::AutoVariable<Unicode::String> m_userDescription;
	Archive::AutoArray<std::pair<std::string, Unicode::String> > m_attributes;
	Archive::AutoVariable<NetworkId>       m_locationId;
	Archive::AutoVariable<NetworkId>       m_itemId;
	Archive::AutoVariable<int>             m_itemType;
	Archive::AutoVariable<int>             m_itemTemplateId;
	Archive::AutoVariable<int>             m_expireTimer;
	Archive::AutoVariable<int>             m_itemNameLength;
	Archive::AutoVariable<Unicode::String> m_itemName;
	Archive::AutoVariable<NetworkId>       m_ownerId;
	Archive::AutoVariable<int>             m_flags;
	Archive::AutoVariable<int>             m_itemSize;
};

#endif
