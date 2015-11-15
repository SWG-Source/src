#ifndef OnGetPlayerVendorCountMessage_h
#define OnGetPlayerVendorCountMessage_h

#include "sharedNetworkMessages/GameNetworkMessage.h"
#include <vector>

class OnGetPlayerVendorCountMessage : public GameNetworkMessage
{
public:
	OnGetPlayerVendorCountMessage(
		int responseId,
		const NetworkId & playerId,
		int vendorCount,
		const std::vector<NetworkId> & vendorList);


	OnGetPlayerVendorCountMessage(Archive::ReadIterator & source);
	~OnGetPlayerVendorCountMessage();

	const NetworkId &              GetPlayerId() const {return m_playerId.get();}
	int                            GetVendorCount() const {return m_vendorCount.get();}
	const std::vector<NetworkId> & GetVendorList() const {return m_vendorList.get();}
	int                            GetResponseId () const {return m_responseId.get();}
	int                            GetTrackId    () const {return m_trackId.get();}
	void                           SetTrackId    (int trackId) {m_trackId.set(trackId);}

protected:

private:
	Archive::AutoVariable<int>                     m_responseId;
	Archive::AutoVariable<int>                     m_trackId;
	Archive::AutoVariable<NetworkId>               m_playerId;
	Archive::AutoVariable<int>                     m_vendorCount;
	Archive::AutoVariable<std::vector<NetworkId> > m_vendorList;
};

#endif
