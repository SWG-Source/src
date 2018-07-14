//
// FileName.h
// asommers 5-26-99
//
// copyright 1999, bootprint entertainment
//

#include "sharedUtility/FirstSharedUtility.h"
#include "sharedUtility/FileName.h"

#include <cstdio>

//-------------------------------------------------------------------

FileName::Table FileName::pathTable [P_COUNT] =
{
	{"",                        ""   }, // P_none
	{"animationmap/",          "map"}, // P_animationMap
	{"camera/",                "iff"}, // P_camera
	{"inputmap/",              "iff"}, // P_inputmap
	{"appearance/",            ""   }, // P_appearance
	{"animation",               ""   }, // P_animation
	{"music/",                 "mus"}, // P_music
	{"object/",                "iff"}, // P_object
	{"sample/",                "sam"}, // P_sound
	{"scene/",                 "scn"}, // P_scene
	{"script/",                "iff"}, // P_script
	{"shader/",                "sht"}, // P_shader
	{"effect/",                "eft"}, // P_effect
	{"terrain/",               ""   }, // P_terrain
	{"texture/",               "dds"}  // P_texture
};

//-------------------------------------------------------------------

FileName::FileName (const char* const newName) :
	fullName (0)
{
	set (newName);
}

FileName::FileName (FileName::Path path, const char* filename, const char* ext)
: fullName (0)
{
	//-- verify args
	DEBUG_FATAL (path < P_none || path >= P_COUNT, ("FileName::FileName - bad path!"));
	DEBUG_FATAL (!filename, ("FileName::FileName - filename is 0"));

	//-- make sure we have an extention
	if (!ext)
		ext = pathTable [path].ext;

	// see if the filename already begins with the path
	const char *prePath = pathTable [path].path;
	if (prePath != nullptr && *prePath != '\0')
	{
		if (strncmp(filename, prePath, strlen(prePath)) == 0)
			prePath = "";
	}

	// see if the filename already ends in the extension
	if (ext != nullptr && *ext != '\0')
	{
		int extLen = strlen(ext);
		int filenameLen = strlen(filename);
		if (filenameLen > extLen + 1)
		{
			for (--extLen, --filenameLen; extLen >= 0; --extLen, --filenameLen)
			{
				if (ext[extLen] != filename[filenameLen])
					break;
			}
			if (extLen == -1 && filename[filenameLen] == '.')
				ext = "";
		}
	}

	fullName = new char [strlen(prePath) + strlen(filename) + 1 + strlen(ext) + 1];
	sprintf(fullName, "%s%s%s%s", prePath, filename, strlen(ext) ? "." : "", ext);
}

//-------------------------------------------------------------------

FileName::FileName (const char* path, const char* filename, const char* ext) :
	fullName (0)
{
	NOT_NULL (path);
	NOT_NULL (filename);

	fullName = new char [strlen(path) + 1 + strlen(filename) + 1 + (ext ? strlen(ext) : 0) + 1];
	sprintf(fullName, "%s%s%s%s%s", path, strlen (path) ? "/" : "", filename, (ext && strlen (ext)) ? "." : "", ext ? ext : "");
}

//-------------------------------------------------------------------

FileName::~FileName (void)
{
	delete [] fullName;
	fullName = 0;
}

//-------------------------------------------------------------------

void FileName::set (const char* const newName)
{
	DEBUG_FATAL (!newName, ("FileName::set - newName is 0"));

	if (fullName)
	{
		delete [] fullName;
		fullName = 0;
	}

	if (newName)
		fullName = DuplicateString (newName);
}

//-------------------------------------------------------------------

void FileName::stripPathAndExt (void)
{
	stripPathAndExt (fullName, istrlen (fullName));
}

//-------------------------------------------------------------------

void FileName::stripPath (void)
{
	stripPath (fullName, istrlen (fullName));
}

//-------------------------------------------------------------------

void FileName::stripSpecificPathAndExt (Path path)
{
    stripSpecificPathAndExt (path, fullName, istrlen (fullName));
}

//-------------------------------------------------------------------

void FileName::stripPathAndExt (char* nameBuffer, int nameBufferLength)
{
	NOT_NULL (nameBuffer);

	//-- warning: this function modifies the buffer of the argument
	char  buffer [1000];

	int   len   = istrlen (nameBuffer);
	char* start = nameBuffer;
	char* end   = nameBuffer + len;
	char* curr  = end;

	//-- start from end of string and search backward for '\' or '/'
	while (curr != start && !(*curr == '\\' || *curr == '/'))
		curr--;

	//-- start from curr and search to the end for '.'
	if (*curr == '\\' || *curr == '/')
		curr++;

	start = curr;
	while (curr < end && *curr != '.')
		curr++;

	//-- copy string
	end  = curr;
	curr = start;
	len  = end - start;
	int i;
	for (i = 0; i < len && i < nameBufferLength; i++)
		buffer [i] = *curr++;

	buffer [i] = 0;

	strncpy (nameBuffer, buffer, nameBufferLength);
}

//-------------------------------------------------------------------

void FileName::stripPath (char* nameBuffer, int nameBufferLength)
{
	NOT_NULL (nameBuffer);

	//-- warning: this function modifies the buffer of the argument
	char  buffer [1000];

	int   len   = istrlen (nameBuffer);
	char* start = nameBuffer;
	char* end   = nameBuffer + len;
	char* curr  = end;

	//-- start from end of string and search backward for '\' or '/'
	while (curr != start && !(*curr == '\\' || *curr == '/'))
		curr--;

	if (curr != start && (*curr == '\\' || *curr == '/'))
		curr++;

	//-- copy string
	start = curr;
	len  = end - start;
	int i;
	for (i = 0; i < len && i < nameBufferLength; i++)
		buffer [i] = *curr++;

	if (i >= nameBufferLength)
		i--;

	DEBUG_FATAL (i < 0, ("i < 0"));

	buffer [i] = 0;

	strncpy (nameBuffer, buffer, nameBufferLength);
	nameBuffer [nameBufferLength - 1] = 0;
}

//-------------------------------------------------------------------

/* This function strips the path and ext from the nameBuffer that correspond to the given path.
 * It will do nothing if the path and ext do not match the one given to it in the path parameter.
 */
void FileName::stripSpecificPathAndExt (Path path, char* nameBuffer, int nameBufferLength)
{
	//-- warning: this function modifies the buffer of the argument
    NOT_NULL(nameBuffer);

    int len = istrlen(nameBuffer);
    char* start = nameBuffer;
    char* end = nameBuffer + len;
    char buffer[1000];

        //string compare nameBuffer to path.path for strlen(path.path)-1 to make sure it is correct.
		//-1 because we don't care about the slash (could be back or forward)
        //@todo these compares could be debug only.
    if (strncmp(nameBuffer, pathTable[path].path, strlen(pathTable[path].path) - 1) != 0)
    {
        DEBUG_REPORT_LOG(true, ("Error in stripSpecificPathAndExt...nameBuffer does not match path\n"));
        return;
    }

        //correct path, now check extension.
        //first make sure this path has an extension
    if (pathTable[path].ext[0] != '\0')
    {
        end = strrchr(nameBuffer, '.');
            //make sure to compare strings starting just after the dot
        if (!end || strncmp(end + 1, pathTable[path].ext, strlen(pathTable[path].ext)) != 0)
        {
            DEBUG_REPORT_LOG(true, ("Error in stripSpecificPathAndExt...nameBuffer does not match path\n"));
            return;
        }
        *end = '\0'; //End points to '.'  Strip the extension
    }
    else //We aren't supposed to have an ext.  Make sure we don't
    {
        char* tmp = strrchr(nameBuffer, '.');
        if (tmp)
        {
            DEBUG_REPORT_LOG(true, ("Error in stripSpecificPathAndExt...nameBuffer does not match path\n"));
            return;
        }
    }


        //correct extension.  End is set to the correct place...just before the dot.
        //or in the case of no extension, the last character in the string.

        //advance start to after path.  Make sure to skip slashes.
    start += strlen(pathTable[path].path);

    while (*start == '\\' || *start == '/')
        start++;

        //copy from start to end (which is \0) into tmp buffer
    strncpy (buffer, start, 1000);
        //move temp buffer into return.
    strncpy (nameBuffer, buffer, nameBufferLength);
}

//-------------------------------------------------------------------
