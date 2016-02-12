// ======================================================================
//
// CommodityServerMetricsData.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "FirstCommodityServer.h"
#include "CommodityServerMetricsData.h"

#include "AuctionMarket.h"
#include "CommodityServer.h"
#include "sharedGame/GameObjectTypes.h"

// ======================================================================

CommodityServerMetricsData::CommodityServerMetricsData() :
	MetricsData(),
	m_numItems(0),
	m_numLocations(0),
	m_loadTime(0),
	m_mapAuctionsCountByGameObjectTypeIndex()
{
	MetricsPair p;

	ADD_METRICS_DATA(numItems, 0, true);
	ADD_METRICS_DATA(numLocations, 0, true);
	ADD_METRICS_DATA(loadTime, 0, true);

	m_data[m_loadTime].m_description = "minutes";
}

//-----------------------------------------------------------------------

CommodityServerMetricsData::~CommodityServerMetricsData()
{
}

//-----------------------------------------------------------------------

void CommodityServerMetricsData::updateData() 
{
	MetricsData::updateData();
	m_data[m_numItems].m_value = AuctionMarket::getInstance().getAuctionCount();
	m_data[m_numLocations].m_value = AuctionMarket::getInstance().getLocationCount();
	m_data[m_loadTime].m_value = CommodityServer::getInstance().getCommoditiesServerLoadTime();

	// handle auctions count by game object type statistics
	std::map<std::string, int> const & auctionsCountByGameObjectType = AuctionMarket::getInstance().getAuctionsCountByGameObjectType();
	std::set<std::string> & auctionsCountByGameObjectTypeChanged = AuctionMarket::getInstance().getAuctionsCountByGameObjectTypeChanged();
	if (!auctionsCountByGameObjectTypeChanged.empty() && !auctionsCountByGameObjectType.empty())
	{
		if (m_mapAuctionsCountByGameObjectTypeIndex.empty())
		{
			int gameObjectType;
			int baseGameObjectType;
			char buffer[1024];
			for (std::map<std::string, int>::const_iterator iterGameObjectType = auctionsCountByGameObjectType.begin(); iterGameObjectType != auctionsCountByGameObjectType.end(); ++iterGameObjectType)
			{
				if (!iterGameObjectType->first.empty() && (iterGameObjectType->second >= 0) && (m_mapAuctionsCountByGameObjectTypeIndex.count(iterGameObjectType->first) < 1))
				{
					gameObjectType = GameObjectTypes::getGameObjectType(iterGameObjectType->first);
					if (GameObjectTypes::isSubType(gameObjectType))
					{
						baseGameObjectType = GameObjectTypes::getMaskedType(gameObjectType);

						snprintf(buffer, sizeof(buffer)-1, "numItems.0x%08X_%s.0x%08X_%s", baseGameObjectType, GameObjectTypes::getCanonicalName(baseGameObjectType).c_str(), gameObjectType, iterGameObjectType->first.c_str());
						buffer[sizeof(buffer)-1] = '\0';
					}
					else
					{
						snprintf(buffer, sizeof(buffer)-1, "numItems.0x%08X_%s", gameObjectType, iterGameObjectType->first.c_str());
						buffer[sizeof(buffer)-1] = '\0';
					}

					m_mapAuctionsCountByGameObjectTypeIndex[iterGameObjectType->first] = addMetric(buffer, 0, nullptr, false, false);
				}
			}
		}

		for (std::set<std::string>::const_iterator iterChanged = auctionsCountByGameObjectTypeChanged.begin(); iterChanged != auctionsCountByGameObjectTypeChanged.end(); ++iterChanged)
		{
			std::map<std::string, int>::const_iterator const iterFindIndex = m_mapAuctionsCountByGameObjectTypeIndex.find(*iterChanged);
			if (iterFindIndex != m_mapAuctionsCountByGameObjectTypeIndex.end())
			{
				std::map<std::string, int>::const_iterator const iterFindGameObjectType = auctionsCountByGameObjectType.find(*iterChanged);
				if (iterFindGameObjectType != auctionsCountByGameObjectType.end())
				{
					updateMetric(iterFindIndex->second, iterFindGameObjectType->second);
				}
			}
		}

		auctionsCountByGameObjectTypeChanged.clear();
	}
}

// ======================================================================
