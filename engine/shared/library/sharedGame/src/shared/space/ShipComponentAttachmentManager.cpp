//======================================================================
//
// ShipComponentAttachmentManager.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/ShipComponentAttachmentManager.h"

#include "UnicodeUtils.h"
#include "sharedDebug/InstallTimer.h"
#include "fileInterface/StdioFile.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/PersistentCrcString.h"
#include "sharedFoundation/TemporaryCrcString.h"
#include "sharedGame/ShipChassis.h"
#include "sharedGame/ShipChassisSlotType.h"
#include "sharedGame/ShipComponentDescriptor.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedObject/ObjectTemplateList.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableWriter.h"
#include <map>
#include <vector>

//lint -e1925 //public data member

//======================================================================

namespace ShipComponentAttachmentManagerNamespace
{
	namespace Transceivers
	{
		MessageDispatch::Transceiver<ShipComponentAttachmentManager::Messages::AttachmentsChanged::Payload const &, ShipComponentAttachmentManager::Messages::AttachmentsChanged> attachmentsChanged;
	}

	typedef ShipComponentAttachmentManager::TemplateHardpointPairVector TemplateHardpointPairVector;
	typedef ShipComponentAttachmentManager::HardpointVector HardpointVector;

	class AttachmentKey
	{
	public:
		uint32 chassisCrc;
		uint32 componentCrc;
		int    chassisSlotType;

		AttachmentKey () :
		chassisCrc       (0),
		componentCrc     (0),
		chassisSlotType  (0)
		{
		}

		AttachmentKey (uint32 _chassisCrc, uint32 _componentCrc, int _chassisSlotType) :
		chassisCrc       (_chassisCrc),
		componentCrc     (_componentCrc),
		chassisSlotType  (_chassisSlotType)
		{
		}

		bool operator< (AttachmentKey const & other) const
		{
			if (chassisCrc < other.chassisCrc)
				return true;

			if (chassisCrc == other.chassisCrc)
			{
				if (componentCrc < other.componentCrc)
					return true;

				if (componentCrc == other.componentCrc)
					return chassisSlotType < other.chassisSlotType;
			}

			return false;
		}
	};

	typedef std::map<AttachmentKey, TemplateHardpointPairVector> AttachmentMap;
	AttachmentMap * s_attachmentMap;

	typedef std::map<AttachmentKey, float> ComponentHitRangeMap;
	ComponentHitRangeMap * s_componentHitRangeMap;

	typedef std::map<AttachmentKey, HardpointVector> ExtraHardpointsMap;
	ExtraHardpointsMap * s_extraHardpointsMap;
	
	std::string s_attachmentTemplatePrefix = "object/tangible/ship/attachment/";
	bool s_installed = false;
}

using namespace ShipComponentAttachmentManagerNamespace;

//----------------------------------------------------------------------

void ShipComponentAttachmentManager::install ()
{
	InstallTimer const installTimer("ShipComponentAttachmentManager::install ");

	DEBUG_FATAL (s_installed, ("already installed"));
	if (s_installed)
		return;

	s_attachmentMap = new AttachmentMap;
	s_componentHitRangeMap = new ComponentHitRangeMap;
	s_extraHardpointsMap = new ExtraHardpointsMap;

	load();

	ExitChain::add (ShipComponentAttachmentManager::remove, "ShipComponentAttachmentManager::remove");
	s_installed = true;
}

//----------------------------------------------------------------------

void ShipComponentAttachmentManager::remove  ()
{
	DEBUG_FATAL (!s_installed, ("not installed"));

	delete s_attachmentMap;
	s_attachmentMap = 0;

	delete s_componentHitRangeMap;
	s_componentHitRangeMap = 0;

	delete s_extraHardpointsMap;
	s_extraHardpointsMap = 0;

	s_installed = false;
}

//----------------------------------------------------------------------

void ShipComponentAttachmentManager::load()
{
	ShipChassis::PersistentCrcStringVector const & shipChassisNames = ShipChassis::getShipChassisCrcVector ();

	for (ShipChassis::PersistentCrcStringVector::const_iterator it = shipChassisNames.begin (); it != shipChassisNames.end (); ++it)
	{
		PersistentCrcString const & name = *(*it);
		
		std::string const & chassis_filename = std::string ("datatables/space/ship_chassis_") + std::string (name.getString ()) + std::string (".iff");
		
		Iff iff;
		
		if (!iff.open (chassis_filename.c_str (), true))
			continue;
		
		DataTable dt;
		dt.load (iff);
		iff.close ();

		const int numRows = dt.getNumRows ();
		
		for (int row = 0; row < numRows; ++row)
		{
			std::string const & componentName = dt.getStringValue (0, row);
			if (componentName.empty())
				continue;

			ShipComponentDescriptor const * const shipComponentDescriptor = 
				ShipComponentDescriptor::findShipComponentDescriptorByName (TemporaryCrcString (componentName.c_str (), true));

			if (shipComponentDescriptor == nullptr)
			{
				WARNING (true, ("ShipComponentAttachmentManager chassis [%s] specified invalid component [%s] at row [%d] in file [%s]", name.getString (), componentName.c_str (), row, chassis_filename.c_str()));
				continue;
			}

			for (int slotTypeIndex = 0; slotTypeIndex < static_cast<int>(ShipChassisSlotType::SCST_num_types); ++slotTypeIndex)
			{
				std::string const & slotName = ShipChassisSlotType::getNameFromType (static_cast<ShipChassisSlotType::Type>(slotTypeIndex));
				if (!dt.doesColumnExist (slotName))
					continue;
				
				std::string const & attachments = dt.getStringValue (slotName, row);
				if (attachments.empty ())
					continue;
				
				ShipComponentType::Type const componentType = 
					static_cast<ShipComponentType::Type>(ShipChassisSlotType::getComponentTypeForSlot (static_cast<ShipChassisSlotType::Type>(slotTypeIndex)));
				std::string const & componentTypeName = ShipComponentType::getNameFromType  (componentType);
				
				TemplateHardpointPairVector thpv;
				HardpointVector extraHardpoints;
				
				size_t pos = 0;
				std::string attachmentToken;
				while (pos != std::string::npos && Unicode::getFirstToken (attachments, pos, pos, attachmentToken, ",")) //lint !e650 !e737 //stupid msvc
				{
					Unicode::trim (attachmentToken);
					
					if (pos != std::string::npos) //lint !e650 !e737 //stupid msvc
						++pos;
					
					if (attachmentToken.empty ())
						continue;
					
					size_t colonPos = attachmentToken.find (':');
					
					if (colonPos == std::string::npos) //lint !e650 !e737 //stupid msvc
					{
						// Component specifies a hardpoint that will be used for targetting only, but no object to attach
						extraHardpoints.push_back(PersistentCrcString(attachmentToken.c_str(), true));
					}
					else
					{
						std::string const & objectTemplateName = 
							s_attachmentTemplatePrefix + componentTypeName + std::string ("/shared_") + attachmentToken.substr (0, colonPos) + std::string (".iff");
						CrcString const & objectTemplateCrcString = ObjectTemplateList::lookUp(objectTemplateName.c_str ());
						if (objectTemplateCrcString.getCrc () == 0)
						{
							WARNING (true, ("ShipComponentAttachmentManager chassis [%s] specified invalid object template [%s (%s)] for component [%s] slot [%s]",
								name.getString (), objectTemplateName.c_str (), attachmentToken.c_str (), componentName.c_str (), slotName.c_str ()));
						}
						
						std::string const & hardpointName = attachmentToken.substr (colonPos + 1);
						
						TemplateHardpointPair const thp (objectTemplateCrcString.getCrc (), PersistentCrcString (hardpointName.c_str (), true));
						thpv.push_back (thp);
					}
				}
				
				AttachmentKey const attachmentKey (name.getCrc (), shipComponentDescriptor->getCrc (), slotTypeIndex);
				IGNORE_RETURN (s_attachmentMap->insert (std::make_pair (attachmentKey, thpv)));
				if (!extraHardpoints.empty())
					IGNORE_RETURN (s_extraHardpointsMap->insert (std::make_pair (attachmentKey, extraHardpoints)));

				if (dt.doesColumnExist("hit_range"))
				{
					float hitRange = dt.getFloatValue("hit_range",row);
					if (hitRange != 0)
						IGNORE_RETURN(s_componentHitRangeMap->insert(std::make_pair (attachmentKey, hitRange)));
				}
			}
		}
	}
}

//----------------------------------------------------------------------

bool ShipComponentAttachmentManager::save(std::string const & dsrcPath, std::string const & dataPath)
{
	bool retval = true;

	ShipChassis::PersistentCrcStringVector const & shipChassisNames = ShipChassis::getShipChassisCrcVector ();

	for (ShipChassis::PersistentCrcStringVector::const_iterator it = shipChassisNames.begin (); it != shipChassisNames.end (); ++it)
	{
		PersistentCrcString const & chassisName = *(*it);
		
		std::string const & chassisFilenameTab = dsrcPath + "/datatables/space/ship_chassis_" + chassisName.getString () + ".tab";
		std::string const & chassisFilenameIff = dataPath + "/datatables/space/ship_chassis_" + chassisName.getString () + ".iff";

		retval = saveChassisInfo(chassisName.getString(), chassisFilenameTab, chassisFilenameIff) && retval;
	}

	return retval;
}

//----------------------------------------------------------------------

bool ShipComponentAttachmentManager::saveChassisInfo(std::string const & chassisName, std::string const & filenameTab, std::string const & filenameIff)
{
	ShipChassis const * const chassis = ShipChassis::findShipChassisByName(ConstCharCrcString(chassisName.c_str()));
	if (nullptr == chassis)
		return false;

	uint32 const chassisCrc = chassis->getCrc();
	ShipComponentDescriptor::StringVector const & componentNames = ShipComponentDescriptor::getComponentDescriptorNames();

	bool columnPresent[ShipChassisSlotType::SCST_num_types];

	{
		for (int chassisSlotType = 0; chassisSlotType < static_cast<int>(ShipChassisSlotType::SCST_num_types); ++chassisSlotType)
			columnPresent[chassisSlotType] = false;

		for (ShipComponentDescriptor::StringVector::const_iterator it = componentNames.begin(); it != componentNames.end(); ++it)
		{
			std::string const & componentName = *it;
			ShipComponentDescriptor const * const shipComponentDescriptor = ShipComponentDescriptor::findShipComponentDescriptorByName(ConstCharCrcString(componentName.c_str()));

			if (nullptr == shipComponentDescriptor)
				return false;
		
			uint32 const componentCrc = shipComponentDescriptor->getCrc();

			for (int chassisSlotType = 0; chassisSlotType < static_cast<int>(ShipChassisSlotType::SCST_num_types); ++chassisSlotType)
				if (!ShipComponentAttachmentManager::getAttachmentsForShip(chassisCrc, componentCrc, chassisSlotType).empty())
					columnPresent[chassisSlotType] = true;
		}
	}

	//-- TAB HEADER COLNAMES
	std::string tabStr("component");

	{
		for (int chassisSlotType = 0; chassisSlotType < static_cast<int>(ShipChassisSlotType::SCST_num_types); ++chassisSlotType)
		{
			if (columnPresent[chassisSlotType])
			{
				std::string const & slotName = ShipChassisSlotType::getNameFromType(static_cast<ShipChassisSlotType::Type>(chassisSlotType));
				tabStr.push_back('\t');
				tabStr += slotName;
			}
		}
	}

	tabStr.push_back('\n');

	//-- TAB HEADER COLTYPES
	tabStr += "s";

	{
		for (int chassisSlotType = 0; chassisSlotType < static_cast<int>(ShipChassisSlotType::SCST_num_types); ++chassisSlotType)
		{
			if (columnPresent[chassisSlotType])
			{
				tabStr += "\ts";
			}
		}
	}

	tabStr.push_back('\n');

	//-- TAB DATA ROWS
	
	for (ShipComponentDescriptor::StringVector::const_iterator it = componentNames.begin(); it != componentNames.end(); ++it)
	{
		std::string const & componentName = *it;
		ShipComponentDescriptor const * const shipComponentDescriptor = ShipComponentDescriptor::findShipComponentDescriptorByName(ConstCharCrcString(componentName.c_str()));

		if (nullptr == shipComponentDescriptor)
			return false;
		
		uint32 const componentCrc = shipComponentDescriptor->getCrc();
		
		bool showThisRow = false;
			
		{
			for (int chassisSlotType = 0; chassisSlotType < static_cast<int>(ShipChassisSlotType::SCST_num_types); ++chassisSlotType)
			{
				TemplateHardpointPairVector const & thpv = ShipComponentAttachmentManager::getAttachmentsForShip (chassisCrc, componentCrc, chassisSlotType);
				if (!thpv.empty())
				{
					showThisRow = true;
					break;
				}
			}
		}			
			
		if (!showThisRow)
			continue;
		
		tabStr += shipComponentDescriptor->getName().getString();

		for (int chassisSlotType = 0; chassisSlotType < static_cast<int>(ShipChassisSlotType::SCST_num_types); ++chassisSlotType)
		{
			if (!columnPresent[chassisSlotType])
				continue;

			TemplateHardpointPairVector const & thpv = ShipComponentAttachmentManager::getAttachmentsForShip (chassisCrc, componentCrc, chassisSlotType);

			tabStr.push_back('\t');

			bool firstThp = true;	
			for (TemplateHardpointPairVector::const_iterator tit = thpv.begin(); tit != thpv.end(); ++tit)
			{
				TemplateHardpointPair const & thp = *tit;
				uint32 const templateCrc = thp.first;
				PersistentCrcString const & hardpointName = thp.second;
				
				if (!firstThp)
					tabStr.push_back(',');

				CrcString const & templateName = ObjectTemplateList::lookUp(templateCrc);

				std::string templateNameStr(templateName.getString());

				if (templateNameStr.empty())
				{
					WARNING(true, ("ShipComponentAttachmentManager::save could not resolve template crc [%d]", static_cast<int>(templateCrc)));
					continue;
				}

				firstThp = false;
				
				std::string const & componentTypeName = 
					ShipComponentType::getNameFromType(
						static_cast<ShipComponentType::Type>(
							ShipChassisSlotType::getComponentTypeForSlot(
								static_cast<ShipChassisSlotType::Type>(chassisSlotType))));

				std::string const & attachmentTemplatePrefix = s_attachmentTemplatePrefix + componentTypeName + std::string ("/shared_");

				if (0 != templateNameStr.find(attachmentTemplatePrefix))
				{
					WARNING(true, ("Malformed template [%s], should start with [%s]", templateNameStr.c_str(), attachmentTemplatePrefix.c_str()));
					return false;
				}

				IGNORE_RETURN(templateNameStr.erase(0, attachmentTemplatePrefix.size()));

				if (templateNameStr.size() <= 4 || (templateNameStr.find(".iff") != templateNameStr.size() - 4))
				{
					WARNING(true, ("Malformed template [%s], should end with [.iff]", templateNameStr.c_str()));
					return false;
				}

				IGNORE_RETURN(templateNameStr.erase(templateNameStr.size() - 4, 4));

				tabStr += templateNameStr.c_str();
				tabStr.push_back(':');
				tabStr += hardpointName.getString();
			}
		}

		tabStr.push_back('\n');
	}

	tabStr.push_back('\n');

	bool retval = false;

	StdioFileFactory sff;
	AbstractFile * const af = sff.createFile(filenameTab.c_str(), "wb");
	if (nullptr != af && af->isOpen())
	{
		int const bytesWritten = af->write(static_cast<int>(tabStr.size()), tabStr.c_str());
		retval = (bytesWritten == static_cast<int>(tabStr.size()));
	}
	else
	{
		WARNING(true, ("ShipComponentDescriptor failed to write file [%s]", filenameTab.c_str()));
	}
	delete af;

	if (!retval)
		return false;
	
	DataTableWriter dtw;
	dtw.loadFromSpreadsheet(filenameTab.c_str());
	if (!dtw.save(filenameIff.c_str(), true))
	{
		WARNING(true, ("ShipComponentAttachmentManager failed to compile file [%s] to [%s]", filenameIff.c_str()));
		return false;
	}

	return true;
}

//----------------------------------------------------------------------

ShipComponentAttachmentManager::TemplateHardpointPairVector const & ShipComponentAttachmentManager::getAttachmentsForShip(uint32 chassisCrc, uint32 componentCrc, int chassisSlotType)
{
	DEBUG_FATAL (!s_installed, ("not installed"));

	AttachmentMap::const_iterator const it = s_attachmentMap->find (AttachmentKey (chassisCrc, componentCrc, chassisSlotType));
	if (it != s_attachmentMap->end ())
		return (*it).second;

	AttachmentKey const emptyAttachmentKey (0, 0, 0);
	return (*s_attachmentMap) [emptyAttachmentKey];
}

//----------------------------------------------------------------------

/**
* @param thpVectors is an array of length ShipChassisSlotType::SCST_num_types
*/

bool ShipComponentAttachmentManager::getAttachmentsForShip(uint32 chassisCrc, uint32 componentCrc, TemplateHardpointPairVector const ** thpVectors)
{
	bool found = false;

	for (int i = 0; i < static_cast<int>(ShipChassisSlotType::SCST_num_types); ++i)
	{
		AttachmentMap::const_iterator const it = s_attachmentMap->find (AttachmentKey (chassisCrc, componentCrc, i));
		if (it != s_attachmentMap->end ())
		{
			TemplateHardpointPairVector const & thpv = (*it).second;
			if (thpv.empty())
			{
				thpVectors[i] = nullptr;
			}
			else
			{
				thpVectors[i] = &thpv;
				found = true;
			}
		}
		else
			thpVectors[i] = nullptr;
	}

	return found;
}

// ----------------------------------------------------------------------

/**
 * Given the a ship, component, and chassis slot type, find any "extra"
 * hardpoints for that component.  An extra hardpoint is a hardpoint that
 * has no object attached to it, but represents the location of that component.
 * It is used for figuring out which component to damage based on collision
 * location, etc.
 */
ShipComponentAttachmentManager::HardpointVector const & ShipComponentAttachmentManager::getExtraHardpointsForComponent (uint32 const chassisCrc, uint32 const componentCrc, int const chassisSlotType)
{
	DEBUG_FATAL (!s_installed, ("not installed"));

	static const HardpointVector emptyHardpointVector;

	ExtraHardpointsMap::const_iterator const it = s_extraHardpointsMap->find (AttachmentKey (chassisCrc, componentCrc, chassisSlotType));
	if (it != s_extraHardpointsMap->end())
		return (*it).second;
	else
		return emptyHardpointVector;
}

// ----------------------------------------------------------------------

float ShipComponentAttachmentManager::getComponentHitRange (uint32 chassisCrc, uint32 componentCrc, int chassisSlotType)
{
	DEBUG_FATAL (!s_installed, ("not installed"));

	ComponentHitRangeMap::const_iterator const it = s_componentHitRangeMap->find (AttachmentKey (chassisCrc, componentCrc, chassisSlotType));
	if (it != s_componentHitRangeMap->end ())
		return (*it).second;

	return -1.0f;
}

//----------------------------------------------------------------------
//-- EDITOR methods
//----------------------------------------------------------------------

void ShipComponentAttachmentManager::setAttachmentsForShip(uint32 chassisCrc, uint32 componentCrc, int chassisSlotType, TemplateHardpointPairVector const & attachments)
{
	AttachmentKey const key(chassisCrc, componentCrc, chassisSlotType);
	IGNORE_RETURN(s_attachmentMap->erase(key));
	if (!attachments.empty())
		IGNORE_RETURN(s_attachmentMap->insert(std::make_pair(key, attachments)));

	Transceivers::attachmentsChanged.emitMessage(Messages::AttachmentsChanged::Payload(chassisCrc, std::make_pair(componentCrc, chassisSlotType)));
}

//----------------------------------------------------------------------

std::string const & ShipComponentAttachmentManager::getAttachmentTemplatePrefix()
{
	return s_attachmentTemplatePrefix;
}

//----------------------------------------------------------------------

/**
*  @param newChassisCrc zero to delete a component
*/

void ShipComponentAttachmentManager::udpateComponentCrc(uint32 const oldComponentCrc, uint32 const newComponentCrc)
{
	if (oldComponentCrc == newComponentCrc)
		return;

	for (AttachmentMap::iterator it = s_attachmentMap->begin(); it != s_attachmentMap->end(); )
	{
		//-- keep a copy of this for later
		AttachmentKey const oldKey = (*it).first;

		if (oldKey.componentCrc == oldComponentCrc)
		{
			//-- change the key by replacing it
			AttachmentKey const newKey(oldKey.chassisCrc, newComponentCrc, oldKey.chassisSlotType);

			//-- change to zero crc is a deletion of this component
			if (0 != newKey.componentCrc)
				(*s_attachmentMap)[newKey] = (*it).second;

			s_attachmentMap->erase(it++);

			Transceivers::attachmentsChanged.emitMessage(Messages::AttachmentsChanged::Payload(newKey.chassisCrc, std::make_pair(oldKey.componentCrc, newKey.chassisSlotType)));

			if (0 != newKey.componentCrc)
				Transceivers::attachmentsChanged.emitMessage(Messages::AttachmentsChanged::Payload(newKey.chassisCrc, std::make_pair(newKey.componentCrc, newKey.chassisSlotType)));

			continue;
		}

		++it;
	}
}

//----------------------------------------------------------------------

/**
*  @param newChassisCrc zero to delete a chassis
*/

void ShipComponentAttachmentManager::udpateChassisCrc(uint32 const oldChassisCrc, uint32 const newChassisCrc)
{
	if (oldChassisCrc == newChassisCrc)
		return;

	for (AttachmentMap::iterator it = s_attachmentMap->begin(); it != s_attachmentMap->end(); )
	{
		//-- keep a copy of this for later
		AttachmentKey const oldKey = (*it).first;

		if (oldKey.chassisCrc == oldChassisCrc)
		{
			//-- change the key by replacing it
			AttachmentKey const newKey(newChassisCrc, oldKey.componentCrc, oldKey.chassisSlotType);

			//-- change to zero crc is a deletion of this chassis
			if (0 != newKey.chassisCrc)
				(*s_attachmentMap)[newKey] = (*it).second;

			s_attachmentMap->erase(it++);

			Transceivers::attachmentsChanged.emitMessage(Messages::AttachmentsChanged::Payload(oldKey.chassisCrc, std::make_pair(newKey.componentCrc, newKey.chassisSlotType)));

			if (0 != newKey.chassisCrc)
				Transceivers::attachmentsChanged.emitMessage(Messages::AttachmentsChanged::Payload(newKey.chassisCrc, std::make_pair(newKey.componentCrc, newKey.chassisSlotType)));

			continue;
		}

		++it;
	}
}

//----------------------------------------------------------------------

void ShipComponentAttachmentManager::copyAttachmentsForComponent(uint32 const oldComponentCrc, uint32 const newComponentCrc)
{
	if (oldComponentCrc == newComponentCrc)
	{
		WARNING(true, ("ShipComponentAttachmentManager::copyAttachmentsForComponent() cannot copy from self"));
		return;
	}

	if (0 == oldComponentCrc || 0 == newComponentCrc)
	{
		WARNING(true, ("ShipComponentAttachmentManager::copyAttachmentsForComponent() cannot copy from/to zero"));
		return;
	}

	for (AttachmentMap::iterator it = s_attachmentMap->begin(); it != s_attachmentMap->end(); ++it)
	{
		//-- keep a copy of this for later
		AttachmentKey const oldKey = (*it).first;

		if (oldKey.componentCrc == oldComponentCrc)
		{
			//-- change the key by replacing it
			AttachmentKey const newKey(oldKey.chassisCrc, newComponentCrc, oldKey.chassisSlotType);

			//-- copy the attachment info into the new entry, 
			(*s_attachmentMap)[newKey] = (*it).second;

			Transceivers::attachmentsChanged.emitMessage(Messages::AttachmentsChanged::Payload(newKey.chassisCrc, std::make_pair(newKey.componentCrc, newKey.chassisSlotType)));
		}
	}
}


//----------------------------------------------------------------------

void ShipComponentAttachmentManager::copyAttachmentsForChassis(uint32 const oldChassisCrc, uint32 const newChassisCrc)
{
	if (oldChassisCrc == newChassisCrc)
	{
		WARNING(true, ("ShipComponentAttachmentManager::copyAttachmentsForChassis() cannot copy from self"));
		return;
	}

	if (0 == oldChassisCrc || 0 == newChassisCrc)
	{
		WARNING(true, ("ShipComponentAttachmentManager::copyAttachmentsForChassis() cannot copy from/to zero"));
		return;
	}

	for (AttachmentMap::iterator it = s_attachmentMap->begin(); it != s_attachmentMap->end(); ++it)
	{
		//-- keep a copy of this for later
		AttachmentKey const oldKey = (*it).first;

		if (oldKey.chassisCrc == oldChassisCrc)
		{
			//-- change the key by replacing it
			AttachmentKey const newKey(newChassisCrc, oldKey.componentCrc, oldKey.chassisSlotType);

			//-- copy the attachment info into the new entry, 
			(*s_attachmentMap)[newKey] = (*it).second;

			Transceivers::attachmentsChanged.emitMessage(Messages::AttachmentsChanged::Payload(newKey.chassisCrc, std::make_pair(newKey.componentCrc, newKey.chassisSlotType)));
		}
	}
}
//======================================================================
