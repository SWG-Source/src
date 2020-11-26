// ======================================================================
//
// DataTableTool.cpp
//
// copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "FirstDataTableTool.h"
#include "DataTableTool.h"

#include "sharedFoundation/CommandLine.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/Os.h"
#include "sharedFoundation/SetupSharedFoundation.h"
#include "sharedThread/SetupSharedThread.h"
#include "sharedCompression/SetupSharedCompression.h"
#include "sharedDebug/SetupSharedDebug.h"
#include "sharedIoWin/SetupSharedIoWin.h"
#include "sharedFile/FileNameUtils.h"
#include "sharedFile/Iff.h"
#include "sharedFile/SetupSharedFile.h"
#include "sharedFile/TreeFile.h"

#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"
#include "sharedUtility/DataTableWriter.h"

#include "sharedXml/SetupSharedXml.h"

#include <cstdio>
#include <string>

// ======================================================================

extern int main(int argc, char **argv);

std::string DataTableTool::m_inputFile;
std::string DataTableTool::m_outputFile;
std::string DataTableTool::m_outputPath;

// ======================================================================
// command line stuff
// ======================================================================

static const char * const LNAME_HELP                 = "help";
static const char * const LNAME_INPUT_FILE           = "inputFile";
static const char * const LNAME_OUTPUT_FILE          = "outputFile";
static const char * const LNAME_TEST                 = "test";
static const char * const LNAME_CLIENT               = "client";
//static const char * const LNAME_P4EDIT               = "edit"; deprecated
//static const char * const LNAME_P4ADD                = "add"; deprecated
static const char         SNAME_HELP                 = 'h';
static const char         SNAME_INPUT_FILE           = 'i';
static const char         SNAME_OUTPUT_FILE          = 'o';
static const char         SNAME_TEST                 = 't';
//static const char       SNAME_P4EDIT               = 'e'; deprecated
//static const char       SNAME_P4ADD                = 'a'; deprecated
static const char         SNAME_CLIENT               = 'c';

static CommandLine::OptionSpec optionSpecArray[] =
{
	OP_BEGIN_SWITCH(OP_NODE_REQUIRED),

		// help
		OP_SINGLE_SWITCH_NODE(SNAME_HELP, LNAME_HELP, OP_ARG_NONE, OP_MULTIPLE_DENIED),


		// real options
		OP_BEGIN_SWITCH_NODE(OP_MULTIPLE_DENIED),
			OP_BEGIN_LIST(),

				// input file required
				OP_SINGLE_LIST_NODE(SNAME_INPUT_FILE, LNAME_INPUT_FILE, OP_ARG_REQUIRED, OP_MULTIPLE_DENIED,  OP_NODE_REQUIRED),

				// output file optional
				OP_SINGLE_LIST_NODE(SNAME_OUTPUT_FILE, LNAME_OUTPUT_FILE, OP_ARG_REQUIRED, OP_MULTIPLE_DENIED, OP_NODE_OPTIONAL),
				
				// output to client (optional)
                // for every sys.shared datatable that is compiled, if this argument is passed, the file will also be
                // compiled and added to the /client directory in the same path of the output directory (e.g. where /data is located)
                // This allows you to mount the /client directory inside a VM to your windows client directory so you can easily
                // and automatically add shared datatables to your client once they have been compiled
                OP_SINGLE_LIST_NODE(SNAME_CLIENT, LNAME_CLIENT, OP_ARG_NONE, OP_MULTIPLE_DENIED, OP_NODE_OPTIONAL),

				// perforce operations (deprecated)
				//OP_SINGLE_LIST_NODE(SNAME_P4ADD, LNAME_P4ADD, OP_ARG_NONE, OP_MULTIPLE_DENIED, OP_NODE_OPTIONAL),
				//OP_SINGLE_LIST_NODE(SNAME_P4EDIT, LNAME_P4EDIT, OP_ARG_NONE, OP_MULTIPLE_DENIED, OP_NODE_OPTIONAL),

				// tests
				OP_SINGLE_LIST_NODE(SNAME_TEST, LNAME_TEST, OP_ARG_NONE, OP_MULTIPLE_DENIED, OP_NODE_OPTIONAL),

			OP_END_LIST(),
		OP_END_SWITCH_NODE(),


	OP_END_SWITCH()
};

static const int optionSpecCount = sizeof(optionSpecArray) / sizeof(optionSpecArray[0]);

// ======================================================================

int main(int argc, char **argv)
{
	//-- thread
	SetupSharedThread::install();

	//-- debug
	SetupSharedDebug::install(4096);

	{
		SetupSharedFoundation::Data data(SetupSharedFoundation::Data::D_console);
		data.argc  = argc;
		data.argv  = argv;
#if defined(PLATFORM_WIN32)
		data.demoMode = false;
#endif
//Remove need for tools.cfg.  You should include the path directly.  This fixes the annoying warning that desigers get.
//		data.configFile = "tools.cfg";
		SetupSharedFoundation::install(data);
	}

	//-- file
	SetupSharedCompression::install();
	SetupSharedFile::install(false);
	DataTableManager::install();

	//-- iowin
	SetupSharedIoWin::install();

	//-- XML
	SetupSharedXml::install();

	SetupSharedFoundation::callbackWithExceptionHandling(DataTableTool::run);
	SetupSharedFoundation::remove();
	SetupSharedThread::remove();

	return 0;
}

// ----------------------------------------------------------------------
void DataTableTool::run(void)
{
	//bool isXml = false;
	int testOutput = 0;

	// printf ("DataTable creation tool " __DATE__ " " __TIME__ "\n");
		// handle options
	const CommandLine::MatchCode mc = CommandLine::parseOptions(optionSpecArray, optionSpecCount);

	if (mc != CommandLine::MC_MATCH)
	{
		printf("Invalid command line specified.  Printing usage...\n");
		usage();
		return;
	}

	if (CommandLine::getOccurrenceCount(SNAME_HELP))
	{
		usage();
		return;
	}
	
	bool doClientOutput = false;
    if (CommandLine::getOccurrenceCount(SNAME_CLIENT)) {
		doClientOutput = true;
	}

	m_inputFile = CommandLine::getOptionString(SNAME_INPUT_FILE);

	//isXml = DataTableWriter::isXmlFile(m_inputFile.c_str());

	if (CommandLine::getOccurrenceCount(SNAME_OUTPUT_FILE))
	{
		m_outputFile = CommandLine::getOptionString(SNAME_OUTPUT_FILE);
	}
	else
	{
		// Only convert the path
		getDefaultOutputFileName(m_inputFile, m_outputPath, true);
		removeFileName(m_outputPath);
	}

	testOutput = CommandLine::getOccurrenceCount(SNAME_TEST);

	DataTableWriter dl;
	dl.loadFromSpreadsheet(m_inputFile.c_str());

	if (!m_outputFile.empty())
	{
		createOutputDirectoryForFile(m_outputFile);

		if (!FileNameUtils::isWritable(m_outputFile))
		{
			printf("ERROR: The output file is not available for writing: %s\n\n", m_outputFile.c_str());
		}

		bool success = dl.save( m_outputFile.c_str() );

		printf("%s creating data table: %s\n", success ? "SUCCESS" : "FAILURE", m_outputFile.c_str());

		if (testOutput)
			runTest(m_outputFile.c_str());
	}
	else
	{
		createOutputDirectoryForFile(m_outputPath);
		dl.setOutputPath(m_outputPath.c_str());

		std::vector<std::string> tableNames;
		dl.getTableNames(tableNames);

		std::vector<std::string>::iterator i = tableNames.begin();
		for(; i != tableNames.end(); ++i)
		{
			std::string const & tableName = *i;

			std::string tableFileName;
			dl.getTableOutputFileName( tableName.c_str(), tableFileName );

			if (!FileNameUtils::isWritable(tableFileName))
			{
				printf("ERROR: The output file is not available for writing: %s\n\n", tableFileName.c_str());
				continue;
			}

			bool success = dl.saveTable( tableName.c_str(), tableFileName.c_str() );

			printf("%s creating data table: %s\n", success ? "SUCCESS" : "FAILURE", tableFileName.c_str());
			
			if(success && doClientOutput) {
                if(tableFileName.find("sys.shared") != std::string::npos) {
                    tableFileName.replace(tableFileName.find("data/sku.0/sys.shared/compiled/game"), 35, "client");
                    createOutputDirectoryForFile(tableFileName);
                    if (!FileNameUtils::isWritable(tableFileName)) {
                        printf("***ERROR*** Could not write datatable to client directory. Did you already make /client and /client/datatables? They must already exist. Path requested was: %s\n\n", tableFileName.c_str());
                        continue;
                    }
                    bool clientSuccess = dl.saveTable(tableName.c_str(), tableFileName.c_str());
                    printf("%s creating data table for client directory: %s\n", clientSuccess ? "SUCCESS" : "FAILURE", tableFileName.c_str());
                }
            }

			if (testOutput)
				runTest(tableFileName.c_str());
		}
	}

	if (testOutput)
		getchar();
}
// ----------------------------------------------------------------------

void DataTableTool::createOutputDirectoryForFile(const std::string & fileName)
{
	std::string::size_type slashPos = fileName.find_last_of('\\');
	if (slashPos == std::string::npos)
	{
		slashPos = fileName.find_last_of('/');
		if (slashPos == std::string::npos)
			return;
	}

	std::string directoryName(fileName, 0, slashPos);
#if defined(PLATFORM_WIN32)
	int result = CreateDirectory(directoryName.c_str(), nullptr);
#elif defined(PLATFORM_LINUX)
	// make sure slashes are forward
	{
		std::string::size_type i;
		while ((i = directoryName.find('\\')) != std::string::npos)
			directoryName[i] = '/';
	}
	int result = mkdir(directoryName.c_str(), 0777);
#else
	#error unsupported platform
#endif
	UNREF(result);
//	DEBUG_FATAL(result == 0, ("Could not create directory %s from file %s", directoryName.c_str(), fileName.c_str()));
}

// ----------------------------------------------------------------------
void DataTableTool::getDefaultOutputFileName(const std::string & inputFileName, std::string & outputFileName, bool onlyConvertPath)
{
	char buffer [2048];
#if defined(PLATFORM_WIN32)
	char* tmp = 0;
	int retval = GetFullPathName (inputFileName.c_str(), sizeof(buffer), buffer, &tmp);
	FATAL(retval <= 0 || retval >= sizeof(buffer), ("Get FullPathName failed with %d", retval));
#elif defined(PLATFORM_LINUX)
	{
		// make sure slashes are forward
		std::string fileName = inputFileName;
		std::string::size_type i;
		while ((i = fileName.find('\\')) != std::string::npos)
			fileName[i] = '/';
		char *retval = realpath(fileName.c_str(), buffer);
		FATAL(!retval, ("realpath failed"));
	}
#else
	#error unsupported platform
#endif
	outputFileName = buffer;
	std::string::size_type dsrcPos = outputFileName.find("dsrc");
	FATAL (dsrcPos == std::string::npos, ("No output file specified and input file is not in the dsrc tree."));

	outputFileName.replace(outputFileName.find("dsrc"), 4, "data");

	if(!onlyConvertPath)
	{
		outputFileName.erase(outputFileName.find_last_of('.'));
		outputFileName.append(".iff");
	}
}

// ----------------------------------------------------------------------

void DataTableTool::removeFileName(std::string & outputFileName)
{
#if defined(PLATFORM_WIN32)
	outputFileName.erase(outputFileName.find_last_of('\\'));
	outputFileName.append("\\");
#elif defined(PLATFORM_LINUX)
	outputFileName.erase(outputFileName.find_last_of('/'));
	outputFileName.append("/");
#else
	#error unsupported platform
#endif
}

// ----------------------------------------------------------------------

bool DataTableTool::runTest(const char * filename)
{
	int i = 0;
	DataTable tester;
	Iff newTest(filename, false);
	tester.load(newTest);

	printf("Test 1....Numbers only ------------------------\n");
	for (i = 0; i < tester.getNumRows(); ++i)
	{
		printf("Row: %d ", i);
		for (int j = 0; j < tester.getNumColumns(); ++j)
		{
			switch(tester.getDataTypeForColumn(j).getBasicType())
			{
			case DataTableColumnType::DT_Int:
				printf("%d ", tester.getIntValue(j,i));
				break;
			case DataTableColumnType::DT_Float:
				printf("%f ", tester.getFloatValue(j,i));
				break;
			case DataTableColumnType::DT_String:
				printf("%s ", tester.getStringValue(j,i));
				break;
			default:
				printf("??? ");
				break;
			}
		}
		printf("\n");

	}
	printf("\n");
	printf("Test 2....Using column names ----------------------\n");

	std::vector<std::string> colNames;
	for (int numCols = 0; numCols < tester.getNumColumns(); ++numCols)
	{
		colNames.push_back(tester.getColumnName(numCols));
	}
	for (i = 0; i < tester.getNumRows(); ++i)
	{
		printf("Row: %d ", i);
		for (int j = 0; j < tester.getNumColumns(); ++j)
		{
			switch(tester.getDataTypeForColumn(colNames[j]).getBasicType())
			{
			case DataTableColumnType::DT_Int:
				printf("%d ", tester.getIntValue(colNames[j],i));
				break;
			case DataTableColumnType::DT_Float:
				printf("%f ", tester.getFloatValue(colNames[j],i));
				break;
			case DataTableColumnType::DT_String:
				printf("%s ", tester.getStringValue(colNames[j],i));
				break;
			default:
				printf("??? ");
				break;
			}
		}
		printf("\n");

	}
	printf("\n");
	return true;
}


void DataTableTool::usage(void)
{
	printf("DataTableTool -i <inputFileName>\n");
	printf("Optional Parameters: \n");
	printf("  -h print this stuff \n");
	printf("  -o specify output file name.  (By default it uses the inputfile name but \n");
	printf("     it replaces the dsrc directory with data and the extension with .iff)\n");
	printf("     For XML files, this option is not valid.\n");
	printf("  -t run diagnotic test \n");
	printf("  -c compile sys.shared datatables to /client directory as well as /data directory  \n");
	getchar();
}

// ======================================================================
