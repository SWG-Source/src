#ifndef SetSalesTaxMessage_h
#define SetSalesTaxMessage_h

//#include "AuctionBase.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"

class SetSalesTaxMessage : public GameNetworkMessage
{

public:
	SetSalesTaxMessage(
		int requestId,
		int salesTax,
		const NetworkId & bankId,
		const std::string & location
	);
	SetSalesTaxMessage(Archive::ReadIterator & source);
	~SetSalesTaxMessage();

	int                        GetSalesTax()   const {return m_salesTax.get();}
	const NetworkId &          GetBankId() const {return m_bankId.get();}
	const std::string &        GetLocation() const {return m_location.get();}
	int                        GetResponseId () const {return m_responseId.get();}
	int                        GetTrackId    () const {return m_trackId.get();}
	void                       SetTrackId    (int trackId) {m_trackId.set(trackId);}

protected:

private:
	Archive::AutoVariable<int>                    m_responseId;
	Archive::AutoVariable<int>                    m_trackId;
	Archive::AutoVariable<int>                    m_salesTax;
	Archive::AutoVariable<NetworkId>              m_bankId;
	Archive::AutoVariable<std::string>            m_location;

};

#endif
