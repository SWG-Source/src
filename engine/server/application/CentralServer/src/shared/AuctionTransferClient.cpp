#include "FirstCentralServer.h"
#include "AuctionTransferClient.h"
#include "sharedLog/Log.h"


AuctionTransferClient::AuctionTransferClient(const char *hostName[], const short port[], int count, const char *identifier[], unsigned identifierCount) :
	AuctionTransferAPI( hostName, port, count, identifier, identifierCount )
{
}

AuctionTransferClient::~AuctionTransferClient()
{
}


// Connection status callbacks
void AuctionTransferClient::onConnect(const char* host, unsigned short port, const short current, const short max)
{
    LOG("AuctionTransferClient", ("onConnect"));
}

void AuctionTransferClient::onDisconnect(const char *host, const short port, const short current, const short max)
{
    LOG("AuctionTransferClient", ("onDisconnect"));
}


/*****************************
void AuctionTransferClient::addCoinToAuction( const ExchangeListCreditsMessage& msg )
{
    //1. get transaction id from auction system (save msg details for use when get response)
    //2. on response: if success:
    //  2a. send prepare transaction
    //  2b. on response:
    //    2bi. if user connected: send VeAuctionCoinReply (with result code) to user's ES
    //    2bii. if user not connected: send immediate abort back to auction service
    
    const unsigned uTrack = getNewTransactionID( nullptr );

    AuctionAssetDetails &details = m_mPendingRequestDetails[ uTrack ];
    details.u8Type = AuctionAssetDetails::TYPE_COIN;
    details.u32AccountNum = msg.getAccountNum();
    details.u32CharDBID = msg.getCharacterId();
    details.i64ItemDBID = 0;
    details.u32ItemStackCount = 0;
    details.i64Credits = msg.getCredits();
    details.sAssetXML = "";  ///msg.getAssetXML();
}
**********************/









