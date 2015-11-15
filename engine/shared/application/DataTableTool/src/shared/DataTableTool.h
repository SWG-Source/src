// ======================================================================
//
// DataTableTool.h
// 
// copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_DataTableTool_H
#define INCLUDED_DataTableTool_H

// ======================================================================

#include <string>

class DataTableTool
{
private:

	DataTableTool(void);
	DataTableTool(const DataTableTool&);
	DataTableTool &operator =(const DataTableTool&);
	
public:

	DataTableTool(const char *fileName);
	~DataTableTool(void);

	static void run();
	static void usage();

private:
	static void createOutputDirectoryForFile(const std::string & fileName);
	static void getDefaultOutputFileName(const std::string & inputFileName, std::string & outputFileName, bool onlyConvertPath);
	static void removeFileName(std::string & outputFileName);

	static std::string m_inputFile;
	static std::string m_outputFile;
	static std::string m_outputPath;

	static bool runTest(const char * filename);

};

// ======================================================================

#endif
