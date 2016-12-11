//======================================================================
//
// ShipChassis.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/ShipChassis.h"

#include "fileInterface/StdioFile.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/LessPointerComparator.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedFoundation/TemporaryCrcString.h"
#include "sharedGame/ShipChassisSlot.h"
#include "sharedGame/ShipChassisWritable.h"
#include "sharedGame/ShipComponentAttachmentManager.h"
#include "sharedUtility/DataTable.h"
#include <algorithm>
#include <map>

//======================================================================

namespace ShipChassisNamespace
{

	typedef std::map<PersistentCrcString, ShipChassis *>       NameChassisMap;
	typedef std::map<uint32,              ShipChassis const *> CrcChassisMap;

	typedef ShipChassis::PersistentCrcStringVector      PersistentCrcStringVector;
	PersistentCrcStringVector * s_shipChassisNameVector;

	//-- the s_nameChassisMap owns the Chassis objects
	NameChassisMap            * s_nameChassisMap;

	//-- the s_nameChassisMap DOES NOT own the Chassis objects
	CrcChassisMap               s_crcChassisMap;

	bool s_useWritableChassis = false;

	bool s_installed = false;

	const char * const s_filename = "datatables/space/ship_chassis.iff";
}

using namespace ShipChassisNamespace;

//----------------------------------------------------------------------

void ShipChassis::install ()
{
	InstallTimer const installTimer("ShipChassis::install ");

	if (s_installed)
		return;
	
	s_nameChassisMap = new NameChassisMap;
	s_shipChassisNameVector = new PersistentCrcStringVector;

	s_installed = true;

	load();

	ExitChain::add(ShipChassis::remove, "ShipChassis::remove");
}

//----------------------------------------------------------------------

void ShipChassis::load()
{	
	Iff iff;	
	if (!iff.open (s_filename, true))
	{
		WARNING (true, ("Data file %s not available.", s_filename));
		return;
	}
	
	DataTable dt;
	dt.load (iff);
	iff.close ();
	
	std::string const & s_colnameName                = "name";
	std::string const & s_colnameSuffix_hitweight    = "_hitweight";
	std::string const & s_colnameSuffix_targetable   = "_targetable";
	std::string const & s_colnameFlyBySound          = "flyby_sound";
	std::string const & s_colnameWingOpenSpeedFactor = "wing_open_speed_factor";
	std::string const & s_colnameHitSoundGroup = "hit_sound_group";
	
	const int numRows = dt.getNumRows ();
	
	for (int row = 0; row < numRows; ++row)
	{
		ShipChassis * const chassis = s_useWritableChassis ? new ShipChassisWritable : new ShipChassis;
		
		TemporaryCrcString const nameString (dt.getStringValue (s_colnameName, row), true);
		IGNORE_RETURN(chassis->setName(nameString));
		
		chassis->setFlyBySound(dt.getStringValue (s_colnameFlyBySound, row));

		float const wingOpenSpeedFactor = dt.getFloatValue(s_colnameWingOpenSpeedFactor, row);

		chassis->setWingOpenSpeedFactor(wingOpenSpeedFactor);

		std::string const & hitSoundGroup = dt.getStringValue(s_colnameHitSoundGroup, row);

		chassis->setHitSoundGroup(hitSoundGroup);
		
		for (int shipChassisSlotType = 0; shipChassisSlotType < static_cast<int>(ShipChassisSlotType::SCST_num_explicit_types); ++shipChassisSlotType)
		{
			std::string const & colnameSlotName         = ShipChassisSlotType::getNameFromType (static_cast<ShipChassisSlotType::Type>(shipChassisSlotType));
			std::string const & colnameCompatibilities  = colnameSlotName;

			int const colNumber = dt.findColumnNumber(colnameCompatibilities);
			if (colNumber >= 0)
			{	
				std::string const & compatibilities = dt.getStringValue(colNumber, row);

				if (!compatibilities.empty())
				{
					std::string const & colnameHitWeight = colnameSlotName + s_colnameSuffix_hitweight;
					std::string const & colnameTargetable = colnameSlotName + s_colnameSuffix_targetable;
					int const hitWeight = dt.getIntValue(colnameHitWeight, row);
					bool const targetable = (dt.getIntValue(colnameTargetable, row) == 1) ? true : false;
				
					ShipChassisSlot const slot(static_cast<ShipChassisSlotType::Type>(shipChassisSlotType), compatibilities, hitWeight, targetable);				
					chassis->addSlot (slot);
				
					if (targetable)
					{
						chassis->setSlotTargetable(slot.getSlotType(), true);
					}
				}
			}
		}
		
		IGNORE_RETURN(chassis->addChassis(false));
	} //lint !e429 //custodial chassis
	
	std::sort (s_shipChassisNameVector->begin(), s_shipChassisNameVector->end(), LessPointerComparator());
}

//----------------------------------------------------------------------

bool ShipChassis::save(std::string const & filename)
{	
	//-- fill this string with the text contents of the tab file
	std::string tabStr;
	
	//-- TAB HEADER COLNAMES
	tabStr += "name\t";
	tabStr += "flyby_sound\t";
	tabStr += "hit_sound_group\t";
	tabStr += "wing_open_speed_factor";
	
	{
		for (int chassisSlotType = 0; chassisSlotType < static_cast<int>(ShipChassisSlotType::SCST_num_explicit_types); ++chassisSlotType)
		{
			tabStr.push_back('\t');
			std::string const & slotTypeName = ShipChassisSlotType::getNameFromType(static_cast<ShipChassisSlotType::Type>(chassisSlotType));
			tabStr += slotTypeName + "\t";
			tabStr += slotTypeName + "_hitweight\t";
			tabStr += slotTypeName + "_targetable";
		}
	}
	
	//-- TAB HEADER COLTYPES
	tabStr.push_back('\n');
	tabStr += "s\t";
	tabStr += "s\t";
	tabStr += "s\t";
	tabStr += "f[1.0]";
	
	{
		for (int chassisSlotType = 0; chassisSlotType < static_cast<int>(ShipChassisSlotType::SCST_num_explicit_types); ++chassisSlotType)
		{
			tabStr.push_back('\t');
			tabStr += "s\t";
			tabStr += "i[10]\t";
			tabStr += "b[0]";
		}
	}

	//-- TAB DATA ROWS
	{

		char buf[128];
		size_t const buf_size = sizeof(buf);

		StringVector chassisNames;
		IGNORE_RETURN(getSortedNameList(chassisNames));
		
		for (StringVector::const_iterator it = chassisNames.begin(); it != chassisNames.end(); ++it)
		{
			std::string const & chassisName = *it;
			ShipChassis const * const shipChassis = NON_NULL(findShipChassisByName(ConstCharCrcString(chassisName.c_str())));

			tabStr.push_back('\n');
			tabStr += chassisName + "\t";			
			tabStr += shipChassis->getFlyBySound() + "\t";
			tabStr += shipChassis->getHitSoundGroup() + "\t";
			IGNORE_RETURN(snprintf(buf, buf_size, "%2.2f", shipChassis->getWingOpenSpeedFactor()));
			tabStr += buf;

			for (int chassisSlotType = 0; chassisSlotType < static_cast<int>(ShipChassisSlotType::SCST_num_explicit_types); ++chassisSlotType)
			{
				tabStr.push_back('\t');

				ShipChassisSlot const * const chassisSlot = shipChassis->getSlot(static_cast<ShipChassisSlotType::Type>(chassisSlotType));

				if (nullptr == chassisSlot)
				{
					tabStr += "\t\t";
					continue;
				}

				//-- compatibilities are canonically space separated
				bool firstCompat = true;
				ShipChassisSlot::CompatibilityVector const & cv = chassisSlot->getCompatibilities();
				for (ShipChassisSlot::CompatibilityVector::const_iterator cit = cv.begin(); cit != cv.end(); ++cit)
				{
					if (!firstCompat)
						tabStr.push_back(' ');

					firstCompat = false;

					CrcString const * const cs = NON_NULL(*cit);
					tabStr += std::string(cs->getString());
				}
				
				tabStr.push_back('\t');
				
				if (10 != chassisSlot->getHitWeight())
					tabStr += 10;
				tabStr.push_back('\t');
				
				if (chassisSlot->isTargetable())
					tabStr += "1";
			}
		}
	}
	
	tabStr.push_back('\n');

	bool retval = false;

	StdioFileFactory sff;
	AbstractFile * const af = sff.createFile(filename.c_str(), "wb");
	if (nullptr != af && af->isOpen())
	{
		int const bytesWritten = af->write(static_cast<int>(tabStr.size()), tabStr.c_str());
		retval = (bytesWritten == static_cast<int>(tabStr.size()));
	}
	else
	{
		WARNING(true, ("ShipChassis failed to write file [%s]", filename.c_str()));
	}

	delete af;
	return retval;
}

//----------------------------------------------------------------------

void ShipChassis::remove  ()
{
	for (NameChassisMap::iterator it = s_nameChassisMap->begin (); it != s_nameChassisMap->end (); ++it)
		delete (*it).second;

	s_nameChassisMap->clear ();
	delete s_nameChassisMap;
	s_nameChassisMap = 0;

	s_crcChassisMap.clear  ();

	std::for_each(s_shipChassisNameVector->begin (), s_shipChassisNameVector->end (), PointerDeleter ());
	s_shipChassisNameVector->clear ();
	delete s_shipChassisNameVector;
	s_shipChassisNameVector = 0;

	s_installed = false;
}

//----------------------------------------------------------------------

ShipChassis::ShipChassis () :
m_name              (),
m_slots             (new SlotVector),
m_targetableSlots   (new IntVector),
m_flybySound        (),
m_wingOpenSpeedFactor(1.0f),
m_hitSoundGroup()
{

}

//----------------------------------------------------------------------

ShipChassis::~ShipChassis ()
{
	delete m_slots;
	m_slots = 0;
	delete m_targetableSlots;
	m_targetableSlots = 0;
}

//----------------------------------------------------------------------

ShipChassis const * ShipChassis::findShipChassisByName        (CrcString const & chassisName)
{
	DEBUG_FATAL (!s_installed, ("not installed"));

	const NameChassisMap::const_iterator it = s_nameChassisMap->find (PersistentCrcString(chassisName));
	if (it != s_nameChassisMap->end ())
		return (*it).second;

	return nullptr;
}

//----------------------------------------------------------------------

ShipChassis const * ShipChassis::findShipChassisByCrc            (uint32 chassisCrc)
{
	DEBUG_FATAL (!s_installed, ("not installed"));

	const CrcChassisMap::const_iterator it = s_crcChassisMap.find (chassisCrc);
	if (it != s_crcChassisMap.end ())
		return (*it).second;

	return nullptr;
}

//----------------------------------------------------------------------

void ShipChassis::addSlot (ShipChassisSlot const & slot)
{
	m_slots->push_back (slot);
}

//----------------------------------------------------------------------

void ShipChassis::removeSlot(int chassisSlotType)
{
	{
		for (SlotVector::iterator it = m_slots->begin (); it != m_slots->end (); ++it)
		{
			ShipChassisSlot const & slot = *it;
			if (slot.getSlotType() == chassisSlotType)
			{
				IGNORE_RETURN(m_slots->erase(it));
				break;
			}
		}
	}

	setSlotTargetable(chassisSlotType, false);
}

//----------------------------------------------------------------------

void ShipChassis::setSlotTargetable(int const chassisSlotType, bool targetable)
{
	if (targetable)
	{
		if (nullptr == getSlot(static_cast<ShipChassisSlotType::Type>(chassisSlotType)))
		{
			WARNING(true, ("ShipChassis cannot set non existant slot [%d] targetable", chassisSlotType));
			return;
		}

		if (std::find(m_targetableSlots->begin(), m_targetableSlots->end(), chassisSlotType) == m_targetableSlots->end())
			m_targetableSlots->push_back(chassisSlotType);
	}
	else
	{
		IGNORE_RETURN(m_targetableSlots->erase(std::remove(m_targetableSlots->begin(), m_targetableSlots->end(), chassisSlotType), m_targetableSlots->end()));
	}
}

//----------------------------------------------------------------------

ShipChassisSlot const * ShipChassis::getSlot(ShipChassisSlotType::Type shipChassisSlotType) const
{
	return const_cast<ShipChassis *>(this)->getSlot(shipChassisSlotType);
}

//----------------------------------------------------------------------

ShipChassisSlot * ShipChassis::getSlot(ShipChassisSlotType::Type shipChassisSlotType)
{
	for (SlotVector::iterator it = m_slots->begin (); it != m_slots->end (); ++it)
	{
		ShipChassisSlot & slot = *it;
		if (slot.getSlotType () == shipChassisSlotType)
			return &slot;
	}
	
	return nullptr;
}

//----------------------------------------------------------------------

ShipChassisSlotType::Type ShipChassis::getPreviousTargetableSlot (uint32 chassisCrc, ShipChassisSlotType::Type shipChassisSlotType)
{
	ShipChassis const * const chassis = findShipChassisByCrc(chassisCrc);
	if (chassis)
		return chassis->getPreviousTargetableSlot(shipChassisSlotType);

	return ShipChassisSlotType::SCST_num_types;
}

//----------------------------------------------------------------------

ShipChassisSlotType::Type  ShipChassis::getPreviousTargetableSlot (ShipChassisSlotType::Type currentlyTargetedSlotType) const
{
	if (m_targetableSlots->empty())
		return ShipChassisSlotType::SCST_num_types;
	
	//if we're given a "none" slot, return the last targetable slot
	if (currentlyTargetedSlotType == ShipChassisSlotType::SCST_num_types)
		return static_cast<ShipChassisSlotType::Type>(m_targetableSlots->back());
	
	//if we're given the first slot, return the none slot
	if (m_targetableSlots->front() == currentlyTargetedSlotType)
		return ShipChassisSlotType::SCST_num_types;
	
	//otherwise, find the given slot, and return the previous one
	for (IntVector::const_iterator it = m_targetableSlots->begin (); it != m_targetableSlots->end (); ++it)
	{
		int const slotType = *it;

		//we found the currently targeted slot, return the previous one
		if (slotType == currentlyTargetedSlotType)
			return static_cast<ShipChassisSlotType::Type>(*(--it));
	}

	return ShipChassisSlotType::SCST_num_types;
}

//----------------------------------------------------------------------

ShipChassisSlotType::Type ShipChassis::getNextTargetableSlot (uint32 chassisCrc, ShipChassisSlotType::Type shipChassisSlotType)
{
	int result = ShipChassisSlotType::SCST_num_types;
	ShipChassis const * const chassis = findShipChassisByCrc(chassisCrc);
	if(chassis && !chassis->m_targetableSlots->empty())
	{
		//if we're given a "none" slot, return the first targetable slot
		if(shipChassisSlotType == ShipChassisSlotType::SCST_num_types)
		{
			IntVector::const_iterator it = chassis->m_targetableSlots->begin();
			return static_cast<ShipChassisSlotType::Type>(*it);
		}

		//otherwise, take the given slot, find it, then return the next slot
		IntVector::const_iterator const firstIt = chassis->m_targetableSlots->begin ();
		int const firstSlotType = *firstIt;
		result = firstSlotType;

		for (IntVector::const_iterator it = chassis->m_targetableSlots->begin (); it != chassis->m_targetableSlots->end (); ++it)
		{
			int const slotType = *it;
			//we found the currently targeted slot, return the next one
			if (slotType == shipChassisSlotType)
			{
				//grab the next item
				if(++it != chassis->m_targetableSlots->end ())
				{
					return static_cast<ShipChassisSlotType::Type>(slotType);
				}
				else
				{
					//if we're at the end, we'll return a "none" target 
					return ShipChassisSlotType::SCST_num_types;
				}
			}
		}
	}

	return static_cast<ShipChassisSlotType::Type>(result);
}

//----------------------------------------------------------------------

ShipChassisSlotType::Type  ShipChassis::getNextTargetableSlot (ShipChassisSlotType::Type currentlyTargetedSlotType) const
{
	if (m_targetableSlots->empty())
		return ShipChassisSlotType::SCST_num_types;
	
	//if we're given a "none" slot, return the first targetable slot
	if (currentlyTargetedSlotType == ShipChassisSlotType::SCST_num_types)
		return static_cast<ShipChassisSlotType::Type>(m_targetableSlots->front());
	
	//if we're given the last slot, return the none slot
	if (m_targetableSlots->back() == currentlyTargetedSlotType)
		return ShipChassisSlotType::SCST_num_types;
	
	//otherwise, find the given slot, and return the previous one
	for (IntVector::reverse_iterator it = m_targetableSlots->rbegin (); it != m_targetableSlots->rend (); ++it)
	{
		int const slotType = *it;

		//we found the currently targeted slot, return the previous (next) one
		if (slotType == currentlyTargetedSlotType)
			return static_cast<ShipChassisSlotType::Type>(*(--it));
	}

	return ShipChassisSlotType::SCST_num_types;
}

//----------------------------------------------------------------------

bool ShipChassis::isSlotTargetable(int chassisSlotType) const
{
	return std::find(m_targetableSlots->begin(), m_targetableSlots->end(), chassisSlotType) != m_targetableSlots->end();
}

//----------------------------------------------------------------------

bool ShipChassis::isSlotTargetable (uint32 chassisCrc, ShipChassisSlotType::Type shipChassisSlotType)
{
	ShipChassis const * const chassis = findShipChassisByCrc(chassisCrc);
	if(chassis)
	{
		return chassis->isSlotTargetable(shipChassisSlotType);
	}
	return false;
}

//----------------------------------------------------------------------

ShipChassisSlotType::Type  ShipChassis::getSlotPowerPriority  (int slotIndex) const
{
	if (slotIndex < static_cast<int>(ShipChassisSlotType::SCST_first) || 
		slotIndex >= static_cast<int>(ShipChassisSlotType::SCST_num_types))
		return ShipChassisSlotType::SCST_num_types;

	//@todo: data drive this
	return static_cast<ShipChassisSlotType::Type>(slotIndex);
}

//----------------------------------------------------------------------

ShipChassis::PersistentCrcStringVector const & ShipChassis::getShipChassisCrcVector         ()
{
	DEBUG_FATAL (!s_installed, ("not installed"));
	return *NON_NULL(s_shipChassisNameVector);
}

//----------------------------------------------------------------------

ShipChassis::ShipChassis(ShipChassis const & rhs) :
m_name(rhs.m_name),
m_slots(new SlotVector(*rhs.m_slots)),
m_targetableSlots(new IntVector(*rhs.m_targetableSlots)),
m_flybySound(rhs.m_flybySound),
m_wingOpenSpeedFactor(rhs.m_wingOpenSpeedFactor),
m_hitSoundGroup(rhs.m_hitSoundGroup)
{
}

//----------------------------------------------------------------------

void ShipChassis::setFlybySound(std::string const & flybySound)
{
	m_flybySound = flybySound;
}

//----------------------------------------------------------------------

void ShipChassis::setUseWritableChassis(bool onlyUseThisForTools)
{
	DEBUG_FATAL (s_installed, ("already installed, setUseWritableChassis must be called prior to installation"));

	s_useWritableChassis = onlyUseThisForTools;
}

//----------------------------------------------------------------------

bool ShipChassis::addChassis(bool doSort)
{
	if (doSort && nullptr != findShipChassisByCrc(getCrc()))
	{
		WARNING(true, ("ShipChassis attempt to add multiple [%s] chassis", getName().getString()));
		return false;
	}

	IGNORE_RETURN (s_nameChassisMap->insert             (std::make_pair (m_name, this)));
	IGNORE_RETURN (s_crcChassisMap.insert               (std::make_pair (getCrc (), this)));
	s_shipChassisNameVector->push_back (new PersistentCrcString(getName()));

	if (doSort)
	{
		std::sort (s_shipChassisNameVector->begin(), s_shipChassisNameVector->end(), LessPointerComparator());
	}

	return true;
}

//----------------------------------------------------------------------

bool ShipChassis::removeChassis()
{
	if (s_nameChassisMap->find(m_name) == s_nameChassisMap->end() || s_crcChassisMap.find(getCrc()) == s_crcChassisMap.end())
		return false;

	IGNORE_RETURN(s_nameChassisMap->erase(m_name));
	IGNORE_RETURN(s_crcChassisMap.erase(getCrc()));

	for (PersistentCrcStringVector::iterator it = s_shipChassisNameVector->begin(); it != s_shipChassisNameVector->end(); ++it)
	{
		PersistentCrcString const * const pcs = *it;
		if (pcs->getCrc() == getCrc())
		{
			IGNORE_RETURN(s_shipChassisNameVector->erase(it));
			break;
		}
	}

	return true;
}

//----------------------------------------------------------------------

bool ShipChassis::setName(CrcString const & name)
{
	ShipChassis const * const dupeNameShipChassis = findShipChassisByName(name);
	if (nullptr != dupeNameShipChassis)
	{
		WARNING(true, ("ShipChassis attempt to set name [%s] already exists", name.getString()));
		return false;
	}

	ShipChassis const * const chassisInList = findShipChassisByCrc(getCrc());

	if (this == chassisInList)
	{
		ShipComponentAttachmentManager::udpateChassisCrc(m_name.getCrc(), name.getCrc());

		if (!removeChassis())
			return false;

		m_name.CrcString::set(name);

		if (!addChassis(true))
			return false;
	}
	else
	{
		m_name.CrcString::set(name);
	}

	return true;
}

//----------------------------------------------------------------------

ShipChassis::StringVector & ShipChassis::getSortedNameList(StringVector & chassisNames)
{
	ShipChassis::PersistentCrcStringVector const & shipChassisNameVector = ShipChassis::getShipChassisCrcVector();
	chassisNames.reserve(shipChassisNameVector.size());
	for (ShipChassis::PersistentCrcStringVector::const_iterator it = shipChassisNameVector.begin(); it != shipChassisNameVector.end(); ++it)
	{
		PersistentCrcString const * const pcs = *it;
		chassisNames.push_back(pcs->getString());
	}
	
	std::sort(chassisNames.begin(), chassisNames.end());
	return chassisNames;
}

//----------------------------------------------------------------------

char const * const ShipChassis::getFilename()
{
	return s_filename;
}

//----------------------------------------------------------------------

float ShipChassis::getWingOpenSpeedFactor() const
{
	return m_wingOpenSpeedFactor;
}

//----------------------------------------------------------------------

std::string const & ShipChassis::getHitSoundGroup() const
{
	return m_hitSoundGroup;
}

//----------------------------------------------------------------------

void ShipChassis::setWingOpenSpeedFactor(float const wingOpenSpeedFactor)
{
	m_wingOpenSpeedFactor = wingOpenSpeedFactor;
}

//----------------------------------------------------------------------

void ShipChassis::setHitSoundGroup(std::string const & hitSoundGroup)
{
	m_hitSoundGroup = hitSoundGroup;
}

//======================================================================
