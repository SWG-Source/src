//======================================================================
//
// RadialMenuManager.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_RadialMenuManager_H
#define INCLUDED_RadialMenuManager_H

//======================================================================

struct ObjectMenuRequestData;

//----------------------------------------------------------------------

class RadialMenuManager
{
public:

	typedef std::vector<ObjectMenuRequestData> DataVector;

	static int         getMenuTypeByName     (const std::string & name);
	static bool        getRangeForMenuType   (int menuType, float &range);
	static bool        getCommandForMenuType (int menuType, std::string & commandName, bool & useRadialTarget);

	static int                            addRootMenu           (DataVector & dv, int menuType, const Unicode::String & label, bool serverNotify);
	static int                            addSubMenu            (DataVector & dv, int parent, int menuType, const Unicode::String & label, bool serverNotify);
	static const ObjectMenuRequestData *  findMenuByType        (const DataVector & dv, int menuType);
	static const ObjectMenuRequestData *  findMenuById          (const DataVector & dv, int id);
	static uint8                          findNextId            (const DataVector & dv);
};

//======================================================================

#endif
