#ifndef SetGameTimeMessage_h
#define SetGameTimeMessage_h

//#include "AuctionBase.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

class SetGameTimeMessage : public GameNetworkMessage
{

public:
	SetGameTimeMessage(int requestId, int gameTime);
	SetGameTimeMessage(Archive::ReadIterator & source);
	~SetGameTimeMessage();

	int                        GetGameTime() const {return m_gameTime.get();}
	int                        GetResponseId () const {return m_responseId.get();}
	int                        GetTrackId    () const {return m_trackId.get();}
	void                       SetTrackId    (int trackId) {m_trackId.set(trackId);}

protected:

private:
	Archive::AutoVariable<int>                    m_responseId;
	Archive::AutoVariable<int>                    m_trackId;
	Archive::AutoVariable<int>                    m_gameTime;

};

#endif
