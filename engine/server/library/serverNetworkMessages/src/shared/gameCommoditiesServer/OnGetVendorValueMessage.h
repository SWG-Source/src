#ifndef OnGetVendorValueMessage_h
#define OnGetVendorValueMessage_h

//#include "AuctionBase.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

class OnGetVendorValueMessage : public GameNetworkMessage
{

public:
	OnGetVendorValueMessage(int responseId, const std::string & location, int value );
	OnGetVendorValueMessage(Archive::ReadIterator & source);
	~OnGetVendorValueMessage();

	int                        GetValue() const {return m_value.get();}
	const std::string &        GetLocation() const {return m_location.get();}
	int                        GetResponseId () const {return m_responseId.get();}
	int                        GetTrackId    () const {return m_trackId.get();}
	void                       SetTrackId    (int trackId) {m_trackId.set(trackId);}

protected:

private:
	Archive::AutoVariable<int>              m_responseId;
	Archive::AutoVariable<int>              m_trackId;
	Archive::AutoVariable<std::string>      m_location;
	Archive::AutoVariable<int>              m_value;

};

#endif
