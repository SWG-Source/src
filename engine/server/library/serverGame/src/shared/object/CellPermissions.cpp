// ======================================================================
//
// CellPermissions.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/CellPermissions.h"

#include "UnicodeUtils.h"
#include "serverGame/BuildingObject.h"
#include "serverGame/CellObject.h"
#include "serverGame/Client.h"
#include "serverGame/ContainerInterface.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/GuildInterface.h"
#include "serverGame/NameManager.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedLog/Log.h"
#include "sharedObject/Container.h"
#include "sharedNetworkMessages/UpdateCellPermissionMessage.h"

// ======================================================================

static std::set<BuildingObject const*> s_activeUpdaters;

// ======================================================================

namespace Archive
{
	// The prefixes we use to write permission strings to the database
	const std::string PERMISSION_STRING_PREFIX_UNKNOWN_OLD    = "u:";
	const std::string PERMISSION_STRING_PREFIX_UNKNOWN_NEW    = "U:";
	const std::string PERMISSION_STRING_PREFIX_GUILD_NAME_OLD = "g:";
	const std::string PERMISSION_STRING_PREFIX_GUILD_NAME_NEW = "G:";
	const std::string PERMISSION_STRING_PREFIX_CHARACTER_NAME = "c:";
	const std::string PERMISSION_STRING_PREFIX_NUMERIC_VALUE  = "n:";

	// ----------------------------------------------------------------------

	void get(ReadIterator &source, CellPermissions::PermissionObject &target)
	{
		std::string permissionString;
		get(source, permissionString);

		// Assume the string in the database has not been converted
		CellPermissions::PermissionObject::PermissionFormat format = CellPermissions::PermissionObject::PF_UNCONVERTED;

		// Determine whether the string in the database has already been converted
		const std::string prefix( permissionString, 0, 2 );
		if ( prefix == PERMISSION_STRING_PREFIX_CHARACTER_NAME )
		{
			format = CellPermissions::PermissionObject::PF_CHARACTER_NAME;
		}
		else if ( prefix == PERMISSION_STRING_PREFIX_GUILD_NAME_OLD )
		{
			// We used to store guilds by name instead of guild ID.
			// So we will try to reconvert the guild name to a guild ID
			format = CellPermissions::PermissionObject::PF_UNCONVERTED;
			permissionString.erase( 0, prefix.length() );
		}
		else if ( prefix == PERMISSION_STRING_PREFIX_GUILD_NAME_NEW )
		{
			format = CellPermissions::PermissionObject::PF_GUILD_NAME;
		}
		else if ( prefix == PERMISSION_STRING_PREFIX_NUMERIC_VALUE )
		{
			format = CellPermissions::PermissionObject::PF_NUMERIC_VALUE;
		}
		else if ( prefix == PERMISSION_STRING_PREFIX_UNKNOWN_OLD )
		{
			// We had an issue where we may have been storing full names in the database
			// and we did not know they were player names since NameManager only
			// converts first names to player names.  So we will try to reconvert
			// the unknown entries again
			format = CellPermissions::PermissionObject::PF_UNCONVERTED;
			permissionString.erase( 0, prefix.length() );
		}
		else if ( prefix == PERMISSION_STRING_PREFIX_UNKNOWN_NEW )
		{
			format = CellPermissions::PermissionObject::PF_UNKNOWN_STRING;
		}

		// Remove the prefix if necessary
		if ( format != CellPermissions::PermissionObject::PF_UNCONVERTED )
		{
			permissionString.erase( 0, prefix.length() );
		}

		// When buildings and cells load, it is their responsibility to convert the string if necessary
		target = CellPermissions::PermissionObject( format, permissionString );
	}

	// ----------------------------------------------------------------------

	void put( ByteStream &target, CellPermissions::PermissionObject const &source )
	{
		std::string permissionString = source.m_permissionString;

		// Before writing out the permission string, we need to append a prefix
		switch ( source.m_originalPermissionFormat )
		{
			case CellPermissions::PermissionObject::PF_CHARACTER_NAME:
				permissionString = PERMISSION_STRING_PREFIX_CHARACTER_NAME + permissionString;
				break;

			case CellPermissions::PermissionObject::PF_GUILD_NAME:
				permissionString = PERMISSION_STRING_PREFIX_GUILD_NAME_NEW + permissionString;
				break;

			case CellPermissions::PermissionObject::PF_NUMERIC_VALUE:
				permissionString = PERMISSION_STRING_PREFIX_NUMERIC_VALUE + permissionString;
				break;

			case CellPermissions::PermissionObject::PF_UNKNOWN_STRING:
				permissionString = PERMISSION_STRING_PREFIX_UNKNOWN_NEW + permissionString;
				break;

			default:
				// Do nothing to the string and it will be considered "unconverted" when it is read back in
				break;
		}

		put(target, permissionString);
	}

}; // namespace Archive

// ======================================================================

CellPermissions::PermissionObject::PermissionObject() :
	m_originalPermissionFormat( PF_UNCONVERTED ),
	m_permissionString()
{
}

// ----------------------------------------------------------------------

CellPermissions::PermissionObject::PermissionObject( const std::string& name ) :
	m_originalPermissionFormat( PF_UNKNOWN_STRING ),
	m_permissionString( Unicode::getTrim(name) )
{
	// See if we have a guild name
	if (_strnicmp( name.c_str(), "Guild:", 6 ) == 0 )
	{
		// Try to convert the name to a guild ID
		int guildId = GuildInterface::findGuild( Unicode::getTrim((name).substr(6)) );
		if ( guildId != 0 )
		{
			// Convert the guild ID to a string
			char buffer[32];
			sprintf( buffer, "%d", guildId );

			m_originalPermissionFormat = PF_GUILD_NAME;
			m_permissionString         = buffer;
		}
		else
		{
			// We will leave the format as "unknown" and keep the raw string
		}
	}
	else
	{
		// The name string might be an actual name or a network ID...

		// See if there are any characters in the string
		std::string::size_type idx = name.find_first_not_of( "1234567890" );
		if ( idx == std::string::npos )
		{
			// Everything in the string is a number
			m_originalPermissionFormat = PF_NUMERIC_VALUE;
			m_permissionString         = name;
		}
		else
		{
			// Try to convert the name to a network ID
			const NetworkId playerNetworkId = NameManager::getInstance().getPlayerId( NameManager::normalizeName( name ) );
			if ( playerNetworkId != NetworkId::cms_invalid )
			{
				// We found the network ID for the player
				m_originalPermissionFormat = PF_CHARACTER_NAME;
				m_permissionString         = playerNetworkId.getValueString();
			}
			else
			{
				// We will leave the format as "unknown" and keep the raw string
			}
		}
	}
}

// ----------------------------------------------------------------------

CellPermissions::PermissionObject::PermissionObject( PermissionFormat format, const std::string& name ) :
	m_originalPermissionFormat( format ),
	m_permissionString( name )
{
}

// ----------------------------------------------------------------------

bool CellPermissions::PermissionObject::operator<(PermissionObject const &rhs) const
{
	return (m_permissionString < rhs.m_permissionString);
}

// ----------------------------------------------------------------------

std::string CellPermissions::PermissionObject::getName() const
{
	std::string nameString;

	// Depending on the original format, we may need to convert the permission string
	switch ( m_originalPermissionFormat )
	{
		case PF_CHARACTER_NAME:
			{
				const NetworkId playerNetworkId = NetworkId( m_permissionString );

				// Try to convert the network ID to an actual player name
				// NOTE: We grab the full name so that capitalization is correct
				const std::string fullName = NameManager::getInstance().getPlayerFullName( playerNetworkId );

				// Parse the first name from the full name
				size_t curpos = 0;
				std::string firstName;
				if ( Unicode::getFirstToken( fullName, curpos, curpos, firstName ) )
				{
					nameString = firstName;
				}
			}
			break;

		case PF_GUILD_NAME:
			{
				const int guildId = atoi( m_permissionString.c_str() );

				// Try to convert the guildId to an actual guild name
				if ( guildId != 0 )
				{
					// Prefix with "Guild: " for legacy reasons
					nameString = "Guild:" + GuildInterface::getGuildAbbrev( guildId );
				}
			}
			break;

		case PF_NUMERIC_VALUE:
		case PF_UNKNOWN_STRING:
		default:
			// Just return the stored string for these cases
			nameString = m_permissionString;
			break;
	}

	return nameString;
}

// ----------------------------------------------------------------------

bool CellPermissions::PermissionObject::hasBeenConverted() const
{
	return (m_originalPermissionFormat != PF_UNCONVERTED);
}

// ======================================================================

CellPermissions::UpdateObserver::UpdateObserver(CellObject *cell, Archive::AutoDeltaObserverOp) :
	m_profilerBlock("CellPermissions::UpdateObserver - cell"),
	m_building(0),
	m_permissions()
{
	if (cell)
	{
		BuildingObject *building = cell->getOwnerBuilding();
		if (building)
			init(building);
	}
}

// ----------------------------------------------------------------------

CellPermissions::UpdateObserver::UpdateObserver(TangibleObject *tangible, Archive::AutoDeltaObserverOp) :
	m_profilerBlock("CellPermissions::UpdateObserver - building"),
	m_building(0),
	m_permissions()
{
	if (tangible)
	{
		BuildingObject * const building = tangible->asBuildingObject();
		if (building)
			init(building);
	}
}

// ----------------------------------------------------------------------

CellPermissions::UpdateObserver::~UpdateObserver()
{
	if (m_building)
	{
		s_activeUpdaters.erase(m_building);
		// send permission updates for any changes to any cells of this building,
		// and collect list of creatures we need to expel due to these changes.
		unsigned int permPos = 0;
		std::vector<CreatureObject*> expel;
		handleCellPermissionsUpdateIfNeeded(*m_building, permPos, expel);
		// expel any creatures from the building that no longer belong.
		for (std::vector<CreatureObject*>::const_iterator i = expel.begin(); i != expel.end(); ++i)
			m_building->expelObject(**i);
	}
}

// ----------------------------------------------------------------------

void CellPermissions::UpdateObserver::init(BuildingObject *building)
{
	if (!building || s_activeUpdaters.find(building) != s_activeUpdaters.end())
		return;

	m_building = building;
	s_activeUpdaters.insert(building);
	// get permissions for all observers for all cells of this building
	getCellPermissions(*building);
}

// ----------------------------------------------------------------------

void CellPermissions::UpdateObserver::getCellPermissions(ServerObject &obj)
{
	Container * const container = ContainerInterface::getContainer(obj);
	if (container)
	{
		for (ContainerIterator i = container->begin(); i != container->end(); ++i)
		{
			ServerObject * const content = safe_cast<ServerObject*>((*i).getObject());
			if (content)
			{
				CellObject * const cell = content->asCellObject();
				if (cell)
				{
					std::set<Client *> const &observers = m_building->getObservers();
					for (std::set<Client *>::const_iterator j = observers.begin(); j != observers.end(); ++j)
					{
						ServerObject * const so = (*j)->getCharacterObject();
						if (so)
						{
							CreatureObject * const creature = so->asCreatureObject();
							if (creature)
								m_permissions.push_back(cell->isAllowed(*creature));
						}
					}
				}
				getCellPermissions(*content);
			}
		}
	}
}

// ----------------------------------------------------------------------

void CellPermissions::UpdateObserver::handleCellPermissionsUpdateIfNeeded(ServerObject &obj, unsigned int &permPos, std::vector<CreatureObject*> &expel)
{
	Container *container = ContainerInterface::getContainer(obj);
	if (container)
	{
		for (ContainerIterator i = container->begin(); i != container->end(); ++i)
		{
			ServerObject * const content = safe_cast<ServerObject *>((*i).getObject());
			if (content)
			{
				CellObject * const cell = content->asCellObject();
				if (cell)
				{
					std::set<Client *> const &observers = m_building->getObservers();
					for (std::set<Client *>::const_iterator j = observers.begin(); j != observers.end(); ++j)
					{
						ServerObject * const so = (*j)->getCharacterObject();
						if (so)
						{
							CreatureObject * const creature = so->asCreatureObject();
							if (creature)
							{
								bool const allowed = cell->isAllowed(*creature);
								if (allowed != m_permissions[permPos])
								{
									LOG("CellPermUpdate", ("sending UpdateCellPermission for %s to client %s (%d)",
										cell->getNetworkId().getValueString().c_str(),
										(*j)->getCharacterObjectId().getValueString().c_str(),
										allowed ? 1 : 0));
									UpdateCellPermissionMessage const message(cell->getNetworkId(), allowed);
									(*j)->send(message, true);
								}
								if (!allowed && creature->getAttachedTo() == cell)
									expel.push_back(creature);
								++permPos;
							}
						}
					}
				}
				handleCellPermissionsUpdateIfNeeded(*content, permPos, expel);
			}
		}
	}
}

// ======================================================================

CellPermissions::ViewerChangeObserver::ViewerChangeObserver(CreatureObject *creature) :
	m_profilerBlock("CellPermissions::ViewerChangeObserver"),
	m_cells(),
	m_permissions(),
	m_creature(creature)
{
	if (creature)
	{
		Client * const client = creature->getClient();
		if (client)
		{
			Client::ObservingList const &observing = client->getObserving();
			for (Client::ObservingList::const_iterator i = observing.begin(); i != observing.end(); ++i)
			{
				CellObject * const cell = (*i)->asCellObject();
				if (cell)
				{
					m_cells.push_back(cell);
					m_permissions.push_back(cell->isAllowed(*creature));
				}
			}
		}
	}
}

// ----------------------------------------------------------------------

CellPermissions::ViewerChangeObserver::~ViewerChangeObserver()
{
	if (!m_creature)
		return;

	// send any needed permission updates
	Client * const client = m_creature->getClient();
	if (client)
	{
		for (unsigned int i = 0; i < m_cells.size(); ++i)
		{
			bool const allowed = m_cells[i]->isAllowed(*m_creature);
			if (allowed != m_permissions[i])
			{
				UpdateCellPermissionMessage const message(m_cells[i]->getNetworkId(), allowed);
				client->send(message, true);
			}
		}
	}

	// expel from the current building if they're not allowed in anymore
	CellObject * const cell = ContainerInterface::getContainingCellObject(*m_creature);
	if (cell && !cell->isAllowed(*m_creature))
	{
		BuildingObject * const building = cell->getOwnerBuilding();
		if (building)
			building->expelObject(*m_creature);
	}
}

// ======================================================================

bool CellPermissions::isOnList(PermissionList const &permList, std::string const &name) // static
{
	for (PermissionList::const_iterator i = permList.begin(); i != permList.end(); ++i)
		if (!_stricmp((*i).getName().c_str(), name.c_str()))
			return true;

	return false;
}

// ----------------------------------------------------------------------

bool CellPermissions::isOnList(PermissionList const &permList, CreatureObject const &who) // static
{
	const int guildId = who.getGuildId();

	// if either their name or their guild is on the list, consider them on the list
	for (PermissionList::const_iterator i = permList.begin(); i != permList.end(); ++i)
	{
		const std::string& name = (*i).getName();

		// objectId
		if (name == who.getNetworkId().getValueString())
			return true;

		// first name
		if (!_stricmp(name.c_str(), Unicode::wideToNarrow(who.getAssignedObjectFirstName()).c_str()))
			return true;

		// guilds
		if (guildId && !_strnicmp(name.c_str(), "Guild:", 6))
		{
			std::string checkStr(Unicode::getTrim((name).substr(6)));
			if (   !_stricmp(checkStr.c_str(), GuildInterface::getGuildAbbrev(guildId).c_str())
			    || !_stricmp(checkStr.c_str(), GuildInterface::getGuildName(guildId).c_str()))
				return true;
		}
	}
	return false;
}

// ======================================================================

