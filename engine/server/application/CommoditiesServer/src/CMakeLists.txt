
if (${CMAKE_BUILD_TYPE} STREQUAL "MINSIZEREL" AND EXISTS "${PROJECT_SOURCE_DIR}/src/linux/CommoditiesServer.profdata")
	add_compile_options(-fprofile-instr-use=${PROJECT_SOURCE_DIR}/src/linux/CommoditiesServer.profdata)
endif()

set(SHARED_SOURCES
    shared/AuctionBid.cpp
    shared/AuctionBid.h
    shared/Auction.cpp
    shared/Auction.h
    shared/AuctionItem.cpp
    shared/AuctionItem.h
    shared/AuctionLocation.cpp
    shared/AuctionLocation.h
    shared/AuctionMarket.cpp
    shared/AuctionMarket.h
    shared/CentralServerConnection.cpp
    shared/CentralServerConnection.h
    shared/CommodityServer.cpp
    shared/CommodityServer.h
    shared/CommodityServerMetricsData.cpp
    shared/CommodityServerMetricsData.h
    shared/ConfigCommodityServer.cpp
    shared/ConfigCommodityServer.h
    shared/DatabaseServerConnection.cpp
    shared/DatabaseServerConnection.h
    shared/FirstCommodityServer.h
    shared/GameServerConnection.cpp
    shared/GameServerConnection.h
)

if(WIN32)
	set(PLATFORM_SOURCES 
		win32/WinMain.cpp
	)
else()
	set(PLATFORM_SOURCES
		linux/main.cpp
	)
endif()

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/shared
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedCompression/include/public
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedDebug/include/public
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedFile/include/public
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedFoundation/include/public
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedFoundationTypes/include/public
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedGame/include/public
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedLog/include/public
    
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedMessageDispatch/include/public
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedNetwork/include/public
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedNetworkMessages/include/public
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedObject/include/public
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedRandom/include/public
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedThread/include/public
    ${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedUtility/include/public
    ${SWG_ENGINE_SOURCE_DIR}/server/library/serverMetrics/include/public
    ${SWG_ENGINE_SOURCE_DIR}/server/library/serverNetworkMessages/include/public
    ${SWG_ENGINE_SOURCE_DIR}/server/library/serverUtility/include/public
    ${SWG_EXTERNALS_SOURCE_DIR}/ours/library/archive/include
    ${SWG_EXTERNALS_SOURCE_DIR}/ours/library/fileInterface/include/public
    ${SWG_EXTERNALS_SOURCE_DIR}/ours/library/localization/include
    ${SWG_EXTERNALS_SOURCE_DIR}/ours/library/localizationArchive/include/public
    ${SWG_EXTERNALS_SOURCE_DIR}/ours/library/singleton/include
    ${SWG_EXTERNALS_SOURCE_DIR}/ours/library/unicode/include
    ${SWG_EXTERNALS_SOURCE_DIR}/ours/library/unicodeArchive/include/public
)

add_executable(CommoditiesServer
	${SHARED_SOURCES}
	${PLATFORM_SOURCES}
)

target_link_libraries(CommoditiesServer
    sharedCompression
    sharedDebug
    sharedFile
    sharedFoundation
    sharedGame
    
    sharedLog
    sharedMessageDispatch
    sharedNetwork
    sharedNetworkMessages
    sharedObject
    sharedRandom
    sharedThread
    sharedUtility
    serverMetrics
    serverNetworkMessages
    serverUtility
    ${CMAKE_DL_LIBS}
)
