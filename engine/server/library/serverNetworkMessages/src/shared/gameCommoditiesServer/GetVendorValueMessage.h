#ifndef GetVendorValueMessage_h
#define GetVendorValueMessage_h

//#include "AuctionBase.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

class GetVendorValueMessage : public GameNetworkMessage
{
public:
	GetVendorValueMessage(
		int responseId,
		const std::string & location);


	GetVendorValueMessage(Archive::ReadIterator & source);
	~GetVendorValueMessage();

	const std::string &        GetLocation() const {return m_location.get();}
	int                        GetResponseId () const {return m_responseId.get();}
	int                        GetTrackId    () const {return m_trackId.get();}
	void                       SetTrackId    (int trackId) {m_trackId.set(trackId);}

protected:

private:
	Archive::AutoVariable<int>              m_responseId;
	Archive::AutoVariable<int>              m_trackId;
	Archive::AutoVariable<std::string>      m_location;
};

#endif
