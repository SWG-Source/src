//======================================================================
//
// ShipChassisSlot.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/ShipChassisSlot.h"
#include "sharedGame/ShipComponentType.h"

#include "UnicodeUtils.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/LessPointerComparator.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedGame/ShipComponentDescriptor.h"
#include <algorithm>
#include <vector>

//======================================================================

namespace ShipChassisSlotNamespace
{
	bool s_installed = false;

	void install ()
	{
		if (s_installed)
			return;
	}

	//----------------------------------------------------------------------

	typedef std::vector<CrcString *> CompatibilityVector;

	//----------------------------------------------------------------------

	void copyCompatibilities(CompatibilityVector const & src, CompatibilityVector & dst)
	{
		std::for_each(dst.begin(), dst.end(), PointerDeleter());
		dst.clear();
		dst.reserve(src.size());
		for (CompatibilityVector::const_iterator it = src.begin(); it != src.end(); ++it)
		{
			CrcString const * const str = *it;
			dst.push_back(new PersistentCrcString(*str));
		}
	}
}

using namespace ShipChassisSlotNamespace;

//----------------------------------------------------------------------

ShipChassisSlot::ShipChassisSlot () :
m_slotType        (ShipChassisSlotType::SCST_num_types),
m_compatibilities (new CompatibilityVector),
m_hitWeight       (1),
m_targetable      (false)
{
}

//----------------------------------------------------------------------

ShipChassisSlot::ShipChassisSlot (ShipChassisSlotType::Type slotType, std::string const & compatibilities, int hitWeight, bool targetable) :
m_slotType        (slotType),
m_compatibilities (new CompatibilityVector),
m_hitWeight       (hitWeight),
m_targetable      (targetable)
{
	unpackCompatibilities (compatibilities);
}

//----------------------------------------------------------------------

ShipChassisSlot::~ShipChassisSlot ()
{
	std::for_each(m_compatibilities->begin(), m_compatibilities->end(), PointerDeleter());
	m_compatibilities->clear();
	delete m_compatibilities;
	m_compatibilities = nullptr;
}

//----------------------------------------------------------------------

ShipChassisSlot::ShipChassisSlot (const ShipChassisSlot & rhs) :
m_slotType        (rhs.m_slotType),
m_compatibilities (new CompatibilityVector),
m_hitWeight       (rhs.m_hitWeight),
m_targetable      (rhs.m_targetable)
{
	copyCompatibilities(*rhs.m_compatibilities, *m_compatibilities);
}

//----------------------------------------------------------------------

ShipChassisSlot & ShipChassisSlot::operator= (const ShipChassisSlot & rhs)
{
	if (this != &rhs)
	{
		m_slotType         = rhs.m_slotType;
		copyCompatibilities(*rhs.m_compatibilities, *m_compatibilities);
		m_hitWeight        = rhs.m_hitWeight;
		m_targetable       = rhs.m_targetable;
	}

	return *this;
} //lint !e1539 // m_compatibilities is assigned

//----------------------------------------------------------------------

bool ShipChassisSlot::canAcceptComponent (ShipComponentDescriptor const & shipComponentDescriptor) const
{
	if (canAcceptComponentType (shipComponentDescriptor.getComponentType ()))
	{
		if (canAcceptCompatibility (shipComponentDescriptor.getCompatibility ()))
			return true;
	}

	return false;
}

//----------------------------------------------------------------------

std::string const ShipChassisSlot::getMatchingCompatibilityString(ShipComponentDescriptor const & shipComponentDescriptor) const
{
	if (canAcceptComponent (shipComponentDescriptor))
	{
		return std::string(shipComponentDescriptor.getCompatibility().getString());		
	}
	return std::string();
}

//----------------------------------------------------------------------

bool ShipChassisSlot::canAcceptComponentType (int shipComponentType) const
{
	if (shipComponentType >= static_cast<int>(ShipComponentType::SCT_num_types))
		return false;

	return ShipChassisSlotType::getComponentTypeForSlot (m_slotType) == shipComponentType;
}

//----------------------------------------------------------------------

bool ShipChassisSlot::canAcceptCompatibility (CrcString const & compatibility) const
{
	//-- nullptr compatibility components are universally accepted
	if (compatibility.isEmpty ())
		return true;

	//-- m_compatibilities must be maintained in sorted order
	return std::binary_search (m_compatibilities->begin (), m_compatibilities->end (), &compatibility, &LessPointerComparatorStatic<CrcString const>::compare);
}

//----------------------------------------------------------------------

void ShipChassisSlot::unpackCompatibilities (std::string const & compatibilities)
{
	std::for_each(m_compatibilities->begin(), m_compatibilities->end(), PointerDeleter());
	m_compatibilities->clear();

	size_t pos = 0;

	std::string token;
	while (Unicode::getFirstToken (compatibilities, pos, pos, token, ", \n\r\t"))
	{
		m_compatibilities->push_back (new PersistentCrcString (token.c_str (), true));

		if (pos != std::string::npos) //lint !e650 !e737 // stupid msvc
			++pos;
		else
			break;
	}

	std::sort(m_compatibilities->begin(), m_compatibilities->end(), LessPointerComparator());
}

//----------------------------------------------------------------------

std::string const ShipChassisSlot::getCompatibilityString() const
{
	static std::string result;
	result.clear();
	
	for (ShipChassisSlot::CompatibilityVector::const_iterator cit = m_compatibilities->begin (); cit != m_compatibilities->end (); ++cit)
	{
		CrcString const & compatibility = *NON_NULL(*cit);
		result += compatibility.getString ();
		result.push_back (' ');
	}
	
	return result;
}

//----------------------------------------------------------------------

void ShipChassisSlot::setTargetable(bool targetable)
{
	m_targetable = targetable;
}

//----------------------------------------------------------------------

void ShipChassisSlot::setHitWeight(int hitWeight)
{
	m_hitWeight = hitWeight;
}

//----------------------------------------------------------------------

void ShipChassisSlot::setCompatibilities(std::string const & compatibilities)
{
	unpackCompatibilities(compatibilities);
}

//----------------------------------------------------------------------

void ShipChassisSlot::removeCompatibility(std::string const & compat)
{
	ConstCharCrcString const cccs(compat.c_str());
	for (CompatibilityVector::iterator it = m_compatibilities->begin(); it != m_compatibilities->end(); ++it)
	{
		CrcString * const existingCompat = NON_NULL(*it);
		if (*existingCompat == cccs)
		{
			IGNORE_RETURN(m_compatibilities->erase(it));
			return;
		}
	}
}

//----------------------------------------------------------------------

void ShipChassisSlot::addCompatibility(std::string const & compat)
{
	ConstCharCrcString const cccs(compat.c_str());
	if (std::binary_search (m_compatibilities->begin (), m_compatibilities->end (), &cccs, &LessPointerComparatorStatic<CrcString const>::compare))
	{
		WARNING(true, ("ShipChassisSlot::addCompatibility attempt to add exising compatibility"));
		return;
	}

	m_compatibilities->push_back(new PersistentCrcString(cccs.getString(), cccs.getCrc()));
	std::sort(m_compatibilities->begin(), m_compatibilities->end(), LessPointerComparator());
}

//----------------------------------------------------------------------

Unicode::String ShipChassisSlot::getLocalizedSlotName () const
{
	return getLocalizedSlotInformation("ship_slot_n", "ship_slot_compatabilities_n", true, "/");
}

//----------------------------------------------------------------------

Unicode::String ShipChassisSlot::getLocalizedSlotDescription () const
{
	return getLocalizedSlotInformation("ship_slot_d", "ship_slot_compatabilities_d", true, " ");
}

//----------------------------------------------------------------------

Unicode::String ShipChassisSlot::getLocalizedSlotInformation (std::string const & stringTableGeneral, std::string const & stringTableSpecific,
															  bool allowCompoundInfo, std::string const & compoundSeperator) const
{
	CompatibilityVector const & compatibilities = getCompatibilities();
	if(compatibilities.empty())
		return Unicode::emptyString;

	//if it can take multiple types, try to build a compound name
	//otherwise just use the generic name
	bool useSlotName = false;

	//special case armor, use general information to prevent confusion
	if(m_slotType == ShipChassisSlotType::SCST_armor_0 || m_slotType == ShipChassisSlotType::SCST_armor_1)
		useSlotName = true;

	int numCompatibilities = static_cast<int>(compatibilities.size());
	if(!useSlotName && numCompatibilities > 1)
	{
		//try to build a compound name
		if(allowCompoundInfo)
		{
			Unicode::String compoundName;
			Unicode::String compatibilityName;
			for(int i = 0; i < numCompatibilities; ++i)
			{
				CrcString * slotNameCrcString = compatibilities[i];
				StringId sid(stringTableSpecific, slotNameCrcString->getString());

				if(sid.localize(compatibilityName))
				{
					if(!compoundName.empty())
					{
						compoundName += Unicode::narrowToWide(compoundSeperator);
					}
					compoundName += compatibilityName;
				}
			}

			if(!compoundName.empty())
			{
				return compoundName;
			}
		}

		//fall back to using the slot name
		useSlotName = true;
	}
		
	if(useSlotName)
	{
		StringId sid(stringTableGeneral, ShipChassisSlotType::getNameFromType(m_slotType));
		return sid.localize();
	}

	//else, find the more specific name for it
	CrcString * slotNameCrcString = compatibilities[0];
	StringId sid(stringTableSpecific, slotNameCrcString->getString());
	return sid.localize();
}

//======================================================================
