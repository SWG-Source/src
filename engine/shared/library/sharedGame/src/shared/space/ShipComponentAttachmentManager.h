//======================================================================
//
// ShipComponentAttachmentManager.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ShipComponentAttachmentManager_H
#define INCLUDED_ShipComponentAttachmentManager_H

//======================================================================

class PersistentCrcString;

class ShipComponentAttachmentManager
{
public:

	class Messages
	{
	public:
		struct AttachmentsChanged
		{
			//-- chassisCrc, componentCrc, chassisSlotType
			typedef std::pair<uint32, std::pair<uint32, int> > Payload;
		};
	};

	static void install ();
	static void remove  ();

	static void load();
	static bool save(std::string const & dsrcPath, std::string const & dataPath);
	static bool saveChassisInfo(std::string const & chassisName, std::string const & chassisFilenameTab, std::string const & chassisFilenameIff);

	typedef std::pair<uint32, PersistentCrcString>  TemplateHardpointPair;
	typedef std::vector<TemplateHardpointPair>   TemplateHardpointPairVector;
	typedef std::vector<PersistentCrcString>     HardpointVector;

	static TemplateHardpointPairVector const & getAttachmentsForShip          (uint32 chassisCrc, uint32 componentCrc, int chassisSlotType);
	static bool getAttachmentsForShip(uint32 chassisCrc, uint32 componentCrc, TemplateHardpointPairVector const ** thpVectors);
	static float                               getComponentHitRange           (uint32 chassisCrc, uint32 componentCrc, int chassisSlotType);
	static HardpointVector const &             getExtraHardpointsForComponent (uint32 chassisCrc, uint32 componentCrc, int chassisSlotType);

public:

	//-- these methods are for editor use only

	static void setAttachmentsForShip(uint32 chassisCrc, uint32 componentCrc, int chassisSlotType, TemplateHardpointPairVector const & attachments);
	static std::string const & getAttachmentTemplatePrefix();
	static void udpateComponentCrc(uint32 const oldComponentCrc, uint32 const newComponentCrc);
	static void udpateChassisCrc(uint32 const oldChassisCrc, uint32 const newChassisCrc);
	static void copyAttachmentsForComponent(uint32 const oldComponentCrc, uint32 const newComponentCrc);
	static void copyAttachmentsForChassis(uint32 const oldChassisCrc, uint32 const newChassisCrc);
};

//======================================================================

#endif
