
#ifndef AUCTIONSTRANFERCLIENT_H
#define AUCTIONSTRANFERCLIENT_H

#ifndef _WIN32
#include "AuctionTransferGameAPI/AuctionTransferAPI.h"
#else

namespace AuctionTransfer
{
	class AuctionTransferAPI
	{
	public:
		AuctionTransferAPI( const char *[], const short [], int , const char *[], unsigned )
		{};
		virtual void process() {};
	};

};

#endif


class AuctionTransferClient : public AuctionTransfer::AuctionTransferAPI
{
public:
    AuctionTransferClient(const char *hostName[], const short port[], int count, const char *identifier[], unsigned identifierCount);
    virtual ~AuctionTransferClient();


    // Connection status callbacks
    virtual void onConnect(const char* host, unsigned short port, const short current, const short max);
    virtual void onDisconnect(const char *host, const short port, const short current, const short max);


    // Callbacks that are responses
    void onSendPrepareTransaction(unsigned int, unsigned int, void *) {};
    void onSendPrepareTransactionCompressed(unsigned int, unsigned int, void *) {};
    void onSendCommitTransaction(unsigned int, unsigned int, void *) {};
    void onSendAbortTransaction(unsigned int, unsigned int, void *) {};
    void onSendAuditAssetTransfer(unsigned int, unsigned int, void *) {};

    void onGetNewTransactionID(unsigned int, unsigned int, int64, void *) {};

    // Responses to reply requests
    void onReplyReceivePrepareTransaction(unsigned int, unsigned int, void *) {};
    void onReplyReceiveCommitTransaction(unsigned int, unsigned int, void *) {};
    void onReplyReceiveAbortTransaction(unsigned int, unsigned int, void *) {};
    void onReplyReceiveGetCharacterList(unsigned int, unsigned int, void *) {};

    // house keeping
    void onIdentifyHost(unsigned int, unsigned int, void *) {};

    // callbacks initiated by auction
    void onReceivePrepareTransaction(unsigned int, int64, unsigned int, unsigned int, int64, const char *) {};
    void onReceiveCommitTransaction(unsigned int, int64) {};
    void onReceiveAbortTransaction(unsigned int, int64) {};
    void onReceiveGetCharacterList(unsigned int, unsigned int, const char *) {};

private:

/*************************************************
    struct AuctionAssetDetails
    {
        enum eAssetTypes
        {
            TYPE_ITEM,
            TYPE_COIN,
            TYPE_CHARACTER,
            TYPE_FOR_IMAGE_UPLOAD
        };

        AuctionAssetDetails()
            : u8Type( TYPE_ITEM ),
              u32AccountNum( 0 ),
              u32CharDBID( 0 ),
              i64ItemDBID( INVALID_EQUID ),
              u32ItemStackCount( 0 ),
              i64AssetID( 0 )
        {}

        uns8 u8Type;
        uns32 u32AccountNum;
        uns32 u32CharDBID;
        int64 i64ItemDBID;
        uns32 u32ItemStackCount;
        int64 i64Copper;
        CharString sAssetXML;

        int64 i64AssetID;
    };

    typedef std::map< unsigned, AuctionAssetDetails > PendingRequestMap;
    PendingRequestMap m_mPendingRequestDetails;





    struct IncomingPrepareDetails
    {
        int64 i64TransactionID;
        int64 i64AssetID;
        uns32 u32DestStationID;
        uns32 u32DestCharDBID;
    };
    typedef std::map< unsigned, IncomingPrepareDetails > IncomingPrepareMap;//key=tracking
    IncomingPrepareMap m_mPendingIncomingPrepareDetails;



    typedef std::map< unsigned, int64 > IncomingCommitMap;//key=tracking, val=transaction
    IncomingCommitMap m_mPendingIncomingCommitDetails;



    struct TransferDetails
    {
        TransferDetails()
            : i64AssetID( 0 ),
              u32DestStationID( 0 ),
              u32DestCharDBID( INVALID_EQUID )
        {}

        int64 i64AssetID;
        uns32 u32DestStationID;
        uns32 u32DestCharDBID;

        //used when committing
        std::vector< uns8 > aAssetSerialData;
        int32 i32Copper;
    };
    typedef std::map< int64, TransferDetails > PendingTransferMap;
    PendingTransferMap m_mPendingTransferDetails;
**********************************************/


};


#endif
