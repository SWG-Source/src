// ======================================================================
//
// CommandLine.h
// Todd Fiala
//
// copyright 1999, Bootprint Entertainment
//
// ======================================================================

#ifndef COMMAND_LINE_H
#define COMMAND_LINE_H

// ======================================================================

enum OP_ListType
{
	OPLT_Normal,
	OPLT_MinimumMatch
};

#define OP_BEGIN_LIST()                                        { CommandLine::OST_BeginList,       static_cast<int>(OPLT_Normal),       0,                0,         0        }
#define OP_END_LIST()                                          { CommandLine::OST_EndList,         0,                 0,                0,         0        }
#define OP_BEGIN_MINIMUM_MATCH_LIST(minimumNodeCount)          { CommandLine::OST_BeginList,       static_cast<int>(OPLT_MinimumMatch), minimumNodeCount, 0,         0        }                 
#define OP_END_MINIMUM_MATCH_LIST()                            { CommandLine::OST_EndList,         0,                 0,                0,         0        }
#define OP_BEGIN_SWITCH(oneIsRequired)                         { CommandLine::OST_BeginSwitch,     oneIsRequired,     0,                0,         0        }
#define OP_END_SWITCH()                                        { CommandLine::OST_EndSwitch,       0,                 0,                0,         0        }
#define OP_BEGIN_LIST_NODE(multipleIsAllowed, isRequiredNode)  { CommandLine::OST_BeginListNode,   multipleIsAllowed, isRequiredNode,   0,         0        }
#define OP_END_LIST_NODE()                                     { CommandLine::OST_EndListNode,     0,                 0,                0,         0        }
#define OP_BEGIN_SWITCH_NODE(multipleIsAllowed)                { CommandLine::OST_BeginSwitchNode, multipleIsAllowed, 0,                0,         0        }
#define OP_END_SWITCH_NODE()                                   { CommandLine::OST_EndSwitchNode,   0,                 0,                0,         0        }
#define OP_OPTION(shortName, longName, argumentPolicy)         { CommandLine::OST_Option,          argumentPolicy,    0,                shortName, longName }

#define OP_SINGLE_LIST_NODE(shortName, longName, argMode, multipleIsAllowed, isRequiredNode) OP_BEGIN_LIST_NODE(multipleIsAllowed, isRequiredNode), OP_OPTION(shortName, longName, argMode), OP_END_LIST_NODE()
#define OP_SINGLE_SWITCH_NODE(shortName, longName, argMode, multipleIsAllowed) OP_BEGIN_SWITCH_NODE(multipleIsAllowed), OP_OPTION(shortName, longName, argMode), OP_END_SWITCH_NODE()

//lint -save -e1923 // #define could become const

#define OP_ARG_NONE           0
#define OP_ARG_REQUIRED       1
#define OP_ARG_OPTIONAL       2

#define OP_NODE_REQUIRED      1
#define OP_NODE_OPTIONAL      0

#define OP_MULTIPLE_ALLOWED   1
#define OP_MULTIPLE_DENIED    0

#define OP_SNAME_UNTAGGED     1
#define OP_LNAME_UNTAGGED     static_cast<const char*>(0)

//lint -restore

// ======================================================================

class CommandLine
{

private:

	enum
	{
		STATIC_BUFFER_SIZE = 4096
	};

public:

	enum MatchCode
	{
		MC_MATCH                      = 1,
		MC_NO_MATCH                   = 0,
		MC_REQUIRED_MATCH_MISSING     = -1,
		MC_TOO_MANY_CHILDREN_MATCHES  = -2,
		MC_TOO_MANY_NODE_MATCHES      = -3,
		MC_ARG_NOT_EXPECTED           = -4,
		MC_ARG_MISSING                = -5,
		MC_UNMATCHED_OPTIONS          = -6,
		MC_TOO_FEW_CHILDREN_MATCHES   = -7,

		// parse errors from client's command line
		MC_CL_OPTION_UNKNOWN          = -101,
		MC_CL_PARSE_ERROR             = -102,
		MC_CL_EXPECTING_ARG           = -103,
		MC_CL_UNEXPECTED_ARG          = -104,
		MC_CL_UNTAGGED_ARG_DENIED     = -105
	};

	enum OptionSpecType
	{
		OST_BeginList,
		OST_EndList,
		OST_BeginSwitch,
		OST_EndSwitch,
		OST_BeginListNode,
		OST_EndListNode,
		OST_BeginSwitchNode,
		OST_EndSwitchNode,
		OST_Option
	};

	enum
	{
		MAX_LONG_NAME_SIZE = 64,
		MAX_ARGUMENT_SIZE = 1024
	};

	struct OptionSpec
	{
		OptionSpecType  optionType;
		int             int1;
		int             int2;
		char            char1;
		const char     *charp1;
	};

	enum ArgumentPolicy
	{
		AP_None,
		AP_Required,
		AP_Optional
	};

	// ----------------------------------------------------------------------
	// -TF- clients, don't try to use this, it is here for compiler issues

	class Option
	{
	private:

		char            shortName;
		char           *longName;
		ArgumentPolicy  argumentPolicy;

	protected:

		// disabled
		Option(void);
		Option(const Option&);
		Option &operator =(const Option&);

	private:

		Option(char newShortName, const char *newLongName, ArgumentPolicy newArgumentPolicy);

	public:

		~Option(void);

		static bool     isOptionNext(const OptionSpec *optionSpec, int optionSpecCount);
		static Option  *createOption(const OptionSpec **optionSpec, int *optionSpecCount);

		char            getShortName(void) const;
		const char     *getLongName(void) const;
		ArgumentPolicy  getArgumentPolicy(void) const;

		MatchCode       match(void);

	};

	// ----------------------------------------------------------------------

	class Collection
	{

	public:

		virtual ~Collection(void) = 0;
		virtual  MatchCode match(void) const = 0;

		static bool        isCollectionNext(const OptionSpec *optionSpec, int optionSpecCount);
		static Collection *createCollection(const OptionSpec **optionSpec, int *optionSpecCount);
	};

	// ----------------------------------------------------------------------

private:

	class List : public Collection
	{
	friend class Collection;

	private:

		class Node
		{
		private:

			Option       *option;
			Collection   *collection;
			bool          multipleIsAllowed;
			bool          isRequiredNode;
			
			mutable Node *next;

		private:
		
			// disabled
			Node(void);
			Node(const Node&);
			Node &operator =(const Node&);

		public:

			Node(Option *newOption, Collection *newCollection, bool newMultipleIsAllowed, bool newIsRequiredNode);
			~Node(void);

			const Option     *getOption(void) const;
			const Collection *getCollection(void) const;
			bool              getMultipleIsAllowed(void) const;
			bool              getIsRequiredNode(void) const;

			const Node       *getNext(void) const;
			void              setNext(Node *newNext) const;

			MatchCode         match(void) const;

			static bool  isNode(const OptionSpec *optionSpec, int optionCount);
			static Node *createNode(const OptionSpec **optionSpec, int *optionCount);

		};

	private:

		Node        *firstNode;
		OP_ListType  listType;
		int          minimumNodeCount;

	private:

		// disabled
		List(void);
		List(const List&);
		List &operator =(const List&);

	private:

		List(OP_ListType newListType, Node *newFirstNode);
		List(OP_ListType newListType, Node *newFirstNode, int newMinimumNodeCount);

	public:

		virtual           ~List(void);
		virtual MatchCode  match(void) const;

		static bool  isListNext(const OptionSpec *optionSpec, int optionCount);
		static List *createList(const OptionSpec **optionSpec, int *optionCount);

	};

	// ----------------------------------------------------------------------

	class Switch : public Collection
	{
	friend class Collection;
	
	private:

		class Node
		{
		private:

			Option       *option;
			Collection   *collection;
			bool          multipleIsAllowed;
			
			mutable Node *next;

		private:

			// disabled
			Node(void);
			Node(const Node&);
			Node &operator =(const Node&);

		public:

			Node(Option *newOption, Collection *newCollection, bool newMultipleIsAllowed);
			~Node(void);

			const Option     *getOption(void) const;
			const Collection *getCollection(void) const;
			bool              getMultipleIsAllowed(void) const;

			const Node       *getNext(void) const;
			void              setNext(Node *newNext) const;

			MatchCode         match(void) const;

			static bool  isNode(const OptionSpec *optionSpec, int optionCount);
			static Node *createNode(const OptionSpec **optionSpec, int *optionCount);

		};

	private:

		Node *firstNode;
		bool  oneNodeIsRequired;

	private:

		// disabled
		Switch(void);
		Switch(const Switch&);
		Switch &operator =(const Switch&);

	private:

		Switch(Node *newFirstNode, bool newOneNodeIsRequired);

	public:

		virtual           ~Switch(void);
		virtual MatchCode  match(void) const;

		static bool        isSwitchNext(const OptionSpec *optionSpec, int optionCount);
		static Switch     *createSwitch(const OptionSpec **optionSpec, int *optionCount);


	};


	// ----------------------------------------------------------------------

	class OptionTable
	{

	public:

		class Record
		{
		private:

			enum 
			{
				MAX_OCCURRENCE_COUNT = 128
			};

		private:
		
			char            shortName;
			char           *longName;
			int             writeCount;
			int             matchCount;
			char           *stringArray[MAX_OCCURRENCE_COUNT];
			ArgumentPolicy  argumentPolicy;

			mutable Record *next;

		private:

			// disabled
			Record(void);
			Record(const Record&);
			Record &operator =(const Record&);

		public:

			Record(char newShortName, const char *newLongName, ArgumentPolicy newArgumentPolicy);
			~Record(void);

			char          getShortName(void) const;
			const char   *getLongName(void) const;

			void          addOccurrence(const char *newString);
			void          addMatch(void);

			const char   *getString(int occurrenceIndex) const;
			int           getWriteCount(void) const;
			int           getMatchCount(void) const;

			const Record *getNext(void) const;
			Record       *getNext(void);
			void          setNext(Record *newNext) const;

			ArgumentPolicy getArgumentPolicy(void) const;
		};

	private:

		Record *firstRecord;

	private:

		// disabled
		OptionTable(const OptionTable&);
		OptionTable &operator =(const OptionTable&);

	public:

		OptionTable(void);
		~OptionTable(void);

		Record *createOptionRecord(char shortName, const char *longName, ArgumentPolicy newArgumentPolicy);

		Record *findOptionRecord(char shortName) const;
		Record *findOptionRecord(const char *longName) const;

		bool    getAllOptionsMatched(void) const;

	};

	// ----------------------------------------------------------------------

	class Lexer
	{
	public:

		enum TokenType
		{
			TT_Error,
			TT_End,
			TT_DoubleDash,
			TT_ShortOption,
			TT_LongOption,
			TT_Equal,
			TT_Argument
		};

		class Token
		{
		friend class Lexer;

		private:

			TokenType  tokenType;
			char       name[MAX_LONG_NAME_SIZE];
			char       argument[MAX_ARGUMENT_SIZE];

		private:

			// disabled
			Token(const Token&);
			Token &operator =(const Token&);

		public:

			Token(void);

			TokenType   getTokenType(void) const;
			char        getShortName(void) const;
			const char *getLongName(void);
			const char *getArgument(void);
			
		};

	// -TF- so Token can see the TokenType enums --- this is making me puke...
	friend class Token;

	private:

		const char *nextCharacter;

	private:

		// disabled
		Lexer(void);
		Lexer(const Lexer&);
		Lexer &operator =(const Lexer&);

	private:

		void  gobbleWhitespace(void);
		bool  gobbleString(bool isRequired, char *stringBuffer, int bufferSize);

	public:

		explicit Lexer(const char *newBuffer);
		~Lexer(void);

		bool        getNextToken(Token *token);
		const char *getNextCharacter(void);
	
	};

	// ----------------------------------------------------------------------

	// needed so that option can access variable optionTable
	friend class Option;

private:

	static bool         installed;
	static bool         wasParsed;
	static char         buffer[STATIC_BUFFER_SIZE];
	static int          bufferSize;
	static Collection  *treeRoot;
	static OptionTable *optionTable;
	static int          absorbedStringCount;

private:

	static bool       findNextOccurence(char shortName, int *occurrenceIndex);
	static bool       findNextOccurence(const char *longName, int *occurrenceIndex);

	static bool       getOptionArgExists(char shortName, int occurrenceIndex);
	static bool       getOptionArgExists(const char *longName, int occurrenceIndex);

	static void       buildOptionTree(const OptionSpec *specList, int specCount);
	static MatchCode  parseCommandLineBuffer(void); 

public:

	static void install(void);
	static void remove(void);

	static void absorbString(const char *newString);
	static void absorbStrings(const char **stringArray, int stringCount);

	static const char *getPostCommandLineString(void);

	static MatchCode   parseOptions(const OptionSpec *optionTree, int optionSpecCount);

	static bool        getOptionExists(char shortName, int occurrenceIndex = 0);
	static bool        getOptionExists(const char *longName, int occurrenceIndex = 0);

	static int         getOccurrenceCount(char shortName);
	static int         getOccurrenceCount(const char *longName);

	static const char *getOptionString(char shortName, int occurrenceIndex = 0);
	static const char *getOptionString(const char *longName, int occurrenceIndex = 0);
	
	static int         getOptionInt(char shortName, int occurrenceIndex = 0);
	static int         getOptionInt(const char *longName, int occurrenceIndex = 0);

	static int         getUntaggedOccurrenceCount(void);
	static const char *getUntaggedString(int occurrenceIndex = 0);
	static int         getUntaggedInt(int occurrenceIndex = 0);

};

// ======================================================================

inline char CommandLine::Option::getShortName(void) const
{
	return shortName;
}

// ----------------------------------------------------------------------

inline const char *CommandLine::Option::getLongName(void) const
{
	return longName;
}

// ----------------------------------------------------------------------

inline CommandLine::ArgumentPolicy  CommandLine::Option::getArgumentPolicy(void) const
{
	return argumentPolicy;
}

// ======================================================================

inline const CommandLine::Option *CommandLine::List::Node::getOption(void) const
{
	return option;
}

// ----------------------------------------------------------------------

inline const CommandLine::Collection *CommandLine::List::Node::getCollection(void) const
{
	return collection;
}

// ----------------------------------------------------------------------

inline bool CommandLine::List::Node::getMultipleIsAllowed(void) const
{
	return multipleIsAllowed;
}

// ----------------------------------------------------------------------

inline bool CommandLine::List::Node::getIsRequiredNode(void) const
{
	return isRequiredNode;
}

// ----------------------------------------------------------------------

inline const CommandLine::List::Node *CommandLine::List::Node::getNext(void) const
{
	return next;
}

// ----------------------------------------------------------------------

inline void CommandLine::List::Node::setNext(Node *newNext) const
{
	next = newNext;
}

// ======================================================================

inline const CommandLine::Option *CommandLine::Switch::Node::getOption(void) const
{
	return option;
}

// ----------------------------------------------------------------------

inline const CommandLine::Collection *CommandLine::Switch::Node::getCollection(void) const
{
	return collection;
}

// ----------------------------------------------------------------------

inline bool CommandLine::Switch::Node::getMultipleIsAllowed(void) const
{
	return multipleIsAllowed;
}

// ----------------------------------------------------------------------

inline const CommandLine::Switch::Node *CommandLine::Switch::Node::getNext(void) const
{
	return next;
}

// ----------------------------------------------------------------------

inline void CommandLine::Switch::Node::setNext(
	Node *newNext
	) const
{
	next = newNext;
}

// ======================================================================

inline char CommandLine::OptionTable::Record::getShortName(void) const
{
	return shortName;
}

// ----------------------------------------------------------------------

inline const char *CommandLine::OptionTable::Record::getLongName(void) const
{
	return longName;
}

// ----------------------------------------------------------------------

inline void CommandLine::OptionTable::Record::addMatch(void)
{
	DEBUG_FATAL(matchCount >= writeCount, ("match overflow"));
	++matchCount;
}

// ----------------------------------------------------------------------

inline const char *CommandLine::OptionTable::Record::getString(
	int occurrenceIndex
	) const
{
	DEBUG_FATAL(occurrenceIndex >= writeCount, ("index out of range: val/max %d/%d", occurrenceIndex, writeCount-1));
	return stringArray[occurrenceIndex];
}

// ----------------------------------------------------------------------

inline int CommandLine::OptionTable::Record::getWriteCount(void) const
{
	return writeCount;
}

// ----------------------------------------------------------------------

inline int CommandLine::OptionTable::Record::getMatchCount(void) const
{
	return matchCount;
}

// ----------------------------------------------------------------------

inline const CommandLine::OptionTable::Record *CommandLine::OptionTable::Record::getNext(void) const
{
	return next;
}

// ----------------------------------------------------------------------

inline CommandLine::OptionTable::Record *CommandLine::OptionTable::Record::getNext(void)
{
	return next;
}

// ----------------------------------------------------------------------

inline void CommandLine::OptionTable::Record::setNext(
	Record *newNext
	) const
{
	next = newNext;
}

// ----------------------------------------------------------------------

inline CommandLine::ArgumentPolicy  CommandLine::OptionTable::Record::getArgumentPolicy(void) const
{
	return argumentPolicy;
}

// ======================================================================

inline CommandLine::Lexer::TokenType CommandLine::Lexer::Token::getTokenType(void) const
{
	return tokenType;
}

// ----------------------------------------------------------------------

inline char CommandLine::Lexer::Token::getShortName(void) const
{
	DEBUG_FATAL(tokenType != TT_ShortOption, ("attempted to get short name for token type %d", tokenType));
	return name[0];
}

// ----------------------------------------------------------------------

inline const char *CommandLine::Lexer::Token::getLongName(void)
{
	DEBUG_FATAL(tokenType != TT_LongOption, ("attempted to get long name for token type %d", tokenType));
	return name;
}

// ----------------------------------------------------------------------

inline const char *CommandLine::Lexer::Token::getArgument(void)
{
	DEBUG_FATAL(tokenType != TT_Argument, ("attempted to get argument for token type %d", tokenType));
	return argument;
}

// ======================================================================

inline const char *CommandLine::Lexer::getNextCharacter(void)
{
	return nextCharacter;
}

// ----------------------------------------------------------------------

inline void CommandLine::Lexer::gobbleWhitespace(void)
{
	DEBUG_FATAL(!nextCharacter, ("nullptr nextCharacter"));
	while (isspace(*nextCharacter))
		++nextCharacter;
}

// ----------------------------------------------------------------------

inline int CommandLine::getUntaggedOccurrenceCount(void)
{
	return getOccurrenceCount(OP_SNAME_UNTAGGED);
}

// ----------------------------------------------------------------------

inline const char *CommandLine::getUntaggedString(
	int occurrenceIndex
	)
{
	return getOptionString(OP_SNAME_UNTAGGED, occurrenceIndex);
}

// ----------------------------------------------------------------------

inline int CommandLine::getUntaggedInt(
	int occurrenceIndex
	)
{
	return getOptionInt(OP_SNAME_UNTAGGED, occurrenceIndex);
}

// ======================================================================

#endif
