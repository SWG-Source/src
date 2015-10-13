// ======================================================================
//
// CellPermsUpdateObserver.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef CellPermissions_H
#define CellPermissions_H

// ======================================================================

#include "Archive/AutoDeltaObserverOps.h"
#include "Archive/ByteStream.h"
#include "sharedDebug/Profiler.h"

// ======================================================================

class BuildingObject;
class CellObject;
class Client;
class CreatureObject;
class NetworkId;
class ServerObject;
class TangibleObject;

// ======================================================================

namespace CellPermissions
{
	class PermissionObject;
};

//----------------------------------------------------------------------

namespace Archive
{
	void get(ReadIterator & source, CellPermissions::PermissionObject & target);
	void put(ByteStream & target, const CellPermissions::PermissionObject & source);
};

//----------------------------------------------------------------------

namespace CellPermissions
{
	//----------------------------------------------------------------------

	class PermissionObject
	{
		friend void Archive::get( Archive::ReadIterator & source, CellPermissions::PermissionObject & target );
		friend void Archive::put( Archive::ByteStream & target, const CellPermissions::PermissionObject & source );

		friend class ::BuildingObject;
		friend class ::CellObject;

	public:

		PermissionObject();
		PermissionObject( const std::string& name );

		bool operator<( PermissionObject const &rhs ) const;

		// Return the permission string in the same format as it was given
		std::string        getName() const;

	private:

		// We need to remember the "format" of the string that was given to us
		enum PermissionFormat
		{
			PF_UNCONVERTED,     // A string read from the database that has not been converted
			PF_CHARACTER_NAME,  // A string was given that we could convert to a network ID
			PF_GUILD_NAME,      // A guild name was provided
			PF_NUMERIC_VALUE,   // A numeric value was given to us (probably a network ID)
			PF_UNKNOWN_STRING,  // A string was given to us that doesn't fit the other categories
		};

		// Intended to be used for converting entries from the database
		PermissionObject( PermissionFormat format, const std::string& name );

	private:

		// Query whether the permission string has ever been "converted"
		// NOTE: This is intended to let Building and Cell objects know whether
		//       they need to convert data that has been loaded from the database.
		bool               hasBeenConverted() const;

	private:

		PermissionFormat   m_originalPermissionFormat;
		std::string        m_permissionString;
	};

	typedef stdset<PermissionObject>::fwd PermissionList;

	//----------------------------------------------------------------------

	class UpdateObserver
	{
	public:
		UpdateObserver(CellObject *cell, Archive::AutoDeltaObserverOp operation);
		UpdateObserver(TangibleObject *tangible, Archive::AutoDeltaObserverOp operation);
		~UpdateObserver();

	private:
		UpdateObserver(UpdateObserver const &);
		UpdateObserver &operator=(UpdateObserver const &);

		void init(BuildingObject *building);
		void getCellPermissions(ServerObject &obj);
		void handleCellPermissionsUpdateIfNeeded(ServerObject &obj, unsigned int &permPos, stdvector<CreatureObject*>::fwd &expel);

	private:
		ProfilerAutoBlock m_profilerBlock;
		BuildingObject *  m_building;
		std::vector<bool> m_permissions;
	};

	//----------------------------------------------------------------------

	class ViewerChangeObserver
	{
	public:
		ViewerChangeObserver(CreatureObject *creature);
		~ViewerChangeObserver();

	private:
		ViewerChangeObserver(ViewerChangeObserver const &);
		ViewerChangeObserver &operator=(ViewerChangeObserver const &);

	private:
		ProfilerAutoBlock m_profilerBlock;
		std::vector<CellObject *> m_cells;
		std::vector<bool> m_permissions;
		CreatureObject *m_creature;
	};

	//----------------------------------------------------------------------

	bool isOnList(PermissionList const &permList, std::string const &name);
	bool isOnList(PermissionList const &permList, CreatureObject const &who);

	//----------------------------------------------------------------------
};

// ======================================================================

#endif // CellPermissions_H

