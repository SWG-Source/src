//===========================================================================
//
//    FILENAME: mIFF.cpp [C:\Projects\new\tools\src\miff\src\]
//   COPYRIGHT: (C) 1999 BY Bootprint Entertainment
//
// DESCRIPTION: make IFF (Console version)
//      AUTHOR: Hideki Ikeda
//        DATE: 1/07/99 12:57:20 PM
//
//     HISTORY: 1/07/99 [HAI] - File created
//            : 1/07/99 [HAI] - v1.0 introductory version
//            : 1/12/99 [HAI] - v1.1 switched from DOS to Engine library
//            :               - first attempt was to setup the main entry
//            :                 point via ConsoleEntryPoint() via callback
//            : 1/29/99 [HAI] - changed the parameter in MIFFMessage to allow
//            :                 output even in non-verbose mode (for error
//            :                 message purpose.
//            : 05/07/99 [HAI]- added MIFFallocString() and MIFFfreeString()
//            :                 to work with memory manager.  they are allocated
//            :                 in the lexical analyzer for IDENTIFIERS and STR_LIT
//            :                 deleted after parser parses the rule.
//
//    FUNCTION: main()
//            : evaluateArgs()
//            : help()
//            : handleError()
//            : preprocessSource()
//            : MIFFMessage()
//            : callbackFunction()
//
//===========================================================================

//========================================================== include files ==
#include "sharedFoundation/FirstSharedFoundation.h"

#include "sharedCompression/SetupSharedCompression.h"
#include "sharedDebug/SetupSharedDebug.h"
#include "sharedFile/SetupSharedFile.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/CommandLine.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/Os.h"
#include "sharedFoundation/SetupSharedFoundation.h"

#include "sharedThread/SetupSharedThread.h"

#include "InputFileHandler.h"
#include "OutputFileHandler.h"

#include <string.h>					// for memset()
#include <stdio.h>					// FILE stuff
#include <direct.h>					// for getcwd()
#include <stdlib.h>					// for tolower()
#include <process.h>				// for system()

//================================================= static vars assignment ==
const int	entryPointVersion = 1;				// constantly check DataEntryPoint.h to see if this value has changed

OutputFileHandler   *outfileHandler = NULL;
const int   bufferSize = 16 * 1024 * 1024;
const int   maxStringSize = 256;
const char  version[] = "1.3 September 18, 2000";

// vars set by pragmas or via command line
char        drive[4];                               // should be no more then 2 char "C:"
char        directory[maxStringSize];
char        filename[maxStringSize];
char        extension[8];                           // we'll truncate if the extension is more then 8 chars...
char        inFileName[maxStringSize];

// switches to be sent to mIFF Compiler
char        sourceBuffer[bufferSize];
char        outFileName[(maxStringSize * 2) + 8];   // x2 to combine filename, dir, and ext
bool        usePragma = false;
bool        useCCCP = false;
bool        verboseMode = false;										// default to non-verbose mode
bool        debugMode = false;                      // set this on and the preprocessed source file (mIFF.$$$) won't be deleted

static bool runningUnderNT;

enum errorType {
		ERR_FILENOTFOUND    = -1,
		ERR_ARGSTOOFEW      = -2,
		ERR_BUFFERTOOSMALL  = -3,
		ERR_UNKNOWNDIR      = -4,
		ERR_PREPROCESS      = -5,
		ERR_MULTIPLEINFILE  = -6,
		ERR_PARSER          = -7,
		ERR_ENGINE          = -8,

		ERR_HELPREQUEST     = -9,
		ERR_OPTIONS         = -10,

		ERR_WRITEERROR      = -11,

		ERR_NONE = 0
	};
char        err_msg[512];
errorType   errorFlag = ERR_NONE;            // assume no error (default)


// long and short name definitions for command line options

static const char * const LNAME_HELP          = "help";
static const char * const LNAME_INPUT_FILE    = "inputfile";
static const char * const LNAME_OUTPUT_FILE   = "outputfile";
static const char * const LNAME_PRAGMA_TARGET = "pragmatarget";
static const char * const LNAME_CCCP          = "cccp";
static const char * const LNAME_VERBOSE       = "verbose";
static const char * const LNAME_DEBUG         = "debug";

static const char         SNAME_HELP          = 'h';
static const char         SNAME_INPUT_FILE    = 'i';
static const char         SNAME_OUTPUT_FILE   = 'o';
static const char         SNAME_PRAGMA_TARGET = 'p';
static const char         SNAME_CCCP          = 'c';
static const char         SNAME_VERBOSE       = 'v';
static const char         SNAME_DEBUG         = 'd';

// following is the command line option spec tree needed for command line processing
static CommandLine::OptionSpec optionSpecArray[] =
{
	OP_BEGIN_SWITCH(OP_NODE_REQUIRED),

		// help
		OP_SINGLE_SWITCH_NODE(SNAME_HELP, LNAME_HELP, OP_ARG_NONE, OP_MULTIPLE_DENIED),

		// real options
		OP_BEGIN_SWITCH_NODE(OP_MULTIPLE_DENIED),
			OP_BEGIN_LIST(),
				// input filename required
				OP_SINGLE_LIST_NODE(SNAME_INPUT_FILE, LNAME_INPUT_FILE, OP_ARG_REQUIRED, OP_MULTIPLE_DENIED,  OP_NODE_REQUIRED),

				// optional, mutually exclusive output file specification options
				// if none specified, generate derive output filename from input filename
				OP_BEGIN_LIST_NODE(OP_MULTIPLE_DENIED, OP_NODE_OPTIONAL),
					OP_BEGIN_SWITCH(OP_NODE_OPTIONAL),
						// specify output filename on command line
						OP_SINGLE_SWITCH_NODE(SNAME_OUTPUT_FILE, LNAME_OUTPUT_FILE, OP_ARG_REQUIRED, OP_MULTIPLE_DENIED),
						
						// use pragma target for output filename
						OP_SINGLE_SWITCH_NODE(SNAME_PRAGMA_TARGET, LNAME_PRAGMA_TARGET, OP_ARG_NONE, OP_MULTIPLE_DENIED),
					OP_END_SWITCH(),
				OP_END_LIST_NODE(),

				// if specified, use cccp instead of cpp
				OP_SINGLE_LIST_NODE(SNAME_CCCP,    LNAME_CCCP,    OP_ARG_NONE, OP_MULTIPLE_DENIED, OP_NODE_OPTIONAL),

				// if specified, be verbose
				OP_SINGLE_LIST_NODE(SNAME_VERBOSE, LNAME_VERBOSE, OP_ARG_NONE, OP_MULTIPLE_DENIED, OP_NODE_OPTIONAL),

				// if specified, enter debug info
				OP_SINGLE_LIST_NODE(SNAME_DEBUG,   LNAME_DEBUG,   OP_ARG_NONE, OP_MULTIPLE_DENIED, OP_NODE_OPTIONAL),
			OP_END_LIST(),
		OP_END_SWITCH_NODE(),

	OP_END_SWITCH()
};
static const int optionSpecCount = sizeof(optionSpecArray) / sizeof(optionSpecArray[0]);

//================================================= function prototypes ==
int main(int argc, char *argv[]);
static errorType evaluateArgs(void);
static void help(void);
static void handleError(errorType error);
static int preprocessSource(char *sourceName);
static void callbackFunction(void);
static errorType loadInputToBuffer(void *destAddr, int maxBufferSize);

// functions called by parser.yac and parser.lex
extern "C" void MIFFMessage(char *msg, int forceOut);
extern "C" void MIFFSetError(void);
extern "C" void MIFFSetIFFName(const char *newFileName);
extern "C" void MIFFinsertForm(const char *formName);
extern "C" void MIFFinsertChunk(const char *chunkName);
extern "C" void MIFFinsertChunkData(void * buffer, unsigned bufferSize);
extern "C" int  MIFFloadRawData(char *fname, void * buffer, unsigned maxBufferSize);
extern "C" void MIFFexitChunk(void);
extern "C" void MIFFexitForm(void);
extern "C" unsigned long MIFFgetLabelHash(char *inputStream);

// external functions found in parser.lex file
extern "C" void MIFFCompile(char *inputStream, char *inputFname);
extern "C" void MIFFCompileInit(char *inputStream, char *inputFname);

//---------------------------------------------------------------------------
// main entry point from console call
//
// Return Value:
//   errorType - see enumeration;  0 if no errors
//
// Remarks:
//   
//
// See Also:
//   
//
// Revisions and History:
//   1/07/99 [HAI] - created
//
int main(	int argc,				// number of args in commandline
				char * argv[]			// list of pointers to strings
				)
{
	memset(sourceBuffer, 0, bufferSize);

	SetupSharedThread::install();
	SetupSharedDebug::install(4096);

	SetupSharedFoundation::Data SetupSharedFoundationData (SetupSharedFoundation::Data::D_console);
	SetupSharedFoundationData.useWindowHandle  = false;
	SetupSharedFoundationData.argc = argc;
	SetupSharedFoundationData.argv = argv;	
	SetupSharedFoundationData.demoMode = true;
	SetupSharedFoundation::install (SetupSharedFoundationData);

	SetupSharedCompression::install();

	SetupSharedFile::install(false);

	TreeFile::addSearchAbsolute(0);
	TreeFile::addSearchPath (".", 0);

	SetupSharedFoundation::callbackWithExceptionHandling(callbackFunction);
	SetupSharedFoundation::remove();

	SetupSharedThread::remove();
	return static_cast<int> (errorFlag);
}

//---------------------------------------------------------------------------
// callback function for Engine's console entry point
//
// Return Value:
//   none
//
// Remarks:
//   this is like a substitute of main()
//
// See Also:
//    
//
// Revisions and History:
//   1/12/99 [HAI] - created
//
static void callbackFunction(void)
{
	outfileHandler = NULL;

#ifdef WIN32

	// check if we're running under NT
	OSVERSIONINFO osInfo;
	Zero(osInfo);

	osInfo.dwOSVersionInfoSize  = sizeof(osInfo);
	const BOOL getVersionResult = GetVersionEx(&osInfo);
	if (getVersionResult)
		runningUnderNT = (osInfo.dwPlatformId == VER_PLATFORM_WIN32_NT);
	if (runningUnderNT)
		DEBUG_REPORT_LOG(true, ("MIFF: running under Windows NT platform\n"));
	else
		DEBUG_REPORT_LOG(true, ("MIFF: running under non-NT Windows platform\n"));

#endif

	errorFlag = evaluateArgs();
	if (ERR_NONE == errorFlag)
	{
		outfileHandler = new OutputFileHandler(outFileName);
		MIFFCompile(sourceBuffer, inFileName);
	}
	else
		handleError(errorFlag);

	if (outfileHandler)
	{
		// only write output IF there was no error
		if (ERR_NONE == errorFlag)
		{
			if (!outfileHandler->writeBuffer())
			{
				fprintf(stderr, "MIFF: failed to write output file \"%s\"\n", outFileName);
				errorFlag = ERR_WRITEERROR;
			}
		}
		delete outfileHandler;
	}
}

//---------------------------------------------------------------------------
// Evaluates the command line and sets up the environment variables required for mIFF to function
//
// Return Value:
//   errorType
//
// Remarks:
//   argc's and argv's are substituted with CommandLine::functions()
//
// See Also:
//   
//
// Revisions and History:
//   1/07/99 [HAI] - created
//
static errorType evaluateArgs(void)
{
	errorType  retVal = ERR_NONE;

	// parse the commandline
	const CommandLine::MatchCode mc = CommandLine::parseOptions(optionSpecArray, optionSpecCount);
	if (mc != CommandLine::MC_MATCH)
	{
		// -TF- add call to retrieve command line error buffer for display (as soon as it is written!)
		printf("WARNING: usage error detected, printing help.\n");
		help();
		return ERR_OPTIONS;
	}
	else if (CommandLine::getOccurrenceCount(SNAME_HELP))
	{
		// user specified help
		help();
		retVal = ERR_HELPREQUEST;
		return(retVal);
	}

	// at this point, we can assume a valid combination of options has been specified on the commandline


	// setup input filename
	strcpy(inFileName, CommandLine::getOptionString(SNAME_INPUT_FILE));


	// handle output filename spec
	if (CommandLine::getOccurrenceCount(SNAME_OUTPUT_FILE))
	{
		strcpy(outFileName, CommandLine::getOptionString(SNAME_OUTPUT_FILE));
	}
	else if (CommandLine::getOccurrenceCount(SNAME_PRAGMA_TARGET))
	{
		// use pragma target within iff source for output filename
		usePragma = true;
	}
	else
	{
		// no output option specified on commandline, derive from input filename
		char *terminator;

		// start with input file pathname
		strcpy(outFileName, inFileName);

		// try to terminate at rightmost '.'
		terminator = strrchr(outFileName, '.');
		if (terminator)
			*terminator = 0;

		// append the default iff extension
		strcat(outFileName, ".iff");
	}


	// handle options (get them out of the way, as we use them later)
	useCCCP     = (CommandLine::getOccurrenceCount(SNAME_CCCP) != 0);
	verboseMode = (CommandLine::getOccurrenceCount(SNAME_VERBOSE) != 0);
	debugMode   = (CommandLine::getOccurrenceCount(SNAME_DEBUG) != 0);


	// preprocess the input file
	if (0 == preprocessSource(inFileName))
	{
		if (verboseMode)
		{
			sprintf(err_msg,"Now compiling %s...\n", inFileName);
			MIFFMessage(err_msg, 0);
		}
		if (ERR_NONE == retVal)
			retVal = loadInputToBuffer(sourceBuffer, bufferSize);
	}
	else
	{
		// preprocessSource returned an error...
		retVal = ERR_PREPROCESS;
	}
	if (retVal != ERR_NONE)
		return retVal;

	return retVal;

#if 0

	errorType retVal = ERR_NONE;  // assume no error has been found
	bool		outPathUsed = false;	// flag to monitor if -o flag was used, if so, we can ignore -d, -p, -e, -f
	bool		inFileEntered = false;
	int			argc = CommandLine::getPlainCount();

	// get default values from DOS
	char		currentDir[maxStringSize];
	if (NULL == getcwd(currentDir, maxStringSize))		// get current working directory
	{
		retVal = ERR_UNKNOWNDIR;
		return(retVal);
	}
	drive[0] = currentDir[0];				// drive letter
	drive[1] = 0;							// and null terminate it
	strcpy(extension, "IFF");				// default to uppercase .IFF
	strcpy(directory, &currentDir[2]);		// get everything after the Drive: including the first backslash
	filename[0] = 0;

	// see specs.txt for requests
	// scan for any argv's that has '-' in the argv[n][0]'s character
	for (int index = 0; index < argc; index++)	// note: if using argv[] rather then CommandLine::getPlainString() then start with 1 rather then 0
	{
		if ('-' == CommandLine::getPlainString(index)[0])
		{
			// we've found a parameter switch
			switch (tolower(CommandLine::getPlainString(index)[1]))		// assume non case sensitive switches
			{
				case 'i':			// install via #pragma
				{
					usePragma = true;
					break;
				}

				case 'c':			// use CCCP instead of CPP
					useCCCP = true;
					break;

				case 'v':			// don't show any debug message
					verboseMode = true;
					break;

				case '$':
					debugMode = true;
					break;

				case 'o':			// target output file name and path (complete path)
				{
					index++;			// next param
					outPathUsed = true;
					strcpy(outFileName, CommandLine::getPlainString(index));
					break;
					
				}

				case 'd':			// target drive letter (-p must be present)
				{
					if (!outPathUsed)
					{
						index++;			// next param
						strcpy(drive, CommandLine::getPlainString(index));
					}
					else
						index++;	// skip the drive letter arg that SHOULD follow the -d option
					break;
				}

				case 'p':			// target pathname
				{
					if (!outPathUsed)
					{
						index++;			// next param
						strcpy(directory, CommandLine::getPlainString(index));
					}
					else
						index++;	// skip the pathname arg that follows the -p option
					break;
				}

				case 'f':			// target filename
				{
					if (!outPathUsed)
					{
						index++;			// next param
						strcpy(filename, CommandLine::getPlainString(index));
					}
					else
						index++;	// skip the filename arg that follows the -f
					break;
				}

				case 'e':			// target extension
				{
					if (!outPathUsed)
					{
						index++;			// next param
						strcpy(extension, CommandLine::getPlainString(index));
					}
					else
						index++;	// skip the extension arg that follows the -e
					break;
				}

				case 'h':			// help!
				case '?':
				{
					help();
					index = argc;			// force to exit
					retVal = ERR_HELPREQUEST;
					return(retVal);			// special case, ONLY time I call return() in the middle of the function (because I check for argc < 2 at the end of the code)
					break;
				}

				default:
				{
					sprintf(err_msg, "\nUnknown parameter %s, use -h to seek help...\n", CommandLine::getPlainString(index));
					MIFFMessage(err_msg, 1);
					index = argc;			// force to exit
					break;
				}
			}
		}
		else
		{
			// we found an arg that doesn't start with '-' so let's assume it's a filename
			if (!inFileEntered)
			{
				strcpy(inFileName, CommandLine::getPlainString(index));
				inFileEntered = true;
			}
			else
			{
				retVal = ERR_MULTIPLEINFILE;
				index = argc;
			}

			// now construct the DEFAULT filename for this file by scanning backwards to front and only extracting the filename (no extension, no path)
			if (ERR_NONE == retVal)
			{
				char	sourceName[maxStringSize];
				strcpy(sourceName, inFileName);			// make a duplicate for us to play with
				for (int strIndex = strlen(sourceName); strIndex > 0; strIndex--)
				{
					if ('.' == sourceName[strIndex])
						sourceName[strIndex] = 0;			// put a stopper here...  we are assuming that '.' indicates extension! I'm going to assume that the person is just testing me if s/he decides to use filename like "foo.bar.psych" which will truncate to "foo"
					if ('\\' == sourceName[strIndex])
						break;		// get out, for we've reached the path name...
				}

				// ok, by here, strIndex should point to either beginning of the string, or where the first '\' was found scanning backwards
				strcpy(filename, &sourceName[strIndex]);		// ta-da-!
			}
		}
	}

	if (inFileEntered)
	{
		if (0 == preprocessSource(inFileName))
		{
			if (verboseMode)
			{
				// using err_msg as my temp buffer...
				sprintf(err_msg,"Now compiling %s\n", inFileName);
				MIFFMessage(err_msg, 0);
			}

			if (ERR_NONE == retVal)
				retVal = loadInputToBuffer(sourceBuffer, bufferSize);
		}
		else	// preprocessSource returned an error...
		{
			retVal = ERR_PREPROCESS;
		}
	}
	else	// inFileEntered == false
	{
		MIFFMessage("Missing input filename in command line!", 1);
	}

	// construct a outFileName[] based on drive[], directory[], filename[], and extension[]
	if (!outPathUsed && (ERR_NONE == retVal))
	{
		if (inFileName[0])	// make sure the user has entered a input filename
			sprintf(outFileName,"%s:%s\\%s.%s", drive, directory, filename, extension);
	}

	if (argc < 1)
		retVal = ERR_ARGSTOOFEW;		// we can do this because we know -h was not entered...

	return(retVal);

#endif

}


//---------------------------------------------------------------------------
// reads the tmeporary files spit out by CCCP and stuffs the plain text into source buffer
//
// Return Value:
//   errorType
//
// Remarks:
//   
//
// See Also:
//   
//
// Revisions and History:
//   1/14/99 [HAI] - created
//
static errorType loadInputToBuffer(
			void * 	dest,				// destination address of where you want the date to be read
			int		maxBufferSize		// maximum destination data pool size
		)
{
	errorType			retVal = ERR_NONE;
	InputFileHandler	*inFileHandler = new InputFileHandler("mIFF.$$$");
	if (inFileHandler)
	{
		int sizeRead = inFileHandler->read(dest, maxBufferSize);
		if (sizeRead >= maxBufferSize)
		{
			retVal = ERR_BUFFERTOOSMALL;
		}
		else
		{
			reinterpret_cast<char *>(dest)[sizeRead] = 0;  // so stupid... but if you don't zero-terminate at exact spot, YYInput may chokes because of extra grammer that may exist...
		}
		if (!debugMode)
			inFileHandler->deleteFile("mIFF.$$$", true);	// no need for temp file now...

		// we've successfully read the file, now close it...
		delete inFileHandler;
	}
	else	// inFileName is NULL
	{
		retVal = ERR_FILENOTFOUND;
	}

	return(retVal);
}

//---------------------------------------------------------------------------
// help function called by main upon -h switch
//
// Return Value:
//   none
//
// Remarks:
//   #include's mIFF.dox
//   make sure to update the version when modified.
//   Notice that help() does NOT go thru MIFFMessage() because we want it to
//   print out whether it's verbose mode or not...
//
// See Also:
//   mIFF.dox
//
// Revisions and History:
//   1/07/99 [HAI] - created
//
static void help(void)
{
	printf("\nmIFF v%s (DOS version) - Bootprint Ent. (c) 1999\n", version);
	printf("Hideki Ikeda\n");
#include "mIFF.dox"
}   


//---------------------------------------------------------------------------
// upon exit from main(), if error has been found, it calls here to inform the user of the type of errors it has encounted.
//
// Return Value:
//   none
//
// Remarks:
//   use -q switch to suppress error messages - but in shell, return value can be used to determine the handling
//
// See Also:
//   
//
// Revisions and History:
//   1/07/99 [HAI] - created
//
static void handleError(errorType error)
{
	if (ERR_NONE == error)
		return;
		
	switch (error)
	{
		case ERR_NONE:
			break;

		case ERR_FILENOTFOUND:
			MIFFMessage("ERROR: INPUT File not found!\n", 1);
			break;

		case ERR_ARGSTOOFEW:
			MIFFMessage("ERROR: Not enough arguments.  Use -h for help.\n", 1);
			break;

		case ERR_BUFFERTOOSMALL:
			MIFFMessage("ERROR: Internally allocated buffer for reading\nsource code is too small, increase buffer and re-compile\n", 1);
			break;

		case ERR_UNKNOWNDIR:
			MIFFMessage("ERROR: Directory unknown...\n", 1);
			break;

		case ERR_PREPROCESS:
			MIFFMessage("ERROR: Possible problems running the GNU C Preprocessor.\n", 1);
			break;

		case ERR_MULTIPLEINFILE:
			MIFFMessage("ERROR: There can only be ONE inputfile name.\nPerhaps you've forgotten the -o option flag\n", 1);
			break;

		case ERR_ENGINE:
			MIFFMessage("ERROR: Engine returned a non-zero value...\n", 1);
			break;

		case ERR_PARSER:
			MIFFMessage("ERROR: Parser error\n", 1);
			break;

		case ERR_HELPREQUEST:
			break;

		case ERR_OPTIONS:
			MIFFMessage("ERROR: Failed to handle command line options\n", 1);
			break;

		default:
			MIFFMessage("ERROR: Unknown error, you suck!\n", 1);
			break;
	}
}	


/////////////////////////////////////////////////////////////////////////////
// gotta write all these externs because you can't call C++ class based non-static
// functions from C...  So we will use here as the bridge between the two
// languages


//---------------------------------------------------------------------------
// Message output handler called by ALL external "C" functions
//
// Return Value:
//   none
//
// Remarks:
//   all the messages that are displayed are channeled thru this function.  Note the -q quiet mode suppresses all messages.
//   this is an extern "C" function
//
// See Also:
//   yyerror()
//
// Revisions and History:
//   1/07/99 [] - created
//
extern "C" void MIFFMessage(char *message,			// null terminated string to be displayed
							int  forceOutput)		// if non-zero, it will print out even in quiet mode (for ERRORs)
{
	if (forceOutput)
		fprintf(stdout, "%s\n", message);
	else if (verboseMode)
		fprintf(stdout, "%s\n", message);
	OutputDebugString(message);
	OutputDebugString("\n");
}

// Only call this via parser!!!
extern "C" void MIFFSetError(void)
{
	errorFlag = ERR_PARSER;
}	

//---------------------------------------------------------------------------
// validation of the filename passed are legal.
//
// Return Value:
//   bool usePragma - whether #pragma is ignored or not
//
// Remarks:
//   if -i switch is used then #pragma's are expected
//   this is an extern "C" function
//
// See Also:
//    
//
// Revisions and History:
//   1/07/99 [ ] - created
//
extern "C" int validateTargetFilename(	char		*targetFileName,	// pointer to where we can store the string filename
										unsigned 	maxTargetBufSize	// size of the filename string buffer
									)
{
	if (strlen(outFileName) > maxTargetBufSize)
		MIFFMessage("Internal error, increase string buffer size in parser.yac and recompile!", 1);

	strcpy(targetFileName, outFileName);

	return(usePragma);
}


//---------------------------------------------------------------------------
// function calls CCCP or CPP via shell to preprocess the source code for #include's and #define's via C-Compatible Compiler Preprocessor
//
// Return Value:
//   shell return value (4DOS is very generous on returning different values, while DOS just returns 0 all the time)
//
// Remarks:
//   use -c switch to use CCCP rather then CPP in your search path
//
// See Also:
//    
//
// Revisions and History:
//   1/07/99 [ ] - created
//
static int preprocessSource(char *sourceName)
{
	char	shellCommand[512];
	int		retVal = 0;

	memset(shellCommand, 0, sizeof(shellCommand));

//	if (!runningUnderNT)
	{
	
		if (verboseMode)
			MIFFMessage("Preprocessing... via CCCP", 0);

		// CCCP parameters:
		// -nostdinc -nostdinc++ - do NOT search for standard include directory; without this, your
		//                         puter would be just twiddling its thumb because CCCP can't find it...
		// -pedantic             - issue warnings (use pedantic-errors if you want it as errors)
		//                         required by the ANSI C standard in certain cases such as comments that
		//                         follow the #else/#endif
		// -dD                   - output #defines (for the purpose of error msg I parse)
		// -H                    - display the name of the header/included files (verbose mode)
		// -P                    - originally, I had this...  so it won't show the # line_num "filename" ???
		if (!useCCCP && verboseMode)
		{
			sprintf(shellCommand, "cpp.exe -nostdinc -nostdinc++ -pedantic -Wall -dD -H %s mIFF.$$$", sourceName);
		}
		else if (!useCCCP && !verboseMode)
		{
			sprintf(shellCommand, "cpp.exe -nostdinc -nostdinc++ -pedantic -Wall -dD %s mIFF.$$$", sourceName);
		}
		else if (useCCCP && verboseMode)
		{
			sprintf(shellCommand, "cccp.exe -nostdinc -nostdinc++ -pedantic -Wall -dD -H %s mIFF.$$$", sourceName);
		}
		else
			sprintf(shellCommand, "cccp.exe -nostdinc -nostdinc++ -pedantic -Wall -dD %s mIFF.$$$", sourceName);
	}
//	else
	{
		// running under NT.  Use the MSVC cl since it deals with long filenames on fat16/fat32 partitions correctly
		// and ccp and cccp don't
//		sprintf(shellCommand, "cl /nologo /W4 /EP %s > mIFF.$$$", sourceName);
	}

	retVal = system(shellCommand);
	if (2 == retVal)	// actually, I think 4DOS reports 2 for cannot find file, but DOS returns a 0...
	{
		REPORT_LOG(true, ("failed to execute following shell command (%d):\n", retVal));
		REPORT_LOG(true, ("    %s\n", shellCommand));
		MIFFMessage("\n\nERROR: Cannot find preprocessor (either CCCP.EXE, CPP.EXE or CL.EXE (under NT) in the search path...\n", 1);
		MIFFMessage("Please make sure the preprocessor is in your search path!\n", 1);
	}
	return(retVal);
}	

extern "C" void MIFFSetIFFName(const char *newFileName)
{
	if (ERR_NONE != errorFlag)
		return;
		
	if (outfileHandler)
		outfileHandler->setCurrentFilename(newFileName);
}	

extern "C" void MIFFinsertForm(const char *formName)
{
	if (ERR_NONE != errorFlag)
		return;
		
	if (outfileHandler)
		outfileHandler->insertForm(formName);
}	

extern "C" void MIFFinsertChunk(const char *chunkName)
{
	if (ERR_NONE != errorFlag)
		return;
		
	if (outfileHandler)
		outfileHandler->insertChunk(chunkName);
}	

extern "C" void MIFFinsertChunkData(void * buffer, unsigned bufferSize)
{
	if (ERR_NONE != errorFlag)
		return;
		
	if (outfileHandler)
		outfileHandler->insertChunkData(buffer, bufferSize);
}	

extern "C" int MIFFloadRawData(char *fname, void * buffer, unsigned maxBufferSize)
{
	int sizeRead = -1;

	if (ERR_NONE != errorFlag)
		return(sizeRead);		// should be -1
		
	InputFileHandler * inFileName = new InputFileHandler(fname);
	if (inFileName)
	{
		sizeRead = inFileName->read(buffer, maxBufferSize);
		if (static_cast<unsigned>(sizeRead) >= maxBufferSize)
		{
			handleError(ERR_BUFFERTOOSMALL);
			sizeRead = -1;
		}
		delete inFileName;
	}

	return(sizeRead);
}	

extern "C" void MIFFexitChunk(void)
{
	if (ERR_NONE != errorFlag)
		return;
		
	if (outfileHandler)
		outfileHandler->exitChunk();
}	
extern "C" void MIFFexitForm(void)
{
	if (ERR_NONE != errorFlag)
		return;
		
	if (outfileHandler)
		outfileHandler->exitForm();
}	

extern "C" char * MIFFallocString(int sizeOfString)
{
	return(new char[sizeOfString]);
}

extern "C" void MIFFfreeString(char * pointer)
{
	delete [] pointer;
}

extern "C" unsigned long MIFFgetLabelHash(char * inputStream)
{
	return (unsigned long)Crc::calculate(inputStream);
}

//===========================================================================
//============================================================ End-of-file ==
//===========================================================================

