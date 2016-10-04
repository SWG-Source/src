// ======================================================================
//
// CommoditiesServerConnection.cpp (refactor of original Commodities Market code)
// The old CM API was depricated and all code for comminicating with the
// CM Server now uses this class
//
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "CommoditiesServerConnection.h"
#include "boost/smart_ptr.hpp"
#include "serverGame/CityInfo.h"
#include "serverGame/CityInterface.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/MessageToQueue.h"
#include "serverGame/NameManager.h"
#include "serverScript/GameScriptObject.h"
#include "serverScript/ScriptDictionary.h"
#include "serverScript/ScriptParameters.h"
#include "sharedFoundation/Crc.h"
#include "sharedGame/GameObjectTypes.h"
#include "sharedGame/ResourceClassObject.h"
#include "sharedGame/Universe.h"
#include "sharedLog/Log.h"
#include "sharedNetwork/NetworkSetupData.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "UnicodeUtils.h"
#include "StringId.h"

#include "serverGame/CommoditiesMarket.h"
#include "serverGame/ConsoleManager.h"
#include "serverGame/Chat.h"
#include "serverGame/ServerObject.h"
#include "serverGame/ServerWorld.h"

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
#include "serverNetworkMessages/GetPlayerVendorCountMessage.h"
#include "serverNetworkMessages/GetVendorOwnerMessage.h"
#include "serverNetworkMessages/GetVendorValueMessage.h"
#include "serverNetworkMessages/QueryAuctionHeadersMessage.h"
#include "serverNetworkMessages/SetGameTimeMessage.h"
#include "serverNetworkMessages/CleanupInvalidItemRetrievalMessage.h"

#include "serverNetworkMessages/OnAcceptHighBidMessage.h"
#include "serverNetworkMessages/OnAddAuctionMessage.h"
#include "serverNetworkMessages/OnAddBidMessage.h"
#include "serverNetworkMessages/OnAuctionExpiredMessage.h"
#include "serverNetworkMessages/OnCancelAuctionMessage.h"
#include "serverNetworkMessages/OnCreateVendorMarketMessage.h"
#include "serverNetworkMessages/OnGetItemDetailsMessage.h"
#include "serverNetworkMessages/OnGetItemMessage.h"
#include "serverNetworkMessages/OnGetPlayerVendorCountMessage.h"
#include "serverNetworkMessages/OnGetVendorOwnerMessage.h"
#include "serverNetworkMessages/OnGetVendorValueMessage.h"
#include "serverNetworkMessages/OnItemExpiredMessage.h"
#include "serverNetworkMessages/OnPermanentAuctionPurchasedMessage.h"
#include "serverNetworkMessages/OnVendorRefuseItemMessage.h"
#include "serverNetworkMessages/OnQueryAuctionHeadersMessage.h"
#include "serverNetworkMessages/OnCleanupInvalidItemRetrievalMessage.h"
#include "serverNetworkMessages/VendorStatusChangeMessage.h"
#include "serverNetworkMessages/OnQueryVendorItemCountReplyMessage.h"
#include "serverNetworkMessages/OnUpdateVendorSearchOptionMessage.h"

#include "sharedFoundation/CrcConstexpr.hpp"

// ======================================================================

CommoditiesServerConnection::CommoditiesServerConnection(const std::string & a, const unsigned short p) :
ServerConnection(a, p, NetworkSetupData()),
m_nextRequest(0),
listener(),
m_commoditiesShowAllDebugInfo(ConfigServerGame::getCommoditiesShowAllDebugInfo())
{
}

//-----------------------------------------------------------------------

CommoditiesServerConnection::~CommoditiesServerConnection()
{
}

//-----------------------------------------------------------------------

void CommoditiesServerConnection::onConnectionClosed()
{
	LOG("CMServerConnection", ("CommoditiesAPI : Connection with the Commodities server is closed"));
	static MessageConnectionCallback m("CommoditiesServerConnectionClosed");
	emitMessage(m);
}

//-----------------------------------------------------------------------

void CommoditiesServerConnection::onConnectionOpened()
{
	LOG("CMServerConnection", ("CommoditiesAPI : Connection with the Commodities server is open"));
	static MessageConnectionCallback m("CommoditiesServerConnectionOpened");
	emitMessage(m);
}

//-----------------------------------------------------------------------

void CommoditiesServerConnection::onReceive(const Archive::ByteStream & message)
{
	Archive::ReadIterator ri = message.begin();
	GameNetworkMessage msg(ri);
	ri = message.begin();


	switch (msg.getType()) {
		case constcrc("OnAddAuctionMessage"): {
			OnAddAuctionMessage oaa_message(ri);
			DEBUG_REPORT_LOG(m_commoditiesShowAllDebugInfo, ("[Commodities API] : Received OnAddAuctionMessage.\n"));
			listener.OnAddAuction(oaa_message.GetResponseId(), oaa_message.GetResultCode(), oaa_message
					.GetItemId(), oaa_message.GetOwnerId(), oaa_message.GetOwnerName(), oaa_message
					.GetItemId(), oaa_message.GetVendorId(), NameManager::getInstance()
					.getPlayerFullName(oaa_message.GetVendorId()), oaa_message.GetLocation());
			break;
		}
		case constcrc("OnAddBidMessage"): {
			OnAddBidMessage oab_message(ri);
			DEBUG_REPORT_LOG(m_commoditiesShowAllDebugInfo, ("[Commodities API] : Received OnAddBidMessage.\n"));
			listener.OnAddBid(oab_message.GetResponseId(), oab_message.GetResultCode(), oab_message
					.GetOwnerId(), oab_message.GetItemId(), oab_message.GetBidderId(), oab_message
					.GetPreviousBidderId(), oab_message.GetBid(), oab_message.GetPreviousBid(), oab_message
					.GetMaxProxyBid(), oab_message.GetLocation(), NameManager::getInstance()
					.getPlayerFullName(oab_message.GetOwnerId()), NameManager::getInstance()
					.getPlayerFullName(oab_message.GetPreviousBidderId()), oab_message.GetItemNameLength(), oab_message
					.GetItemName(), oab_message.GetSalesTaxAmount(), oab_message.GetSalesTaxBankId());
			break;
		}
		case constcrc("OnCancelAuctionMessage"): {
			OnCancelAuctionMessage oca_message(ri);
			DEBUG_REPORT_LOG(m_commoditiesShowAllDebugInfo, ("[Commodities API] : Received OnCancelAuctionMessage.\n"));
			listener.OnCancelAuction(oca_message.GetResponseId(), oca_message.GetResultCode(), oca_message
					.GetItemId(), oca_message.GetPlayerId(), oca_message.GetHighBidderId(), oca_message
					.GetHighBid(), oca_message.GetLocation());
			break;
		}
		case constcrc("OnAcceptHighBidMessage"): {
			OnAcceptHighBidMessage oahb_message(ri);
			DEBUG_REPORT_LOG(m_commoditiesShowAllDebugInfo, ("[Commodities API] : Received OnAcceptHighBidMessage.\n"));
			listener.OnAcceptHighBid(oahb_message.GetResponseId(), oahb_message.GetResultCode(), oahb_message
					.GetItemId(), oahb_message.GetPlayerId());
			break;
		}
		case constcrc("OnQueryAuctionHeadersMessage"): {
			OnQueryAuctionHeadersMessage oqah_message(ri);
			DEBUG_REPORT_LOG(m_commoditiesShowAllDebugInfo, ("[Commodities API] : Received OnQueryAuctionHeadersMessage.\n"));
			listener.OnQueryAuctionHeadersReply(oqah_message.GetResponseId(), oqah_message.GetResultCode(), oqah_message
					.GetPlayerId(), oqah_message.GetQueryType(), oqah_message.GetNumAuctions(), oqah_message
					.GetAuctionData(), oqah_message.GetQueryOffset(), oqah_message.HasMorePages());
			break;
		}
		case constcrc("OnGetItemDetailsMessage"): {
			OnGetItemDetailsMessage ogid_message(ri);
			DEBUG_REPORT_LOG(m_commoditiesShowAllDebugInfo, ("[Commodities API OnGetItemDetailsMessage] : Received OnGetItemDetailsMessage.\n"));
			DEBUG_REPORT_LOG(m_commoditiesShowAllDebugInfo, ("[Commodities API OnGetItemDetailsMessage] : Result : %d.\n", ogid_message
					.GetResultCode()));
			DEBUG_REPORT_LOG(m_commoditiesShowAllDebugInfo, ("[Commodities API OnGetItemDetailsMessage] : ItemId : %s.\n", ogid_message
					.GetItemId().getValueString().c_str()));
			DEBUG_REPORT_LOG(m_commoditiesShowAllDebugInfo, ("[Commodities API OnGetItemDetailsMessage] : PlayerId : %s.\n", ogid_message
					.GetPlayerId().getValueString().c_str()));
			DEBUG_REPORT_LOG(m_commoditiesShowAllDebugInfo, ("[Commodities API OnGetItemDetailsMessage] : UserDescriptionLength : %d.\n", ogid_message
					.GetUserDescriptionLength()));
			DEBUG_REPORT_LOG(m_commoditiesShowAllDebugInfo, ("[Commodities API OnGetItemDetailsMessage] : UserDescription : %s.\n", Unicode::wideToNarrow(ogid_message
					.GetUserDescription()).c_str()));
			//		DEBUG_REPORT_LOG(m_commoditiesShowAllDebugInfo, ("[Commodities API OnGetItemDetailsMessage] : OobLength : %d.\n", ogid_message.GetOobLength()));
			//		DEBUG_REPORT_LOG(m_commoditiesShowAllDebugInfo, ("[Commodities API OnGetItemDetailsMessage] : OobActualLength : %d.\n", ogid_message.GetOobData().size()));
			//		DEBUG_REPORT_LOG(m_commoditiesShowAllDebugInfo, ("[Commodities API OnGetItemDetailsMessage] : OobData 	: %s.\n", Unicode::wideToNarrow(ogid_message.GetOobData()).c_str()));
			DEBUG_REPORT_LOG(m_commoditiesShowAllDebugInfo, ("[Commodities API OnGetItemDetailsMessage] : ResponseId : %d.\n", ogid_message
					.GetResponseId()));
			listener.OnGetItemDetailsReply(ogid_message.GetResponseId(), ogid_message.GetResultCode(), ogid_message
					.GetPlayerId(), ogid_message.GetItemId(), ogid_message.GetUserDescriptionLength(), ogid_message
					.GetUserDescription(), ogid_message.GetOobLength(), ogid_message.GetOobData(), ogid_message
					.GetAttributes());
			break;
		}
		case constcrc("OnAuctionExpiredMessage"): {
			OnAuctionExpiredMessage oae_message(ri);
			DEBUG_REPORT_LOG(m_commoditiesShowAllDebugInfo, ("[Commodities API] : Received OnAuctionExpiredMessage.\n"));
			DEBUG_REPORT_LOG(true, ("  -- OwnerId = %s, GetOwnerName = %s, BuyerId = %s, GetBuyerName = %s\n", oae_message
					.GetOwnerId().getValueString().c_str(),
					NameManager::getInstance().getPlayerFullName(oae_message.GetOwnerId()).c_str(),
					oae_message.GetBuyerId().getValueString().c_str(),
					NameManager::getInstance().getPlayerFullName(oae_message.GetBuyerId()).c_str()));
			listener.OnAuctionExpired(oae_message.GetItemId(), oae_message.GetOwnerId(), oae_message
					.IsSold(), oae_message.GetBuyerId(), oae_message.GetBid(), oae_message.GetItemId(), oae_message
					.GetMaxProxyBid(), oae_message.GetLocation(), oae_message.IsImmediate(), NameManager::getInstance()
					.getPlayerFullName(oae_message.GetOwnerId()), NameManager::getInstance()
					.getPlayerFullName(oae_message.GetBuyerId()), oae_message.GetItemNameLength(), oae_message
					.GetItemName(), oae_message.GetSendSellerMail());
			break;
		}
		case constcrc("OnItemExpiredMessage"): {
			OnItemExpiredMessage oie_message(ri);
			DEBUG_REPORT_LOG(m_commoditiesShowAllDebugInfo, ("[Commodities API] : Received OnItemExpiredMessage.\n"));
			listener.OnItemExpired(oie_message.GetOwnerId(), oie_message.GetItemId(),
					NameManager::getInstance().getPlayerFullName(oie_message.GetOwnerId()),
					oie_message.GetItemNameLength(), oie_message.GetItemName(),
					oie_message.GetLocationName(), oie_message.GetLocationId());
			break;
		}
		case constcrc("OnGetItemMessage"): {
			OnGetItemMessage ogi_message(ri);
			DEBUG_REPORT_LOG(m_commoditiesShowAllDebugInfo, ("[Commodities API] : Received OnGetItemMessage.\n"));
			listener.OnGetItemReply(ogi_message.GetResponseId(), ogi_message.GetResultCode(), ogi_message
					.GetItemId(), ogi_message.GetPlayerId(), ogi_message.GetLocation());
			break;
		}
		case constcrc("OnCreateVendorMarketMessage"): {
			OnCreateVendorMarketMessage ocvm_message(ri);
			DEBUG_REPORT_LOG(m_commoditiesShowAllDebugInfo, ("[Commodities API] : Received OnCreateVendorMarketMessage.\n"));
			listener.OnCreateVendorMarket(ocvm_message.GetResponseId(), ocvm_message.GetResultCode(), ocvm_message
					.GetOwnerId(), ocvm_message.GetLocation());
			break;
		}
		case constcrc("OnVendorRefuseItemMessage"): {
			OnVendorRefuseItemMessage ovri_message(ri);
			DEBUG_REPORT_LOG(m_commoditiesShowAllDebugInfo, ("[Commodities API] : Received OnVendorRefuseItemMessage.\n"));
			listener.OnVendorRefuseItem(ovri_message.GetResponseId(), ovri_message.GetResultCode(), ovri_message
					.GetItemId(), ovri_message.GetVendorId(), ovri_message.GetItemOwnerId());
			break;
		}
		case constcrc("OnGetVendorOwnerMessage"): {
			OnGetVendorOwnerMessage ogvo_message(ri);
			DEBUG_REPORT_LOG(m_commoditiesShowAllDebugInfo, ("[Commodities API] : Received OnGetVendorOwnerMessage.\n"));
			listener.OnGetVendorOwner(ogvo_message.GetResponseId(), ogvo_message.GetResultCode(), ogvo_message
							.GetOwnerId(),
					NameManager::getInstance().getPlayerFullName(ogvo_message.GetOwnerId()),
					ogvo_message.GetLocation(), ogvo_message.GetRequesterId());
			break;
		}
		case constcrc("OnGetVendorValueMessage"): {
			OnGetVendorValueMessage ogvv_message(ri);
			DEBUG_REPORT_LOG(m_commoditiesShowAllDebugInfo, ("[Commodities API] : Received OnGetVendorValueMessage.\n"));
			listener.OnGetVendorValue(ogvv_message.GetResponseId(), ogvv_message.GetLocation(), ogvv_message
					.GetValue());
			break;
		}
		case constcrc("OnPermanentAuctionPurchasedMessage"): {
			OnPermanentAuctionPurchasedMessage opa_message(ri);
			DEBUG_REPORT_LOG(m_commoditiesShowAllDebugInfo, ("[Commodities API] : Received OnPermanentAuctionPurchasedMessage.\n"));
			listener.OnPermanentAuctionPurchased(
					opa_message.GetOwnerId(), opa_message.GetBuyerId(), opa_message.GetPrice(), opa_message
							.GetItemId(), opa_message.GetLocation(),
					NameManager::getInstance().getPlayerFullName(opa_message.GetOwnerId()),
					NameManager::getInstance().getPlayerFullName(opa_message.GetBuyerId()),
					opa_message.GetItemNameLength(), opa_message.GetItemName(), opa_message.GetAttributes());
			break;
		}
		case constcrc("OnCleanupInvalidItemRetrievalMessage"): {
			OnCleanupInvalidItemRetrievalMessage ociir_message(ri);
			DEBUG_REPORT_LOG(m_commoditiesShowAllDebugInfo, ("[Commodities API] : Received OnCleanupInvalidItemRetrievalMessage.\n"));
			listener.OnCleanupInvalidItemRetrieval(ociir_message.GetResponseId(), ociir_message
					.GetItemId(), ociir_message.GetPlayerId(), ociir_message.GetCreatorId(), ociir_message
					.GetReimburseAmt());
			break;
		}
		case constcrc("OnGetPlayerVendorCountMessage"): {
			OnGetPlayerVendorCountMessage ogpvc_message(ri);
			DEBUG_REPORT_LOG(m_commoditiesShowAllDebugInfo, ("[Commodities API] : Received OnGetPlayerVendorCountMessage.\n"));
			CommoditiesMarket::playerVendorCountReply(ogpvc_message.GetPlayerId(), ogpvc_message
					.GetVendorCount(), ogpvc_message.GetVendorList());
			break;
		}
		case constcrc("VendorStatusChangeMessage"): {
			VendorStatusChangeMessage vscMessage(ri);
			static char *vendorStatus[] = {"Empty", "Not Empty", "Unaccessed", "Endangered", "Removed"};
			DEBUG_REPORT_LOG(m_commoditiesShowAllDebugInfo, ("[Commodities API] : Received VendorStatusChangeMessage.\n"));
			if (vscMessage.getStatus() >= ARC_VendorStatusEmpty && vscMessage.getStatus() <= ARC_VendorRemoved) {
				LOG("CustomerService", ("Vendor: Vendor %s status changed to %s", vscMessage.getVendorId()
				                                                                            .getValueString()
				                                                                            .c_str(), vendorStatus[
						vscMessage.getStatus() - ARC_VendorStatusEmpty]));
			}
			else {
				WARNING(true, ("[Commodities API] : Invalid vendor status %s %d.\n", vscMessage.getVendorId()
				                                                                               .getValueString()
				                                                                               .c_str(), vscMessage
						.getStatus()));
			}
			CommoditiesMarket::vendorStatusChange(vscMessage.getVendorId(), vscMessage.getStatus());
			break;
		}
		case constcrc("OnQueryVendorItemCountReplyMessage"): {
			OnQueryVendorItemCountReplyMessage qvMessage(ri);
			DEBUG_REPORT_LOG(m_commoditiesShowAllDebugInfo, ("[Commodities API] : Received OnQueryVendorItemCountReplyMessage.\n"));
			CommoditiesMarket::vendorItemCountReply(qvMessage.getVendorId(), qvMessage.getVendorItemCount(), qvMessage
					.getSearchEnabled());
			break;
		}
		case constcrc("OnUpdateVendorSearchOptionMessage"): {
			OnUpdateVendorSearchOptionMessage uvsoMessage(ri);
			DEBUG_REPORT_LOG(m_commoditiesShowAllDebugInfo, ("[Commodities API] : Received OnUpdateVendorSearchOptionMessage.\n"));
			ServerObject *player = ServerWorld::findObjectByNetworkId(uvsoMessage.GetPlayerId());
			if (player) {
				StringId const VENDOR_SEARCH_ENABLED("player_structure", "vendor_search_enabled");
				StringId const VENDOR_SEARCH_DISABLED("player_structure", "vendor_search_disabled");
				if (uvsoMessage.GetEnabled()) {
					Chat::sendSystemMessageSimple(*player, VENDOR_SEARCH_ENABLED, 0);
				}
				else {
					Chat::sendSystemMessageSimple(*player, VENDOR_SEARCH_DISABLED, 0);
				}
			}
			break;
		}
		case constcrc("OnCommodityReplyMessage"): {
			ri = static_cast<const GameNetworkMessage &>(msg).getByteStream().begin();
			GenericValueTypeMessage <std::pair<std::string, std::string>> message(ri);
			DEBUG_REPORT_LOG(m_commoditiesShowAllDebugInfo, ("[Commodities API] : Received OnCommodityReplyMessage.\n"));

			Chat::sendSystemMessage(message.getValue().first, Unicode::narrowToWide(message.getValue()
			                                                                               .second), Unicode::String());
			break;
		}
		case constcrc("CommoditiesItemTypeMap"): {
			GenericValueTypeMessage < std::pair < int, std::map < int, std::set < int > > > > msg(ri);
			DEBUG_REPORT_LOG(m_commoditiesShowAllDebugInfo, ("[Commodities API] : Received CommoditiesItemTypeMap.\n"));
			CommoditiesMarket::updateItemTypeMap(msg.getValue().first, msg.getValue().second);
			break;
		}
		case constcrc("CommoditiesItemTypeAdded"): {
			GenericValueTypeMessage < std::pair < int, std::pair < int, int > > > msg(ri);
			DEBUG_REPORT_LOG(m_commoditiesShowAllDebugInfo, ("[Commodities API] : Received CommoditiesItemTypeAdded.\n"));
			CommoditiesMarket::updateItemTypeMap(msg.getValue().first, msg.getValue().second.first, msg.getValue()
			                                                                                           .second.second);
			break;
		}
		case constcrc("CommoditiesResourceTypeMap"): {
			GenericValueTypeMessage < std::pair < int, std::map < int, std::set < std::string > > > > msg(ri);
			DEBUG_REPORT_LOG(m_commoditiesShowAllDebugInfo, ("[Commodities API] : Received CommoditiesResourceTypeMap.\n"));
			CommoditiesMarket::updateResourceTypeMap(msg.getValue().first, msg.getValue().second);
			break;
		}
		case constcrc("CommoditiesResourceTypeAdded"): {
			GenericValueTypeMessage < std::pair < int, std::pair < int, std::string > > > msg(ri);
			DEBUG_REPORT_LOG(m_commoditiesShowAllDebugInfo, ("[Commodities API] : Received CommoditiesResourceTypeAdded.\n"));
			CommoditiesMarket::updateResourceTypeMap(msg.getValue().first, msg.getValue().second.first, msg.getValue()
			                                                                                               .second
			                                                                                               .second);
			break;
		}
		case constcrc("RequestCommoditiesExcludedGotTypes"): {
			GenericValueTypeMessage<int> msg(ri);
			UNREF(msg);
			DEBUG_REPORT_LOG(m_commoditiesShowAllDebugInfo, ("[Commodities API] : Received RequestCommoditiesExcludedGotTypes.\n"));

			GenericValueTypeMessage <std::map<int, std::string>> reply("ResponseCommoditiesExcludedGotTypes", GameObjectTypes::getTypesExcludedFromCommodities());
			send(reply, true);
			break;
		}
		case constcrc("RequestCommoditiesExcludedResourceClasses"): {
			GenericValueTypeMessage<int> msg(ri);
			UNREF(msg);
			DEBUG_REPORT_LOG(m_commoditiesShowAllDebugInfo, ("[Commodities API] : Received RequestCommoditiesExcludedResourceClasses.\n"));

			GenericValueTypeMessage <std::set<std::string>> reply("ResponseCommoditiesExcludedResourceClasses", ResourceClassObject::getClassesExcludedFromCommodities());
			send(reply, true);
			break;
		}
		case constcrc("RequestResourceTreeHierarchy"): {
			GenericValueTypeMessage<int> msg(ri);
			UNREF(msg);
			DEBUG_REPORT_LOG(m_commoditiesShowAllDebugInfo, ("[Commodities API] : Received RequestResourceTreeHierarchy.\n"));

			ResourceClassObject const *resourceTreeRoot = Universe::getInstance().getResourceTreeRoot();
			if (resourceTreeRoot) {
				std::vector < ResourceClassObject const * > leafChildren;
				resourceTreeRoot->getLeafChildren(leafChildren);

				std::map<int, std::set<int> > resourceTreeHierarchy;
				ResourceClassObject const *parent;
				for (std::vector < ResourceClassObject const * > ::const_iterator
				iter = leafChildren.begin();
				iter != leafChildren.end();
				++iter)
				{
					std::set<int> &parents = resourceTreeHierarchy[static_cast<int>(Crc::calculate((*iter)
							->getResourceClassName().c_str()))];
					parent = (*iter)->getParent();
					while (parent) {
						parents.insert(static_cast<int>(Crc::calculate(parent->getResourceClassName().c_str())));

						if (parent->isRoot()) {
							break;
						}

						parent = parent->getParent();
					}
				}

				GenericValueTypeMessage < std::map < int, std::set < int > > >
				                                          reply("ResponseResourceTreeHierarchy", resourceTreeHierarchy);
				send(reply, true);
			}
			break;
		}
		case constcrc("GetItemAttributeDataRsp"): {
			GenericValueTypeMessage < std::pair < std::pair < NetworkId, std::string >, std::string > >
			const response(ri);

			ServerObject *const requester = ServerWorld::findObjectByNetworkId(response.getValue().first.first);
			if (requester && requester->getClient()) {
				if (response.getValue().first.second == std::string("(~!@#$%^&*)system_mesage(*&^%$#@!~)")) {
					ConsoleMgr::broadcastString(response.getValue().second, requester->getClient());
				}
				else {
					GenericValueTypeMessage <std::pair<std::string, std::string>> const data("SaveTextOnClient", std::make_pair(response
							.getValue().first.second, response.getValue().second));
					requester->getClient()->send(data, true);
				}
			}
			break;
		}
		case constcrc("DisplayStringForPlayer"): {
			GenericValueTypeMessage <std::pair<NetworkId, std::string>> const response(ri);

			ServerObject *const requester = ServerWorld::findObjectByNetworkId(response.getValue().first);
			if (requester && requester->getClient()) {
				ConsoleMgr::broadcastString(response.getValue().second, requester->getClient());
			}
			break;
		}
		case constcrc("GetVendorInfoForPlayerRsp"): {
			GenericValueTypeMessage < std::pair < std::pair < NetworkId, std::string >, std::map < std::string,
					std::vector < std::string > > > >
			const response(ri);

			std::vector<const char *> name;
			std::vector<const char *> location;
			std::vector<const char *> tax;
			std::vector <std::string> taxCityTemp;
			std::vector<const char *> taxCity;
			std::vector<const char *> emptyDate;
			std::vector<const char *> lastAccessDate;
			std::vector<const char *> inactiveDate;
			std::vector<const char *> status;
			std::vector<const char *> searchable;
			std::vector<const char *> entranceCharge;
			std::vector<const char *> itemCount;
			std::vector<const char *> offerCount;
			std::vector<const char *> stockRoomCount;

			std::map < std::string, std::vector < std::string > > ::const_iterator
			iterFind = response.getValue().second.find("name");
			if (iterFind != response.getValue().second.end()) {
				for (std::vector<std::string>::const_iterator iter = iterFind->second.begin();
						iter != iterFind->second.end(); ++iter) {
					name.push_back(iter->c_str());
				}
			}

			iterFind = response.getValue().second.find("location");
			if (iterFind != response.getValue().second.end()) {
				for (std::vector<std::string>::const_iterator iter = iterFind->second.begin();
						iter != iterFind->second.end(); ++iter) {
					location.push_back(iter->c_str());
				}
			}

			iterFind = response.getValue().second.find("tax");
			if (iterFind != response.getValue().second.end()) {
				for (std::vector<std::string>::const_iterator iter = iterFind->second.begin();
						iter != iterFind->second.end(); ++iter) {
					tax.push_back(iter->c_str());
				}
			}

			iterFind = response.getValue().second.find("taxCity");
			if (iterFind != response.getValue().second.end()) {
				taxCityTemp = iterFind->second;
				NetworkId cityHallId;
				int cityId;
				std::vector<std::string>::iterator iterTaxCity;
				for (iterTaxCity = taxCityTemp.begin(); iterTaxCity != taxCityTemp.end(); ++iterTaxCity) {
					cityHallId = NetworkId(*iterTaxCity);
					if (cityHallId.isValid()) {
						cityId = CityInterface::findCityByCityHall(cityHallId);
						if (cityId != 0) {
							*iterTaxCity = CityInterface::getCityInfo(cityId).getCityName();
						}
						else {
							*iterTaxCity = "";
						}
					}
					else {
						*iterTaxCity = "";
					}
				}

				for (iterTaxCity = taxCityTemp.begin(); iterTaxCity != taxCityTemp.end(); ++iterTaxCity) {
					taxCity.push_back(iterTaxCity->c_str());
				}
			}

			iterFind = response.getValue().second.find("emptyDate");
			if (iterFind != response.getValue().second.end()) {
				for (std::vector<std::string>::const_iterator iter = iterFind->second.begin();
						iter != iterFind->second.end(); ++iter) {
					emptyDate.push_back(iter->c_str());
				}
			}

			iterFind = response.getValue().second.find("lastAccessDate");
			if (iterFind != response.getValue().second.end()) {
				for (std::vector<std::string>::const_iterator iter = iterFind->second.begin();
						iter != iterFind->second.end(); ++iter) {
					lastAccessDate.push_back(iter->c_str());
				}
			}

			iterFind = response.getValue().second.find("inactiveDate");
			if (iterFind != response.getValue().second.end()) {
				for (std::vector<std::string>::const_iterator iter = iterFind->second.begin();
						iter != iterFind->second.end(); ++iter) {
					inactiveDate.push_back(iter->c_str());
				}
			}

			iterFind = response.getValue().second.find("status");
			if (iterFind != response.getValue().second.end()) {
				for (std::vector<std::string>::const_iterator iter = iterFind->second.begin();
						iter != iterFind->second.end(); ++iter) {
					status.push_back(iter->c_str());
				}
			}

			iterFind = response.getValue().second.find("searchable");
			if (iterFind != response.getValue().second.end()) {
				for (std::vector<std::string>::const_iterator iter = iterFind->second.begin();
						iter != iterFind->second.end(); ++iter) {
					searchable.push_back(iter->c_str());
				}
			}

			iterFind = response.getValue().second.find("entranceCharge");
			if (iterFind != response.getValue().second.end()) {
				for (std::vector<std::string>::const_iterator iter = iterFind->second.begin();
						iter != iterFind->second.end(); ++iter) {
					entranceCharge.push_back(iter->c_str());
				}
			}

			iterFind = response.getValue().second.find("itemCount");
			if (iterFind != response.getValue().second.end()) {
				for (std::vector<std::string>::const_iterator iter = iterFind->second.begin();
						iter != iterFind->second.end(); ++iter) {
					itemCount.push_back(iter->c_str());
				}
			}

			iterFind = response.getValue().second.find("offerCount");
			if (iterFind != response.getValue().second.end()) {
				for (std::vector<std::string>::const_iterator iter = iterFind->second.begin();
						iter != iterFind->second.end(); ++iter) {
					offerCount.push_back(iter->c_str());
				}
			}

			iterFind = response.getValue().second.find("stockRoomCount");
			if (iterFind != response.getValue().second.end()) {
				for (std::vector<std::string>::const_iterator iter = iterFind->second.begin();
						iter != iterFind->second.end(); ++iter) {
					stockRoomCount.push_back(iter->c_str());
				}
			}

			ScriptParams params;
			params.addParam(response.getValue().first.second.c_str(), "summary");
			params.addParam(name, "name");
			params.addParam(location, "location");
			params.addParam(tax, "tax");
			params.addParam(taxCity, "taxCity");
			params.addParam(emptyDate, "emptyDate");
			params.addParam(lastAccessDate, "lastAccessDate");
			params.addParam(inactiveDate, "inactiveDate");
			params.addParam(status, "status");
			params.addParam(searchable, "searchable");
			params.addParam(entranceCharge, "entranceCharge");
			params.addParam(itemCount, "itemCount");
			params.addParam(offerCount, "offerCount");
			params.addParam(stockRoomCount, "stockRoomCount");

			ScriptDictionaryPtr dictionary;
			GameScriptObject::makeScriptDictionary(params, dictionary);
			if (dictionary.get() != nullptr) {
				dictionary->serialize();
				MessageToQueue::getInstance().sendMessageToJava(response.getValue().first.first,
						"getVendorInfoForPlayerRsp", dictionary->getSerializedData(), 0, false);
			}
			break;
		}
	}
}

//-----------------------------------------------------------------------

int CommoditiesServerConnection::GiveTime()
{
  // This used to re-establish connect using the udp manager
  // if connection hadn't received ping from CM Server in 60 sec
	return 1;
}

//-----------------------------------------------------------------------

int CommoditiesServerConnection::SetGameTime(time_t gameTime)
{
    ++m_nextRequest;
	SetGameTimeMessage message(m_nextRequest, int(gameTime));
	DEBUG_REPORT_LOG(m_commoditiesShowAllDebugInfo, ("[Commodities API] : Sending SetGameTimeMessage.\n"));
	send(message, true);
	return m_nextRequest;
}

//-----------------------------------------------------------------------

int CommoditiesServerConnection::CancelAuction(const NetworkId & auctionId, const NetworkId & playerId)
{
	++m_nextRequest;
	CancelAuctionMessage message(
		m_nextRequest,
		auctionId,
		playerId);
	DEBUG_REPORT_LOG(m_commoditiesShowAllDebugInfo, ("[Commodities API] : Sending CancelAuctionMessage.\n"));
	send(message, true);
	return m_nextRequest;
}

//-----------------------------------------------------------------------

int CommoditiesServerConnection::AddAuction(
	const NetworkId &  ownerId,
	const std::string & ownerName,
	int  minBid,
	time_t timer,
	const NetworkId &  itemId,
	int    itemNameLength,
	const Unicode::String & itemName,
	int  itemType,
	int  itemTemplateId,
	time_t itemTimer,
	const NetworkId &  locationId,
	const std::string &  location,
	int flags,
	int userDescriptionLength,
	const Unicode::String & userDescription,
	std::vector<std::pair<std::string, Unicode::String> > const & attributes,
	int itemSize,
	int vendorLimit,
	int vendorItemLimit)
{
	++m_nextRequest;
	AddAuctionMessage message(
		m_nextRequest,
		ownerId,
		ownerName,
		minBid,
		(int)timer,
		itemId,
		itemNameLength,
		itemName,
		itemType,
		itemTemplateId,
		(int)itemTimer,
		locationId,
		location,
		flags,
		userDescriptionLength,
		userDescription,
		attributes,
		itemSize,
		vendorLimit,
		vendorItemLimit);
	DEBUG_REPORT_LOG(m_commoditiesShowAllDebugInfo, ("[Commodities API] : Sending AddAuctionMessage.\n"));
	send(message, true);
    return m_nextRequest;
}

//-----------------------------------------------------------------------

int CommoditiesServerConnection::AddImmediateAuction(
	const NetworkId &  ownerId,
	const std::string & ownerName,
	int  price,
	time_t timer,
	const NetworkId &  itemId,
	int    itemNameLength,
	const Unicode::String & itemName,
	int  itemType,
	int  itemTemplateId,
	time_t itemTimer,
	const NetworkId &  locationId,
	const std::string &  location,
	int  flags,
	int userDescriptionLength,
	const Unicode::String & userDescription,
	std::vector<std::pair<std::string, Unicode::String> > const & attributes,
	int itemSize,
	int vendorLimit,
	int vendorItemLimit)
{
	++m_nextRequest;
	AddImmediateAuctionMessage message(
		m_nextRequest,
		ownerId,
		ownerName,
		price,
		int(timer),
		itemId,
		itemNameLength,
		itemName,
		itemType,
		itemTemplateId,
		(int)itemTimer,
		locationId,
		location,
		flags,
		userDescriptionLength,
		userDescription,
		attributes,
		itemSize,
		vendorLimit,
		vendorItemLimit);
	DEBUG_REPORT_LOG(m_commoditiesShowAllDebugInfo, ("[Commodities API] : Sending AddImmediateAuctionMessage.\n"));
	send(message, true);
	return m_nextRequest;
}

//-----------------------------------------------------------------------

int CommoditiesServerConnection::AddBid(const NetworkId & auctionId,
	const NetworkId & playerId,
	const std::string & playerName,
	int bidAmount,
	int maxProxyBid)
{
	++m_nextRequest;
	AddBidMessage message(
		m_nextRequest,
		auctionId,
		playerId,
		bidAmount,
		maxProxyBid,
		playerName);
	DEBUG_REPORT_LOG(m_commoditiesShowAllDebugInfo, ("[Commodities API] : Sending AddBidMessage.\n"));
	send(message, true);
	return m_nextRequest;
}

//-----------------------------------------------------------------------

int CommoditiesServerConnection::AcceptHighBid(const NetworkId & auctionId, const NetworkId & playerId)
{
	++m_nextRequest;
	AcceptHighBidMessage message(
		m_nextRequest,
		auctionId,
		playerId);
	DEBUG_REPORT_LOG(m_commoditiesShowAllDebugInfo, ("[Commodities API] : Sending AcceptHighBidMessage.\n"));
	send(message, true);
	return m_nextRequest;
}

//-----------------------------------------------------------------------

int CommoditiesServerConnection::QueryAuctionHeaders(int requestId,
	const NetworkId & playerId,
	const NetworkId & vendorId,
	int queryType,
	int itemType,
	bool itemTypeExactMatch,
	int itemTemplateId,
	const Unicode::String & textFilterAll,
	const Unicode::String & textFilterAny,
	int priceFilterMin,
	int priceFilterMax,
	bool priceFilterIncludesFee,
	const std::list<AuctionQueryHeadersMessage::SearchCondition> & advancedSearch,
	AuctionQueryHeadersMessage::AdvancedSearchMatchAllAny advancedSearchMatchAllAny,
	const std::string & searchStringPlanet,
	const std::string & searchStringRegion,
	const NetworkId & searchAuctionLocationId,
	bool searchMyVendorsOnly,
	bool overrideVendorSearchFlag,
	int queryOffset)
{
	UNREF(requestId);

	++m_nextRequest;
	QueryAuctionHeadersMessage message(
		m_nextRequest,
		playerId,
		vendorId,
		queryType,
		itemType,
		itemTypeExactMatch,
		itemTemplateId,
		textFilterAll,
		textFilterAny,
		priceFilterMin,
		priceFilterMax,
		priceFilterIncludesFee,
		advancedSearch,
		advancedSearchMatchAllAny,
		searchStringPlanet,
		searchStringRegion,
		searchAuctionLocationId,
		searchMyVendorsOnly,
		overrideVendorSearchFlag,
		queryOffset);
	DEBUG_REPORT_LOG(m_commoditiesShowAllDebugInfo, ("[Commodities API] : Sending QueryAuctionHeadersMessage.\n"));
	send(message, true);
	return m_nextRequest;
}

//-----------------------------------------------------------------------

int CommoditiesServerConnection::GetItemDetails(int requestId, const NetworkId & playerId, const NetworkId & itemId)
{
	UNREF(requestId);
	++m_nextRequest;
	GetItemDetailsMessage message(
		m_nextRequest,
		itemId,
		playerId);
	DEBUG_REPORT_LOG(m_commoditiesShowAllDebugInfo, ("[Commodities API] : Sending GetItemDetailsMessage.\n"));
	send(message, true);
	return m_nextRequest;
}

//-----------------------------------------------------------------------

int CommoditiesServerConnection::GetVendorValue(
	const std::string & location
)
{
    ++m_nextRequest;
	GetVendorValueMessage message(
		m_nextRequest,
		location);
	DEBUG_REPORT_LOG(m_commoditiesShowAllDebugInfo, ("[Commodities API] : Sending GetVendorValueMessage.\n"));
	send(message, true);
	return m_nextRequest;
}

//-----------------------------------------------------------------------

int CommoditiesServerConnection::CreateVendorMarket(
	const NetworkId & ownerId,
	const std::string & location,
	int playerVendorLimit,
	int entranceCharge
)
{
	++m_nextRequest;
	CreateVendorMarketMessage message(
		m_nextRequest,
		ownerId,
		location,
		playerVendorLimit,
		entranceCharge);
	DEBUG_REPORT_LOG(m_commoditiesShowAllDebugInfo, ("[Commodities API] : Sending CreateVendorMarketMessage.\n"));
	send(message, true);
	return m_nextRequest;
}

//-----------------------------------------------------------------------

int CommoditiesServerConnection::DestroyVendorMarket(
	const NetworkId & ownerId,
	const std::string & ownerName,
	const std::string & location
)
{
	++m_nextRequest;
	DestroyVendorMarketMessage message(
		m_nextRequest,
		ownerId,
		ownerName,
		location);
	DEBUG_REPORT_LOG(m_commoditiesShowAllDebugInfo, ("[Commodities API] : Sending DestroyVendorMarketMessage.\n"));
	send(message, true);
	return m_nextRequest;
}

//-----------------------------------------------------------------------

int CommoditiesServerConnection::deleteAuctionLocation(
	const NetworkId & locationId,
	const std::string & whoRequested
)
{
	++m_nextRequest;
	DeleteAuctionLocationMessage message(
		m_nextRequest,
		locationId,
		whoRequested);
	DEBUG_REPORT_LOG(m_commoditiesShowAllDebugInfo, ("[Commodities API] : Sending DeleteAuctionLocationMessage.\n"));
	send(message, true);
	return m_nextRequest;
}

//-----------------------------------------------------------------------

int CommoditiesServerConnection::GetVendorOwner(
	const NetworkId & requesterId,
	const std::string & location
)
{
    ++m_nextRequest;
	GetVendorOwnerMessage message(
		m_nextRequest,
		requesterId,
		location);
	DEBUG_REPORT_LOG(m_commoditiesShowAllDebugInfo, ("[Commodities API] : Sending GetVendorOwnerMessage.\n"));
	send(message, true);
	return m_nextRequest;
}

//-----------------------------------------------------------------------

int CommoditiesServerConnection::GetItem(const NetworkId & itemId, const NetworkId & playerId, const std::string & location)
{
	++m_nextRequest;
	GetItemMessage message(
		m_nextRequest,
		itemId,
		playerId,
		location);
	DEBUG_REPORT_LOG(m_commoditiesShowAllDebugInfo, ("[Commodities API] : Sending GetItemMessage.\n"));
	send(message, true);
	return m_nextRequest;
}

//-----------------------------------------------------------------------

int CommoditiesServerConnection::CleanupInvalidItemRetrieval( const NetworkId & itemId)
{
	++m_nextRequest;
	CleanupInvalidItemRetrievalMessage message(m_nextRequest, itemId);
	DEBUG_REPORT_LOG(m_commoditiesShowAllDebugInfo, ("[Commodities API] : Sending CleanupInvalidItemRetrievalMessage.\n"));
	send(message, true);
	return m_nextRequest;
}

//-----------------------------------------------------------------------

int CommoditiesServerConnection::SetSalesTax(
	int salesTax,
	const NetworkId & bankId,
	const std::string & location
)
{
	++m_nextRequest;
	SetSalesTaxMessage message(
		m_nextRequest,
		salesTax,
		bankId,
		location);
	//printf("CommoditiesServerConnection: location for sales tax = '%s'\n", message.GetLocation());
	DEBUG_REPORT_LOG(m_commoditiesShowAllDebugInfo, ("[Commodities API] : Sending SetSalesTaxMessage.\n"));
	send(message, true);
	return m_nextRequest;
}

//-----------------------------------------------------------------------

int CommoditiesServerConnection::GetPlayerVendorCount(const NetworkId & playerId)
{
	++m_nextRequest;
	GetPlayerVendorCountMessage message(
		m_nextRequest,
		playerId);
	
	DEBUG_REPORT_LOG(m_commoditiesShowAllDebugInfo, ("[Commodities API] : Sending GetPlayerVendorCountMessage.\n"));
	send(message, true);
	return m_nextRequest;
}

//-----------------------------------------------------------------------
