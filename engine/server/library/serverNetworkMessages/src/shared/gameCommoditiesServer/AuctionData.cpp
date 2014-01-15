#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include <string.h>
#include "AuctionData.h"

AuctionDataHeader::AuctionDataHeader() :
type(0),
auctionId(),
itemId(),
itemNameLength(0),
itemName(),
minBid(0),
highBid(0),
timer(0),
buyNowPrice(0),
location(),
ownerId(),
highBidderId(),
maxProxyBid(0),
myBid(0),
itemType(0),
resourceContainerClassCrc(0),
flags(0),
entranceCharge(0)
{

}

AuctionDataHeader::AuctionDataHeader(const AuctionDataHeader & original) :
type(original.type),
auctionId(original.auctionId),
itemId(original.itemId),
itemNameLength(original.itemNameLength),
itemName(original.itemName),
minBid(original.minBid),
highBid(original.highBid),
timer(original.timer),
buyNowPrice(original.buyNowPrice),
location(original.location),
ownerId(original.ownerId),
highBidderId(original.highBidderId),
maxProxyBid(original.maxProxyBid),
myBid(original.myBid),
itemType(original.itemType),
resourceContainerClassCrc(original.resourceContainerClassCrc),
flags(original.flags),
entranceCharge(original.entranceCharge)
{

}

AuctionDataHeader::~AuctionDataHeader()
{

}
