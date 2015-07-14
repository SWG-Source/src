/* A Bison parser, made by GNU Bison 2.5.  */

/* Bison implementation for Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2011 Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.5"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Copy the first part of user declarations.  */

/* Line 268 of yacc.c  */
#line 2 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"

/*-----------------------------------------------------------------------------**
** FILE:  parser.yac                                                           **
** (c) 1998 - Bootprint GTInteractive                                          **
**                                                                             **
** DESCRIPTION: parser for mIFF                                                **
**                                                                             **
** AUTHOR: Hideki Ikeda                                                        **
** HISTORY:                                                                    **
**                                                                             **
** Notes: companion to parser.lex                                              **
**-----------------------------------------------------------------------------*/

/*---------------**
** C declaration **
**---------------*/
#pragma warning (disable: 4005)		/* macro redefinition - bision.simple redefines alloca() */
#pragma warning (disable: 4127)		/* conditional expression is constant - in bison.simple */
#pragma warning (disable: 4131)		/* uses old-style declarator - mostly in bison.simple */
#pragma warning (disable: 4244)		/* possible loss of data due to conversion from one type to another - bision.simple */
#pragma warning (disable: 4701)   /* local variable ('yyval' in first case) may be used without having been initialed */
// #pragma warning (disable: 6311)   /* compiler malloc.h: see previous definition of alloca() */

/* include files */
#include <math.h>			/* for pow() and stuff */
#include <string.h>
#include <stdlib.h>			/* for toupper() */
#include <wchar.h>			/* for wide character (16bit) strings */

#include <stdio.h>

/*----------------------------------------------------------------**
** debug options, turn these on to TEST ONLY!  don't leave these  **
** switch on, it's annoying as hell in console mode!              **
** NOTE: if you turn YYERROR_VERBOSE on, you MUST have YYDEBUG!   **
** IMHO, it's better off using primitive printf() method to debug **
**----------------------------------------------------------------*/
#define	YYERROR_VERBOSE	1
#define	YYDEBUG			1

#undef	YYERROR_VERBOSE
#undef	YYDEBUG

/* external prototype declaration */
extern void MIFFMessage(char *message);		/* found in mIFF.CPP */
extern char * MIFFallocString(int sizeOfString);
extern void MIFFfreeString(char * pointer);
extern int  validateTargetFilename(char *fname, int fnameSize);		/* found mIFF.CPP */
extern void yyerror(char *);
extern int  yylex(void);
extern void setCurrentLineNumber(int lineNum, char * fileName, int mysteryNum);
extern void MIFFSetIFFName(const char *newFileName);
extern void MIFFinsertForm(const char *formName);
extern void MIFFinsertChunk(const char *chunkName);
extern void MIFFinsertChunkData(void * buffer, unsigned bufferSize);
extern int  MIFFloadRawData(char *fname, void * buffer, unsigned maxBufferSize);
extern void MIFFexitChunk(void);
extern void MIFFexitForm(void);
extern unsigned long MIFFgetLabelHash(char *inputStream);

/* local prototype declaration */
void	initGlobalVars(void);
void	checkArgs(void);
void	checkPragmas(void);

void	includeBinary(char *fname);

void	write32(long i32);
void	write16(short i16);
void	write8(char i8);
void	writeU32(unsigned long ui32);
void	writeU16(unsigned short ui16);
void	writeU8(unsigned char u8);
void	writeDouble(double d);
void	writeFloat(float f);
void	writeString(char *s);
void	writeString16(char *s);
void	writeLabelHash(char *s);
void	writeTag(char *tag);
void	writeSize(unsigned long size);
void	writeData(void *dataPtr, unsigned dataSize);

void	initSymTable(void);
long	searchEnumSymbolTable(char *symString);
long	getEnumValue(long index);
void	addEnumSymbol(char *symString, long value);
void	parseESCstring(char *str, char *targetBuffer, int sizeOfTarget);

/*----------------------------------------------**
** Global vars used by all functions and parser **
**----------------------------------------------*/
char	err_msg[256];
int		errorFlag;

/*-----------------------------------------------------------------------------**
** NOTE: this symbol table is ONLY used to construct symbols for enum table!!! **
**-----------------------------------------------------------------------------*/
#define	MAX_SYMBOLS		(1024)			/* total number of symbols it can grow to... */
#define	MAX_SYMCHARS	(128)			/* I label thee insane if you have more the 128 char for your variable! */
struct structEnumSymTableType
{
	char	symbol[MAX_SYMCHARS];
	long	value;
};

struct structEnumSymTableType		symbolEnumTable[MAX_SYMBOLS];
unsigned					currSymIndex = 0;
long						lastValue = -1;
char						id[MAX_SYMCHARS];

/* vars set by pragmas */
#define	MAX_BUFFER_SIZE				(16 * 1024 * 1024)
#define MAX_STRING_SIZE				(512)
char	drive[8];
char	directory[MAX_STRING_SIZE/2];
char	filename[MAX_STRING_SIZE/2];
char	extension[8];
char	outFileName[MAX_STRING_SIZE];
int		usePragmas;

/*------------------------------------------------------------------------**
** The IRONY of these so-called temp-data's that are GLOBAL is            **
** that they aren't temp if functions called within are also using it     **
** This really gives me the creeps and goosbumps!  DOWN with GLOBAL VARS! **
**------------------------------------------------------------------------*/
int		iTemp, jTemp;
char	byteTemp;
short	wordTemp;
char	tempStr[MAX_STRING_SIZE];



/* Line 268 of yacc.c  */
#line 204 "/swg/whitengold/src/engine/client/application/Miff/src/parser.c"

/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     CHAR_LIT = 258,
     STR_LIT = 259,
     IDENTIFIER = 260,
     LIT = 261,
     FLOAT_LIT = 262,
     INT32 = 263,
     INT16 = 264,
     INT8 = 265,
     UINT32 = 266,
     UINT16 = 267,
     UINT8 = 268,
     FLOAT = 269,
     DOUBLE = 270,
     STRING = 271,
     WSTRING = 272,
     LABELHASH = 273,
     FORM = 274,
     CHUNK = 275,
     PRAGMA = 276,
     PRAGMA_DRIVE = 277,
     PRAGMA_DIR = 278,
     PRAGMA_FNAME = 279,
     PRAGMA_EXT = 280,
     ENUMSTRUCT = 281,
     INCLUDESOURCE = 282,
     INCLUDEBIN = 283,
     INCLUDEIFF = 284,
     SIN = 285,
     COS = 286,
     TAN = 287,
     ACOS = 288,
     ASIN = 289,
     ATAN = 290,
     POUND = 291,
     SHIFTRIGHT = 292,
     SHIFTLEFT = 293,
     RAISEDPOWER = 294
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 293 of yacc.c  */
#line 137 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"

	long			ltype;
	double			dtype;
	char			*stype;
	char			chtype;

	int				tokentype;



/* Line 293 of yacc.c  */
#line 290 "/swg/whitengold/src/engine/client/application/Miff/src/parser.c"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


/* Copy the second part of user declarations.  */


/* Line 343 of yacc.c  */
#line 302 "/swg/whitengold/src/engine/client/application/Miff/src/parser.c"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int yyi)
#else
static int
YYID (yyi)
    int yyi;
#endif
{
  return yyi;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)				\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack_alloc, Stack, yysize);			\
	Stack = &yyptr->Stack_alloc;					\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  3
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   192

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  56
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  38
/* YYNRULES -- Number of rules.  */
#define YYNRULES  111
/* YYNRULES -- Number of states.  */
#define YYNSTATES  198

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   294

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,    52,    47,     2,
      54,    55,    50,    45,    43,    46,     2,    51,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    42,
       2,    44,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    49,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    40,    48,    41,    53,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     4,     5,    10,    11,    14,    17,    21,
      25,    29,    33,    37,    41,    46,    47,    51,    55,    58,
      61,    64,    67,    75,    76,    78,    79,    81,    85,    89,
      91,    92,    94,    96,   100,   103,   106,   109,   112,   116,
     118,   121,   124,   128,   129,   132,   135,   138,   141,   144,
     147,   150,   153,   156,   159,   162,   165,   168,   171,   174,
     176,   180,   182,   186,   188,   192,   194,   198,   200,   204,
     206,   210,   212,   216,   218,   222,   226,   230,   234,   238,
     242,   246,   250,   252,   256,   260,   264,   266,   268,   271,
     274,   277,   281,   283,   287,   291,   295,   297,   301,   305,
     307,   309,   311,   314,   317,   321,   326,   331,   336,   341,
     346,   351
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      57,     0,    -1,    -1,    -1,    58,    60,    59,    63,    -1,
      -1,    60,    61,    -1,    60,    62,    -1,    21,    22,     4,
      -1,    21,    23,     4,    -1,    21,    24,     4,    -1,    21,
      25,     4,    -1,    21,     5,     4,    -1,    36,     6,     4,
      -1,    36,     6,     4,     6,    -1,    -1,    63,    73,    74,
      -1,    63,    76,    77,    -1,    63,    64,    -1,    63,    70,
      -1,    63,    72,    -1,    63,    62,    -1,    26,    65,    40,
      66,    41,    68,    42,    -1,    -1,     5,    -1,    -1,    67,
      -1,    66,    43,    67,    -1,     5,    44,    88,    -1,     5,
      -1,    -1,    69,    -1,     5,    -1,    69,    43,     5,    -1,
      27,     4,    -1,    28,     4,    -1,    29,     4,    -1,    19,
       4,    -1,    40,    75,    41,    -1,    63,    -1,    28,     4,
      -1,    20,     4,    -1,    40,    78,    41,    -1,    -1,    78,
      79,    -1,    78,    71,    -1,    78,    62,    -1,    78,    72,
      -1,     8,    80,    -1,     9,    81,    -1,    10,    82,    -1,
      11,    83,    -1,    12,    84,    -1,    13,    85,    -1,    14,
      86,    -1,    15,    87,    -1,    18,     4,    -1,    16,     4,
      -1,    17,     4,    -1,    88,    -1,    80,    43,    88,    -1,
      88,    -1,    81,    43,    88,    -1,    88,    -1,    82,    43,
      88,    -1,    88,    -1,    83,    43,    88,    -1,    88,    -1,
      84,    43,    88,    -1,    88,    -1,    85,    43,    88,    -1,
      91,    -1,    86,    43,    91,    -1,    91,    -1,    87,    43,
      91,    -1,    88,    45,    89,    -1,    88,    46,    89,    -1,
      88,    38,    89,    -1,    88,    37,    89,    -1,    88,    47,
      89,    -1,    88,    48,    89,    -1,    88,    49,    89,    -1,
      89,    -1,    89,    50,    90,    -1,    89,    51,    90,    -1,
      89,    52,    90,    -1,    90,    -1,     6,    -1,    53,    90,
      -1,    46,    90,    -1,    45,    90,    -1,    54,    88,    55,
      -1,     5,    -1,    91,    45,    92,    -1,    91,    46,    92,
      -1,    91,    39,    92,    -1,    92,    -1,    92,    50,    93,
      -1,    92,    51,    93,    -1,    93,    -1,     7,    -1,     6,
      -1,    46,    93,    -1,    45,    93,    -1,    54,    91,    55,
      -1,    30,    54,    91,    55,    -1,    31,    54,    91,    55,
      -1,    32,    54,    91,    55,    -1,    33,    54,    91,    55,
      -1,    34,    54,    91,    55,    -1,    35,    54,    91,    55,
      -1,     5,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   198,   198,   204,   198,   214,   215,   216,   219,   233,
     243,   248,   253,   267,   268,   274,   275,   276,   277,   278,
     279,   280,   286,   289,   290,   293,   294,   295,   298,   302,
     308,   309,   312,   313,   319,   322,   325,   331,   367,   369,
     370,   379,   414,   416,   417,   418,   419,   420,   423,   424,
     425,   426,   427,   428,   429,   430,   431,   434,   439,   446,
     447,   450,   451,   454,   455,   458,   459,   462,   463,   466,
     467,   470,   471,   474,   475,   482,   483,   484,   485,   486,
     487,   488,   489,   492,   493,   494,   495,   498,   499,   500,
     501,   502,   503,   512,   513,   514,   515,   518,   519,   520,
     523,   524,   525,   526,   527,   528,   529,   530,   531,   532,
     533,   534
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "CHAR_LIT", "STR_LIT", "IDENTIFIER",
  "LIT", "FLOAT_LIT", "INT32", "INT16", "INT8", "UINT32", "UINT16",
  "UINT8", "FLOAT", "DOUBLE", "STRING", "WSTRING", "LABELHASH", "FORM",
  "CHUNK", "PRAGMA", "PRAGMA_DRIVE", "PRAGMA_DIR", "PRAGMA_FNAME",
  "PRAGMA_EXT", "ENUMSTRUCT", "INCLUDESOURCE", "INCLUDEBIN", "INCLUDEIFF",
  "SIN", "COS", "TAN", "ACOS", "ASIN", "ATAN", "POUND", "SHIFTRIGHT",
  "SHIFTLEFT", "RAISEDPOWER", "'{'", "'}'", "';'", "','", "'='", "'+'",
  "'-'", "'&'", "'|'", "'^'", "'*'", "'/'", "'%'", "'~'", "'('", "')'",
  "$accept", "mIFFSource", "$@1", "$@2", "preprocessor", "pragma",
  "debugInfo", "body", "enumSection", "enumDeclare", "enumBody",
  "enumAssign", "enumType", "enumList", "includesource", "includebin",
  "includeIFF", "form", "formbody", "formelements", "chunk", "chunkbody",
  "chunkelements", "memalloc", "l32AllocExpr", "l16AllocExpr",
  "l8AllocExpr", "lU32AllocExpr", "lU16AllocExpr", "lU8AllocExpr",
  "fAllocExpr", "dAllocExpr", "exprL", "factorL", "termL", "exprD",
  "factorD", "termD", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     123,   125,    59,    44,    61,    43,    45,    38,   124,    94,
      42,    47,    37,   126,    40,    41
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    56,    58,    59,    57,    60,    60,    60,    61,    61,
      61,    61,    61,    62,    62,    63,    63,    63,    63,    63,
      63,    63,    64,    65,    65,    66,    66,    66,    67,    67,
      68,    68,    69,    69,    70,    71,    72,    73,    74,    75,
      75,    76,    77,    78,    78,    78,    78,    78,    79,    79,
      79,    79,    79,    79,    79,    79,    79,    79,    79,    80,
      80,    81,    81,    82,    82,    83,    83,    84,    84,    85,
      85,    86,    86,    87,    87,    88,    88,    88,    88,    88,
      88,    88,    88,    89,    89,    89,    89,    90,    90,    90,
      90,    90,    90,    91,    91,    91,    91,    92,    92,    92,
      93,    93,    93,    93,    93,    93,    93,    93,    93,    93,
      93,    93
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     0,     4,     0,     2,     2,     3,     3,
       3,     3,     3,     3,     4,     0,     3,     3,     2,     2,
       2,     2,     7,     0,     1,     0,     1,     3,     3,     1,
       0,     1,     1,     3,     2,     2,     2,     2,     3,     1,
       2,     2,     3,     0,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     1,
       3,     1,     3,     1,     3,     1,     3,     1,     3,     1,
       3,     1,     3,     1,     3,     3,     3,     3,     3,     3,
       3,     3,     1,     3,     3,     3,     1,     1,     2,     2,
       2,     3,     1,     3,     3,     3,     1,     3,     3,     1,
       1,     1,     2,     2,     3,     4,     4,     4,     4,     4,
       4,     1
};

/* YYDEFACT[STATE-NAME] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       2,     0,     5,     1,     3,     0,     0,    15,     6,     7,
       0,     0,     0,     0,     0,     0,     4,    12,     8,     9,
      10,    11,    13,     0,     0,    23,     0,     0,    21,    18,
      19,    20,     0,     0,    14,    37,    41,    24,     0,    34,
      36,    15,    16,    43,    17,    25,     0,    39,     0,     0,
      29,     0,    26,    40,    38,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    42,    46,    45,
      47,    44,     0,    30,     0,    92,    87,     0,     0,     0,
       0,    48,    59,    82,    86,    49,    61,    50,    63,    51,
      65,    52,    67,    53,    69,   111,   101,   100,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    54,    71,    96,
      99,    55,    73,    57,    58,    56,    35,    28,    32,     0,
      31,    27,    90,    89,    88,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   103,   102,
       0,     0,     0,     0,     0,     0,     0,     0,    22,     0,
      91,    60,    78,    77,    75,    76,    79,    80,    81,    83,
      84,    85,    62,    64,    66,    68,    70,     0,     0,     0,
       0,     0,     0,   104,    72,    95,    93,    94,    97,    98,
      74,    33,   105,   106,   107,   108,   109,   110
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,     1,     2,     7,     4,     8,    28,    16,    29,    38,
      51,    52,   119,   120,    30,    69,    31,    32,    42,    48,
      33,    44,    49,    71,    81,    85,    87,    89,    91,    93,
     107,   111,    82,    83,    84,   108,   109,   110
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -125
static const yytype_int16 yypact[] =
{
    -125,     6,  -125,  -125,    -5,    12,    35,  -125,  -125,  -125,
      39,    41,    49,    50,    61,    64,   106,  -125,  -125,  -125,
    -125,  -125,    63,    72,    82,    67,    84,   108,  -125,  -125,
    -125,  -125,    75,    87,  -125,  -125,  -125,  -125,   103,  -125,
    -125,   111,  -125,  -125,  -125,   126,   145,   106,   112,    88,
     107,    -1,  -125,  -125,  -125,    21,    21,    21,    21,    21,
      21,    16,    16,   148,   163,   164,   165,  -125,  -125,  -125,
    -125,  -125,    21,   166,   126,  -125,  -125,    21,    21,    21,
      21,   127,   110,   -32,  -125,   129,   110,   130,   110,   131,
     110,   132,   110,   133,   110,  -125,  -125,  -125,   123,   124,
     125,   128,   134,   135,    16,    16,    16,   137,    -7,    58,
    -125,   138,    -7,  -125,  -125,  -125,  -125,   110,  -125,   141,
     142,  -125,  -125,  -125,  -125,    73,    21,    21,    21,    21,
      21,    21,    21,    21,    21,    21,    21,    21,    21,    21,
      21,    21,    16,    16,    16,    16,    16,    16,  -125,  -125,
     -31,    16,    16,    16,    16,    16,    16,    16,  -125,   179,
    -125,   110,   -32,   -32,   -32,   -32,   -32,   -32,   -32,  -125,
    -125,  -125,   110,   110,   110,   110,   110,    18,    32,    68,
      91,    95,    99,  -125,    -7,    58,    58,    58,  -125,  -125,
      -7,  -125,  -125,  -125,  -125,  -125,  -125,  -125
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -125,  -125,  -125,  -125,  -125,  -125,     3,   146,  -125,  -125,
    -125,   116,  -125,  -125,  -125,  -125,   143,  -125,  -125,  -125,
    -125,  -125,  -125,  -125,  -125,  -125,  -125,  -125,  -125,  -125,
    -125,  -125,   -47,    33,   -76,   -62,  -124,  -100
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const yytype_uint8 yytable[] =
{
     112,   122,   123,   124,   148,   149,     3,     9,   152,    86,
      88,    90,    92,    94,   153,   154,     5,    10,   134,   135,
     136,    95,    96,    97,   183,   117,    75,    76,   185,   186,
     187,     6,   152,   125,    11,    12,    13,    14,   153,   154,
      73,    15,    74,    17,   150,    18,    98,    99,   100,   101,
     102,   103,    68,    19,    20,   188,   189,   152,   169,   170,
     171,   104,   105,   153,   154,    21,    77,    78,    22,    34,
     106,   152,    37,   192,    79,    80,    35,   153,   154,   161,
     177,   178,   179,   180,   181,   182,    36,   193,    39,   184,
     172,   173,   174,   175,   176,   190,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,   152,   155,   156,
     127,   128,    40,   153,   154,    41,    66,    27,   129,   130,
     131,   132,   133,   194,     6,    23,    24,    43,   160,    67,
     152,    50,    25,    26,   152,    27,   153,   154,   152,    46,
     153,   154,     6,    45,   153,   154,   195,   127,   128,    53,
     196,    72,   113,    54,   197,   129,   130,   131,   132,   133,
     162,   163,   164,   165,   166,   167,   168,   114,   115,   116,
     126,   118,   137,   138,   139,   140,   141,   142,   143,   144,
     151,   157,   145,   158,   191,   159,     0,    47,   146,   147,
     121,     0,    70
};

#define yypact_value_is_default(yystate) \
  ((yystate) == (-125))

#define yytable_value_is_error(yytable_value) \
  YYID (0)

static const yytype_int16 yycheck[] =
{
      62,    77,    78,    79,   104,   105,     0,     4,    39,    56,
      57,    58,    59,    60,    45,    46,    21,     5,    50,    51,
      52,     5,     6,     7,    55,    72,     5,     6,   152,   153,
     154,    36,    39,    80,    22,    23,    24,    25,    45,    46,
      41,     6,    43,     4,   106,     4,    30,    31,    32,    33,
      34,    35,    49,     4,     4,   155,   156,    39,   134,   135,
     136,    45,    46,    45,    46,     4,    45,    46,     4,     6,
      54,    39,     5,    55,    53,    54,     4,    45,    46,   126,
     142,   143,   144,   145,   146,   147,     4,    55,     4,   151,
     137,   138,   139,   140,   141,   157,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    39,    50,    51,
      37,    38,     4,    45,    46,    40,    28,    29,    45,    46,
      47,    48,    49,    55,    36,    19,    20,    40,    55,    41,
      39,     5,    26,    27,    39,    29,    45,    46,    39,    28,
      45,    46,    36,    40,    45,    46,    55,    37,    38,     4,
      55,    44,     4,    41,    55,    45,    46,    47,    48,    49,
     127,   128,   129,   130,   131,   132,   133,     4,     4,     4,
      43,     5,    43,    43,    43,    43,    43,    54,    54,    54,
      43,    43,    54,    42,     5,    43,    -1,    41,    54,    54,
      74,    -1,    49
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    57,    58,     0,    60,    21,    36,    59,    61,    62,
       5,    22,    23,    24,    25,     6,    63,     4,     4,     4,
       4,     4,     4,    19,    20,    26,    27,    29,    62,    64,
      70,    72,    73,    76,     6,     4,     4,     5,    65,     4,
       4,    40,    74,    40,    77,    40,    28,    63,    75,    78,
       5,    66,    67,     4,    41,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    28,    41,    62,    71,
      72,    79,    44,    41,    43,     5,     6,    45,    46,    53,
      54,    80,    88,    89,    90,    81,    88,    82,    88,    83,
      88,    84,    88,    85,    88,     5,     6,     7,    30,    31,
      32,    33,    34,    35,    45,    46,    54,    86,    91,    92,
      93,    87,    91,     4,     4,     4,     4,    88,     5,    68,
      69,    67,    90,    90,    90,    88,    43,    37,    38,    45,
      46,    47,    48,    49,    50,    51,    52,    43,    43,    43,
      43,    43,    54,    54,    54,    54,    54,    54,    93,    93,
      91,    43,    39,    45,    46,    50,    51,    43,    42,    43,
      55,    88,    89,    89,    89,    89,    89,    89,    89,    90,
      90,    90,    88,    88,    88,    88,    88,    91,    91,    91,
      91,    91,    91,    55,    91,    92,    92,    92,    93,    93,
      91,     5,    55,    55,    55,    55,    55,    55
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  However,
   YYFAIL appears to be in use.  Nevertheless, it is formally deprecated
   in Bison 2.4.2's NEWS entry, where a plan to phase it out is
   discussed.  */

#define YYFAIL		goto yyerrlab
#if defined YYFAIL
  /* This is here to suppress warnings from the GCC cpp's
     -Wunused-macros.  Normally we don't worry about that warning, but
     some users do, and we want to make it easy for users to remove
     YYFAIL uses, which will produce warnings from Bison 2.5.  */
#endif

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* This macro is provided for backward compatibility. */

#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
    yytype_int16 *yybottom;
    yytype_int16 *yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (0, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  YYSIZE_T yysize1;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = 0;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - Assume YYFAIL is not used.  It's too flawed to consider.  See
       <http://lists.gnu.org/archive/html/bison-patches/2009-12/msg00024.html>
       for details.  YYERROR is fine as it does not invoke this
       function.
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                yysize1 = yysize + yytnamerr (0, yytname[yyx]);
                if (! (yysize <= yysize1
                       && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                  return 2;
                yysize = yysize1;
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  yysize1 = yysize + yystrlen (yyformat);
  if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
    return 2;
  yysize = yysize1;

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */
#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */


/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;


/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.

       Refer to the stacks thru separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yytoken = 0;
  yyss = yyssa;
  yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */
  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss_alloc, yyss);
	YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:

/* Line 1806 of yacc.c  */
#line 198 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    {
			/* initialize some global varibles before we start */
			initGlobalVars();
			checkArgs();
		}
    break;

  case 3:

/* Line 1806 of yacc.c  */
#line 204 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    {
			/* take care of any mIFF related preprocessors that we might encounter */
			checkPragmas();
		}
    break;

  case 5:

/* Line 1806 of yacc.c  */
#line 214 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    { /* we don't really have to have preprocessor */}
    break;

  case 8:

/* Line 1806 of yacc.c  */
#line 219 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    {
												if (usePragmas)
												{
													strcpy(drive, (yyvsp[(3) - (3)].stype));
													if (strlen(drive) > 2)
													{
														sprintf(err_msg, "Drive [%s] is not a valid drive [must be in C: format]", drive);
														yyerror(err_msg);
													}
													drive[1] = ':';
													drive[2] = 0;
												}
												MIFFfreeString((yyvsp[(3) - (3)].stype));
											}
    break;

  case 9:

/* Line 1806 of yacc.c  */
#line 233 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    {
												if (usePragmas)
												{
													strcpy(directory, (yyvsp[(3) - (3)].stype));
													if (directory[strlen(directory)] != '\\')
														directory[strlen(directory)] = '\\';
													directory[strlen(directory) + 1] = 0;
												}
												MIFFfreeString((yyvsp[(3) - (3)].stype));
											}
    break;

  case 10:

/* Line 1806 of yacc.c  */
#line 243 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    {
												if (usePragmas)
													strcpy(filename, (yyvsp[(3) - (3)].stype));
												MIFFfreeString((yyvsp[(3) - (3)].stype));
											}
    break;

  case 11:

/* Line 1806 of yacc.c  */
#line 248 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    {
												if (usePragmas)
													strcpy(extension, (yyvsp[(3) - (3)].stype));
												MIFFfreeString((yyvsp[(3) - (3)].stype));
											}
    break;

  case 12:

/* Line 1806 of yacc.c  */
#line 253 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    {
												sprintf(err_msg, "Unknown PRAGMA identifier [%s]", (yyvsp[(2) - (3)].stype)); 
												yyerror(err_msg); 
												MIFFfreeString((yyvsp[(2) - (3)].stype));
												MIFFfreeString((yyvsp[(3) - (3)].stype));
											}
    break;

  case 13:

/* Line 1806 of yacc.c  */
#line 267 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    { setCurrentLineNumber((yyvsp[(2) - (3)].ltype), (yyvsp[(3) - (3)].stype), 0); MIFFfreeString((yyvsp[(3) - (3)].stype));}
    break;

  case 14:

/* Line 1806 of yacc.c  */
#line 268 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    { setCurrentLineNumber((yyvsp[(2) - (4)].ltype), (yyvsp[(3) - (4)].stype), (yyvsp[(4) - (4)].ltype)); MIFFfreeString((yyvsp[(3) - (4)].stype));}
    break;

  case 19:

/* Line 1806 of yacc.c  */
#line 278 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    { /* do nothing... should not exist if went thru the preprocessor */ }
    break;

  case 22:

/* Line 1806 of yacc.c  */
#line 286 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    { /* enumSection */ }
    break;

  case 23:

/* Line 1806 of yacc.c  */
#line 289 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    { /* could have no declaration */ lastValue = -1; }
    break;

  case 24:

/* Line 1806 of yacc.c  */
#line 290 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    { /* enumDeclare: ID */           lastValue = -1; MIFFfreeString((yyvsp[(1) - (1)].stype));}
    break;

  case 25:

/* Line 1806 of yacc.c  */
#line 293 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    { /* enumBody: can be empty list */ }
    break;

  case 26:

/* Line 1806 of yacc.c  */
#line 294 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    { /* enumAssign */ }
    break;

  case 27:

/* Line 1806 of yacc.c  */
#line 295 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    { /* enumAssign, enumBody */ }
    break;

  case 28:

/* Line 1806 of yacc.c  */
#line 298 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    {
											addEnumSymbol((yyvsp[(1) - (3)].stype), (yyvsp[(3) - (3)].ltype));
											MIFFfreeString((yyvsp[(1) - (3)].stype));
										}
    break;

  case 29:

/* Line 1806 of yacc.c  */
#line 302 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    { /* in this case, inc 1 from last count */
											addEnumSymbol((yyvsp[(1) - (1)].stype),  ++lastValue);
											MIFFfreeString((yyvsp[(1) - (1)].stype));
										}
    break;

  case 32:

/* Line 1806 of yacc.c  */
#line 312 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    { /* enumList: just ID */ MIFFfreeString((yyvsp[(1) - (1)].stype));}
    break;

  case 33:

/* Line 1806 of yacc.c  */
#line 313 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    { /* enumList: ID, enumList */ MIFFfreeString((yyvsp[(3) - (3)].stype));}
    break;

  case 34:

/* Line 1806 of yacc.c  */
#line 319 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    { /* do nothing... because CCCP/preprocessor takes care of this*/ MIFFfreeString((yyvsp[(2) - (2)].stype));}
    break;

  case 35:

/* Line 1806 of yacc.c  */
#line 322 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    { includeBinary((yyvsp[(2) - (2)].stype)); MIFFfreeString((yyvsp[(2) - (2)].stype));}
    break;

  case 36:

/* Line 1806 of yacc.c  */
#line 325 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    { includeBinary((yyvsp[(2) - (2)].stype)); MIFFfreeString((yyvsp[(2) - (2)].stype));}
    break;

  case 37:

/* Line 1806 of yacc.c  */
#line 331 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    {
								/* first make sure we have 4 char for FORM name */
								if (strlen((yyvsp[(2) - (2)].stype)) > 4)
								{
									sprintf(err_msg, "FORM name %s greater then 4 char", (yyvsp[(2) - (2)].stype));
									yyerror(err_msg);
								}
								else if (!errorFlag)
								{
									/* pack the string with ' ' (spaces) if less then 4 chars */
									strcpy(tempStr, (yyvsp[(2) - (2)].stype));
									if (strlen(tempStr) < 4)
									{
										/* pack the string */
										for (iTemp = strlen(tempStr);iTemp < 4; iTemp++)
										{
											tempStr[iTemp] = ' ';	/* pack it with space */
										}
									}

									/* let's make sure we don't have a smart ass who wants to do form "FORM" */
									if ((toupper(tempStr[0]) == 'F') &&
										(toupper(tempStr[1]) == 'O') &&
										(toupper(tempStr[2]) == 'R') &&
										(toupper(tempStr[3]) == 'M'))
									{
										yyerror("FORM name CANNOT BE 'FORM'... nice try bozo!");
									}

									/* FORM <size> <tagname> */
									MIFFinsertForm(tempStr);
								}
								MIFFfreeString((yyvsp[(2) - (2)].stype));
							}
    break;

  case 38:

/* Line 1806 of yacc.c  */
#line 367 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    { if (!errorFlag) MIFFexitForm(); }
    break;

  case 39:

/* Line 1806 of yacc.c  */
#line 369 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    { /* recursion of multiple depth in form is allowed */ }
    break;

  case 40:

/* Line 1806 of yacc.c  */
#line 370 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    { 
										yyerror("Found attempt to include binary file inside FORM\nBinary file inclusion ONLY allowed inside a CHUNK!\nError"); 
										MIFFfreeString((yyvsp[(2) - (2)].stype));
									}
    break;

  case 41:

/* Line 1806 of yacc.c  */
#line 379 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    {
								/* first make sure we have 4 char for CHUNK name */
								if (strlen((yyvsp[(2) - (2)].stype)) > 4)
								{
									sprintf(err_msg, "CHUNK name %s greater then 4 char", (yyvsp[(2) - (2)].stype));
									yyerror(err_msg);
								}
								else if (!errorFlag)
								{
									/* pack the string with ' ' (spaces) if less then 4 chars */
									strcpy(tempStr, (yyvsp[(2) - (2)].stype));
									if (strlen(tempStr) < 4)
									{
										/* pack the string */
										for (iTemp = strlen(tempStr);iTemp < 4; iTemp++)
										{
											tempStr[iTemp] = ' ';	/* pack it with space */
										}
									}

									/* let's make sure we don't have a smart ass who wants to do chunk "FORM" */
									if ((toupper(tempStr[0]) == 'F') &&
										(toupper(tempStr[1]) == 'O') &&
										(toupper(tempStr[2]) == 'R') &&
										(toupper(tempStr[3]) == 'M'))
									{
										yyerror("CHUNK name CANNOT BE 'FORM'... nice try bozo!");
									}

									MIFFinsertChunk(tempStr);
								}
								MIFFfreeString((yyvsp[(2) - (2)].stype));
							}
    break;

  case 42:

/* Line 1806 of yacc.c  */
#line 414 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    { if (!errorFlag) MIFFexitChunk(); }
    break;

  case 47:

/* Line 1806 of yacc.c  */
#line 420 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    { yyerror("Found attempt to include IFF (binary) file inside CHUNK\nIFF inclusion ONLY allowed outside a CHUNK!\nError"); }
    break;

  case 48:

/* Line 1806 of yacc.c  */
#line 423 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    { }
    break;

  case 49:

/* Line 1806 of yacc.c  */
#line 424 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    { }
    break;

  case 50:

/* Line 1806 of yacc.c  */
#line 425 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    { }
    break;

  case 51:

/* Line 1806 of yacc.c  */
#line 426 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    { }
    break;

  case 52:

/* Line 1806 of yacc.c  */
#line 427 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    { }
    break;

  case 53:

/* Line 1806 of yacc.c  */
#line 428 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    { }
    break;

  case 54:

/* Line 1806 of yacc.c  */
#line 429 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    { }
    break;

  case 55:

/* Line 1806 of yacc.c  */
#line 430 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    { }
    break;

  case 56:

/* Line 1806 of yacc.c  */
#line 431 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    { 
										writeLabelHash((yyvsp[(2) - (2)].stype));
									}
    break;

  case 57:

/* Line 1806 of yacc.c  */
#line 434 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    { writeString((yyvsp[(2) - (2)].stype));
										/* now, add a NULL termination for this string */
										byteTemp = 0; write8(byteTemp);
										MIFFfreeString((yyvsp[(2) - (2)].stype));
									}
    break;

  case 58:

/* Line 1806 of yacc.c  */
#line 439 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    { writeString16((yyvsp[(2) - (2)].stype));
										/* now, add a NULL termination for this string */
										wordTemp = 0; write16(wordTemp);
										MIFFfreeString((yyvsp[(2) - (2)].stype));
									}
    break;

  case 59:

/* Line 1806 of yacc.c  */
#line 446 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    { write32((yyvsp[(1) - (1)].ltype)); }
    break;

  case 60:

/* Line 1806 of yacc.c  */
#line 447 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    { write32((yyvsp[(3) - (3)].ltype)); }
    break;

  case 61:

/* Line 1806 of yacc.c  */
#line 450 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    { write16((short) (yyvsp[(1) - (1)].ltype)); }
    break;

  case 62:

/* Line 1806 of yacc.c  */
#line 451 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    { write16((short) (yyvsp[(3) - (3)].ltype)); }
    break;

  case 63:

/* Line 1806 of yacc.c  */
#line 454 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    { write8((char) (yyvsp[(1) - (1)].ltype)); }
    break;

  case 64:

/* Line 1806 of yacc.c  */
#line 455 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    { write8((char) (yyvsp[(3) - (3)].ltype)); }
    break;

  case 65:

/* Line 1806 of yacc.c  */
#line 458 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    { writeU32((yyvsp[(1) - (1)].ltype)); }
    break;

  case 66:

/* Line 1806 of yacc.c  */
#line 459 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    { writeU32((yyvsp[(3) - (3)].ltype)); }
    break;

  case 67:

/* Line 1806 of yacc.c  */
#line 462 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    { writeU16((unsigned short) (yyvsp[(1) - (1)].ltype)); }
    break;

  case 68:

/* Line 1806 of yacc.c  */
#line 463 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    { writeU16((unsigned short) (yyvsp[(3) - (3)].ltype)); }
    break;

  case 69:

/* Line 1806 of yacc.c  */
#line 466 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    { writeU8((unsigned char) (yyvsp[(1) - (1)].ltype)); }
    break;

  case 70:

/* Line 1806 of yacc.c  */
#line 467 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    { writeU8((unsigned char) (yyvsp[(3) - (3)].ltype)); }
    break;

  case 71:

/* Line 1806 of yacc.c  */
#line 470 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    { writeFloat((float) (yyvsp[(1) - (1)].dtype)); }
    break;

  case 72:

/* Line 1806 of yacc.c  */
#line 471 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    { writeFloat((float) (yyvsp[(3) - (3)].dtype)); }
    break;

  case 73:

/* Line 1806 of yacc.c  */
#line 474 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    { writeDouble((yyvsp[(1) - (1)].dtype)); }
    break;

  case 74:

/* Line 1806 of yacc.c  */
#line 475 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    { writeDouble((yyvsp[(3) - (3)].dtype)); }
    break;

  case 75:

/* Line 1806 of yacc.c  */
#line 482 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    { (yyval.ltype) = (yyvsp[(1) - (3)].ltype) + (yyvsp[(3) - (3)].ltype);	}
    break;

  case 76:

/* Line 1806 of yacc.c  */
#line 483 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    { (yyval.ltype) = (yyvsp[(1) - (3)].ltype) - (yyvsp[(3) - (3)].ltype);	}
    break;

  case 77:

/* Line 1806 of yacc.c  */
#line 484 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    { (yyval.ltype) = (yyvsp[(1) - (3)].ltype) << (yyvsp[(3) - (3)].ltype);	}
    break;

  case 78:

/* Line 1806 of yacc.c  */
#line 485 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    { (yyval.ltype) = (yyvsp[(1) - (3)].ltype) >> (yyvsp[(3) - (3)].ltype);	}
    break;

  case 79:

/* Line 1806 of yacc.c  */
#line 486 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    { (yyval.ltype) = (yyvsp[(1) - (3)].ltype) & (yyvsp[(3) - (3)].ltype);	}
    break;

  case 80:

/* Line 1806 of yacc.c  */
#line 487 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    { (yyval.ltype) = (yyvsp[(1) - (3)].ltype) | (yyvsp[(3) - (3)].ltype);	}
    break;

  case 81:

/* Line 1806 of yacc.c  */
#line 488 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    { (yyval.ltype) = (yyvsp[(1) - (3)].ltype) ^ (yyvsp[(3) - (3)].ltype);	}
    break;

  case 82:

/* Line 1806 of yacc.c  */
#line 489 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    { (yyval.ltype) = ((yyvsp[(1) - (1)].ltype));				}
    break;

  case 83:

/* Line 1806 of yacc.c  */
#line 492 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    { (yyval.ltype) = (yyvsp[(1) - (3)].ltype) * (yyvsp[(3) - (3)].ltype);	}
    break;

  case 84:

/* Line 1806 of yacc.c  */
#line 493 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    { (yyval.ltype) = (yyvsp[(1) - (3)].ltype) / (yyvsp[(3) - (3)].ltype);	}
    break;

  case 85:

/* Line 1806 of yacc.c  */
#line 494 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    { (yyval.ltype) = (yyvsp[(1) - (3)].ltype) % (yyvsp[(3) - (3)].ltype);	}
    break;

  case 86:

/* Line 1806 of yacc.c  */
#line 495 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    { (yyval.ltype) = ((yyvsp[(1) - (1)].ltype));				}
    break;

  case 87:

/* Line 1806 of yacc.c  */
#line 498 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    { (yyval.ltype) = (yyvsp[(1) - (1)].ltype);	}
    break;

  case 88:

/* Line 1806 of yacc.c  */
#line 499 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    { (yyval.ltype) = ~(yyvsp[(2) - (2)].ltype);	}
    break;

  case 89:

/* Line 1806 of yacc.c  */
#line 500 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    { (yyval.ltype) = -(yyvsp[(2) - (2)].ltype);	}
    break;

  case 90:

/* Line 1806 of yacc.c  */
#line 501 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    { (yyval.ltype) = (yyvsp[(2) - (2)].ltype);	}
    break;

  case 91:

/* Line 1806 of yacc.c  */
#line 502 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    { (yyval.ltype) = ((yyvsp[(2) - (3)].ltype));	}
    break;

  case 92:

/* Line 1806 of yacc.c  */
#line 503 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    { /* assume it's enum symbol */
											(yyval.ltype) = (signed long) getEnumValue(searchEnumSymbolTable((yyvsp[(1) - (1)].stype)));
											MIFFfreeString((yyvsp[(1) - (1)].stype));
										}
    break;

  case 93:

/* Line 1806 of yacc.c  */
#line 512 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    { (yyval.dtype) = (yyvsp[(1) - (3)].dtype) + (yyvsp[(3) - (3)].dtype);	}
    break;

  case 94:

/* Line 1806 of yacc.c  */
#line 513 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    { (yyval.dtype) = (yyvsp[(1) - (3)].dtype) - (yyvsp[(3) - (3)].dtype);	}
    break;

  case 95:

/* Line 1806 of yacc.c  */
#line 514 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    { (yyval.dtype) = pow((yyvsp[(1) - (3)].dtype), (yyvsp[(3) - (3)].dtype));}
    break;

  case 96:

/* Line 1806 of yacc.c  */
#line 515 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    { (yyval.dtype) = ((yyvsp[(1) - (1)].dtype));				}
    break;

  case 97:

/* Line 1806 of yacc.c  */
#line 518 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    { (yyval.dtype) = (yyvsp[(1) - (3)].dtype) * (yyvsp[(3) - (3)].dtype);	}
    break;

  case 98:

/* Line 1806 of yacc.c  */
#line 519 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    { (yyval.dtype) = (yyvsp[(1) - (3)].dtype) / (yyvsp[(3) - (3)].dtype);	}
    break;

  case 99:

/* Line 1806 of yacc.c  */
#line 520 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    { (yyval.dtype) = ((yyvsp[(1) - (1)].dtype));				}
    break;

  case 100:

/* Line 1806 of yacc.c  */
#line 523 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    { (yyval.dtype) = (yyvsp[(1) - (1)].dtype);			}
    break;

  case 101:

/* Line 1806 of yacc.c  */
#line 524 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    { (yyval.dtype) = (double) (yyvsp[(1) - (1)].ltype);	}
    break;

  case 102:

/* Line 1806 of yacc.c  */
#line 525 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    { (yyval.dtype) = -(yyvsp[(2) - (2)].dtype);			}
    break;

  case 103:

/* Line 1806 of yacc.c  */
#line 526 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    { (yyval.dtype) = (yyvsp[(2) - (2)].dtype);			}
    break;

  case 104:

/* Line 1806 of yacc.c  */
#line 527 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    { (yyval.dtype) = ((yyvsp[(2) - (3)].dtype));			}
    break;

  case 105:

/* Line 1806 of yacc.c  */
#line 528 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    { (yyval.dtype) = sin( (yyvsp[(3) - (4)].dtype) );		}
    break;

  case 106:

/* Line 1806 of yacc.c  */
#line 529 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    { (yyval.dtype) = cos( (yyvsp[(3) - (4)].dtype) );		}
    break;

  case 107:

/* Line 1806 of yacc.c  */
#line 530 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    { (yyval.dtype) = tan( (yyvsp[(3) - (4)].dtype) );		}
    break;

  case 108:

/* Line 1806 of yacc.c  */
#line 531 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    { (yyval.dtype) = acos( (yyvsp[(3) - (4)].dtype) );	}
    break;

  case 109:

/* Line 1806 of yacc.c  */
#line 532 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    { (yyval.dtype) = asin( (yyvsp[(3) - (4)].dtype) );	}
    break;

  case 110:

/* Line 1806 of yacc.c  */
#line 533 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    { (yyval.dtype) = atan( (yyvsp[(3) - (4)].dtype) );	}
    break;

  case 111:

/* Line 1806 of yacc.c  */
#line 534 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"
    { /* assume it's enum symbol */
											(yyval.dtype) = (signed long) getEnumValue(searchEnumSymbolTable((yyvsp[(1) - (1)].stype)));
											MIFFfreeString((yyvsp[(1) - (1)].stype));
										}
    break;



/* Line 1806 of yacc.c  */
#line 2511 "/swg/whitengold/src/engine/client/application/Miff/src/parser.c"
      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;


      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  *++yyvsp = yylval;


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined(yyoverflow) || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}



/* Line 2067 of yacc.c  */
#line 540 "/swg/whitengold/src/engine/client/application/Miff/src/linux/parser.yac"


/*-------------------------------------------**
** and now... the supporting C functions...  **
**-------------------------------------------*/
void initGlobalVars(void)
{
	/* assign defaults */
	drive[0] = 0;
	directory[0] = 0;
	filename[0] = 0;
	extension[0] = 0;
	err_msg[0] = 0;
	usePragmas = 1;				/* default to #pragmas enabled because as a stand-alone .YAC, we have no knowledge of outFileName */

	errorFlag = 0;

	initSymTable();
}

void checkPragmas(void)
{
	int  indexOriginal, indexDest;
	char _tempStr[512];

	if (usePragmas)
	{
		/* check after pre processor if output target is still NULL */
		if (!drive[0] && !errorFlag)
		{
			yyerror("Drive pragma not defined");
			errorFlag = 1;
		}
		if (!directory[0] && !errorFlag)
		{
			yyerror("Directory pragma not defined");
			errorFlag = 1;
		}
		if (!filename[0] && !errorFlag)
		{
			yyerror("Filename pragma not defined");
			errorFlag = 1;
		}
		if (!extension[0] && !errorFlag)
		{
			yyerror("Extension pragma not defined");
			errorFlag = 1;
		}

		if (!errorFlag)
		{
			/* create an output file */
			strcpy(_tempStr, directory);   /* copy directory to _tempStr because we'll be messing with directory */
			for (indexOriginal = 0, indexDest = 0; indexOriginal < (int) strlen(_tempStr); indexOriginal++)
			{
				/* search for double-slashes and convert it to single slash */
				if ((_tempStr[indexOriginal] == '\\') && (_tempStr[indexOriginal+1] == '\\'))
				{
					directory[indexDest] = '\\';
					indexOriginal++;
				}
				else
					directory[indexDest] = _tempStr[indexOriginal];  /* copy current position of _tempStr to directory */
				indexDest++;
				directory[indexDest] = 0;  /* force NULL termination */
			}
			sprintf(outFileName, "%s%s%s%s", drive, directory, filename, extension);
		}
	}	/* if usePragmas */

	MIFFSetIFFName(outFileName);    /* tell mIFF we want to use this filename instead of whatever it has! */
}

/*--------------------------------------------------------------------------------------**
** This function is called early in the process to find out if usePragma flag was set   **
** in the command line or not.  If it was, mIFF already has the outfile and will stuff  **
** it into the outFileName data pool.  Even if mIFF had an idea of what the output file **
** name is, if usePragma flag returned said it is true, then #pragas in the mIFF source **
** is used instead and overrides whatever file name it has returned.                    **
** see usePragmas() for more details.                                                   **
**--------------------------------------------------------------------------------------*/
void checkArgs(void)
{
	/* request an external function (found in either mIFF.CPP or mIFFView.CPP) to see if pragmas and filename was set... */
	usePragmas = validateTargetFilename(outFileName, sizeof(outFileName));
}

/*----------------------------**
** Write to FILE functions... **
**----------------------------*/
void write32(long i32)
{
	MIFFinsertChunkData(&i32, sizeof(long));
}

void write16(short i16)
{
	MIFFinsertChunkData(&i16, sizeof(short));
}

void write8(char i8)
{
	MIFFinsertChunkData(&i8, sizeof(char));
}

void writeU32(unsigned long ui32)
{
	MIFFinsertChunkData(&ui32, sizeof(long));
}

void writeU16(unsigned short ui16)
{
	MIFFinsertChunkData(&ui16, sizeof(short));
}

void writeU8(unsigned char ui8)
{
	MIFFinsertChunkData(&ui8, sizeof(char));
}

void writeDouble(double d)
{
	MIFFinsertChunkData(&d, sizeof(double));
}

void writeFloat(float f)
{
	MIFFinsertChunkData(&f, sizeof(float));
}

void writeString(char *s)
{
	char  tempS[MAX_STRING_SIZE];
	parseESCstring(s, tempS, MAX_STRING_SIZE);

	MIFFinsertChunkData(tempS, strlen(tempS));
}

void writeString16(char *s)
{
	char  tempS[MAX_STRING_SIZE];
	int     charCount = 0;
	wchar_t wtempStr[512];			/* just to be on the safe side, allocating huge array... */

	parseESCstring(s, tempS, MAX_STRING_SIZE);

	/* make sure string length is less then the allocated wchar size */
	if ((strlen(tempS) * sizeof(wchar_t)) > (512* sizeof(wchar_t)))
		yyerror("wstring: 16bit string too long to handle in buffer!\n");
	else
	{
		/* call MultiByteString to WideCharString function */
		charCount = mbstowcs(wtempStr, tempS, strlen(tempS));
		writeData(wtempStr, charCount * sizeof(wchar_t));
	}
}

void writeLabelHash(char *s)
{
	writeU32(MIFFgetLabelHash(s));
}

/* search from escape string such as \n and convert it to actual byte */
void parseESCstring(char *str, char *targetBuffer, int sizeOfTarget)
{
	char  *sPtr = str;
	int   strIndex = 0;
	char  numString[32];  /* hopefully, never go over 3 char i.e. \x0FF */
	int   numIndex = 0;
	int   tempNum = 0;
	int   exitParser = 0;
	int   loopFlag = 0;

	/* memory hog but cute way to convert ascii hex to number */
	int   hexTable[256];
	memset(hexTable, 0xFF, 256);
	hexTable['0'] = 0x00;
	hexTable['1'] = 0x01;
	hexTable['2'] = 0x02;
	hexTable['3'] = 0x03;
	hexTable['4'] = 0x04;
	hexTable['5'] = 0x05;
	hexTable['6'] = 0x06;
	hexTable['7'] = 0x07;
	hexTable['8'] = 0x08;
	hexTable['9'] = 0x09;
	hexTable['A'] = 0x0A;		hexTable['a'] = 0x0A;
	hexTable['B'] = 0x0B;		hexTable['b'] = 0x0B;
	hexTable['C'] = 0x0C;		hexTable['c'] = 0x0C;
	hexTable['D'] = 0x0D;		hexTable['d'] = 0x0D;
	hexTable['E'] = 0x0E;		hexTable['e'] = 0x0E;
	hexTable['F'] = 0x0F;		hexTable['f'] = 0x0F;

	memset(targetBuffer, 0, sizeOfTarget);
	while ((*sPtr) && !exitParser)		/* assume we can go until NULL termination */
	{
		/* check for escape sequences */
		if (*sPtr == '\\')
		{
			sPtr++;  /* check out next character */
			switch (tolower(*sPtr))
			{
				case 'a':   /* BELL */
				{
					targetBuffer[strIndex++] = '\a';
					break;
				}

				case 'b':   /* BACKSPACE */
				{
					targetBuffer[strIndex++] = '\b';
					break;
				}

				case 'f':   /* FORMFEED */
				{
					targetBuffer[strIndex++] = '\a';
					break;
				}

				case 'n':   /* NEWLINE */
				{
					targetBuffer[strIndex++] = '\n';
					break;
				}

				case 'r':   /* CARRIAGE RETURN */
				{
					targetBuffer[strIndex++] = '\r';
					break;
				}

				case 't':   /* TAB */
				{
					targetBuffer[strIndex++] = '\t';
					break;
				}

				case 'v':   /* VERTICAL TAB */
				{
					targetBuffer[strIndex++] = '\v';
					break;
				}

				case '\'':   /* SINGLE QUOTE */
				{
					targetBuffer[strIndex++] = '\'';
					break;
				}

				case '\"':   /* DOUBLE QUOTE */
				{
					targetBuffer[strIndex++] = '\"';
					break;
				}

				case '\\':   /* BACKSLASH */
				{
					targetBuffer[strIndex++] = '\\';
					break;
				}

				case '?':   /* LITERAL QUESTION MARK */
				{
					targetBuffer[strIndex++] = '\?';
					break;
				}

				case '0':   /* ASCII octal */
				case '1':   /* ASCII octal */
				case '2':   /* ASCII octal */
				case '3':   /* ASCII octal */
				case '4':   /* ASCII octal */
				case '5':   /* ASCII octal */
				case '6':   /* ASCII octal */
				case '7':   /* ASCII octal */
				{
					/* read until non-digit encountered - if octal value is greater then \377 (400 or more) it is bigger the 255! */
					numIndex = 0;

					while ((*sPtr >= '0') && (*sPtr <= '7') && (*sPtr))
					{
						numString[numIndex++] = *sPtr;
						sPtr++;
						if (numIndex > 3)
						{
							sprintf(err_msg, "Escape sequence Octal numbers greater then\noctal o400 (256 decimal)! [more then 3 digits]", *sPtr);
							yyerror(err_msg);
							exitParser = 1;
						}
					}

					/* now we should have string of octal number in numString */
					if (!exitParser)
					{
						if (numIndex == 3)
							tempNum = (hexTable[numString[0]] * 64) + (hexTable[numString[1]] * 8) + hexTable[numString[2]];
						else if (numIndex == 2)
							tempNum = (hexTable[numString[0]] * 8) + hexTable[numString[1]];
						else
							tempNum = hexTable[numString[0]];

						if (tempNum > 255)
						{
							sprintf(err_msg, "Escape sequence Octal numbers greater then\noctal o400 (256 decimal)!", tempNum);
							yyerror(err_msg);
							exitParser = 1;
						}

						if (!exitParser)
							targetBuffer[strIndex++] = tempNum;
					}
					break;
				}

				case 'x':   /* ASCII hex */
				{
					/* we have to make sure the hex value is less then 256! */
					numIndex = 0;
					loopFlag = 1;
					sPtr++;  /* skip the 'x' */

					while ((0xFF != hexTable[*sPtr]) && (*sPtr))
					{
						numString[numIndex++] = *sPtr;
						sPtr++;
						if (numIndex > 3)
						{
							sprintf(err_msg, "Escape sequence HEX numbers greater then 0x100\n(256 decimal)! [more then 3 digits]", *sPtr);
							yyerror(err_msg);
							exitParser = 1;
						}
					}

					if (!exitParser)
					{
						if (numIndex == 3)
							tempNum = (hexTable[numString[0]] * 0x100) + (hexTable[numString[1]] * 0x10) + hexTable[numString[2]];
						else if (numIndex == 2)
							tempNum = (hexTable[numString[0]] * 0x10) + hexTable[numString[1]];
						else
							tempNum = hexTable[numString[0]];
					}

					if (tempNum > 255)
					{
						sprintf(err_msg, "Escape sequence HEX numbers greater then 0x100\n(256 decimal)!", *sPtr);
						yyerror(err_msg);
						exitParser = 1;
					}

					if (!exitParser)
						targetBuffer[strIndex++] = tempNum;

					break;
				}

				case '8':
				case '9':
				{
					/* they tried to do octal mode, but 8 and 9 is not in the definition of octal */
					yyerror("Attempted to enter escape sequence with non-octal value");
					exitParser = 1;
					break;
				}

				default:
					sprintf(err_msg, "Unknown ESCape sequence \\%c found in string.\n", *sPtr);
					yyerror(err_msg);
					exitParser = 1;
					break;
			}
		}
		else
			targetBuffer[strIndex++] = *sPtr;
		sPtr++;
	}
}	


void writeData(void *dataPtr, unsigned dataSize)
{
	MIFFinsertChunkData(dataPtr, dataSize);
}


/* NOTE: includeBinary modifies fsize for the caller to access for adjusting the chunk size */
void includeBinary(char *fname)
{
	char	buffer[MAX_BUFFER_SIZE+1];
	int		fsize = 0;

	fsize = MIFFloadRawData(fname, buffer, MAX_BUFFER_SIZE);
	MIFFinsertChunkData(buffer, fsize);
}

/*--------------------------------------------------------------------------------**
** Following functions below are all used for constructing, adding, and searching **
** the symbol table created by enum keyword.  It is at this moment, set in a way  **
** that if two identical symbols are added to the list, it will use the first     **
** symbol added to the list and ignores the rest (because of forward search)      **
**--------------------------------------------------------------------------------*/
void initSymTable(void)
{
	memset(symbolEnumTable, 0, sizeof(struct structEnumSymTableType) * MAX_SYMBOLS);
	currSymIndex = 0;
	lastValue = -1;
}

/* Searches thru the symbol table and returns the index */
long searchEnumSymbolTable(char *symString)
{
	long index = 0;
	int found = 0;
	while ((0 != symbolEnumTable[index].symbol[0]) && !found)
	{

		if (0 == strcmp(symbolEnumTable[index].symbol, symString))
		{
			found = 1;
			break;
		}
		else
			index++;
	}

	if (!found)
	{
		index = -1;
		sprintf(err_msg, "Undefined symbol %s", symString);
		yyerror(err_msg);
	}

	return(index);
}

long getEnumValue(long index)
{
	if (index >= 0)
		return(symbolEnumTable[index].value);
	return(-1);
}

void addEnumSymbol(char *symString, long value)
{
	if (MAX_SYMCHARS < strlen(symString))
	{
		/* somebody insane decided to use variable longer then max size! */
		sprintf(err_msg, "%s is longer then %d characters! [value: %d]", symString, MAX_SYMCHARS, value);
		yyerror(err_msg);
	}
	else
	{
		strcpy(symbolEnumTable[currSymIndex].symbol, symString);
		symbolEnumTable[currSymIndex].value = value;
		lastValue = value;
		currSymIndex++;
		if (MAX_SYMBOLS < currSymIndex)
		{
			sprintf(err_msg, "Symbol table reached maximum size of %d", MAX_SYMBOLS);
			yyerror(err_msg);
		}
	}
}

