// ======================================================================
//
// CreateAuctionResponseMessage.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef	_CreateAuctionResponseMessage_H
#define	_CreateAuctionResponseMessage_H

//-----------------------------------------------------------------------

#include "Archive/AutoDeltaByteStream.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedNetworkMessages/AuctionErrorCodes.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class CreateAuctionResponseMessage : public GameNetworkMessage
{
public:

	static const char * const MessageType;

	CreateAuctionResponseMessage(NetworkId itemId, AuctionResult result, std::string const &itemRestrictedRejectionMessage);
	explicit CreateAuctionResponseMessage(Archive::ReadIterator &source);

	~CreateAuctionResponseMessage();

public: // methods

	NetworkId                        getItemId() const;
	AuctionResult                    getResult() const;
	std::string const &              getItemRestrictedRejectionMessage() const;

public: // types

private: 
	Archive::AutoVariable<NetworkId> m_itemId;        //the item to be auctioned
	Archive::AutoVariable<int>       m_result;        //the auction container
	Archive::AutoVariable<std::string> m_itemRestrictedRejectionMessage;
};

// ----------------------------------------------------------------------

inline NetworkId CreateAuctionResponseMessage::getItemId() const
{
	return m_itemId.get();
}

// ----------------------------------------------------------------------

inline AuctionResult CreateAuctionResponseMessage::getResult() const
{
	return static_cast<AuctionResult>(m_result.get());
}

// ----------------------------------------------------------------------

inline std::string const & CreateAuctionResponseMessage::getItemRestrictedRejectionMessage() const
{
	return m_itemRestrictedRejectionMessage.get();
}

// ----------------------------------------------------------------------

#endif // _CreateAuctionResponseMessage_H

