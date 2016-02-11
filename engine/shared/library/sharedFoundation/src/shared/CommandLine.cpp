// ======================================================================
//
// CommandLine.h
// Todd Fiala
//
// copyright 1999, Bootprint Entertainment
//
// ======================================================================

#include "sharedFoundation/FirstSharedFoundation.h"
#include "sharedFoundation/CommandLine.h"

#include "sharedFoundation/ExitChain.h"

#include <cstdlib>

// ======================================================================

bool                      CommandLine::installed;
bool                      CommandLine::wasParsed;
char                      CommandLine::buffer[STATIC_BUFFER_SIZE];
int                       CommandLine::bufferSize;
CommandLine::Collection  *CommandLine::treeRoot;
CommandLine::OptionTable *CommandLine::optionTable;
int                       CommandLine::absorbedStringCount;

// ======================================================================

CommandLine::Option::Option(
	char            newShortName,
	const char     *newLongName,
	ArgumentPolicy  newArgumentPolicy
	) :
	shortName(newShortName),
	longName(0),
	argumentPolicy(newArgumentPolicy)
{
	DEBUG_FATAL(shortName && !(isalnum(shortName) || (shortName == OP_SNAME_UNTAGGED)), ("specified shortname must satisfy isalnum() test or be special untagged character"));

	if (newLongName)
	{
		const size_t length = strlen(newLongName);
		if (length)
		{
			longName = new char[length+1];
			memcpy(longName, newLongName, length+1);
		}
	}
}

// ----------------------------------------------------------------------

CommandLine::Option::~Option(void)
{
	delete [] longName;
	longName = 0;
}

// ----------------------------------------------------------------------

bool CommandLine::Option::isOptionNext(
	const OptionSpec *optionSpec,
	int               optionSpecCount
	)
{
	DEBUG_FATAL(!optionSpec, ("nullptr optionSpec arg"));
	if (!optionSpecCount)
		return false;
	else
		return (optionSpec->optionType == OST_Option);
}

// ----------------------------------------------------------------------

CommandLine::Option *CommandLine::Option::createOption(
	const OptionSpec **optionSpec,
	int               *optionCount
	)
{
	DEBUG_FATAL(!optionSpec, ("nullptr optionSpec arg"));
	DEBUG_FATAL(!optionCount, ("nullptr optionCount arg"));
	DEBUG_FATAL(!isOptionNext(*optionSpec, *optionCount), ("attempted to create option with non-option data"));

	const char      newShortName = (*optionSpec)->char1;
	const char     *newLongName = (*optionSpec)->charp1;
	ArgumentPolicy  newArgumentPolicy = AP_None;

	switch ((*optionSpec)->int1)
	{
	case OP_ARG_NONE:
		newArgumentPolicy = AP_None;
		break;
	case OP_ARG_REQUIRED:
		newArgumentPolicy = AP_Required;
		break;
	case OP_ARG_OPTIONAL:
		newArgumentPolicy = AP_Optional;
		break;
	default:
		DEBUG_FATAL(true, ("illegal argument policy(%d) specified in option spec", (*optionSpec)->int1));
	}

	// gobble up OST_Option
	++(*optionSpec);
	--(*optionCount);

	// add the option to the option table
	// first check if its already in the table
	OptionTable::Record *record = 0;
	if (newShortName)
		record = optionTable->findOptionRecord(newShortName);
	if (!record && newLongName)
		record = optionTable->findOptionRecord(newLongName);
	if (!record)
	{
		// option not already recorded as a valid option for the option table, add it now
		static_cast<void>(optionTable->createOptionRecord(newShortName, newLongName, newArgumentPolicy));
	}

	// create the new option for the search tree
	return new Option(newShortName, newLongName, newArgumentPolicy);
}

// ----------------------------------------------------------------------

CommandLine::MatchCode CommandLine::Option::match(void)
{
	// we match if we find an unmatched short or long command line option
	// matching this short or long name.  If the arg specs don't match, we've
	// got an argument matching error.

	DEBUG_FATAL(!optionTable, ("internal error: nullptr option table"));

	OptionTable::Record *record = 0;

	// get the option info record for this option
	if (shortName)
	{
		record = optionTable->findOptionRecord(shortName);
		DEBUG_FATAL(!record, ("failed to find option info record for option -%c", shortName));
	}
	else if (longName)
	{
		record = optionTable->findOptionRecord(longName);
		DEBUG_FATAL(!record, ("failed to find option info record for option --%s", longName));
	}
	else
		DEBUG_FATAL(true, ("corrupted option?  both short and long name are nullptr"));


	// attempt to match against this option against commandline-specified options
	const int occurrenceIndex = record->getMatchCount();
	if (occurrenceIndex >= record->getWriteCount())
	{
		// this option has already been fully matched, so we don't match
		return MC_NO_MATCH;
	}
	else
	{
		// we matched options
		record->addMatch();

		// ensure we match argument policy
		const char *arg = record->getString(occurrenceIndex);
		bool haveArgument = (arg && strlen(arg));

		// fall out if wrong arg policy
		switch(argumentPolicy)
		{
		case AP_None:
			if (haveArgument)
				return MC_ARG_NOT_EXPECTED;
			break;
		case AP_Required:
			if (!haveArgument)
				return MC_ARG_MISSING;
			break;
		case AP_Optional:
			// always okay
			break;
		default:
			DEBUG_FATAL(true, ("internal error: unknown argument policy(%d)", argumentPolicy));
		}

		// we're okay
		return MC_MATCH;
	}
}

// ======================================================================

CommandLine::Collection::~Collection(void)
{
}

// ----------------------------------------------------------------------

bool CommandLine::Collection::isCollectionNext(
	const OptionSpec *optionSpec,
	int               optionSpecCount
	)
{
	DEBUG_FATAL(!optionSpec, ("nullptr optionSpec arg"));
	if (!optionSpecCount)
		return false;
	else
		return CommandLine::List::isListNext(optionSpec, optionSpecCount) || Switch::isSwitchNext(optionSpec, optionSpecCount);
}

// ----------------------------------------------------------------------

CommandLine::Collection *CommandLine::Collection::createCollection(
	const OptionSpec **optionSpec,
	int               *optionSpecCount
	)
{
	DEBUG_FATAL(!optionSpec, ("nullptr optionSpec arg"));
	DEBUG_FATAL(!optionSpecCount, ("nullptr optionSpecCount arg"));

	DEBUG_FATAL(!isCollectionNext(*optionSpec, *optionSpecCount), ("tried to create collection from non-collection optionSpec"));
	if ((*optionSpec)->optionType == OST_BeginList)
		return List::createList(optionSpec, optionSpecCount);
	else if ((*optionSpec)->optionType == OST_BeginSwitch)
		return Switch::createSwitch(optionSpec, optionSpecCount);
	else
	{
		DEBUG_FATAL(true, ("error: unknown collection type %d", (*optionSpec)->optionType));
		return 0; //lint !e527 // Unreachable
	}
}

// ======================================================================

CommandLine::List::Node::Node(
	Option     *newOption,
	Collection *newCollection,
	bool        newMultipleIsAllowed,
	bool        newIsRequiredNode
	) :
	option(newOption),
	collection(newCollection),
	multipleIsAllowed(newMultipleIsAllowed),
	isRequiredNode(newIsRequiredNode),
	next(0)
{
	DEBUG_FATAL(!newOption && !newCollection, ("neither option nor collection specified"));
}

// ----------------------------------------------------------------------

CommandLine::List::Node::~Node(void)
{
	delete option;
	option = 0;

	delete collection;
	collection = 0;

	next = 0;
}

// ----------------------------------------------------------------------

CommandLine::MatchCode CommandLine::List::Node::match(void) const
{
	MatchCode  mc = MC_MATCH;

	// if we have an option, check if the option matches
	if (option)
	{
		mc = option->match();

		// only go forward if we match the option
		if (mc != MC_MATCH)
			return mc;
	}

	// if we have a collection, check if the collection matches
	if (collection)
		mc = collection->match();

	return mc;
}

// ----------------------------------------------------------------------

bool CommandLine::List::Node::isNode(
	const OptionSpec *optionSpec,
	int               optionCount
	)
{
	DEBUG_FATAL(!optionSpec, ("nullptr optionSpec arg"));
	if (!optionCount)
		return false;
	else
		return (optionSpec->optionType == OST_BeginListNode);
}

// ----------------------------------------------------------------------

CommandLine::List::Node *CommandLine::List::Node::createNode(
	const OptionSpec **optionSpec,
	int               *optionCount
	)
{
	DEBUG_FATAL(!optionSpec, ("nullptr optionSpec arg"));
	DEBUG_FATAL(!optionCount, ("nullptr optionCount arg"));
	DEBUG_FATAL(!isNode(*optionSpec, *optionCount), ("attempted to create list node with non-list-node data"));

	Option     *newOption = 0;
	Collection *newCollection = 0;
	bool        newIsMultipleAllowed;
	bool        newIsRequiredNode;

	newIsMultipleAllowed = ( (*optionSpec)->int1 != 0 );
	newIsRequiredNode    = ( (*optionSpec)->int2 != 0 );

	// move past OST_BeginListNode
	++(*optionSpec);
	--(*optionCount);

	// create optional option
	if (Option::isOptionNext(*optionSpec, *optionCount))
		newOption = Option::createOption(optionSpec, optionCount);

	// create optional collection
	if (Collection::isCollectionNext(*optionSpec, *optionCount))
		newCollection = Collection::createCollection(optionSpec, optionCount);

	DEBUG_FATAL(!newOption && !newCollection, ("neither option nor collection specified---client must specify at least one"));

	// ensure we're at the end of the list node spec
	DEBUG_FATAL((*optionSpec)->optionType != OST_EndListNode, ("expecting end of list node, found %d instead", (*optionSpec)->optionType));

	// move past OST_EndListNode
	++(*optionSpec);
	--(*optionCount);

	return new Node(newOption, newCollection, newIsMultipleAllowed, newIsRequiredNode);
}

// ======================================================================

CommandLine::List::List(
	OP_ListType  newListType,
	Node        *newFirstNode
	) :
	Collection(),
	firstNode(newFirstNode),
	listType(newListType),
	minimumNodeCount(0)
{
	DEBUG_FATAL(!newFirstNode, ("nullptr newFirstNode arg"));
	DEBUG_FATAL(newListType != OPLT_Normal, ("constructor only good for OPLT_Normal lists, caller passed %d", newListType));
}

// ----------------------------------------------------------------------

CommandLine::List::List(
	OP_ListType  newListType,
	Node        *newFirstNode,
	int          newMinimumNodeCount
	) :
	Collection(),
	firstNode(newFirstNode),
	listType(newListType),
	minimumNodeCount(newMinimumNodeCount)
{
	DEBUG_FATAL(!newFirstNode, ("nullptr newFirstNode arg"));
	DEBUG_FATAL(newListType != OPLT_MinimumMatch, ("constructor only good for OPLT_MinimumMatch lists, caller passed %d", newListType));
}

// ----------------------------------------------------------------------

CommandLine::List::~List(void)
{
	Node *killNode;

	while (firstNode)
	{
		killNode  = firstNode;
		firstNode = const_cast<Node*>(firstNode->getNext());
		delete killNode;
	}
}

// ----------------------------------------------------------------------

bool CommandLine::List::isListNext(
	const OptionSpec *optionSpec,
	int               optionCount
	)
{
	DEBUG_FATAL(!optionSpec, ("nullptr optionSpec arg"));
	if (!optionCount)
		return false;
	else
		return (optionSpec->optionType == OST_BeginList);
}

// ----------------------------------------------------------------------

CommandLine::List *CommandLine::List::createList(
	const OptionSpec **optionSpec,
	int               *optionCount
	)
{
	DEBUG_FATAL(!optionSpec, ("nullptr optionSpec arg"));
	DEBUG_FATAL(!optionCount, ("nullptr optionCount arg"));
	DEBUG_FATAL(!isListNext(*optionSpec, *optionCount), ("tried to create list from non-list optionSpec"));

	Node *newFirstNode = 0;
	Node *lastNode = 0;

	OP_ListType newListType         = static_cast<OP_ListType>((*optionSpec)->int1);
	int         newMinimumNodeCount = (*optionSpec)->int2;

	// move past OST_BeginList
	++(*optionSpec);
	--(*optionCount);

	while (*optionCount && ((*optionSpec)->optionType != OST_EndList))
	{
		const int lastCount = *optionCount;

		DEBUG_FATAL(!Node::isNode(*optionSpec, *optionCount), ("expecting list node, found %d", (*optionSpec)->optionType));
		Node *newNode = Node::createNode(optionSpec, optionCount);
		DEBUG_FATAL(!newNode, ("failed to create list node"));

		// append to node list
		if (!lastNode)
			newFirstNode = newNode;
		else
			lastNode->setNext(newNode);
		lastNode = newNode;

		DEBUG_FATAL(lastCount == *optionCount, ("node creation failed to move forward through optionSpec array"));
		UNREF(lastCount);
	}

	DEBUG_FATAL(!*optionCount, ("failed to find end of option list"));
	DEBUG_FATAL((*optionSpec)->optionType != OST_EndList, ("expecting OST_EndSwitch node"));

	// gobble up the end of list marker
	--(*optionCount);
	++(*optionSpec);

	switch (newListType)
	{
		case OPLT_Normal:
			return new List(newListType, newFirstNode);
		case OPLT_MinimumMatch:
			return new List(newListType, newFirstNode, newMinimumNodeCount);
		default:
			FATAL(true, ("tried to create unknown list type %d", newListType));
	}
	return 0; //lint !e527 // Unreachable
}

// ----------------------------------------------------------------------

CommandLine::MatchCode CommandLine::List::match(void) const
{
	// for lists, a match is successful if we match all required nodes and
	// possibly match non-required nodes

	int childMatchCount  = 0;

	for (const Node *node = firstNode; node; node = node->getNext())
	{
		int        thisNodeMatchCount = 0;
		MatchCode  mc;

		while ( (mc = node->match()) == MC_MATCH )
			++thisNodeMatchCount;

		if (mc != MC_NO_MATCH)
		{
			// we fell out of other loop for a problem
			return mc;
		}

		if (thisNodeMatchCount)
		{
			++childMatchCount;
			if ((thisNodeMatchCount > 1) && !node->getMultipleIsAllowed())
			{
				// matched this node too many times
				return MC_TOO_MANY_NODE_MATCHES;
			}
		}
		else if (node->getIsRequiredNode())
		{
			if (!childMatchCount)
			{
				// we haven't matched any nodes in this list yet,
				// so claim there isn't a match against this entire list
				return MC_NO_MATCH;
			}
			else
			{
				// we've started to match against this list, but we didn't match against this required node.
				// This is a legitimate problem.
				return MC_REQUIRED_MATCH_MISSING;
			}
		}

		// we're okay, we've either matched one, which is always okay,
		// we've matched 0 and it wasn't required, or we matched multiple
		// and multiple is okay.  continue on with next node in list.
	}

	if (listType == OPLT_MinimumMatch)
	{
		// ensure we matched the minimum number of list nodes

		// if none matched, indicate a no match, otherwise ensure we matched at least the minimum or there's an error
		if (!childMatchCount)
			return MC_NO_MATCH;
		else if (childMatchCount < minimumNodeCount)
			return MC_TOO_FEW_CHILDREN_MATCHES;
	}

	// we matched if we made it through all the nodes
	return MC_MATCH;
}

// ======================================================================

CommandLine::Switch::Node::Node(
	Option     *newOption,
	Collection *newCollection,
	bool        newMultipleIsAllowed
	) :
	option(newOption),
	collection(newCollection),
	multipleIsAllowed(newMultipleIsAllowed),
	next(0)
{
	DEBUG_FATAL(!(newOption || newCollection), ("neither option nor collection specified for switch node --- must have at least one"));
}

// ----------------------------------------------------------------------

CommandLine::Switch::Node::~Node(void)
{
	delete option;
	option = 0;

	delete collection;
	collection = 0;

	next = 0;
}

// ----------------------------------------------------------------------

CommandLine::MatchCode CommandLine::Switch::Node::match(void) const
{
	MatchCode  mc = MC_MATCH;

	// if we have an option, check if the option matches
	if (option)
	{
		mc = option->match();

		// only go forward if we match the option
		if (mc != MC_MATCH)
			return mc;
	}

	// if we have a collection, check if the collection matches
	if (collection)
		mc = collection->match();

	return mc;
}

// ----------------------------------------------------------------------

bool CommandLine::Switch::Node::isNode(
	const OptionSpec *optionSpec,
	int               optionCount)
{
	DEBUG_FATAL(!optionSpec, ("nullptr optionSpec arg"));
	if (!optionCount)
		return false;
	else
		return (optionSpec->optionType == OST_BeginSwitchNode);
}

// ----------------------------------------------------------------------

CommandLine::Switch::Node *CommandLine::Switch::Node::createNode(
	const OptionSpec **optionSpec,
	int               *optionCount
	)
{
	DEBUG_FATAL(!optionSpec, ("nullptr optionSpec arg"));
	DEBUG_FATAL(!optionCount, ("nullptr optionCount arg"));
	DEBUG_FATAL(!isNode(*optionSpec, *optionCount), ("attempted to create list node with non-list-node data"));

	Option     *newOption = 0;
	Collection *newCollection = 0;
	bool        newIsMultipleAllowed;

	newIsMultipleAllowed = ( (*optionSpec)->int1 != 0 );

	// move past OST_BeginSwitchNode
	++(*optionSpec);
	--(*optionCount);

	// create optional option
	if (Option::isOptionNext(*optionSpec, *optionCount))
		newOption = Option::createOption(optionSpec, optionCount);

	// create optional collection
	if (Collection::isCollectionNext(*optionSpec, *optionCount))
		newCollection = Collection::createCollection(optionSpec, optionCount);

	DEBUG_FATAL(!newOption && !newCollection, ("neither option nor collection specified---client must specify at least one"));

	// ensure we're at the end of the list node spec
	DEBUG_FATAL((*optionSpec)->optionType != OST_EndSwitchNode, ("expecting end of list node, found %d instead", (*optionSpec)->optionType));

	// move past OST_EndSwitchNode
	++(*optionSpec);
	--(*optionCount);

	return new Node(newOption, newCollection, newIsMultipleAllowed);
}

// ======================================================================

CommandLine::Switch::Switch(
	Node *newFirstNode,
	bool  newOneNodeIsRequired
	) :
	Collection(),
	firstNode(newFirstNode),
	oneNodeIsRequired(newOneNodeIsRequired)
{
	DEBUG_FATAL(!newFirstNode, ("empty switch collection created"));
}

// ----------------------------------------------------------------------

CommandLine::Switch::~Switch(void)
{
	Node *killNode;

	while (firstNode)
	{
		killNode  = firstNode;
		firstNode = const_cast<Node*>(firstNode->getNext());
		delete killNode;
	}
}

// ----------------------------------------------------------------------

bool CommandLine::Switch::isSwitchNext(
	const OptionSpec *optionSpec,
	int               optionCount
	)
{
	DEBUG_FATAL(!optionSpec, ("nullptr optionSpec arg"));
	if (!optionCount)
		return false;
	else
		return (optionSpec->optionType == OST_BeginSwitch);
}

// ----------------------------------------------------------------------

CommandLine::Switch *CommandLine::Switch::createSwitch(
	const OptionSpec **optionSpec,
	int               *optionCount
	)
{
	DEBUG_FATAL(!optionSpec, ("nullptr optionSpec arg"));
	DEBUG_FATAL(!optionCount, ("nullptr optionCount arg"));
	DEBUG_FATAL(!isSwitchNext(*optionSpec, *optionCount), ("tried to create switch from non-switch option spec"));

	const bool newOneNodeIsRequired = ((*optionSpec)->int1 != 0);
	Node *newFirstNode = 0;
	Node *lastNode = 0;

	// move past OST_BeginSwitch
	++(*optionSpec);
	--(*optionCount);

	while (*optionCount && ((*optionSpec)->optionType != OST_EndSwitch))
	{
		const int lastCount = *optionCount;

		DEBUG_FATAL(!Node::isNode(*optionSpec, *optionCount), ("expecting list node, found %d", (*optionSpec)->optionType));
		Node *newNode = Node::createNode(optionSpec, optionCount);
		DEBUG_FATAL(!newNode, ("failed to create switch node"));

		// append to node list
		if (!lastNode)
			newFirstNode = newNode;
		else
			lastNode->setNext(newNode);
		lastNode = newNode;

		DEBUG_FATAL(lastCount == *optionCount, ("node creation failed to move forward through optionSpec array"));
		UNREF(lastCount);
	}

	DEBUG_FATAL(!*optionCount, ("failed to find end of option switch"));
	DEBUG_FATAL((*optionSpec)->optionType != OST_EndSwitch, ("expecting OST_EndSwitch node"));

	// gobble up the end of switch marker
	--(*optionCount);
	++(*optionSpec);

	return new Switch(newFirstNode, newOneNodeIsRequired);
}

// ----------------------------------------------------------------------

CommandLine::MatchCode CommandLine::Switch::match(void) const
{
	// for switch, matching policy is based on value of
	// oneNodeIsRequired:
	//   true: exactly one of the nodes must match.  That node
	//         can optionally match multiple times.  Error if
	//         no matches or more than one match are made.
	//   false: one node is a match, zero node is a no match.
	//         error if more than one match is made.

	int childMatchCount = 0;

	for (const Node *node = firstNode; node; node = node->getNext())
	{
		int        thisNodeMatchCount = 0;
		MatchCode  mc;

		// -TF- this can get into an infinite loop here if a list has one or more children, all optional, with nothing specified
		// on the commandline.  The command line will keep matching.
		while ( (mc = node->match()) == MC_MATCH )
			++thisNodeMatchCount;

		if (mc != MC_NO_MATCH)
		{
			// we fell out of other loop for a problem
			return mc;
		}

		if (thisNodeMatchCount)
		{
			++childMatchCount;
			if (childMatchCount > 1)
			{
				// ut oh, matched too many children nodes
				return MC_TOO_MANY_CHILDREN_MATCHES;
			}

			if ((thisNodeMatchCount > 1) && !node->getMultipleIsAllowed())
			{
				// matched too many
				return MC_TOO_MANY_NODE_MATCHES;
			}
		}
	}

	if (!childMatchCount)
	{
		// didn't match any nodes
		if (oneNodeIsRequired)
			return MC_REQUIRED_MATCH_MISSING;
		else
			return MC_NO_MATCH;
	}
	else
	{
		// we must have matched exactly one
		return MC_MATCH;
	}
}

// ======================================================================

CommandLine::OptionTable::Record::Record(
	char            newShortName,
	const char     *newLongName,
	ArgumentPolicy  newArgumentPolicy
	) :
	shortName(newShortName),
	longName(0),
	writeCount(0),
	matchCount(0),
	argumentPolicy(newArgumentPolicy),
	next(0)
{
	if (newLongName)
	{
		const size_t length = strlen(newLongName);
		if (length)
		{
			longName = new char[length+1];
			memcpy(longName, newLongName, length+1);
		}
	}
	memset(stringArray, 0, sizeof(stringArray));
}

// ----------------------------------------------------------------------

CommandLine::OptionTable::Record::~Record(void)
{
	for (int i = 0; i < writeCount; ++i)
		delete [] stringArray[i];

	delete [] longName;
	longName = 0;
	next = 0;
}

// ----------------------------------------------------------------------

void CommandLine::OptionTable::Record::addOccurrence(
	const char *newString
	)
{
	DEBUG_FATAL(writeCount + 1 > MAX_OCCURRENCE_COUNT, ("commandline option occurrence overflow, max per option=%d", MAX_OCCURRENCE_COUNT));

	DEBUG_FATAL(stringArray[writeCount], ("internal error: stringArray already set"));
	if (newString && *newString)
	{
		const size_t length = strlen(newString);
		if (length)
		{
			stringArray[writeCount] = new char[length+1];
			memcpy(stringArray[writeCount], newString, length+1);
		}
	}
	++writeCount;
}

// ======================================================================

CommandLine::OptionTable::OptionTable(void)
:
	firstRecord(0)
{
}

// ----------------------------------------------------------------------

CommandLine::OptionTable::~OptionTable(void)
{
	Record *deadRecord;

	while (firstRecord)
	{
		deadRecord  = firstRecord;
		firstRecord = firstRecord->getNext();
		delete deadRecord;
	}
}

// ----------------------------------------------------------------------

CommandLine::OptionTable::Record *CommandLine::OptionTable::createOptionRecord(
	char            shortName,
	const char     *longName,
	ArgumentPolicy  newArgumentPolicy
	)
{
	DEBUG_FATAL(shortName && findOptionRecord(shortName), ("short name %c already exists in list", shortName));
	DEBUG_FATAL(longName && findOptionRecord(longName), ("long name \"%s\" already exists in list", longName));

	Record *record = new Record(shortName, longName, newArgumentPolicy);
	record->setNext(firstRecord);
	firstRecord = record;

	return firstRecord;
}

// ----------------------------------------------------------------------

CommandLine::OptionTable::Record *CommandLine::OptionTable::findOptionRecord(
	char shortName
	) const
{
	DEBUG_FATAL(!shortName, ("nullptr shortName arg"));

	// walk the list
	for (Record *record = firstRecord; record; record = record->getNext())
	{
		if (record->getShortName() == shortName)
			return record;
	}

	// didn't find it
	return 0;
}

// ----------------------------------------------------------------------

CommandLine::OptionTable::Record *CommandLine::OptionTable::findOptionRecord(
	const char *longName
	) const
{
	DEBUG_FATAL(!longName || !*longName, ("invalid longName arg"));

	// walk the list
	for (Record *record = firstRecord; record; record = record->getNext())
	{
		const char *recordLongName = record->getLongName();
		if (!recordLongName)
			continue;
		if (!strcmp(recordLongName, longName))
			return record;
	}

	// didn't find it
	return 0;
}

// ----------------------------------------------------------------------
/**
 * Return whether all written options in the option table were fully
 * matched.
 *
 * @return true if each option written has an equal number of matches.
 * false if any one of the option entries written has not been
 * fully matched.
 */

bool CommandLine::OptionTable::getAllOptionsMatched(void) const
{
	for (Record *record = firstRecord; record; record = record->getNext())
	{
		if (record->getMatchCount() < record->getWriteCount())
			return false;
	}

	// if we got this far, we're okay
	return true;
}

// ======================================================================

CommandLine::Lexer::Token::Token(void)
:
	tokenType(TT_Error)
{
	name[0]     = 0;
	argument[0] = 0;
}

// ======================================================================

CommandLine::Lexer::Lexer(
	const char *newBuffer
	) :
	nextCharacter(newBuffer)
{
	DEBUG_FATAL(!newBuffer, ("nullptr newBuffer arg"));
}

// ----------------------------------------------------------------------

CommandLine::Lexer::~Lexer(void)
{
	nextCharacter = 0;
}

// ----------------------------------------------------------------------
/**
 * Gobble up a string from input.
 *
 * If isRequired is not set, the only way for the function to return false
 * is for a lex error (e.g. "hello  <- is missing trailing quote, or
 * -agp  <- unquoted strings cannot start with hyphen).  If isRequired is
 * set, the function only returns true if a non-zero-length string is
 * found.
 *
 * @param isRequired  [IN]  if true, must find a valid string to gobble.  if false, string is optional
 * @param stringBuffer  [OUT] buffer which will be filled with zero-terminated string upon exit
 * @param stringSize  [IN]  size of the buffer in bytes
 * @return if isRequired is true:
 * true if string is found, stringBuffer filled appropriately.
 * false if the string could not be found or if there was a lex error
 *
 * if isRequired is false:
 * true if the string was found or if no string was found (stringBuffer filled appropriately)
 * false if a lex error occurred
 */

bool CommandLine::Lexer::gobbleString(bool isRequired, char *stringBuffer, int stringSize)
{
	DEBUG_FATAL(!stringBuffer, ("nullptr stringStart arg"));
	DEBUG_FATAL(!stringSize, ("stringSize is zero"));

	DEBUG_FATAL(!nextCharacter, ("nullptr nextChar"));


	int  stringLength = 0;
	bool inQuote = false;

	while ( *nextCharacter && (inQuote || ((*nextCharacter != '-') && !isspace(*nextCharacter) && (*nextCharacter != '='))) )
	{
		if (stringLength >= (stringSize-1))
		{
			DEBUG_REPORT_LOG_PRINT(true, ("string exceeded string buffer size of %d", stringSize));
			return false;
		}

		if (*nextCharacter == '"')
		{
			++nextCharacter;
			inQuote = !inQuote;
		}
		else if (inQuote && (*nextCharacter == '\\'))
		{
			// handle escaped characters
			++nextCharacter;
			switch (*nextCharacter)
			{
			case 'n':
				// CR/LF pair
				if (stringLength + 2 > (stringSize-1))
				{
					DEBUG_REPORT_LOG_PRINT(true, ("string exceeded string buffer size of %d", stringSize));
					return false;
				}
				*(stringBuffer++) = 13;
				*(stringBuffer++) = 10;
				stringLength += 2;
				break;
			case 'r':
				// CR
				*(stringBuffer++) = '\r';
				++stringLength;
				break;
			case 't':
				// tab
				*(stringBuffer++) = '\t';
				++stringLength;
				break;
			default:
				// assume all other escaped characters are the character themselves
				*(stringBuffer++) = *nextCharacter;
				++stringLength;
			}
			// gobble up excaped character
			++nextCharacter;
		}
		else
		{
			// just copy the character
			*(stringBuffer++) = *(nextCharacter++);
			++stringLength;
		}
	}

	if (inQuote)
	{
		// parse error, string ran out while still in quote
		DEBUG_REPORT_LOG_PRINT(true, ("string parser ran out of characters while in quoted string"));
		return false;
	}

	if (isRequired && (stringLength == 0))
		return false;

	// nullptr-terminate the string
	*stringBuffer = 0;
	return true;
}

// ----------------------------------------------------------------------

bool CommandLine::Lexer::getNextToken(Token *token)
{
	DEBUG_FATAL(!token, ("nullptr token arg"));
	DEBUG_FATAL(!nextCharacter, ("nullptr nextCharacter"));

	// clear out the token
	memset(token, 0, sizeof(*token));

	// gobble up whitespace
	gobbleWhitespace();

	if (!*nextCharacter)
	{
		token->tokenType = TT_End;
		return true;
	}
	else if (*nextCharacter == '-')
	{
		// gobble it up
		++nextCharacter;

		if (!*nextCharacter)
		{
			token->tokenType = TT_Error;
			return false;
		}
		else if (*nextCharacter == '-')
		{
			// we have a double dash

			// gobble up
			++nextCharacter;

			if (!*nextCharacter || isspace(*nextCharacter))
			{
				// we've hit the double dash token
				token->tokenType = TT_DoubleDash;
				return true;
			}
			else
			{
				// retrieve the name
				if (!gobbleString(true, token->name, MAX_LONG_NAME_SIZE))
				{
					token->tokenType = TT_Error;
					return false;
				}

				token->tokenType = TT_LongOption;
				return true;
			}
		}
		else if (isalnum(*nextCharacter))
		{
			token->name[0] = *nextCharacter;
			token->name[1] = 0;
			++nextCharacter;

			token->tokenType = TT_ShortOption;
			return true;
		}
		else
		{
			token->tokenType = TT_Error;
			return false;
		}
	}
	else if (*nextCharacter == '=')
	{
		++nextCharacter;
		token->tokenType = TT_Equal;
		return true;
	}
	else
	{
		// either an argument or an error

		// retrieve the argument
		if (!gobbleString(true, token->argument, MAX_ARGUMENT_SIZE))
		{
			token->tokenType = TT_Error;
			return false;
		}
		token->tokenType = TT_Argument;
		return true;
	}
}

// ======================================================================

void CommandLine::install(void)
{
	DEBUG_FATAL(installed, ("CommandLine already installed"));

	installed = true;
	ExitChain::add(remove, "CommandLine::remove");
}

// ----------------------------------------------------------------------

void CommandLine::remove(void)
{
	DEBUG_FATAL(!installed, ("CommandLine not installed"));

	installed = false;

	delete optionTable;
	optionTable = 0;

	// reset state
	bufferSize = 0;
	wasParsed  = false;
}

// ----------------------------------------------------------------------
/**
 * Parse and build the option tree from the option spec list provided
 * by the client.
 *
 * specList must contain entries following the grammar described in
 * the engine SourceSafe file "$/new/doc/CommandLineGrammar.txt".
 *
 * @param specList  [IN] spec list containing description of tree
 * @param specCount  [IN] number of entries in spec list
 */

void CommandLine::buildOptionTree(const OptionSpec *specList, int specCount)
{
	DEBUG_FATAL(!specList, ("nullptr specList arg"));

	if (!specCount)
	{
		// nothing to build
		return;
	}

	DEBUG_FATAL(!Collection::isCollectionNext(specList, specCount), ("expecting collection as first option spec"));
	treeRoot = Collection::createCollection(&specList, &specCount);
	DEBUG_FATAL(specCount, ("failed to consume entire specArray, %d entries remaining", specCount));
}

// ----------------------------------------------------------------------

CommandLine::MatchCode CommandLine::parseCommandLineBuffer(void)
{
	Lexer          lexer(buffer);
	Lexer::Token   token;

	bool rc = lexer.getNextToken(&token);

	do
	{
		if (!rc)
		{
			DEBUG_REPORT_LOG_PRINT(true, ("error getting token"));
			return MC_CL_PARSE_ERROR;
		}

		switch (token.getTokenType())
		{
		case Lexer::TT_DoubleDash:
			// we're done, don't parse any more

			// return "okay"
			// -TF- I don't like this, we're overloading the use of the match code.
			// Maybe it should become a generic error code.
			return MC_MATCH;

		case Lexer::TT_ShortOption:
			// we've found a short option, make sure specified short option exists
			{
				DEBUG_FATAL(!optionTable, ("internal error: nullptr optionTable"));
				OptionTable::Record *record = optionTable->findOptionRecord(token.getShortName());
				if (!record)
				{
					// command line option is not a valid option
					return MC_CL_OPTION_UNKNOWN;
				}

				// consume token, check for argument
				rc = lexer.getNextToken(&token);
				if (!rc)
					break;
				if (record->getArgumentPolicy() == AP_None)
					record->addOccurrence(0);
				else // option can taken an argument
				{
					// if newly consumed token is an argument, post it as this option's argument
					if (token.getTokenType() != Lexer::TT_Argument)
						record->addOccurrence(0);
					else
					{
						record->addOccurrence(token.getArgument());
						// gobble up argument token
						rc = lexer.getNextToken(&token);
					}
				}
			}
			break;

		case Lexer::TT_LongOption:
			// we've found a long option, make sure specified long option exists
			{
				DEBUG_FATAL(!optionTable, ("internal error: nullptr optionTable"));
				OptionTable::Record *record = optionTable->findOptionRecord(token.getLongName());
				if (!record)
				{
					// command line option is not a valid option
					return MC_CL_OPTION_UNKNOWN;
				}

				// consume token, check for option
				rc = lexer.getNextToken(&token);
				if (!rc)
					break;
				if (token.getTokenType() == Lexer::TT_Equal)
				{
					if (record->getArgumentPolicy() == AP_None)
						// not expecting an argument here
						return MC_CL_UNEXPECTED_ARG;

					// consume it, get argument
					rc = lexer.getNextToken(&token);
					if (!rc)
						break;

					if (token.getTokenType() != Lexer::TT_Argument)
						return MC_CL_EXPECTING_ARG;

					record->addOccurrence(token.getArgument());
					rc = lexer.getNextToken(&token);
				}
				else
				{
					if (record->getArgumentPolicy() == AP_Required)
					{
						// expecting an arg
						return MC_CL_EXPECTING_ARG;
					}
					record->addOccurrence(0);
				}
			}
			break;

		case Lexer::TT_Argument:
			{
				DEBUG_FATAL(!optionTable, ("internal error: nullptr optionTable"));
				OptionTable::Record *record = optionTable->findOptionRecord(OP_SNAME_UNTAGGED);
				if (!record)
				{
					// user specified an untagged argument but not supported
					return MC_CL_UNTAGGED_ARG_DENIED;
				}
				record->addOccurrence(token.getArgument());

				// gobble up arg
				rc = lexer.getNextToken(&token);
			}
			break;

		case Lexer::TT_Error:
			return MC_CL_PARSE_ERROR;

		case Lexer::TT_End:
			break;

		case Lexer::TT_Equal:
		default:
			DEBUG_FATAL(true, ("unexpected option code(%d)", token.getTokenType()));
		}

	} while (token.getTokenType() != Lexer::TT_End);

	// made it through okay
	return MC_MATCH;
}

// ----------------------------------------------------------------------
/**
 * Absorb multiple strings into the current CommandLine parsing buffer.
 *
 * A space is inserted into the buffer after the new contents.  This
 * ensures separate buffer absorptions do not bleed together at the
 * edges for parsing purposes.
 *
 * Under apps using the main(argc, argv) entry point, the user
 * must quote arguments using escaped quotes when they need to
 * include spaces in argument names. e.g.:
 * viff \"c:\\my data\\tmp\\test.iff\"
 *
 * Under apps using WinMain(...), the user should not escape
 * quoted strings.  e.g.:
 * viff "c:\\my data\\tmp\\test.iff"
 *
 * Once quoting, characters within the quotes will be interpreted
 * using C-style backslash rules (well, a subset anyway).  The
 * following character escape code substitutions work within quoted strings:
 * \t  tab character
 * \n  CR/LF pair
 * \r  carriage return
 * \\  single backslash (remember, this is required to get a single backslash in a quoted string)
 */

void CommandLine::absorbString(const char *newString)
{
	DEBUG_FATAL(!installed, ("CommandLine not installed"));
	DEBUG_FATAL(!newString, ("nullptr newString arg"));

	const int stringLength   = static_cast<int>(strlen(newString));
	int requiredBufferSpace;
	if (absorbedStringCount)
		requiredBufferSpace = stringLength + 2;
	else
		requiredBufferSpace = stringLength + 1;
	DEBUG_FATAL(bufferSize + requiredBufferSpace >= STATIC_BUFFER_SIZE, ("newString too large by %d bytes", bufferSize + requiredBufferSpace - STATIC_BUFFER_SIZE));

	if (absorbedStringCount)
	{
		// prepend a space
		buffer[bufferSize++] = ' ';
	}

	// copy contents of buffer, including nullptr
	memcpy(buffer + bufferSize, newString, stringLength + 1);
	bufferSize += stringLength;

	++absorbedStringCount;
}

// ----------------------------------------------------------------------
/**
 * Absorb multiple strings into the current CommandLine parsing buffer.
 *
 * A space is inserted into the buffer after each new string.  This
 * ensures separate buffer absorptions do not bleed together at the
 * edges for parsing purposes.
 *
 * Under apps using the main(argc, argv) entry point, the user
 * must quote arguments using escaped quotes when they need to
 * include spaces in argument names. e.g.:
 * viff \"c:\\my data\\tmp\\test.iff\"
 *
 * Under apps using WinMain(...), the user should not escape
 * quoted strings.  e.g.:
 * viff "c:\\my data\\tmp\\test.iff"
 *
 * Once quoting, characters within the quotes will be interpreted
 * using C-style backslash rules (well, a subset anyway).  The
 * following character escape code substitutions work within quoted strings:
 * \t  tab character
 * \n  CR/LF pair
 * \r  carriage return
 * \\  single backslash (remember, this is required to get a single backslash in a quoted string)
 *
 * @param stringArray  [IN] array of string pointers to absorb
 * @param stringCount  [IN] count of array
 */

void CommandLine::absorbStrings(const char **stringArray, int stringCount)
{
	DEBUG_FATAL(!installed, ("CommandLine not installed"));
	DEBUG_FATAL(!stringArray, ("nullptr string array"));

	for (int i = 0; i < stringCount; ++i)
		absorbString(stringArray[i]);
}

// ----------------------------------------------------------------------
/**
 * Return the portion of the command line string not meant for the
 * CommandLine class.
 *
 * Anything following the first double dash with trailing whitespace
 * (i.e. "-- ") will be considered part of the post command line string.
 *
 * @return A read-only pointer to the portion of the command line not meant
 * for CommandLine parsing.  May be nullptr if the entire command line
 * was meant for CommandLine or if no command line was specified.
 */

const char *CommandLine::getPostCommandLineString(void)
{
	Lexer          lexer(buffer);
	Lexer::Token   token;

	// scan for the '--' or EOF ending CommandLine-owned strings
	do
	{
		bool rc = lexer.getNextToken(&token);
		if (!rc)
		{
			DEBUG_REPORT_LOG_PRINT(true, ("error getting token\n"));
			return 0;
		}

		switch (token.getTokenType())
		{
		case Lexer::TT_DoubleDash:
			{
				// we're done, don't parse any more
				const char *result = lexer.getNextCharacter();
				// skip leading whitespace
				while (isspace(*result))
					++result;
				return result;
			}

		case Lexer::TT_Error:
			DEBUG_REPORT_LOG_PRINT(true, ("parse error while scanning for post command line string\n"));
			return 0;

		case Lexer::TT_End:
		case Lexer::TT_ShortOption:
		case Lexer::TT_LongOption:
		case Lexer::TT_Equal:
		case Lexer::TT_Argument:
		default:
			// nothing to do
			break;
		}

	} while (token.getTokenType() != Lexer::TT_End);

	// if we made it here, there are no post command line options
	return 0;
}

// ----------------------------------------------------------------------
/**
 * Parse the command line according to the option parse tree specified.
 *
 * See $/new/doc/CommandLineGrammar.txt for a description of the option
 * parse tree grammar.
 *
 * Options are tags that can be queried for their presence.  In addition,
 * options may have arguments associated with them (according to the
 * rules specified within the option tree).
 *
 * Short options are formed by a hyphen followed immediately by an alphanumeric
 * character (e.g. -c, -h, -i).  If an argument is associated with the
 * short option, it follows the short option with optional whitespace
 * separating them.
 *
 * Long options are formed by two hyphens followed immediately by a text
 * string (e.g. --inputfile, --pragmaTarget, --help).  If an argument is
 * associated with the long option, it is followed by an equal sign (=),
 * then followed by the argument (with whitespace optionally interspersed),
 * as in (--inputfile = \"c:\My Programs\something\").
 *
 * @param optionTree  [IN] option parse tree specification array
 * @param optionSpecCount  [IN] number of entries in the array
 * @return A return code indicating the status of the operation.  A value
 * of MC_MATCH means the command line parsed successfully.  Any other
 * return value indicates some type of error.  Errors can occur in
 * different areas, including the option tree specification and the
 * command line specification.
 */

CommandLine::MatchCode CommandLine::parseOptions(const OptionSpec *optionTree, int optionSpecCount)
{
	DEBUG_FATAL(!installed, ("CommandLine not installed"));

	MatchCode  mc;

	// create the option information table
	delete optionTable;
	optionTable = new OptionTable;

	// build the option tree corresponding to the option spec,
	// filling the command line parse structures at the same time
	buildOptionTree(optionTree, optionSpecCount);

	// parse the command line options into available options
	mc = parseCommandLineBuffer();
	if (mc != MC_MATCH)
	{
		delete treeRoot;
		treeRoot = 0;
		return mc;
	}

	// perform the match
	if (treeRoot)
	{
		mc = treeRoot->match();
		delete treeRoot;
		treeRoot = 0;
	}
	else
		mc = MC_MATCH;

	if (mc == MC_MATCH)
	{
		// ensure all command line options specified were consumed
		if (!optionTable->getAllOptionsMatched())
			mc = MC_UNMATCHED_OPTIONS;
		else
			wasParsed = true;
	}

	// cleanup option tree
	delete treeRoot;
	treeRoot = 0;

	return mc;
}

// ----------------------------------------------------------------------
/**
 * Retrieve whether an option was specified on the command line.
 *
 * To check if a second occurrence of the "-i" was specified, use
 * CommandLine::getOptionExists('i', 1)
 *
 * The client may use either the short or long name version of this
 * command.  They will return the same value regardless of the form
 * of the option specified by the user on the command line.
 *
 * @param shortName  [IN] short name of the option
 * @param occurrenceIndex  [IN] zero-based occurrence number to check
 * @return true if the option with the given was specified on the command line.
 * false if the option was not specified on the command line.
 */

bool CommandLine::getOptionExists(char shortName, int occurrenceIndex)
{
	DEBUG_FATAL(!installed, ("CommandLine not installed"));
	DEBUG_FATAL(!wasParsed, ("commandline not parsed successfully"));
	DEBUG_FATAL(!shortName, ("invalid shortName arg"));

	OptionTable::Record *record = optionTable->findOptionRecord(shortName);
	if (!record)
		return false;
	else
		return (occurrenceIndex < record->getWriteCount());
}

// ----------------------------------------------------------------------
/**
 * Retrieve whether an option was specified on the command line.
 *
 * To check if an occurrence of the "-inputfile" was specified, use
 * CommandLine::getOptionExists("inputfile", 0)
 *
 * The client may use either the short or long name version of this
 * command.  They will return the same value regardless of the form
 * of the option specified by the user on the command line.
 *
 * @param longName  [IN] the long name of the option to check
 * @param occurrenceIndex  [IN] the zero-based occurrence number to check
 * @return true if the option with the given was specified on the command line.
 * false if the option was not specified on the command line.
 */

bool CommandLine::getOptionExists(const char *longName, int occurrenceIndex)
{
	DEBUG_FATAL(!installed, ("CommandLine not installed"));
	DEBUG_FATAL(!wasParsed, ("commandline not parsed successfully"));
	DEBUG_FATAL(!longName || !*longName, ("invalid longName arg"));

	OptionTable::Record *record = optionTable->findOptionRecord(longName);
	if (!record)
		return false;
	else
		return (occurrenceIndex < record->getWriteCount());
}

// ----------------------------------------------------------------------
/**
 * Retrieve the number of times a given option occurred on the command line.
 *
 * The client may use either the short or long name version of this
 * command.  They will return the same value regardless of the form
 * of the option specified by the user on the command line.
 *
 * @param shortName  [IN] short name of the option to check
 * @return The number of times the specified option appeared on the command line.
 */

int CommandLine::getOccurrenceCount(char shortName)
{
	DEBUG_FATAL(!installed, ("CommandLine not installed"));
	DEBUG_FATAL(!wasParsed, ("commandline not parsed successfully"));
	DEBUG_FATAL(!shortName, ("invalid shortName arg"));

	OptionTable::Record *record = optionTable->findOptionRecord(shortName);
	if (!record)
		return 0;
	else
		return record->getWriteCount();
}

// ----------------------------------------------------------------------
/**
 * Retrieve the number of times a given option occurred on the command line.
 *
 * The client may use either the short or long name version of this
 * command.  They will return the same value regardless of the form
 * of the option specified by the user on the command line.
 *
 * @param longName  [IN] long name of the option to check
 * @return The number of times the specified option appeared on the command line.
 */

int CommandLine::getOccurrenceCount(const char *longName)
{
	DEBUG_FATAL(!installed, ("CommandLine not installed"));
	DEBUG_FATAL(!wasParsed, ("commandline not parsed successfully"));
	DEBUG_FATAL(!longName || !*longName, ("invalid longName arg"));

	OptionTable::Record *record = optionTable->findOptionRecord(longName);
	if (!record)
		return 0;
	else
		return record->getWriteCount();
}

// ----------------------------------------------------------------------
/**
 * Retrieve the argument associated with the given option occurrence.
 *
 * The client may use either the short or long name version of this
 * command.  They will return the same value regardless of the form
 * of the option specified by the user on the command line.
 *
 * @param shortName  [IN] short name of the option
 * @param occurrenceIndex  [IN] zero-based occurrence number
 * @return The read-only argument string associated with the specified option occurrence.
 * May be nullptr if no argument was associated with the specified option.
 */

const char *CommandLine::getOptionString(char shortName, int occurrenceIndex)
{
	DEBUG_FATAL(!installed, ("CommandLine not installed"));
	DEBUG_FATAL(!wasParsed, ("commandline not parsed successfully"));
	DEBUG_FATAL(!shortName, ("invalid shortName arg"));

	OptionTable::Record *record = optionTable->findOptionRecord(shortName);
	DEBUG_FATAL(!record, ("requested option string for non-existant option '%c'", shortName));

	return record->getString(occurrenceIndex);
}

// ----------------------------------------------------------------------
/**
 * Retrieve the argument associated with the given option occurrence.
 *
 * The client may use either the short or long name version of this
 * command.  They will return the same value regardless of the form
 * of the option specified by the user on the command line.
 *
 * @param longName  [IN] long name of the option
 * @param occurrenceIndex  [IN] zero-based occurrence number
 * @return The read-only argument string associated with the specified option occurrence.
 * May be nullptr if no argument was associated with the specified option.
 */

const char *CommandLine::getOptionString(const char *longName, int occurrenceIndex)
{
	DEBUG_FATAL(!installed, ("CommandLine not installed"));
	DEBUG_FATAL(!wasParsed, ("commandline not parsed successfully"));
	DEBUG_FATAL(!longName || !*longName, ("invalid longName arg"));

	OptionTable::Record *record = optionTable->findOptionRecord(longName);
	DEBUG_FATAL(!record, ("requested option string for non-existant option \"%s\"", longName));

	return record->getString(occurrenceIndex);
}

// ----------------------------------------------------------------------
/**
 * Retrieve the argument associated with the given option occurrence.
 *
 * The client may use either the short or long name version of this
 * command.  They will return the same value regardless of the form
 * of the option specified by the user on the command line.
 *
 * @param shortName  [IN] short name of the option
 * @param occurrenceIndex  [IN] zero-based occurrence number
 * @return An integerized version of the option string associated with the specified option occurrence.
 * Will return zero if the argument string could not be converted to an integer.
 */

int CommandLine::getOptionInt(char shortName, int occurrenceIndex)
{
	DEBUG_FATAL(!installed, ("CommandLine not installed"));
	DEBUG_FATAL(!wasParsed, ("commandline not parsed successfully"));
	DEBUG_FATAL(!shortName, ("invalid shortName arg"));

	OptionTable::Record *record = optionTable->findOptionRecord(shortName);
	DEBUG_FATAL(!record, ("requested option string for non-existant option '%c'", shortName));

	return atoi(record->getString(occurrenceIndex));
}

// ----------------------------------------------------------------------
/**
 * Retrieve the argument associated with the given option occurrence.
 *
 * The client may use either the short or long name version of this
 * command.  They will return the same value regardless of the form
 * of the option specified by the user on the command line.
 *
 * @param longName  [IN] long name of the option
 * @param occurrenceIndex  [IN] zero-based occurrence number
 * @return An integerized version of the option string associated with the specified option occurrence.
 * Will return zero if the argument string could not be converted to an integer.
 */

int CommandLine::getOptionInt(const char *longName, int occurrenceIndex)
{
	DEBUG_FATAL(!installed, ("CommandLine not installed"));
	DEBUG_FATAL(!wasParsed, ("commandline not parsed successfully"));
	DEBUG_FATAL(!longName || !*longName, ("invalid longName arg"));

	OptionTable::Record *record = optionTable->findOptionRecord(longName);
	DEBUG_FATAL(!record, ("requested option string for non-existant option \"%s\"", longName));

	return atoi(record->getString(occurrenceIndex));
}

// ======================================================================
