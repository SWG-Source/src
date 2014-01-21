//========================================================================
//
// TemplateGlobals.h - program globals
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef _INCLUDED_TemplateGlobals_H
#define _INCLUDED_TemplateGlobals_H

#include <string>
#include "sharedFoundation/Tag.h"

//========================================================================

// constants
const int MAX_DIRECTORY_DEPTH = 64;	// assume we never have a directory deeper than this

const char * const ROOT_TEMPLATE_NAME = "ObjectTemplate";
const char * const COMPILER_ROOT_TEMPLATE_NAME = "TpfTemplate";

const char * const CLIENT_TEMPLATE_PRECOMPILED_HEADER_NAME = "clientGame/FirstClientGame.h";
const char * const SERVER_TEMPLATE_PRECOMPILED_HEADER_NAME = "serverGame/FirstServerGame.h";
const char * const SHARED_TEMPLATE_PRECOMPILED_HEADER_NAME = "sharedGame/FirstSharedGame.h";

const char * const TDF_BEGIN = "//@BEGIN TFD";
const char * const TDF_END = "//@END TFD";
const char * const TDF_ID_BEGIN = "//@BEGIN TFD ID";
const char * const TDF_ID_END = "//@END TFD ID";
const char * const TDF_TEMPLATE_REFS_BEGIN = "//@BEGIN TFD TEMPLATE REFS";
const char * const TDF_TEMPLATE_REFS_END = "//@END TFD TEMPLATE REFS";
const char * const TDF_INSTALL_BEGIN = "//@BEGIN TFD INSTALL";
const char * const TDF_INSTALL_END = "//@END TFD INSTALL";
const char * const TDF_INIT_BEGIN = "//@BEGIN TFD INIT";
const char * const TDF_INIT_END = "//@END TFD INIT";
const char * const TDF_CLEANUP_BEGIN = "//@BEGIN TFD CLEANUP";
const char * const TDF_CLEANUP_END = "//@END TFD CLEANUP";
const char * const TDF_INLINES_BEGIN = "//@BEGIN TFD INLINES";
const char * const TDF_INLINES_END = "//@END TFD INLINES";

const char * const TEMPLATE_DEFINITION_EXTENSION = "tdf";
const char * const TEMPLATE_EXTENSION = "tpf";
const char * const IFF_EXTENSION = "iff";
const char * const TEMPLATE_BINARY_EXSTENSION = IFF_EXTENSION;

const Tag NO_TAG = TAG(0,0,0,0);
const char * const NO_TAG_STRING = "TAG(0,0,0,0)";
const Tag DERIVED_TEMPLATE_TAG = TAG(D,E,R,V);
const char * const DERIVED_TEMPLATE_TAG_STRING = "TAG(D,E,R,V)";

// this is the error code returned by a function that returns a const char *
const char * const CHAR_ERROR = reinterpret_cast<const char * const>(-1);


enum TemplateLocation
{
	LOC_NONE,
	LOC_CLIENT,
	LOC_SERVER,
	LOC_SHARED
};

// map enum TemplateLocation to string types
const char * const EnumLocationTypes[] =
{
	"",
	"Client",
	"Server",
	"Shared"
};


//------------------------------------------------------------------------
// global variables

#ifdef WIN32
extern bool WindowsUnicode;
#endif


//------------------------------------------------------------------------
// global functions

extern int strip(char *buffer);
extern const char *getNextWhitespaceToken(const char *buffer, char *token);
extern const char *getNextToken(const char *buffer, char *token);
extern std::string filenameLowerToUpper(const std::string & filename);
extern std::string filenameUpperToLower(const std::string & filename);
//extern std::string concatPaths(const char *path1, const char *path2);
//extern std::string getNextHighestPath(const char *path);


//------------------------------------------------------------------------
// inline functions

inline std::string ConvertStringToTagString(const char *string)
{
static const char SUFFIX[4] = {',', ',', ',', ')'};
	
	std::string tagString = "TAG(";
	for (int i = 0; i < 4; ++i)
	{
		tagString += string[i];
		tagString += SUFFIX[i];
	}
	return tagString;
}

inline bool isinteger(const char *string)
{
	char *endptr;
	errno = 0;
	strtol(string, &endptr, 10);
	if (errno == ERANGE || *endptr != '\0')
		return false;
	return true;
}

inline bool isfloat(const char *string)
{
	char *endptr;
	strtod(string, &endptr);
	if (errno == ERANGE || *endptr != '\0')
	{
		return false;
	}
	return true;
}


template<class T> 
inline void splitPath(
	const std::basic_string<T> & path, 
	std::vector<std::basic_string<T> > & splitPath,
	T splitter
	)
{
	typename std::basic_string<T>::size_type pos = 0;
	typename std::basic_string<T>::size_type oldPos = 0;
	for (;;)
	{
		oldPos = pos;
		pos = path.find(splitter, pos);
		if (pos != std::basic_string<T>::npos)
		{
			splitPath.push_back(path.substr(oldPos, pos - oldPos));
			++pos;
		}
		else
		{
//			splitPath.push_back(path.substr(oldPos, path.size() - oldPos));
			return;
		}
	}
}

//========================================================================


#endif	// _INCLUDED_TemplateGlobals_H
