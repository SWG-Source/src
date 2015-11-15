// ======================================================================
//
// DatabaseConnection.h
// copyright (c) 2005 Sony Online Entertainment
// Author: Doug Mellencamp
//
// ======================================================================

#ifndef INCLUDED_DatabaseConnection_H
#define INCLUDED_DatabaseConnection_H

// ======================================================================

#include "sharedFoundation/NetworkId.h"
#include "sharedFoundation/StationId.h"

// ======================================================================

class DatabaseConnection 
{
  public:

	static void connect                                          ();
	static void disconnect                                       ();
	static void update                                           ();
	static const std::string getSchemaQualifier                  ();

	static void updateSPCharacterProfileData                     (const std::string & clusterName,const NetworkId & characterId,const std::string & characterName,const std::string & objectName,float x,float y,float z,const std::string & sceneId,float cash_balance,float bank_balance,const std::string & objectTemplateName,int   stationId,const NetworkId & containedBy,int   createTime,int   playedTime,int   numLots);
	static void updateSPCharacterItemData                        ();
	static void updateSPCharacterVendorData                      ();
	static void updateSPCharacterVendorItemData                  ();
	static void updateSPCharacterStructureData                   ();
	static void updateSPCharacterShipData                        ();
	static void updateSPCharacterSkillData                       ();
	static void updateSPCharacterMount_VehicleData               ();
	static void updateSPCharacterWayPointData                    ();
	static void updateSPCharacterQuestData                       ();
	static void updateSPCharacterSchematicData                   ();
	static void updateSPCharacterRankingData                     ();
			
  private:
  
};

// ======================================================================

#endif
