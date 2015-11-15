#ifndef OnGetItemDetailsMessage_h
#define OnGetItemDetailsMessage_h

//#include "AuctionBase.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"
#include "Unicode.h"

class OnGetItemDetailsMessage : public GameNetworkMessage
{

public:
	OnGetItemDetailsMessage(
		int responseId,
		int resultCode,
		const NetworkId & itemId,
		const NetworkId & playerId,
		int userDescriptionLength,
		const Unicode::String & userDescription,
		int oobLength,
		const Unicode::String & oobData,
		std::vector<std::pair<std::string, Unicode::String> > const & attributes
	);
	OnGetItemDetailsMessage(Archive::ReadIterator & source);
	~OnGetItemDetailsMessage();

	const NetworkId &          GetItemId()      const {return m_itemId.get();}
	const NetworkId &          GetPlayerId()    const {return m_playerId.get();}
	int                        GetUserDescriptionLength() const {return m_userDescriptionLength.get();}
	const Unicode::String &    GetUserDescription() const {return m_userDescription.get();}
	int                        GetOobLength()   const {return m_oobLength.get();}
	const Unicode::String &    GetOobData()     const {return m_oobData.get();}
	std::vector<std::pair<std::string, Unicode::String> > const & GetAttributes() const {return m_attributes.get();}
	int                        GetResultCode()  const {return m_resultCode.get();}
	int                        GetResponseId () const {return m_responseId.get();}
	int                        GetTrackId    () const {return m_trackId.get();}
	void                       SetTrackId    (int trackId) {m_trackId.set(trackId);}

protected:

private:
	Archive::AutoVariable<int>              m_responseId;
	Archive::AutoVariable<int>              m_trackId;
	Archive::AutoVariable<NetworkId>        m_itemId;
	Archive::AutoVariable<NetworkId>        m_playerId;
	Archive::AutoVariable<int>              m_userDescriptionLength;
	Archive::AutoVariable<Unicode::String>  m_userDescription;
	Archive::AutoVariable<int>              m_oobLength;
	Archive::AutoVariable<Unicode::String>  m_oobData;
	Archive::AutoArray<std::pair<std::string, Unicode::String> > m_attributes;
	Archive::AutoVariable<int>              m_resultCode;
};

#endif
