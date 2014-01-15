#ifndef AddImmediateAuction_h
#define AddImmediateAuction_h

//#include "AuctionBase.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"
#include "Unicode.h"

class AddImmediateAuctionMessage : public GameNetworkMessage
{

public:
	AddImmediateAuctionMessage(
		int responseId,
		const NetworkId & ownerId,
		const std::string & ownerName,
		int price,
		int auctionTimer,
		const NetworkId & itemId,
		int itemNameLength,
		const Unicode::String & itemName,
		int itemType,
		int itemTemplateId,
		int expireTimer,
		const NetworkId & locationId,
		const std::string & location,
		int flags,
		int userDescriptionLength,
		const Unicode::String & userDescription,
		std::vector<std::pair<std::string, Unicode::String> > const & attributes,
		int itemSize,
		int vendorLimit,
		int vendorItemLimit);
	AddImmediateAuctionMessage(Archive::ReadIterator & source);
	~AddImmediateAuctionMessage();

	const NetworkId &          GetOwnerId() const {return m_ownerId.get();}
	const std::string &        GetOwnerName() const {return m_ownerName.get();}
	int                        GetPrice() const {return m_price.get();}
	int                        GetAuctionTimer() const {return m_auctionTimer.get();}
	const NetworkId &          GetItemId() const {return m_itemId.get();}
	int                        GetItemNameLength() const {return m_itemNameLength.get();}
	const Unicode::String &    GetItemName() const {return m_itemName.get();}
	int                        GetItemType() const {return m_itemType.get();}
	int                        GetItemTemplateId() const {return m_itemTemplateId.get();}
	int                        GetExpireTimer() const {return m_expireTimer.get();}
	const std::string &        GetLocation() const {return m_location.get();}
	const NetworkId &          GetLocationId() const {return m_locationId.get();}
	int                        GetFlags() const {return m_flags.get();}
	int                        GetUserDescriptionLength() const {return m_userDescriptionLength.get();}
	const Unicode::String &    GetUserDescription() const {return m_userDescription.get();}
	std::vector<std::pair<std::string, Unicode::String> > const & GetAttributes() const {return m_attributes.get();}
	int                        GetItemSize() const {return m_itemSize.get();}
	int                        GetVendorLimit() const {return m_vendorLimit.get();}
	int                        GetVendorItemLimit() const {return m_vendorItemLimit.get();}
	int                        GetResponseId () const {return m_responseId.get();}
	int                        GetTrackId    () const {return m_trackId.get();}
	void                       SetTrackId    (int trackId) {m_trackId.set(trackId);}

protected:

private:
	Archive::AutoVariable<int>              m_responseId;
	Archive::AutoVariable<int>              m_trackId;
	Archive::AutoVariable<NetworkId>        m_ownerId;
	Archive::AutoVariable<std::string>      m_ownerName;
	Archive::AutoVariable<int>              m_price;
	Archive::AutoVariable<int>              m_auctionTimer;
	Archive::AutoVariable<NetworkId>        m_itemId;
	Archive::AutoVariable<int>              m_itemNameLength;
	Archive::AutoVariable<Unicode::String>  m_itemName;
	Archive::AutoVariable<int>              m_itemType;
	Archive::AutoVariable<int>              m_itemTemplateId;
	Archive::AutoVariable<int>              m_expireTimer;
	Archive::AutoVariable<NetworkId>        m_locationId;
	Archive::AutoVariable<std::string>      m_location;
	Archive::AutoVariable<int>              m_flags;
	Archive::AutoVariable<int>              m_userDescriptionLength;
	Archive::AutoVariable<Unicode::String>  m_userDescription;
	Archive::AutoArray<std::pair<std::string, Unicode::String> > m_attributes;
	Archive::AutoVariable<int>              m_itemSize;
	Archive::AutoVariable<int>              m_vendorLimit;
	Archive::AutoVariable<int>              m_vendorItemLimit;

};

#endif
