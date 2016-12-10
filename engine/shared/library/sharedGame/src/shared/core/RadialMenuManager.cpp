//======================================================================
//
// RadialMenuManager.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/RadialMenuManager.h"

#include "sharedFile/Iff.h"
#include "sharedFile/TreeFile.h"
#include "sharedUtility/DataTable.h"
#include "sharedNetworkMessages/ObjectMenuRequestData.h"
#include <algorithm>
#include <map>

//======================================================================

namespace
{
	struct RadialMenuInfo
	{
		float       range;
		std::string commandName;
		bool        useRadialTarget;
	};

	typedef std::map<int, RadialMenuInfo> RangeMap;
	RangeMap s_ranges;
	
	typedef std::map<std::string, int> NameMap;
	NameMap  s_names;

	bool s_installed = false;

	enum DtCols
	{
		DC_caption,
		DC_range,
		DC_commandName,
		DC_useRadialTarget
	};

	void install ()
	{
		DEBUG_FATAL (s_installed, ("Already installed"));

		s_installed = true;

		static const char * const filename = "datatables/player/radial_menu.iff";
		
		Iff iff;
		
		if (!iff.open (filename, true))
			WARNING (true, ("Data file %s not available.", filename));
		else
		{
			
			DataTable dt;
			dt.load (iff);
			iff.close ();

			const int numRows = dt.getNumRows ();
			
			for (int i = 0; i < numRows; ++i)
			{
				RadialMenuInfo rmi;
				rmi.range           = dt.getFloatValue  (DC_range,           i);
				rmi.commandName     = dt.getStringValue (DC_commandName,     i), 
				rmi.useRadialTarget = dt.getIntValue    (DC_useRadialTarget, i) != 0;

				s_ranges.insert (std::make_pair (i, rmi));

				const std::string & caption = dt.getStringValue (DC_caption, i);
				s_names.insert  (std::make_pair (caption, i));
			}
		}
	}
}

//----------------------------------------------------------------------

bool RadialMenuManager::getRangeForMenuType (int menuType, float &range)
{
	if (!s_installed)
		install ();

	const RangeMap::const_iterator it = s_ranges.find (menuType);

	if (it != s_ranges.end ())
	{
		const RadialMenuInfo & rmi = (*it).second;
		range = rmi.range;
		return true;
	}

	return false;
}

//----------------------------------------------------------------------

bool  RadialMenuManager::getCommandForMenuType (int menuType, std::string & commandName, bool & useRadialTarget)
{
	if (!s_installed)
		install ();

	const RangeMap::const_iterator it = s_ranges.find (menuType);

	if (it != s_ranges.end ())
	{
		const RadialMenuInfo & rmi = (*it).second;
		commandName     = rmi.commandName;
		useRadialTarget = rmi.useRadialTarget;
		return true;
	}

	return false;
}

//----------------------------------------------------------------------

int RadialMenuManager::getMenuTypeByName     (const std::string & name)
{
	if (!s_installed)
		install ();

	const NameMap::const_iterator it = s_names.find (name);
	if (it != s_names.end ())
	{
		return (*it).second;
	}

	return 0;
}

//----------------------------------------------------------------------

int RadialMenuManager::addRootMenu           (DataVector & dv, int menuType, const Unicode::String & label, bool serverNotify)
{
	if (menuType == 0)
		return 0;

	const ObjectMenuRequestData * const data = findMenuByType (dv, menuType);

	if (data)
		return 0;

	const uint8 id = findNextId (dv);
	dv.push_back (ObjectMenuRequestData (id, 0, static_cast<uint8>(menuType), label, true, serverNotify));

	return id;
}

//----------------------------------------------------------------------

int RadialMenuManager::addSubMenu            (DataVector & dv, int parent, int menuType, const Unicode::String & label, bool serverNotify)
{
	if (menuType == 0 || parent == 0)
		return 0;

	const ObjectMenuRequestData * const data = findMenuByType (dv, menuType);

	if (data)
		return 0;

	const ObjectMenuRequestData * const root = findMenuById (dv, parent);

	if (!root)
		return 0;

	const uint8 id = findNextId (dv);
	dv.push_back (ObjectMenuRequestData (id, static_cast<uint8>(parent), static_cast<uint8>(menuType), label, true, serverNotify));

	return id;
}

//----------------------------------------------------------------------

const ObjectMenuRequestData * RadialMenuManager::findMenuByType         (const DataVector & dv, int menuType)
{
	for (DataVector::const_iterator it = dv.begin (); it != dv.end (); ++it)
	{
		const ObjectMenuRequestData & data = *it;
		if (data.m_menuItemType == menuType)
			return &data;
	}

	return 0;
}

//----------------------------------------------------------------------

const ObjectMenuRequestData * RadialMenuManager::findMenuById          (const DataVector & dv, int id)
{
	for (DataVector::const_iterator it = dv.begin (); it != dv.end (); ++it)
	{
		const ObjectMenuRequestData & data = *it;
		if (data.m_id == id)
			return &data;
	}

	return 0;
}

//----------------------------------------------------------------------

uint8 RadialMenuManager::findNextId            (const DataVector & dv)
{
	uint8 id = 1;
	for (DataVector::const_iterator it = dv.begin (); it != dv.end (); ++it)
	{
		const ObjectMenuRequestData & data = *it;
		id = std::max (id, static_cast<uint8>(data.m_id + 1));
	}
	
	return id;
}

//======================================================================
