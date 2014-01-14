//======================================================================
//
// PlatformFeatureBits.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_PlatformFeatureBits_H
#define INCLUDED_PlatformFeatureBits_H

#include <map>
#include <string>

//These define the bits used by the Platform group (SessionAPI) to distinguish customers
//Ep3 bits specified by Taylor Steil of Platform

namespace ClientGameFeature
{
	int const Base                           = BINARY8(0000,0000,0000,0000,0000,0000,0000,0001);   //  0 (1)
	int const CollectorsEdition              = BINARY8(0000,0000,0000,0000,0000,0000,0000,0010);   //  1 (2)
	int const SpaceExpansionBeta             = BINARY8(0000,0000,0000,0000,0000,0000,0000,0100);   //  2 (4)
	int const SpaceExpansionPreOrder         = BINARY8(0000,0000,0000,0000,0000,0000,0000,1000);   //  3 (8)
	int const SpaceExpansionRetail           = BINARY8(0000,0000,0000,0000,0000,0000,0001,0000);   //  4 (16)
	int const SWGRetail                      = BINARY8(0000,0000,0000,0000,0000,0000,0010,0000);   //  5 (32)
	int const SpaceExpansionPromotion        = BINARY8(0000,0000,0000,0000,0000,0000,0100,0000);   //  6 (64)
	int const JapaneseRetail                 = BINARY8(0000,0000,0000,0000,0000,0000,1000,0000);   //  7 (128)
	int const JapaneseCollectors             = BINARY8(0000,0000,0000,0000,0000,0001,0000,0000);   //  8 (256)
	int const Episode3ExpansionRetail        = BINARY8(0000,0000,0000,0000,0000,0010,0000,0000);   //  9 (512)
	int const CombatUpgrade                  = BINARY8(0000,0000,0000,0000,0000,0100,0000,0000);   // 10 (1024)
	int const Episode3PreorderDownload       = BINARY8(0000,0000,0000,0000,0000,1000,0000,0000);   // 11 (2048)
	int const TrialsOfObiwanBeta             = BINARY8(0000,0000,0000,0000,0001,0000,0000,0000);   // 12 (4096)
	int const JapaneseRecapture              = BINARY8(0000,0000,0000,0000,0010,0000,0000,0000);   // 13 (8192)
	int const TrialsOfObiwanPreorder         = BINARY8(0000,0000,0000,0000,0100,0000,0000,0000);   // 14 (16384)
	int const TrialsOfObiwanRetail           = BINARY8(0000,0000,0000,0000,1000,0000,0000,0000);   // 15 (32768)
	int const StarterKit                     = BINARY8(0000,0000,0000,0001,0000,0000,0000,0000);   // 16 (65536)
	int const April2006Roadshow              = BINARY8(0000,0000,0000,0010,0000,0000,0000,0000);   // 17 (131072)
	int const FreeTrial2                     = BINARY8(0000,0000,0000,0100,0000,0000,0000,0000);   // 18 (262144)
	int const CompleteOnlineAdventures       = BINARY8(0000,0000,0000,1000,0000,0000,0000,0000);   // 19 (524288)
	int const Episode3PreorderDownloadReward = BINARY8(0000,0000,0001,0000,0000,0000,0000,0000);   // 20 (1048576)
	int const Episode3ExpansionRetailReward  = BINARY8(0000,0000,0010,0000,0000,0000,0000,0000);   // 21 (2097152)
	int const TrialsOfObiwanRetailReward     = BINARY8(0000,0000,0100,0000,0000,0000,0000,0000);   // 22 (4194304)
	int const HousePackupReward              = BINARY8(0000,0000,1000,0000,0000,0000,0000,0000);   // 23 (8388608)
	int const Summit2007Reward               = BINARY8(0000,0001,0000,0000,0000,0000,0000,0000);   // 24 (16777216)
	int const CoaDigitalDownloadReward       = BINARY8(0000,0010,0000,0000,0000,0000,0000,0000);   // 25 (33554432)
	int const BuddyProgramReward             = BINARY8(0000,0100,0000,0000,0000,0000,0000,0000);   // 26 (67108864)
	int const GalacticGatheringReward        = BINARY8(0000,1000,0000,0000,0000,0000,0000,0000);   // 27 (134217728)

	inline std::string const getDescription(int features)
	{
		std::string result;
		if (features & Base)
			result += "Base ";
		if (features & CollectorsEdition)
			result += "CollectorsEdition ";
		if (features & SpaceExpansionBeta)
			result += "SpaceExpansionBeta ";
		if (features & SpaceExpansionPreOrder)
			result += "SpaceExpansionPreOrder ";
		if (features & SpaceExpansionRetail)
			result += "SpaceExpansionRetail ";
		if (features & SWGRetail)
			result += "SWGRetail ";
		if (features & SpaceExpansionPromotion)
			result += "SpaceExpansionPromotion ";
		if (features & JapaneseRetail)
			result += "JapaneseRetail ";
		if (features & JapaneseCollectors)
			result += "JapaneseCollectors ";
		if (features & Episode3ExpansionRetail)
			result += "Episode3ExpansionRetail ";
		if (features & CombatUpgrade)
			result += "CombatUpgrade ";
		if (features & Episode3PreorderDownload)
			result += "Episode3PreorderDownload ";
		if (features & TrialsOfObiwanBeta)
			result += "TrialsOfObiwanBeta ";
		if (features & JapaneseRecapture)
			result += "JapaneseRecapture ";
		if (features & TrialsOfObiwanRetail)
			result += "TrialsOfObiwanRetail ";
		if (features & TrialsOfObiwanPreorder)
			result += "TrialsOfObiwanPreorder ";
		if (features & StarterKit)
			result += "StarterKit ";
		if (features & April2006Roadshow)
			result += "April2006Roadshow ";
		if (features & FreeTrial2)
			result += "FreeTrial2 ";
		if (features & CompleteOnlineAdventures)
			result += "CompleteOnlineAdventures ";
		if (features & Episode3PreorderDownloadReward)
			result += "Episode3PreorderDownloadReward ";
		if (features & Episode3ExpansionRetailReward)
			result += "Episode3ExpansionRetailReward ";
		if (features & TrialsOfObiwanRetailReward)
			result += "TrialsOfObiwanRetailReward ";
		if (features & HousePackupReward)
			result += "HousePackupReward ";
		if (features & Summit2007Reward)
			result += "Summit2007Reward ";
		if (features & CoaDigitalDownloadReward)
			result += "CoaDigitalDownloadReward ";
		if (features & BuddyProgramReward)
			result += "BuddyProgramReward ";
		if (features & GalacticGatheringReward)
			result += "GalacticGatheringReward ";

		return result;
	}
}

// ----------------------------------------------------------------------

namespace ClientSubscriptionFeature
{
	const int Base                =       BINARY1(0001);   // 0 (1)
	const int FreeTrial           =       BINARY1(0010);   // 1 (2)
	const int CombatBalanceAccess =       BINARY1(0100);   // 2 (4)
	const int NPENagForTrial      =       BINARY1(1000);   // 3 (8)
	const int NPENagForRental     = BINARY2(0001, 0000);   // 4 (16)
	const int FreeTrial2          = BINARY2(0010, 0000);   // 5 (32)

	inline std::string const getDescription(int features)
	{
		std::string result;
		if (features & Base)
			result += "Base ";
		if (features & FreeTrial)
			result += "FreeTrial ";
		if (features & CombatBalanceAccess)
			result += "CombatBalanceAccess ";
		if (features & NPENagForTrial)
			result += "NPENagForTrial ";
		if (features & NPENagForRental)
			result += "NPENagForRental ";
		if (features & FreeTrial2)
			result += "FreeTrial2 ";

		return result;
	}
}

// ----------------------------------------------------------------------

namespace PlatformGameCode
{
	const unsigned SWG = 4;
	const unsigned SWGTCG = 38;

	const std::pair<unsigned, const std::string> _gamecodeName[] = 
	{  
		std::pair<unsigned, const std::string>(SWG, "SWG"),
		std::pair<unsigned, const std::string>(SWGTCG, "SWGTCG")
	};

	const std::map<unsigned, std::string const> gamecodeName((const std::map<unsigned, std::string const>::value_type *)&_gamecodeName[0],(const std::map<unsigned, std::string const>::value_type *)&_gamecodeName[sizeof(_gamecodeName)/sizeof(_gamecodeName[0])]);

	inline std::string const & getGamecodeName(unsigned gameCode)
	{
		static const std::string empty;

		std::map<unsigned, std::string const>::const_iterator iter = gamecodeName.find(gameCode);
		if (iter != gamecodeName.end())
			return iter->second;

		return empty;
	}
}

//========================================================================

#endif	// INCLUDED_PlatformFeatureBits_H
