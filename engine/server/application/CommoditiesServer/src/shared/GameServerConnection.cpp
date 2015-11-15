// ======================================================================
//
// ConfigCommodityServer.h
// Copyright 2004, Sony Online Entertainment Inc., all rights reserved.
// Author: Doug Mellencamp
// Server Infrastructure by: Justin Randall
//
// This is message handler for incoming gameserver messages.
//
// ======================================================================

#include "FirstCommodityServer.h"
#include "CommodityServer.h"
#include "GameServerConnection.h"
#include "AuctionMarket.h"
#include "ConfigCommodityServer.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "serverNetworkMessages/AcceptHighBidMessage.h"
#include "serverNetworkMessages/AddAuctionMessage.h"
#include "serverNetworkMessages/AddBidMessage.h"
#include "serverNetworkMessages/AddImmediateAuctionMessage.h"
#include "serverNetworkMessages/CancelAuctionMessage.h"
#include "serverNetworkMessages/CreateVendorMarketMessage.h"
#include "serverNetworkMessages/DeleteAuctionLocationMessage.h"
#include "serverNetworkMessages/DestroyVendorMarketMessage.h"
#include "serverNetworkMessages/GetItemDetailsMessage.h"
#include "serverNetworkMessages/GetItemMessage.h"
#include "serverNetworkMessages/SetSalesTaxMessage.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "serverNetworkMessages/GetPlayerVendorCountMessage.h"
#include "serverNetworkMessages/GetVendorOwnerMessage.h"
#include "serverNetworkMessages/GetVendorValueMessage.h"
#include "serverNetworkMessages/QueryAuctionHeadersMessage.h"
#include "serverNetworkMessages/SetGameTimeMessage.h"
#include "serverNetworkMessages/CleanupInvalidItemRetrievalMessage.h"
#include "serverNetworkMessages/QueryVendorItemCountMessage.h"
#include "serverNetworkMessages/UpdateVendorStatusMessage.h"

//-----------------------------------------------------------------------

namespace GameServerConnectionNamespace
{
}

using namespace GameServerConnectionNamespace;


//-----------------------------------------------------------------------

GameServerConnection::GameServerConnection(UdpConnectionMT * u, TcpClient * t) :
ServerConnection(u, t),
m_gameServerId(0),
m_showAllDebugInfo(ConfigCommodityServer::getShowAllDebugInfo())
{
}

//-----------------------------------------------------------------------

GameServerConnection::~GameServerConnection()
{
}

//-----------------------------------------------------------------------

void GameServerConnection::onConnectionClosed()
{
	//REPORT_LOG("GameServerConnection", ("Connection with GameServer has closed\n"));

	CommodityServer::getInstance().removeGameServer(m_gameServerId);
	DEBUG_REPORT_LOG(true, ("[Commodities Server] : Game Server %d disconnected\n", m_gameServerId));
}

//-----------------------------------------------------------------------

void GameServerConnection::onConnectionOpened()
{
	m_gameServerId = CommodityServer::getInstance().addGameServer(*this);
	//setOverflowLimit(ConfigLoginServer::getClientOverflowLimit());
	
	DEBUG_REPORT_LOG(true, ("[Commodities Server] : Game Server %d opened connection.\n", m_gameServerId));
}

//-----------------------------------------------------------------------

void GameServerConnection::onReceive(const Archive::ByteStream & message)
{

	Archive::ReadIterator ri = message.begin();
	GameNetworkMessage msg(ri);
	ri = message.begin();

	if(msg.isType("AcceptHighBidMessage"))
	{
		DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server] : Received message AcceptHighBidMessage from game server id: %d.\n", m_gameServerId ));
		AcceptHighBidMessage message(ri);
		message.SetTrackId(m_gameServerId);
		AuctionMarket::getInstance().AcceptHighBid(message);
	}
	else if(msg.isType("AddAuctionMessage"))
	{
		DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server] : Received message AddAuctionMessage from game server id: %d.\n", m_gameServerId ));
		AddAuctionMessage message(ri);
		message.SetTrackId(m_gameServerId);
		AuctionMarket::getInstance().AddAuction(message);
	}
	else if(msg.isType("AddBidMessage"))
	{
		DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server] : Received message AddBidMessage from game server id: %d.\n", m_gameServerId ));
		AddBidMessage message(ri);
		message.SetTrackId(m_gameServerId);
		AuctionMarket::getInstance().AddBid(message);
	}
	else if(msg.isType("AddImmediateAuctionMessage"))
	{
		DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server] : Received message AddImmediateAuctionMessage from game server id: %d.\n", m_gameServerId ));
		AddImmediateAuctionMessage message(ri);
		message.SetTrackId(m_gameServerId);
		AuctionMarket::getInstance().AddImmediateAuction(message);
	}
	else if(msg.isType("CancelAuctionMessage"))
	{
		DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server] : Received message CancelAuctionMessage from game server id: %d.\n", m_gameServerId ));
		CancelAuctionMessage message(ri);
		message.SetTrackId(m_gameServerId);
		AuctionMarket::getInstance().CancelAuction(message);
	}
	else if(msg.isType("CreateVendorMarketMessage"))
	{
		DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server] : Received message CreateVendorMarketMessage from game server id: %d.\n", m_gameServerId ));
		CreateVendorMarketMessage message(ri);
		message.SetTrackId(m_gameServerId);
		AuctionMarket::getInstance().CreateVendorMarket(message);
	}
	else if(msg.isType("DestroyVendorMarketMessage"))
	{
		DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server] : Received message DestroyVendorMarketMessage from game server id: %d.\n", m_gameServerId ));
		DestroyVendorMarketMessage message(ri);
		message.SetTrackId(m_gameServerId);
		AuctionMarket::getInstance().DestroyVendorMarket(message);
	}
	else if(msg.isType("DeleteAuctionLocationMessage"))
	{
		DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server] : Received message DeleteAuctionLocationMessage from game server id: %d.\n", m_gameServerId ));
		DeleteAuctionLocationMessage message(ri);
		message.SetTrackId(m_gameServerId);
		AuctionMarket::getInstance().DeleteAuctionLocation(message);
	}
	else if(msg.isType("GetItemDetailsMessage"))
	{
		DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server] : Received message GetItemDetailsMessage from game server id: %d.\n", m_gameServerId ));
		GetItemDetailsMessage message(ri);
		message.SetTrackId(m_gameServerId);
		AuctionMarket::getInstance().GetItemDetails(message);
	}
	else if(msg.isType("GetItemMessage"))
	{
		DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server] : Received message GetItemMessage from game server id: %d.\n", m_gameServerId ));
		GetItemMessage message(ri);
		message.SetTrackId(m_gameServerId);
		AuctionMarket::getInstance().GetItem(message);
	}
	else if(msg.isType("SetSalesTaxMessage"))
	{
		DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server] : Received message SetSalesTaxMessage from game server id: %d.\n", m_gameServerId ));
		SetSalesTaxMessage message(ri);
		message.SetTrackId(m_gameServerId);
		AuctionMarket::getInstance().SetSalesTax(message);
	}
	else if(msg.isType("GetVendorOwnerMessage"))
	{
		DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server] : Received message GetVendorOwnerMessage from game server id: %d.\n", m_gameServerId ));
		GetVendorOwnerMessage message(ri);
		message.SetTrackId(m_gameServerId);
		AuctionMarket::getInstance().GetVendorOwner(message);
	}
	else if(msg.isType("GetVendorValueMessage"))
	{
		DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server] : Received message GetVendorValueMessage from game server id: %d.\n", m_gameServerId ));
		GetVendorValueMessage message(ri);
		message.SetTrackId(m_gameServerId);
		AuctionMarket::getInstance().GetVendorValue(message);
	}
	else if(msg.isType("QueryAuctionHeadersMessage"))
	{
		DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server] : Received message QueryAuctionHeadersMessage from game server id: %d.\n", m_gameServerId ));
		QueryAuctionHeadersMessage message(ri);
		message.SetTrackId(m_gameServerId);
		AuctionMarket::getInstance().QueryAuctionHeaders(message);
	}
	else if(msg.isType("SetGameTimeMessage"))
	{
		DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server] : Received message SetGameTimeMessage from game server id: %d.\n", m_gameServerId ));
		SetGameTimeMessage message(ri);
		message.SetTrackId(m_gameServerId);
		AuctionMarket::getInstance().SetGameTime(message);
	}
	else if(msg.isType("CleanupInvalidItemRetrievalMessage"))
	{
		DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server] : Received message CleanupInvalidItemRetrievalMessage from game server id: %d.\n", m_gameServerId ));
		CleanupInvalidItemRetrievalMessage message(ri);
		message.SetTrackId(m_gameServerId);
		AuctionMarket::getInstance().CleanupInvalidItemRetrieval(message);
	}
	else if(msg.isType("QueryVendorItemCountMessage"))
	{
		DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server] : Received message QueryVendorItemCountMessage from game server id: %d.\n", m_gameServerId ));
		QueryVendorItemCountMessage message(ri);
		message.SetTrackId(m_gameServerId);
		AuctionMarket::getInstance().QueryVendorItemCount(message);
	}
	else if(msg.isType("GetPlayerVendorCountMessage"))
	{
		DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server] : Received message GetPlayerVendorCountMessage from game server id: %d.\n", m_gameServerId ));
		GetPlayerVendorCountMessage message(ri);
		message.SetTrackId(m_gameServerId);
		AuctionMarket::getInstance().GetPlayerVendorCount(message);
	}
	else if(msg.isType("UpdateVendorSearchOptionMessage"))
	{
		DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server] : Received message UpdateVendorSearchOptionMessage from game server id: %d.\n", m_gameServerId ));
		UpdateVendorSearchOptionMessage message(ri);
		message.SetTrackId(m_gameServerId);
		AuctionMarket::getInstance().UpdateVendorSearchOption(message);
	}
	else if(msg.isType("SetEntranceChargeMessage"))
	{
		DEBUG_REPORT_LOG(/*m_showAllDebugInfo*/ true, ("[Commodities Server] : Received message SetEntranceChargeMessage from game server id: %d.\n", m_gameServerId ));
		SetEntranceChargeMessage message(ri);
		message.SetTrackId(m_gameServerId);
		AuctionMarket::getInstance().SetEntranceCharge(message);
	}
	else if(msg.isType("ResponseCommoditiesExcludedGotTypes"))
	{
		DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server] : Received message ResponseCommoditiesExcludedGotTypes from game server id: %d.\n", m_gameServerId ));
		GenericValueTypeMessage<std::map<int, std::string> > message(ri);
		AuctionMarket::getInstance().VerifyExcludedGotTypes(message.getValue());
	}
	else if(msg.isType("ResponseCommoditiesExcludedResourceClasses"))
	{
		DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server] : Received message ResponseCommoditiesExcludedResourceClasses from game server id: %d.\n", m_gameServerId ));
		GenericValueTypeMessage<std::set<std::string> > message(ri);
		AuctionMarket::getInstance().VerifyExcludedResourceClasses(message.getValue());
	}
	else if(msg.isType("ResponseResourceTreeHierarchy"))
	{
		DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server] : Received message ResponseResourceTreeHierarchy from game server id: %d.\n", m_gameServerId ));
		GenericValueTypeMessage<std::map<int, std::set<int> > > message(ri);
		AuctionMarket::getInstance().SetResourceTreeHierarchy(message.getValue());
	}
	else if(msg.isType("UpdateVendorStatusMessage"))
	{
		DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server] : Received message UpdateVendorStatusMessage from game server id: %d.\n", m_gameServerId));
		UpdateVendorStatusMessage message(ri);
		AuctionMarket::getInstance().UpdateVendorStatus(message);
	}
	else if(msg.isType("UpdateVendorLocationMessage"))
	{
		DEBUG_REPORT_LOG(m_showAllDebugInfo, ("[Commodities Server] : Received message UpdateVendorLocationMessage from game server id: %d.\n", m_gameServerId));
		GenericValueTypeMessage<std::pair<NetworkId, std::string> > const message(ri);
		AuctionMarket::getInstance().UpdateVendorLocation(message.getValue().first, message.getValue().second);
	}
	else if(msg.isType("GetItemAttributeData"))
	{
		GenericValueTypeMessage<std::pair<std::pair<NetworkId, std::string>, std::pair<std::pair<int, bool>, std::pair<bool, int> > > > const message(ri);
		AuctionMarket::getInstance().getItemAttributeData(m_gameServerId, message.getValue().first.first, message.getValue().first.second, message.getValue().second.first.first, message.getValue().second.first.second, message.getValue().second.second.first, message.getValue().second.second.second);
	}
	else if(msg.isType("GetItemAttributeDataValues"))
	{
		GenericValueTypeMessage<std::pair<std::pair<NetworkId, std::pair<int, bool> >, std::pair<std::string, int> > > const message(ri);
		AuctionMarket::getInstance().getItemAttributeDataValues(m_gameServerId, message.getValue().first.first, message.getValue().first.second.first, message.getValue().first.second.second, message.getValue().second.first, message.getValue().second.second);
	}
	else if(msg.isType("GetItemAttribute"))
	{
		GenericValueTypeMessage<std::pair<NetworkId, NetworkId> > const message(ri);
		AuctionMarket::getInstance().getItemAttribute(m_gameServerId, message.getValue().first, message.getValue().second);
	}
	else if(msg.isType("GetVendorInfoForPlayer"))
	{
		GenericValueTypeMessage<std::pair<std::pair<NetworkId, NetworkId>, bool> > const message(ri);
		AuctionMarket::getInstance().getVendorInfoForPlayer(m_gameServerId, message.getValue().first.first, message.getValue().first.second, message.getValue().second);
	}
	else if(msg.isType("GetALPQ"))
	{
		GenericValueTypeMessage<std::pair<NetworkId, int> > const message(ri);
		AuctionMarket::getInstance().getAuctionLocationPriorityQueue(m_gameServerId, message.getValue().first, message.getValue().second);
	}
}

//-----------------------------------------------------------------------
