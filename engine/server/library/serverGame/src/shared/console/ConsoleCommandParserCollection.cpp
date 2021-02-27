// ======================================================================
//
// ConsoleCommandParserCollection.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ConsoleCommandParserCollection.h"

#include "UnicodeUtils.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/MessageToQueue.h"
#include "serverGame/PlanetObject.h"
#include "serverGame/PlayerCreatureController.h"
#include "serverGame/PlayerObject.h"
#include "serverGame/ServerObject.h"
#include "serverGame/ServerUniverse.h"
#include "serverGame/ServerWorld.h"
#include "sharedFoundation/CalendarTime.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedFoundation/NetworkId.h"

// ======================================================================
 
static const CommandParser::CmdInfo cmds[] =
{
	{"adjustSlotValue",                 3, "<oid> <slot name> <adjustment>",      "Modify the value of a collection slot by the specified adjustment value"},
	{"grantAllSlotsForCollection",      2, "<oid> <collection name>",             "Mark all the collection slots in the specified collection as completed"},
	{"grantAllSlotsForPage",            2, "<oid> <page name>",                   "Mark all the collection slots in the specified page as completed"},
	{"grantAllSlotsForBook",            2, "<oid> <book name>",                   "Mark all the collection slots in the specified book as completed"},
	{"revokeAllSlotsForCollection",     2, "<oid> <collection name>",             "Mark all the collection slots in the specified collection as not completed"},
	{"revokeAllSlotsForPage",           2, "<oid> <page name>",                   "Mark all the collection slots in the specified page as not completed"},
	{"revokeAllSlotsForBook",           2, "<oid> <book name>",                   "Mark all the collection slots in the specified book as not completed"},
	{"displayCollections",              1, "<oid>",                               "Display collections for specified character"},
	{"displayServerFirst",              0, "",                                    "Display collections \"server first\" information"},
	{"addServerFirst",                  2, "<oid> <collection name>",             "Make the specified character \"server first\" for the collection"},
	{"setServerFirstTime",              7, "<collection name> <yyyy> <mm> <dd> <h> <m> <s>", "Set the \"server first\" completion time for the collection"},
	{"removeServerFirst",               1, "<collection name>",                   "Remove \"server first\" for the collection"},
	{"",                                0, "", ""} // this must be last
};

//-----------------------------------------------------------------

ConsoleCommandParserCollection::ConsoleCommandParserCollection (void) :
CommandParser ("collection", 0, "...", "Collection related commands.", 0)
{
	createDelegateCommands (cmds);
}

//-----------------------------------------------------------------

bool ConsoleCommandParserCollection::performParsing (const NetworkId & userId, const StringVector_t & argv, const String_t & originalCommand, String_t & result, const CommandParser * node)
{
	NOT_NULL (node);
	UNREF (userId);

	UNREF(originalCommand);

    CreatureObject * const playerObject = dynamic_cast<CreatureObject *>(ServerWorld::findObjectByNetworkId(userId));
    if (!playerObject)
    {
        WARNING_STRICT_FATAL(true, ("Console command executed on invalid player object %s", userId.getValueString().c_str()));
        return false;
    }

    if (!playerObject->getClient()->isGod()) {
        return false;
    }

	// ----------------------------------------------------------------------

	if (isCommand (argv [0], "adjustSlotValue"))
	{
		NetworkId oid(Unicode::wideToNarrow(argv[1]));
		ServerObject * o = dynamic_cast<ServerObject *>(ServerWorld::findObjectByNetworkId(oid));
		if (o == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}

		CreatureObject * c = o->asCreatureObject();
		if (c == nullptr)
		{
			result += Unicode::narrowToWide("specified object is not a creature object\n");
			return true;
		}

		PlayerObject * p = PlayerCreatureController::getPlayerObject(c);
		if (p == nullptr)
		{
			result += Unicode::narrowToWide("specified object is not a character object\n");
			return true;
		}

		CollectionsDataTable::CollectionInfoSlot const * slot = CollectionsDataTable::getSlotByName(Unicode::wideToNarrow(argv[2]));
		if (!slot)
		{
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("%s is not a valid collection slot name\n", Unicode::wideToNarrow(argv[2]).c_str()));
			return true;
		}

		// quickie way to convert to an int64
		NetworkId const adjustment(Unicode::wideToNarrow(argv[3]));

		// adjustment value of 0 means leave it alone
		if (adjustment.getValue() == 0)
		{
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("adjustment value is 0, so will ***NOT*** adjust collection slot %s/%s/%s/%s for character object %s (%s)\n", slot->collection.page.book.name.c_str(), slot->collection.page.name.c_str(), slot->collection.name.c_str(), Unicode::wideToNarrow(argv[2]).c_str(), oid.getValueString().c_str(), Unicode::wideToNarrow(o->getAssignedObjectName()).c_str()));
			return true;
		}

		// bit-type slot
		if (!slot->counterTypeSlot)
		{
			// set bit-type slot
			if (adjustment.getValue() > 0)
			{
				if (p->hasCompletedCollectionSlot(*slot))
				{
					result += Unicode::narrowToWide(FormattedString<512>().sprintf("character object %s (%s) already has collection slot %s/%s/%s/%s\n", oid.getValueString().c_str(), Unicode::wideToNarrow(o->getAssignedObjectName()).c_str(), slot->collection.page.book.name.c_str(), slot->collection.page.name.c_str(), slot->collection.name.c_str(), Unicode::wideToNarrow(argv[2]).c_str()));
					return true;
				}

				std::vector<CollectionsDataTable::CollectionInfoSlot const *> missingPrereqs;
				if (!p->hasCompletedCollectionSlotPrereq(Unicode::wideToNarrow(argv[2]), &missingPrereqs))
				{
					result += Unicode::narrowToWide(FormattedString<512>().sprintf("cannot set collection slot %s/%s/%s/%s for character object %s (%s) because the following prerequisite collection slot(s) are not yet completed:\n", slot->collection.page.book.name.c_str(), slot->collection.page.name.c_str(), slot->collection.name.c_str(), Unicode::wideToNarrow(argv[2]).c_str(), oid.getValueString().c_str(), Unicode::wideToNarrow(o->getAssignedObjectName()).c_str()));

					for (std::vector<CollectionsDataTable::CollectionInfoSlot const *>::const_iterator iter = missingPrereqs.begin(); iter != missingPrereqs.end(); ++iter)
						result += Unicode::narrowToWide(FormattedString<512>().sprintf("%s/%s/%s/%s\n", (*iter)->collection.page.book.name.c_str(), (*iter)->collection.page.name.c_str(), (*iter)->collection.name.c_str(), (*iter)->name.c_str()));

					return true;
				}

				result += Unicode::narrowToWide(FormattedString<512>().sprintf("setting collection slot %s/%s/%s/%s for character object %s (%s)\n", slot->collection.page.book.name.c_str(), slot->collection.page.name.c_str(), slot->collection.name.c_str(), Unicode::wideToNarrow(argv[2]).c_str(), oid.getValueString().c_str(), Unicode::wideToNarrow(o->getAssignedObjectName()).c_str()));
				p->modifyCollectionSlotValue(Unicode::wideToNarrow(argv[2]), 1);
			}
			// clear bit-type slot
			else if (adjustment.getValue() < 0)
			{
				if (!p->hasCompletedCollectionSlot(*slot))
				{
					result += Unicode::narrowToWide(FormattedString<512>().sprintf("character object %s (%s) doesn't have collection slot %s/%s/%s/%s\n", oid.getValueString().c_str(), Unicode::wideToNarrow(o->getAssignedObjectName()).c_str(), slot->collection.page.book.name.c_str(), slot->collection.page.name.c_str(), slot->collection.name.c_str(), Unicode::wideToNarrow(argv[2]).c_str()));
					return true;
				}

				result += Unicode::narrowToWide(FormattedString<512>().sprintf("removing collection slot %s/%s/%s/%s for character object %s (%s)\n", slot->collection.page.book.name.c_str(), slot->collection.page.name.c_str(), slot->collection.name.c_str(), Unicode::wideToNarrow(argv[2]).c_str(), oid.getValueString().c_str(), Unicode::wideToNarrow(o->getAssignedObjectName()).c_str()));
				p->modifyCollectionSlotValue(Unicode::wideToNarrow(argv[2]), -1);
			}
		}
		// counter-type slot
		else
		{
			if (adjustment.getValue() > 0)
			{
				std::vector<CollectionsDataTable::CollectionInfoSlot const *> missingPrereqs;
				if (!p->hasCompletedCollectionSlotPrereq(Unicode::wideToNarrow(argv[2]), &missingPrereqs))
				{
					result += Unicode::narrowToWide(FormattedString<512>().sprintf("cannot modify collection slot %s/%s/%s/%s for character object %s (%s) because the following prerequisite collection slot(s) are not yet completed:\n", slot->collection.page.book.name.c_str(), slot->collection.page.name.c_str(), slot->collection.name.c_str(), Unicode::wideToNarrow(argv[2]).c_str(), oid.getValueString().c_str(), Unicode::wideToNarrow(o->getAssignedObjectName()).c_str()));

					for (std::vector<CollectionsDataTable::CollectionInfoSlot const *>::const_iterator iter = missingPrereqs.begin(); iter != missingPrereqs.end(); ++iter)
						result += Unicode::narrowToWide(FormattedString<512>().sprintf("%s/%s/%s/%s\n", (*iter)->collection.page.book.name.c_str(), (*iter)->collection.page.name.c_str(), (*iter)->collection.name.c_str(), (*iter)->name.c_str()));

					return true;
				}
			}

			unsigned long currentValue;
			IGNORE_RETURN(p->getCollectionSlotValue(*slot, currentValue));

			result += Unicode::narrowToWide(FormattedString<512>().sprintf("modifying collection slot %s/%s/%s/%s value of %lu by %s for character object %s (%s)\n", slot->collection.page.book.name.c_str(), slot->collection.page.name.c_str(), slot->collection.name.c_str(), Unicode::wideToNarrow(argv[2]).c_str(), currentValue, adjustment.getValueString().c_str(), oid.getValueString().c_str(), Unicode::wideToNarrow(o->getAssignedObjectName()).c_str()));
			p->modifyCollectionSlotValue(Unicode::wideToNarrow(argv[2]), adjustment.getValue());
		}

		return true;
	}

	// ----------------------------------------------------------------------

	else if (isCommand (argv [0], "grantAllSlotsForCollection"))
	{
		NetworkId oid(Unicode::wideToNarrow(argv[1]));
		ServerObject * o = dynamic_cast<ServerObject *>(ServerWorld::findObjectByNetworkId(oid));
		if (o == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}

		CreatureObject * c = o->asCreatureObject();
		if (c == nullptr)
		{
			result += Unicode::narrowToWide("specified object is not a creature object\n");
			return true;
		}

		PlayerObject * p = PlayerCreatureController::getPlayerObject(c);
		if (p == nullptr)
		{
			result += Unicode::narrowToWide("specified object is not a character object\n");
			return true;
		}

		std::vector<CollectionsDataTable::CollectionInfoSlot const *> const & slots = CollectionsDataTable::getSlotsInCollection(Unicode::wideToNarrow(argv[2]));
		if (slots.empty())
		{
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("%s is not a valid collection name\n", Unicode::wideToNarrow(argv[2]).c_str()));
			return true;
		}

		result += Unicode::narrowToWide(FormattedString<512>().sprintf("granting collection slot(s) in collection %s which character object %s (%s) hasn't already completed\n", Unicode::wideToNarrow(argv[2]).c_str(), oid.getValueString().c_str(), Unicode::wideToNarrow(o->getAssignedObjectName()).c_str()));

		for (std::vector<CollectionsDataTable::CollectionInfoSlot const *>::const_iterator iter = slots.begin(); iter != slots.end(); ++iter)
		{
			if (!p->hasCompletedCollectionSlot(**iter))
			{
				// don't do anything for counter-type slot that can never be completed
				if ((*iter)->counterTypeSlot && ((*iter)->maxSlotValue == 0))
				{
					result += Unicode::narrowToWide(FormattedString<512>().sprintf("ignoring collection slot %s/%s/%s/%s because it can never be completed\n", (*iter)->collection.page.book.name.c_str(), (*iter)->collection.page.name.c_str(), (*iter)->collection.name.c_str(), (*iter)->name.c_str()));
					continue;
				}

				// don't do anything for slot that haven't met prerequisite
				if (!p->hasCompletedCollectionSlotPrereq(**iter))
				{
					result += Unicode::narrowToWide(FormattedString<512>().sprintf("ignoring collection slot %s/%s/%s/%s because its prerequisite collection slot(s) are not yet completed\n", (*iter)->collection.page.book.name.c_str(), (*iter)->collection.page.name.c_str(), (*iter)->collection.name.c_str(), (*iter)->name.c_str()));
					continue;
				}

				if (!(*iter)->counterTypeSlot)
				{
					// grant bit type slot
					result += Unicode::narrowToWide(FormattedString<512>().sprintf("setting collection slot %s/%s/%s/%s\n", (*iter)->collection.page.book.name.c_str(), (*iter)->collection.page.name.c_str(), (*iter)->collection.name.c_str(), (*iter)->name.c_str()));
					p->modifyCollectionSlotValue((*iter)->name, 1);
				}
				else
				{
					// grant counter-type slot
					unsigned long currentValue;
					IGNORE_RETURN(p->getCollectionSlotValue(**iter, currentValue));

					// quickie way to convert to an int64
					NetworkId const adjustment(static_cast<int64>((*iter)->maxSlotValue) - static_cast<int64>(currentValue));

					result += Unicode::narrowToWide(FormattedString<512>().sprintf("modifying collection slot %s/%s/%s/%s value of %lu by %s\n", (*iter)->collection.page.book.name.c_str(), (*iter)->collection.page.name.c_str(), (*iter)->collection.name.c_str(), (*iter)->name.c_str(), currentValue, adjustment.getValueString().c_str()));
					p->modifyCollectionSlotValue((*iter)->name, adjustment.getValue());
				}
			}
		}

		return true;
	}

	// ----------------------------------------------------------------------

	else if (isCommand (argv [0], "grantAllSlotsForPage"))
	{
		NetworkId oid(Unicode::wideToNarrow(argv[1]));
		ServerObject * o = dynamic_cast<ServerObject *>(ServerWorld::findObjectByNetworkId(oid));
		if (o == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}

		CreatureObject * c = o->asCreatureObject();
		if (c == nullptr)
		{
			result += Unicode::narrowToWide("specified object is not a creature object\n");
			return true;
		}

		PlayerObject * p = PlayerCreatureController::getPlayerObject(c);
		if (p == nullptr)
		{
			result += Unicode::narrowToWide("specified object is not a character object\n");
			return true;
		}

		std::vector<CollectionsDataTable::CollectionInfoSlot const *> const & slots = CollectionsDataTable::getSlotsInPage(Unicode::wideToNarrow(argv[2]));
		if (slots.empty())
		{
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("%s is not a valid page name\n", Unicode::wideToNarrow(argv[2]).c_str()));
			return true;
		}

		result += Unicode::narrowToWide(FormattedString<512>().sprintf("granting collection slot(s) in page %s which character object %s (%s) hasn't already completed\n", Unicode::wideToNarrow(argv[2]).c_str(), oid.getValueString().c_str(), Unicode::wideToNarrow(o->getAssignedObjectName()).c_str()));

		for (std::vector<CollectionsDataTable::CollectionInfoSlot const *>::const_iterator iter = slots.begin(); iter != slots.end(); ++iter)
		{
			if (!p->hasCompletedCollectionSlot(**iter))
			{
				// don't do anything for counter-type slot that can never be completed
				if ((*iter)->counterTypeSlot && ((*iter)->maxSlotValue == 0))
				{
					result += Unicode::narrowToWide(FormattedString<512>().sprintf("ignoring collection slot %s/%s/%s/%s because it can never be completed\n", (*iter)->collection.page.book.name.c_str(), (*iter)->collection.page.name.c_str(), (*iter)->collection.name.c_str(), (*iter)->name.c_str()));
					continue;
				}

				// don't do anything for slot that haven't met prerequisite
				if (!p->hasCompletedCollectionSlotPrereq(**iter))
				{
					result += Unicode::narrowToWide(FormattedString<512>().sprintf("ignoring collection slot %s/%s/%s/%s because its prerequisite collection slot(s) are not yet completed\n", (*iter)->collection.page.book.name.c_str(), (*iter)->collection.page.name.c_str(), (*iter)->collection.name.c_str(), (*iter)->name.c_str()));
					continue;
				}

				if (!(*iter)->counterTypeSlot)
				{
					// grant bit type slot
					result += Unicode::narrowToWide(FormattedString<512>().sprintf("setting collection slot %s/%s/%s/%s\n", (*iter)->collection.page.book.name.c_str(), (*iter)->collection.page.name.c_str(), (*iter)->collection.name.c_str(), (*iter)->name.c_str()));
					p->modifyCollectionSlotValue((*iter)->name, 1);
				}
				else
				{
					// grant counter-type slot
					unsigned long currentValue;
					IGNORE_RETURN(p->getCollectionSlotValue(**iter, currentValue));

					// quickie way to convert to an int64
					NetworkId const adjustment(static_cast<int64>((*iter)->maxSlotValue) - static_cast<int64>(currentValue));

					result += Unicode::narrowToWide(FormattedString<512>().sprintf("modifying collection slot %s/%s/%s/%s value of %lu by %s\n", (*iter)->collection.page.book.name.c_str(), (*iter)->collection.page.name.c_str(), (*iter)->collection.name.c_str(), (*iter)->name.c_str(), currentValue, adjustment.getValueString().c_str()));
					p->modifyCollectionSlotValue((*iter)->name, adjustment.getValue());
				}
			}
		}

		return true;
	}

	// ----------------------------------------------------------------------

	else if (isCommand (argv [0], "grantAllSlotsForBook"))
	{
		NetworkId oid(Unicode::wideToNarrow(argv[1]));
		ServerObject * o = dynamic_cast<ServerObject *>(ServerWorld::findObjectByNetworkId(oid));
		if (o == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}

		CreatureObject * c = o->asCreatureObject();
		if (c == nullptr)
		{
			result += Unicode::narrowToWide("specified object is not a creature object\n");
			return true;
		}

		PlayerObject * p = PlayerCreatureController::getPlayerObject(c);
		if (p == nullptr)
		{
			result += Unicode::narrowToWide("specified object is not a character object\n");
			return true;
		}

		std::vector<CollectionsDataTable::CollectionInfoSlot const *> const & slots = CollectionsDataTable::getSlotsInBook(Unicode::wideToNarrow(argv[2]));
		if (slots.empty())
		{
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("%s is not a valid book name\n", Unicode::wideToNarrow(argv[2]).c_str()));
			return true;
		}

		result += Unicode::narrowToWide(FormattedString<512>().sprintf("granting collection slot(s) in book %s which character object %s (%s) hasn't already completed\n", Unicode::wideToNarrow(argv[2]).c_str(), oid.getValueString().c_str(), Unicode::wideToNarrow(o->getAssignedObjectName()).c_str()));

		for (std::vector<CollectionsDataTable::CollectionInfoSlot const *>::const_iterator iter = slots.begin(); iter != slots.end(); ++iter)
		{
			if (!p->hasCompletedCollectionSlot(**iter))
			{
				// don't do anything for counter-type slot that can never be completed
				if ((*iter)->counterTypeSlot && ((*iter)->maxSlotValue == 0))
				{
					result += Unicode::narrowToWide(FormattedString<512>().sprintf("ignoring collection slot %s/%s/%s/%s because it can never be completed\n", (*iter)->collection.page.book.name.c_str(), (*iter)->collection.page.name.c_str(), (*iter)->collection.name.c_str(), (*iter)->name.c_str()));
					continue;
				}

				// don't do anything for slot that haven't met prerequisite
				if (!p->hasCompletedCollectionSlotPrereq(**iter))
				{
					result += Unicode::narrowToWide(FormattedString<512>().sprintf("ignoring collection slot %s/%s/%s/%s because its prerequisite collection slot(s) are not yet completed\n", (*iter)->collection.page.book.name.c_str(), (*iter)->collection.page.name.c_str(), (*iter)->collection.name.c_str(), (*iter)->name.c_str()));
					continue;
				}

				if (!(*iter)->counterTypeSlot)
				{
					// grant bit type slot
					result += Unicode::narrowToWide(FormattedString<512>().sprintf("setting collection slot %s/%s/%s/%s\n", (*iter)->collection.page.book.name.c_str(), (*iter)->collection.page.name.c_str(), (*iter)->collection.name.c_str(), (*iter)->name.c_str()));
					p->modifyCollectionSlotValue((*iter)->name, 1);
				}
				else
				{
					// grant counter-type slot
					unsigned long currentValue;
					IGNORE_RETURN(p->getCollectionSlotValue(**iter, currentValue));

					// quickie way to convert to an int64
					NetworkId const adjustment(static_cast<int64>((*iter)->maxSlotValue) - static_cast<int64>(currentValue));

					result += Unicode::narrowToWide(FormattedString<512>().sprintf("modifying collection slot %s/%s/%s/%s value of %lu by %s\n", (*iter)->collection.page.book.name.c_str(), (*iter)->collection.page.name.c_str(), (*iter)->collection.name.c_str(), (*iter)->name.c_str(), currentValue, adjustment.getValueString().c_str()));
					p->modifyCollectionSlotValue((*iter)->name, adjustment.getValue());
				}
			}
		}

		return true;
	}

	// ----------------------------------------------------------------------

	else if (isCommand (argv [0], "revokeAllSlotsForCollection"))
	{
		NetworkId oid(Unicode::wideToNarrow(argv[1]));
		ServerObject * o = dynamic_cast<ServerObject *>(ServerWorld::findObjectByNetworkId(oid));
		if (o == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}

		CreatureObject * c = o->asCreatureObject();
		if (c == nullptr)
		{
			result += Unicode::narrowToWide("specified object is not a creature object\n");
			return true;
		}

		PlayerObject * p = PlayerCreatureController::getPlayerObject(c);
		if (p == nullptr)
		{
			result += Unicode::narrowToWide("specified object is not a character object\n");
			return true;
		}

		std::vector<CollectionsDataTable::CollectionInfoSlot const *> const & slots = CollectionsDataTable::getSlotsInCollection(Unicode::wideToNarrow(argv[2]));
		if (slots.empty())
		{
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("%s is not a valid collection name\n", Unicode::wideToNarrow(argv[2]).c_str()));
			return true;
		}

		result += Unicode::narrowToWide(FormattedString<512>().sprintf("revoking collection slot(s) in collection %s which character object %s (%s) has already completed\n", Unicode::wideToNarrow(argv[2]).c_str(), oid.getValueString().c_str(), Unicode::wideToNarrow(o->getAssignedObjectName()).c_str()));

		for (std::vector<CollectionsDataTable::CollectionInfoSlot const *>::const_iterator iter = slots.begin(); iter != slots.end(); ++iter)
		{
			if (p->hasCompletedCollectionSlot(**iter))
			{
				if (!(*iter)->counterTypeSlot)
				{
					// revoke bit type slot
					result += Unicode::narrowToWide(FormattedString<512>().sprintf("removing collection slot %s/%s/%s/%s\n", (*iter)->collection.page.book.name.c_str(), (*iter)->collection.page.name.c_str(), (*iter)->collection.name.c_str(), (*iter)->name.c_str()));
					p->modifyCollectionSlotValue((*iter)->name, -1);
				}
				else
				{
					// revoke counter-type slot
					unsigned long currentValue;
					IGNORE_RETURN(p->getCollectionSlotValue(**iter, currentValue));

					// quickie way to convert to an int64
					NetworkId const adjustment(-(static_cast<int64>(currentValue)));

					result += Unicode::narrowToWide(FormattedString<512>().sprintf("modifying collection slot %s/%s/%s/%s value of %lu by %s\n", (*iter)->collection.page.book.name.c_str(), (*iter)->collection.page.name.c_str(), (*iter)->collection.name.c_str(), (*iter)->name.c_str(), currentValue, adjustment.getValueString().c_str()));
					p->modifyCollectionSlotValue((*iter)->name, adjustment.getValue());
				}
			}
		}

		return true;
	}

	// ----------------------------------------------------------------------

	else if (isCommand (argv [0], "revokeAllSlotsForPage"))
	{
		NetworkId oid(Unicode::wideToNarrow(argv[1]));
		ServerObject * o = dynamic_cast<ServerObject *>(ServerWorld::findObjectByNetworkId(oid));
		if (o == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}

		CreatureObject * c = o->asCreatureObject();
		if (c == nullptr)
		{
			result += Unicode::narrowToWide("specified object is not a creature object\n");
			return true;
		}

		PlayerObject * p = PlayerCreatureController::getPlayerObject(c);
		if (p == nullptr)
		{
			result += Unicode::narrowToWide("specified object is not a character object\n");
			return true;
		}

		std::vector<CollectionsDataTable::CollectionInfoSlot const *> const & slots = CollectionsDataTable::getSlotsInPage(Unicode::wideToNarrow(argv[2]));
		if (slots.empty())
		{
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("%s is not a valid page name\n", Unicode::wideToNarrow(argv[2]).c_str()));
			return true;
		}

		result += Unicode::narrowToWide(FormattedString<512>().sprintf("revoking collection slot(s) in page %s which character object %s (%s) has already completed\n", Unicode::wideToNarrow(argv[2]).c_str(), oid.getValueString().c_str(), Unicode::wideToNarrow(o->getAssignedObjectName()).c_str()));

		for (std::vector<CollectionsDataTable::CollectionInfoSlot const *>::const_iterator iter = slots.begin(); iter != slots.end(); ++iter)
		{
			if (p->hasCompletedCollectionSlot(**iter))
			{
				if (!(*iter)->counterTypeSlot)
				{
					// revoke bit type slot
					result += Unicode::narrowToWide(FormattedString<512>().sprintf("removing collection slot %s/%s/%s/%s\n", (*iter)->collection.page.book.name.c_str(), (*iter)->collection.page.name.c_str(), (*iter)->collection.name.c_str(), (*iter)->name.c_str()));
					p->modifyCollectionSlotValue((*iter)->name, -1);
				}
				else
				{
					// revoke counter-type slot
					unsigned long currentValue;
					IGNORE_RETURN(p->getCollectionSlotValue(**iter, currentValue));

					// quickie way to convert to an int64
					NetworkId const adjustment(-(static_cast<int64>(currentValue)));

					result += Unicode::narrowToWide(FormattedString<512>().sprintf("modifying collection slot %s/%s/%s/%s value of %lu by %s\n", (*iter)->collection.page.book.name.c_str(), (*iter)->collection.page.name.c_str(), (*iter)->collection.name.c_str(), (*iter)->name.c_str(), currentValue, adjustment.getValueString().c_str()));
					p->modifyCollectionSlotValue((*iter)->name, adjustment.getValue());
				}
			}
		}

		return true;
	}

	// ----------------------------------------------------------------------

	else if (isCommand (argv [0], "revokeAllSlotsForBook"))
	{
		NetworkId oid(Unicode::wideToNarrow(argv[1]));
		ServerObject * o = dynamic_cast<ServerObject *>(ServerWorld::findObjectByNetworkId(oid));
		if (o == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}

		CreatureObject * c = o->asCreatureObject();
		if (c == nullptr)
		{
			result += Unicode::narrowToWide("specified object is not a creature object\n");
			return true;
		}

		PlayerObject * p = PlayerCreatureController::getPlayerObject(c);
		if (p == nullptr)
		{
			result += Unicode::narrowToWide("specified object is not a character object\n");
			return true;
		}

		std::vector<CollectionsDataTable::CollectionInfoSlot const *> const & slots = CollectionsDataTable::getSlotsInBook(Unicode::wideToNarrow(argv[2]));
		if (slots.empty())
		{
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("%s is not a valid book name\n", Unicode::wideToNarrow(argv[2]).c_str()));
			return true;
		}

		result += Unicode::narrowToWide(FormattedString<512>().sprintf("revoking collection slot(s) in book %s which character object %s (%s) has already completed\n", Unicode::wideToNarrow(argv[2]).c_str(), oid.getValueString().c_str(), Unicode::wideToNarrow(o->getAssignedObjectName()).c_str()));

		for (std::vector<CollectionsDataTable::CollectionInfoSlot const *>::const_iterator iter = slots.begin(); iter != slots.end(); ++iter)
		{
			if (p->hasCompletedCollectionSlot(**iter))
			{
				if (!(*iter)->counterTypeSlot)
				{
					// revoke bit type slot
					result += Unicode::narrowToWide(FormattedString<512>().sprintf("removing collection slot %s/%s/%s/%s\n", (*iter)->collection.page.book.name.c_str(), (*iter)->collection.page.name.c_str(), (*iter)->collection.name.c_str(), (*iter)->name.c_str()));
					p->modifyCollectionSlotValue((*iter)->name, -1);
				}
				else
				{
					// revoke counter-type slot
					unsigned long currentValue;
					IGNORE_RETURN(p->getCollectionSlotValue(**iter, currentValue));

					// quickie way to convert to an int64
					NetworkId const adjustment(-(static_cast<int64>(currentValue)));

					result += Unicode::narrowToWide(FormattedString<512>().sprintf("modifying collection slot %s/%s/%s/%s value of %lu by %s\n", (*iter)->collection.page.book.name.c_str(), (*iter)->collection.page.name.c_str(), (*iter)->collection.name.c_str(), (*iter)->name.c_str(), currentValue, adjustment.getValueString().c_str()));
					p->modifyCollectionSlotValue((*iter)->name, adjustment.getValue());
				}
			}
		}

		return true;
	}

	// ----------------------------------------------------------------------

	else if (isCommand (argv [0], "displayCollections"))
	{
		NetworkId oid(Unicode::wideToNarrow(argv[1]));
		ServerObject * o = dynamic_cast<ServerObject *>(ServerWorld::findObjectByNetworkId(oid));
		if (o == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}

		CreatureObject * c = o->asCreatureObject();
		if (c == nullptr)
		{
			result += Unicode::narrowToWide("specified object is not a creature object\n");
			return true;
		}

		PlayerObject * p = PlayerCreatureController::getPlayerObject(c);
		if (p == nullptr)
		{
			result += Unicode::narrowToWide("specified object is not a character object\n");
			return true;
		}

		result += Unicode::narrowToWide(FormattedString<512>().sprintf("Displaying collections for character object %s (%s)\n", oid.getValueString().c_str(), Unicode::wideToNarrow(o->getAssignedObjectName()).c_str()));

		std::string slotInfo;
		std::string collectionInfo;
		std::string pageInfo;

		std::vector<CollectionsDataTable::CollectionInfoBook const *> const & books = CollectionsDataTable::getAllBooks();
		for (std::vector<CollectionsDataTable::CollectionInfoBook const *>::const_iterator iterBook = books.begin(); iterBook != books.end(); ++iterBook)
		{
			std::vector<CollectionsDataTable::CollectionInfoPage const *> const & pages = CollectionsDataTable::getPagesInBook((*iterBook)->name);
			pageInfo.clear();
			for (std::vector<CollectionsDataTable::CollectionInfoPage const *>::const_iterator iterPage = pages.begin(); iterPage != pages.end(); ++iterPage)
			{
				std::vector<CollectionsDataTable::CollectionInfoCollection const *> const & collections = CollectionsDataTable::getCollectionsInPage((*iterPage)->name);
				collectionInfo.clear();
				for (std::vector<CollectionsDataTable::CollectionInfoCollection const *>::const_iterator iterCollection = collections.begin(); iterCollection != collections.end(); ++iterCollection)
				{
					std::vector<CollectionsDataTable::CollectionInfoSlot const *> const & slots = CollectionsDataTable::getSlotsInCollection((*iterCollection)->name);
					slotInfo.clear();
					for (std::vector<CollectionsDataTable::CollectionInfoSlot const *>::const_iterator iterSlot = slots.begin(); iterSlot != slots.end(); ++iterSlot)
					{
						if (!(*iterSlot)->counterTypeSlot)
						{
							if (p->hasCompletedCollectionSlot(**iterSlot))
								slotInfo += FormattedString<512>().sprintf("            %s\n", (*iterSlot)->name.c_str());
						}
						else
						{
							unsigned long value;
							IGNORE_RETURN(p->getCollectionSlotValue(**iterSlot, value));
							if (value == 0)
								continue;

							if ((*iterSlot)->maxSlotValue == 0)
								slotInfo += FormattedString<512>().sprintf("            %s (%lu)\n", (*iterSlot)->name.c_str(), value);
							else
								slotInfo += FormattedString<512>().sprintf("            %s (%lu/%lu)\n", (*iterSlot)->name.c_str(), value, (*iterSlot)->maxSlotValue);
						}
					}

					if (!slotInfo.empty())
					{
						if (p->hasCompletedCollection((*iterCollection)->name))
							collectionInfo += FormattedString<512>().sprintf("        *%s\n", (*iterCollection)->name.c_str());
						else
							collectionInfo += FormattedString<512>().sprintf("        %s\n", (*iterCollection)->name.c_str());

						collectionInfo += slotInfo;
					}
				}

				if (!collectionInfo.empty())
				{
					pageInfo += FormattedString<512>().sprintf("    %s\n", (*iterPage)->name.c_str());
					pageInfo += collectionInfo;
				}
			}

			if (!pageInfo.empty())
			{
				result += Unicode::narrowToWide(FormattedString<512>().sprintf("%s\n", (*iterBook)->name.c_str()));
				result += Unicode::narrowToWide(pageInfo);
			}
		}

		return true;
	}

	// ----------------------------------------------------------------------

	else if (isCommand (argv [0], "displayServerFirst"))
	{
		std::map<std::string, CollectionsDataTable::CollectionInfoCollection const *> const & allServerFirstCollections = CollectionsDataTable::getAllServerFirstCollections();
		std::map<std::string, CollectionsDataTable::CollectionInfoCollection const *>::const_iterator iter;
		int discovered = 0;

		for (iter = allServerFirstCollections.begin(); iter != allServerFirstCollections.end(); ++iter)
		{
			if (iter->second->serverFirstClaimTime > 0)
				++discovered;
		}

		int const undiscovered = allServerFirstCollections.size() - discovered;

		result += Unicode::narrowToWide(FormattedString<512>().sprintf("%d Discovered:\n", discovered));
		for (iter = allServerFirstCollections.begin(); iter != allServerFirstCollections.end(); ++iter)
		{
			if (iter->second->serverFirstClaimTime > 0)
				result += Unicode::narrowToWide(FormattedString<512>().sprintf("%s discovered by (%s, %s) at %ld (%s)\n", iter->second->name.c_str(), iter->second->serverFirstClaimantId.getValueString().c_str(), Unicode::wideToNarrow(iter->second->serverFirstClaimantName).c_str(), iter->second->serverFirstClaimTime, CalendarTime::convertEpochToTimeStringLocal(iter->second->serverFirstClaimTime).c_str()));
		}

		result += Unicode::narrowToWide(FormattedString<512>().sprintf("%d Undiscovered:\n", undiscovered));
		for (iter = allServerFirstCollections.begin(); iter != allServerFirstCollections.end(); ++iter)
		{
			if (iter->second->serverFirstClaimTime <= 0)
				result += Unicode::narrowToWide(FormattedString<512>().sprintf("%s\n", iter->second->name.c_str()));
		}

		return true;
	}

	// ----------------------------------------------------------------------

	else if (isCommand (argv [0], "addServerFirst"))
	{
		NetworkId oid(Unicode::wideToNarrow(argv[1]));
		ServerObject * o = dynamic_cast<ServerObject *>(ServerWorld::findObjectByNetworkId(oid));
		if (o == nullptr)
		{
			result += getErrorMessage(argv[0], ERR_INVALID_OBJECT);
			return true;
		}

		CreatureObject * c = o->asCreatureObject();
		if (c == nullptr)
		{
			result += Unicode::narrowToWide("specified object is not a creature object\n");
			return true;
		}

		PlayerObject * p = PlayerCreatureController::getPlayerObject(c);
		if (p == nullptr)
		{
			result += Unicode::narrowToWide("specified object is not a character object\n");
			return true;
		}

		CollectionsDataTable::CollectionInfoCollection const * const collectionInfo = CollectionsDataTable::getCollectionByName(Unicode::wideToNarrow(argv[2]));
		if (!collectionInfo)
		{
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("%s is not a valid collection name\n", Unicode::wideToNarrow(argv[2]).c_str()));
			return true;
		}

		if (!collectionInfo->trackServerFirst)
		{
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("%s is not a \"server first\" tracked collection\n", Unicode::wideToNarrow(argv[2]).c_str()));
			return true;
		}

		if (collectionInfo->serverFirstClaimTime > 0)
		{
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("%s has already been discovered by (%s, %s) at %ld (%s)\n", Unicode::wideToNarrow(argv[2]).c_str(), collectionInfo->serverFirstClaimantId.getValueString().c_str(), Unicode::wideToNarrow(collectionInfo->serverFirstClaimantName).c_str(), collectionInfo->serverFirstClaimTime, CalendarTime::convertEpochToTimeStringLocal(collectionInfo->serverFirstClaimTime).c_str()));
			return true;
		}

		PlanetObject * const planetObject = ServerUniverse::getInstance().getTatooinePlanet();
		if (!planetObject)
		{
			result += Unicode::narrowToWide("could not locate the tatooine PlanetObject\n");
			return true;
		}

		result += Unicode::narrowToWide(FormattedString<512>().sprintf("requesting character object %s (%s) be made \"server first\" for collection %s\n", oid.getValueString().c_str(), Unicode::wideToNarrow(o->getAssignedObjectName()).c_str(), Unicode::wideToNarrow(argv[2]).c_str()));
		
		planetObject->setCollectionServerFirst(*collectionInfo, oid, o->getAssignedObjectName(), p->getAccountDescription());
		
		return true;
	}

	// ----------------------------------------------------------------------

	else if (isCommand (argv [0], "setServerFirstTime"))
	{
		time_t const rawtime = ::time(nullptr);
		struct tm * timeinfo = ::localtime(&rawtime);
		timeinfo->tm_year = atoi(Unicode::wideToNarrow(argv[2]).c_str()) - 1900;
		timeinfo->tm_mon = atoi(Unicode::wideToNarrow(argv[3]).c_str()) - 1;
		timeinfo->tm_mday = atoi(Unicode::wideToNarrow(argv[4]).c_str());
		timeinfo->tm_hour = atoi(Unicode::wideToNarrow(argv[5]).c_str());
		timeinfo->tm_min = atoi(Unicode::wideToNarrow(argv[6]).c_str());
		timeinfo->tm_sec = atoi(Unicode::wideToNarrow(argv[7]).c_str());
		time_t const specifiedTime = ::mktime(timeinfo);

		if (specifiedTime <= 0)
		{
			result += Unicode::narrowToWide("specified time is invalid\n");
			return true;
		}

		CollectionsDataTable::CollectionInfoCollection const * const collectionInfo = CollectionsDataTable::getCollectionByName(Unicode::wideToNarrow(argv[1]));
		if (!collectionInfo)
		{
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("%s is not a valid collection name\n", Unicode::wideToNarrow(argv[1]).c_str()));
			return true;
		}

		if (!collectionInfo->trackServerFirst)
		{
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("%s is not a \"server first\" tracked collection\n", collectionInfo->name.c_str()));
			return true;
		}

		if (collectionInfo->serverFirstClaimTime <= 0)
		{
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("collection %s has not yet been discovered\n", collectionInfo->name.c_str()));
			return true;
		}

		PlanetObject * const planetObject = ServerUniverse::getInstance().getTatooinePlanet();
		if (!planetObject)
		{
			result += Unicode::narrowToWide("could not locate the tatooine PlanetObject\n");
			return true;
		}

		result += Unicode::narrowToWide(FormattedString<512>().sprintf("setting \"server first\" time for collection %s:\n", collectionInfo->name.c_str()));
		result += Unicode::narrowToWide(FormattedString<512>().sprintf("***FROM*** %ld (%s, %s)\n", static_cast<long>(collectionInfo->serverFirstClaimTime), CalendarTime::convertEpochToTimeStringLocal(collectionInfo->serverFirstClaimTime).c_str(), CalendarTime::convertEpochToTimeStringGMT(collectionInfo->serverFirstClaimTime).c_str()));
		result += Unicode::narrowToWide(FormattedString<512>().sprintf("****TO**** %ld (%s, %s)\n", static_cast<long>(specifiedTime), CalendarTime::convertEpochToTimeStringLocal(specifiedTime).c_str(), CalendarTime::convertEpochToTimeStringGMT(specifiedTime).c_str()));

		char buffer[1024];
		snprintf(buffer, sizeof(buffer)-1, "%s|%ld", collectionInfo->name.c_str(), static_cast<long>(specifiedTime));
		buffer[sizeof(buffer)-1] = '\0';

		MessageToQueue::getInstance().sendMessageToC(planetObject->getNetworkId(),
			"C++SetCollectionServerFirstTime",
			buffer,
			0,
			false);

		return true;
	}

	// ----------------------------------------------------------------------

	else if (isCommand (argv [0], "removeServerFirst"))
	{
		CollectionsDataTable::CollectionInfoCollection const * const collectionInfo = CollectionsDataTable::getCollectionByName(Unicode::wideToNarrow(argv[1]));
		if (!collectionInfo)
		{
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("%s is not a valid collection name\n", Unicode::wideToNarrow(argv[1]).c_str()));
			return true;
		}

		if (!collectionInfo->trackServerFirst)
		{
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("%s is not a \"server first\" tracked collection\n", Unicode::wideToNarrow(argv[1]).c_str()));
			return true;
		}

		if (collectionInfo->serverFirstClaimTime <= 0)
		{
			result += Unicode::narrowToWide(FormattedString<512>().sprintf("%s has not yet been discovered\n", Unicode::wideToNarrow(argv[1]).c_str()));
			return true;
		}

		PlanetObject * const planetObject = ServerUniverse::getInstance().getTatooinePlanet();
		if (!planetObject)
		{
			result += Unicode::narrowToWide("could not locate the tatooine PlanetObject\n");
			return true;
		}

		result += Unicode::narrowToWide(FormattedString<512>().sprintf("requesting clearing of \"server first\" for collection %s [discovered by (%s, %s) at %ld (%s)]\n", Unicode::wideToNarrow(argv[1]).c_str(), collectionInfo->serverFirstClaimantId.getValueString().c_str(), Unicode::wideToNarrow(collectionInfo->serverFirstClaimantName).c_str(), collectionInfo->serverFirstClaimTime, CalendarTime::convertEpochToTimeStringLocal(collectionInfo->serverFirstClaimTime).c_str()));

		planetObject->removeCollectionServerFirst(*collectionInfo);

		return true;
	}

	// ----------------------------------------------------------------------

	else
	{
		result += getErrorMessage(argv[0], ERR_NO_HANDLER);
	}
	return true;

}	// ConsoleCommandParserCollection::performParsing

// ======================================================================
