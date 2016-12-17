//========================================================================
//
// TdfCompiler.cpp - entry point for tdfCompiler
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "FirstTemplateDefinitionCompiler.h"
#include "sharedCompression/SetupSharedCompression.h"
#include "sharedDebug/SetupSharedDebug.h"
#include "sharedFile/SetupSharedFile.h"
#include "sharedFoundation/PerThreadData.h"
#include "sharedFoundation/SetupSharedFoundation.h"
#include "sharedObject/SetupSharedObject.h"
#include "sharedRandom/SetupSharedRandom.h"
#include "sharedRegex/SetupSharedRegex.h"
#include "sharedThread/SetupSharedThread.h"
#include "sharedTemplateDefinition/File.h"
#include "sharedTemplateDefinition/TemplateGlobals.h"
#include "sharedTemplateDefinition/TemplateDefinitionFile.h"
#include "sharedTemplateDefinition/TemplateData.h"

//#pragma warning (disable:4100) // unreferenced formal parameters abound in the perforce clientAPI
//#include "clientapi.h"
//#pragma warning (default:4100)

#include <ctime>

//==============================================================================
// file variables

const TemplateData *currentTemplateData = nullptr;


//==============================================================================
// functions

/**
 * Reads lines of a file until we get to a specified line.
 *
 * @param fp			the file to read from
 * @param endString		the string that flags an exit
 * @param buffer		buffer to read fhe file into
 * @param bufferSize	the size of buffer
 *
 * @return true if we got to endString, false on error
 */
bool skipOldCode(File &fp, const char * endString, char *buffer,
	int bufferSize)
{
	NOT_NULL(endString);
	NOT_NULL(buffer);

	size_t len = strlen(endString);
	int result;
	do
	{
		result = fp.readRawLine(buffer, bufferSize);
		if (result < 0)
		{
			if (result == -1)
			{
				fp.printError("unexpected end-of-file in old template");
				return false;
			}
			else
			{
				fp.printError("error reading old template");
				return false;
			}
		}
	} while (strncmp(buffer, endString, len) != 0);
	return true;
}	// skipOldCode

/**
 * Writes the header file for a template.
 *
 * @param tdfFile		parsed template definition file
 * @param filename		file to write to
 */
void writeTemplateHeader(const TemplateDefinitionFile &tdfFile, const Filename &filename)
{
static const int BUFFER_SIZE = 1024;
File fp;

	const TemplateData *currentTemplate = tdfFile.getTemplateData(tdfFile.getHighestVersion());
	NOT_NULL(currentTemplate);

	Filename headername = filename;
	headername.setExtension(".h");

#ifndef ALWAYS_OVERWRITE
	char buffer[BUFFER_SIZE];

	// see if the template header already exists
	if (!fp.exists(headername))
	{
#endif
		// this is a new template header
		if (!fp.open(headername, "wt"))
		{
			fprintf(stderr, "error opening file [%s]\n", fp.getFilename().getFullFilename().c_str());
			return;
		}
		tdfFile.writeClassHeaderBegin(fp);
		currentTemplate->writeHeaderParams(fp);
		tdfFile.writeClassHeaderEnd(fp, *currentTemplate);
		fp.close();
#ifndef ALWAYS_OVERWRITE
	}
	else
	{
		// the template header exists, make a copy and replace the auto-generated
		// code
		if (!fp.open(headername, "rt"))
			return;

		File temp_fp;
		char tmpname[] = "/tmp/templatecompXXXXXX";
		int tmpfd = mkstemp(tmpname);

		if (tmpfd >= 0 && temp_fp.open(tmpname, "wt"))
		{
			fprintf(stderr, "error opening temp file for template header "
				"replacement [%s]\n", fp.getFilename().getFullFilename().c_str());
			return;
		}

		for (;;)
		{
			int result = fp.readRawLine(buffer, BUFFER_SIZE);
			if (result < 0)
			{
				if (result == -1)
					break;
				else
				{
					fp.printError("error reading old template header file");
					return;
				}
			}
			if (strncmp(buffer, TDF_ID_BEGIN, strlen(TDF_ID_BEGIN)) == 0)
			{
				// write out the new tdf data
				tdfFile.writeTemplateId(temp_fp);
				// skip the old tdf data
				if (!skipOldCode(fp, TDF_ID_END, buffer, BUFFER_SIZE))
					return;
			}
			else if (strncmp(buffer, TDF_TEMPLATE_REFS_BEGIN, strlen(TDF_TEMPLATE_REFS_BEGIN)) == 0)
			{
				// write out the new tdf data
				currentTemplate->writeHeaderFwdDecls(temp_fp);
				// skip the old tdf data
				if (!skipOldCode(fp, TDF_TEMPLATE_REFS_END, buffer, BUFFER_SIZE))
					return;
			}
			else if (strncmp(buffer, TDF_INSTALL_BEGIN, strlen(TDF_INSTALL_BEGIN)) == 0)
			{
				// write out the new tdf data
				currentTemplate->writeRegisterTemplate(temp_fp, "\t");
				// skip the old tdf data
				if (!skipOldCode(fp, TDF_INSTALL_END, buffer, BUFFER_SIZE))
					return;
			}
			else if (strncmp(buffer, TDF_BEGIN, strlen(TDF_BEGIN)) == 0)
			{
				// write out the new tdf data
				currentTemplate->writeHeaderParams(temp_fp);
				// skip the old tdf data
				if (!skipOldCode(fp, TDF_END, buffer, BUFFER_SIZE))
					return;
			}
			else if (temp_fp.puts(buffer) < 0)
			{
				fprintf(stderr, "error writing to temp header file [%s]\n", fp.getFilename().getFullFilename().c_str());
				return;
			}
		}

		fp.close();
		temp_fp.close();
		if (remove(fp.getFilename()) != 0)
		{
			fprintf(stderr, "error removing old template header [%s]\n", fp.getFilename().getFullFilename().c_str());
			return;
		}
		if (rename(temp_fp.getFilename(), fp.getFilename()) != 0)
		{
			fprintf(stderr, "error updating template header\n[%s]\nto\n[%s]\n", temp_fp.getFilename().getFullFilename().c_str(), fp.getFilename().getFullFilename().c_str());
			return;
		}
	}
#endif	// ALWAYS_OVERWRITE
}	// writeTemplateHeader

/**
 * Writes the source file for a template.
 *
 * @param tdfFile		parsed template definition file
 * @param filename		file to write to
 *
 * @resurn 0 on success, -1 on error
 */
int writeTemplateSource(const TemplateDefinitionFile &tdfFile, const Filename &filename)
{
static const int BUFFER_SIZE = 1024;
File fp;
int result;

	const TemplateData *currentTemplate = tdfFile.getTemplateData(tdfFile.getHighestVersion());
	NOT_NULL(currentTemplate);

	Filename sourcename = filename;
	sourcename.setExtension(".cpp");

#ifndef ALWAYS_OVERWRITE
	char buffer[BUFFER_SIZE];

	// see if the template source already exists
	if (!fp.exists(sourcename))
	{
#endif
		// this is a new template source
		if (!fp.open(sourcename, "wt"))
		{
			fprintf(stderr, "error opening file [%s]\n", fp.getFilename().getFullFilename().c_str());
			return -1;
		}
		tdfFile.writeClassSourceBegin(fp, *currentTemplate);
		result = currentTemplate->writeSourceMethods(fp);
		fp.close();
		if (result != 0)
			return result;
#ifndef ALWAYS_OVERWRITE
	}
	else
	{
		// the template source exists, make a copy and replace the auto-generated
		// code
		if (!fp.open(sourcename, "rt"))
		{
			fprintf(stderr, "error opening template source file %s\n", sourcename.getFullFilename().c_str());
			return -1;
		}

                File temp_fp;
                char tmpname[] = "/tmp/templatecompXXXXXX";
                int tmpfd = mkstemp(tmpname);

                if (tmpfd >= 0 && temp_fp.open(tmpname, "wt"))
		{
			fprintf(stderr, "error opening temp file for template source "
				"replacement [%s]\n", temp_fp.getFilename().getFullFilename().c_str());
			return -1;
		}

		for (;;)
		{
			int result = fp.readRawLine(buffer, BUFFER_SIZE);
			if (result < 0)
			{
				if (result == -1)
					break;
				else
				{
					fp.printError("error reading old template source file");
					return -1;
				}
			}
			if (strncmp(buffer, TDF_TEMPLATE_REFS_BEGIN, strlen(TDF_TEMPLATE_REFS_BEGIN)) == 0)
			{
				// write out the new tdf data
				currentTemplate->writeSourceTemplateIncludes(temp_fp);
				// skip the old tdf data
				if (!skipOldCode(fp, TDF_TEMPLATE_REFS_END, buffer, BUFFER_SIZE))
					return -1;
			}
			else if (strncmp(buffer, TDF_INIT_BEGIN, strlen(TDF_INIT_BEGIN)) == 0)
			{
				// write out the new tdf data
				currentTemplate->writeSourceLoadedFlagInit(temp_fp);
				// skip the old tdf data
				if (!skipOldCode(fp, TDF_INIT_END, buffer, BUFFER_SIZE))
					return -1;
			}
			else if (strncmp(buffer, TDF_CLEANUP_BEGIN, strlen(TDF_CLEANUP_BEGIN)) == 0)
			{
				// write out the new tdf data
				currentTemplate->writeSourceCleanup(temp_fp);
				// skip the old tdf data
				if (!skipOldCode(fp, TDF_CLEANUP_END, buffer, BUFFER_SIZE))
					return -1;
			}
			else if (strncmp(buffer, TDF_BEGIN, strlen(TDF_BEGIN)) == 0)
			{
				// write out the new tdf data
				result = currentTemplate->writeSourceMethods(temp_fp);
				if (result != 0)
					return -1;
				// skip the old tdf data
				if (!skipOldCode(fp, TDF_END, buffer, BUFFER_SIZE))
					return -1;
			}
			else if (temp_fp.puts(buffer) < 0)
			{
				fprintf(stderr, "error writing to temp source file [%s]\n", temp_fp.getFilename().getFullFilename().c_str());
				return -1;
			}
		}

		fp.close();
		temp_fp.close();
		if (remove(fp.getFilename()) != 0)
		{
			fprintf(stderr, "error removing old template source [%s]\n", fp.getFilename().getFullFilename().c_str());
			return -1;
		}
		if (rename(temp_fp.getFilename(), fp.getFilename()) != 0)
		{
			fprintf(stderr, "error updating template source\n[%s]\nto\n[%s]\n", temp_fp.getFilename().getFullFilename().c_str(), fp.getFilename().getFullFilename().c_str());
			return -1;
		}
	}
#endif	// ALWAYS_OVERWRITE
	return 0;
}	// writeTemplateSource

/**
 * Writes the header and source files for a template.
 *
 * @param tdfFile		the parsed template definition file
 * @param path			path to write the file to
 *
 * @return 0 on success, -1 on error
 */
int writeTemplate(TemplateDefinitionFile &tdfFile, const Filename &path)
{
int result;

//	std::string oldTemplateName = tdfFile.getTemplateName();
//	std::string oldBaseName = tdfFile.getBaseName();

	Filename fullName(nullptr, path.getPath().c_str(), tdfFile.getTemplateName().c_str(),
		nullptr);

	writeTemplateHeader(tdfFile, fullName);
	result = writeTemplateSource(tdfFile, fullName);

//	tdfFile.setTemplateName(oldTemplateName);
//	tdfFile.setBaseName(oldBaseName);

	return result;
}	// writeTemplate

/**
 * Main parser for a tdf file. Parses initialization data, creates the header and
 * source files, and passes parameter definitions to the appropriate function.
 *
 * @param fp				file to parse
 *
 * @return 0 on success, -1 on error
 */
int parseTemplateDefinitionFile(File &fp)
{
TemplateDefinitionFile tdfFile;

	int result = tdfFile.parse(fp);
	if (result != 0)
		return result;

	if (tdfFile.getTemplateName() == ROOT_TEMPLATE_NAME)
	{
		fprintf(stderr, "Warning: will not generate C++ code for class %s, "
			"invalid template name\n", tdfFile.getTemplateName().c_str());
		return -1;
	}

	tdfFile.setWriteForCompiler(false);
	result = writeTemplate(tdfFile, tdfFile.getPath());
	if (result == 0)
	{
		tdfFile.setWriteForCompiler(true);
		result = writeTemplate(tdfFile, tdfFile.getCompilerPath());
	}

	return result;
}	// parseTemplateDefinitionFile

/**
 * Checks out a template file and the source files associated with it from Perforce.
 *
 * @param filename		the filename of the template
 *
 * @return 0 on success, error code on fail
 */
//int checkOut(const char *filename)
//{
//MyPerforceUser ui;
//ClientApi client;
//Error e;
//
//	// check filename extensions
//	Filename templateFileName(nullptr, nullptr, filename, TEMPLATE_DEFINITION_EXTENSION);
//
//	// Connect to Perforce server
//	client.Init( &e );
//	if (e.Test())
//	{
//		StrBufFixed msg;
//		e.Fmt(&msg);
//		fprintf(stderr, msg.Text());
//		return -1;
//	}
//
//	// check out the template file
//	const char * commands[2];
//	commands[0] = "edit";
//	commands[1] = templateFileName;
//	client.SetArgv( 1, const_cast<char **>(&commands[1]) );
//	client.Run( commands[0], &ui );
//	if (ui.errorOccurred())
//		return -1;
//
//	// find the client and server paths
//	File fp(templateFileName, "rt");
//	if (!fp.isOpened())
//	{
//		fprintf(stderr, "cannot open file %s\n", templateFileName.getFullFilename().c_str());
//		return -1;
//	}
//	File::setBasePath(templateFileName.getPath().c_str());
//	TemplateDefinitionFile tdfFile;
//	IGNORE_RETURN(tdfFile.parse(fp));
//	fp.close();
//
//	if (tdfFile.getPath().getFullFilename().size() != 0)
//	{
//		// check out the source files
//		Filename sourceName(nullptr, tdfFile.getPath().getPath().c_str(),
//			tdfFile.getTemplateName().c_str(), "cpp");
//		commands[1] = sourceName;
//		client.SetArgv( 1, const_cast<char **>(&commands[1]) );
//		client.Run( commands[0], &ui );
//		if (ui.errorOccurred())
//			return -1;
//		sourceName.setExtension("h");
//		client.SetArgv( 1, const_cast<char **>(&commands[1]) );
//		client.Run( commands[0], &ui );
//		if (ui.errorOccurred())
//			return -1;
//	}
//
//	if (tdfFile.getCompilerPath().getFullFilename().size() != 0)
//	{
//		// check out the compiler source files
//		std::string compilerFilename;
//		compilerFilename = EnumLocationTypes[tdfFile.getTemplateLocation()] +
//			filenameLowerToUpper(templateFileName.getName());
//		Filename compilerName(nullptr, tdfFile.getCompilerPath().getPath().c_str(),
//			tdfFile.getTemplateName().c_str(), "cpp");
//		commands[1] = compilerName;
//		client.SetArgv( 1, const_cast<char **>(&commands[1]) );
//		client.Run( commands[0], &ui );
//		if (ui.errorOccurred())
//			return -1;
//		compilerName.setExtension("h");
//		client.SetArgv( 1, const_cast<char **>(&commands[1]) );
//		client.Run( commands[0], &ui );
//		if (ui.errorOccurred())
//			return -1;
//	}
//
//	// Close connection
//	return client.Final( &e );
//}	// checkOut

/**
 * Checks in a template file and the source files associated with it to Perforce.
 *
 * @param filename		the filename of the template
 *
 * @return 0 on success, error code on fail
 */
//int checkIn(const char *filename)
//{
//MyPerforceUser ui;
//ClientApi client;
//Error e;
//
//	// check filename extensions
//	Filename templateFileName(nullptr, nullptr, filename, TEMPLATE_DEFINITION_EXTENSION);
//
//	// find the client and server paths
//	File fp(templateFileName, "rt");
//	if (!fp.isOpened())
//	{
//		fprintf(stderr, "cannot open file %s\n", templateFileName.getFullFilename().c_str());
//		return -1;
//	}
//	File::setBasePath(templateFileName.getPath().c_str());
//	TemplateDefinitionFile tdfFile;
//	int result = tdfFile.parse(fp);
//	if (result != 0)
//	{
//		// don't allow check-in if there are errors
//		return result;
//	}
//	fp.close();
//
//	// Connect to Perforce server
//	client.Init( &e );
//	if (e.Test())
//	{
//		StrBufFixed msg;
//		e.Fmt(&msg);
//		fprintf(stderr, msg.Text());
//		return -1;
//	}
//
//	// try to submit the files
//	const char * commands[4];
//	char param1[256];
//	for (;;)
//	{
//		sprintf(param1, "//depot/.../%s.*", templateFileName.getName().c_str());
//		commands[0] = "submit";
//		commands[1] = param1;
//		// don't report an error if the files need to be added before submitting
//		ui.addFilteredError(SUBMIT_NO_FILE_ERR);
//		client.SetArgv( 1, const_cast<char **>(&commands[1]) );
//		client.Run( commands[0], &ui );
//		if (!ui.errorOccurred())
//			break;
//		if (ui.getLastError() != SUBMIT_NO_FILE_ERR)
//			return -1;
//		ui.clearLastError();
//		ui.clearFilteredErrors();
//
//		// we need to add the files to Perforce before submitting
//		commands[0] = "add";
//
//		// add the template file
//		commands[1] = templateFileName;
//		client.SetArgv( 1, const_cast<char **>(&commands[1]) );
//		client.Run( commands[0], &ui );
//		if (ui.errorOccurred())
//			return -1;
//
//		if (tdfFile.getPath().getFullFilename().size() != 0)
//		{
//			// check in the source files
//			Filename sourceName(nullptr, tdfFile.getPath().getPath().c_str(),
//				tdfFile.getTemplateName().c_str(), "cpp");
//			commands[1] = sourceName;
//			client.SetArgv( 1, const_cast<char **>(&commands[1]) );
//			client.Run( commands[0], &ui );
//			if (ui.errorOccurred())
//				return -1;
//			sourceName.setExtension("h");
//			client.SetArgv( 1, const_cast<char **>(&commands[1]) );
//			client.Run( commands[0], &ui );
//			if (ui.errorOccurred())
//				return -1;
//		}
//
//		if (tdfFile.getCompilerPath().getFullFilename().size() != 0)
//		{
//			// check in the compiler source files
//			Filename compilerName(nullptr, tdfFile.getCompilerPath().getPath().c_str(),
//				tdfFile.getTemplateName().c_str(), "cpp");
//			commands[1] = compilerName;
//			client.SetArgv( 1, const_cast<char **>(&commands[1]) );
//			client.Run( commands[0], &ui );
//			if (ui.errorOccurred())
//				return -1;
//			compilerName.setExtension("h");
//			client.SetArgv( 1, const_cast<char **>(&commands[1]) );
//			client.Run( commands[0], &ui );
//			if (ui.errorOccurred())
//				return -1;
//		}
//	}
//
//	// Close connection
//	return client.Final( &e );
//}	// checkIn

/**
 * Prints the command syntax to the console.
 */
void printSyntax(void)
{
	printf("Compiler commands:\n");
	printf("-compile <filename1>[.tdf] [<filename2>[.tdf] ...]\n");
	//printf("Perforce commands:\n");
	//printf("-edit <filename1>[.tdf] [<filename2>[.tdf] ...]\n");
	//printf("-submit <filename1>[.tdf] [<filename2>[.tdf] ...]\n");
}	// printSyntax

/**
 * Processes the args sent to main.
 */
int processArgs(int argc, char *argv[ ])
{
	if (argc < 3)
	{
		printSyntax();
		return 0;
	}

	//if (strcmp(argv[1], "-edit") == 0)
	//{
	//	for (int i = 2; i < argc; ++i)
	//	{
	//		int result = checkOut(argv[i]);
	//		if (result != 0)
	//			return result;
	//	}
	//}
	//else if (strcmp(argv[1], "-submit") == 0)
	//{
	//	for (int i = 2; i < argc; ++i)
	//	{
	//		int result = checkIn(argv[i]);
	//		if (result != 0)
	//			return result;
	//	}
	//}
	if (strcmp(argv[1], "-compile") == 0)
	{
		Filename filename;
		for (int i = 2; i < argc; ++i)
		{
			File::setBasePath("");
			filename.clear();
			filename.setExtension(TEMPLATE_DEFINITION_EXTENSION);
			filename.setName(argv[i]);
			File fp(filename, "rt");
			if (!fp.isOpened())
			{
				fprintf(stderr, "cannot open file %s\n", argv[i]);
				return -1;
			}
			else
			{
				File::setBasePath(filename.getPath().c_str());
				int result = parseTemplateDefinitionFile(fp);
				fp.close();
				if (result != 0)
					return result;
			}
		}
	}
	else
	{
		printSyntax();
		return 0;
	}

	return 0;
}	// processArgs

/**
 * Program entry point.
 */
int main(int argc, char *argv[ ])
{
	SetupSharedThread::install();
	SetupSharedDebug::install(4096);

	{
		SetupSharedFoundation::Data data(SetupSharedFoundation::Data::D_console);
#ifdef WIN32
		char buffer[1024];
		GetModuleFileName(GetModuleHandle(nullptr), buffer, 1024);
		Filename configName;
		configName.setName(buffer);
		configName.setName("templateCompiler.cfg");
		data.configFile = configName.getFullFilename().c_str();
#endif
		SetupSharedFoundation::install (data);
	}

	SetupSharedRegex::install();

	SetupSharedCompression::install();
	SetupSharedFile::install(false);

	// setup the random number generator
	// @todo need a better seed
	SetupSharedRandom::install(static_cast<uint32>(time(nullptr)));

	int result = processArgs(argc, argv);

	// cleanup
	SetupSharedFoundation::remove();
	PerThreadData::threadRemove();

	return result;
}	// main


// ======================================================================
