//========================================================================
//
// TemplateGlobals.cpp - global functions/data
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "sharedTemplateDefinition/FirstSharedTemplateDefinition.h"
#include "TemplateGlobals.h"

#ifdef WIN32
bool WindowsUnicode = false;
#endif


//==============================================================================
// functions

/**
 * Strips a character string of leading and trailing whitespace.
 *
 * @param buffer		the string to strip
 *
 * @return the length of the stripped string
 */
int strip(char *buffer)
{
	NOT_NULL(buffer);

	// find the 1st non-space character
	char *to = buffer;
	char *from = buffer;
	while (isspace(*from) && *from != '\0')
		++from;
	if (*from == '\0')		// entire line was whitespace
		return 0;
	// move the string forward
	if (from != to)
	{
		while (*from != '\0')
			*to++ = *from++;
		*to = '\0';
	}
	from = buffer + strlen(buffer) - 1;
	while (isspace(*from))
		--from;
	*(from + 1) = '\0';
	return from - buffer + 1;
}	// strip

/**
 * Gets the next whitespace-delimited token from a string.
 * 
 * @param buffer		string to get the token from
 * @param token			buffer to store the token in; assume that it is at least
 *						the size of buffer
 *
 * @return the next non-whitespace character in the string after the token, or
 * nullptr if the end of line has been reached
 */
const char *getNextWhitespaceToken(const char *buffer, char *token)
{
	NOT_NULL(token);

	*token = '\0';
	if (buffer == nullptr)
		return nullptr;

	const char *from = buffer;
	char *to = token;
	
	// skip any leading whitespace
	while (isspace(*from) && *from != '\0')
		++from;
	if (*from == '\0')
		return nullptr;

	// copy the token
	while (!isspace(*from) && *from != '\0')
		*to++ = *from++;
	*to = '\0';

	// skip trailing whitespace
	while (isspace(*from) && *from != '\0')
		++from;
	if (*from == '\0')
		return nullptr;
	return from;
}	// getNextWhitespaceToken

/**
 * Gets the next token from a string.
 * 
 * @param buffer		string to get the token from
 * @param token			buffer to store the token in; assume that it is at least
 *						the size of buffer
 *
 * @return the next token, defined by the 1st non-whitespace character in buffer:
 *      if it is '/' and the next character is '/', nullptr
 *      if it is a double-quote, the text until the next double quote (not including \")
 *		if it is a symbol, the symbol
 *		if it is a number, the next characters that make a valid integer or float
 *		if it is a character, the text until the next whitespace or symbol, not including _
 *		if it is nullptr, nullptr
 */
const char *getNextToken(const char *buffer, char *token)
{
	NOT_NULL(token);

	*token = '\0';
	if (buffer == nullptr)
		return nullptr;

	const char *from = buffer;
	char *to = token;
	
	// skip any leading whitespace
	while (isspace(*from) && *from != '\0')
		++from;
	if (*from == '\0')
		return nullptr;

	if (*from == '/' && *(from+1) == '/')
	{
		// comment
		return nullptr;
	}
	else if (isdigit(*from) || 
		((*from == '+' || *from == '-') && isdigit(*(from + 1)))
		)
	{
		*to++ = *from++;
		while (isdigit(*from) && *from != '\0')
			*to++ = *from++;
		if (*from == '.' && isdigit(*(from + 1)))
		{
			*to++ = *from++;
			while (isdigit(*from) && *from != '\0')
				*to++ = *from++;
		}
	}
	else if (isalpha(*from))
	{
		while ((isalnum(*from) || *from == '_') && *from != '\0')
			*to++ = *from++;
	}
	else if (*from == '"')
	{
		*to++ = *from++;
		while (*from != '"' && *from != '\0')
		{
			if (*from != '\\')
				*to++ = *from++;
			else
			{
				// copy the \ and the next character blindly
				*to++ = *from++;
				if (*from != '\0')
					*to++ = *from++;
			}
		}
		if (*from == '"')
			*to++ = *from++;
	}
	else
	{
		*to++ = *from++;
	}
	*to = '\0';	

	// skip trailing whitespace
	while (isspace(*from) && *from != '\0')
		++from;
	if (*from == '\0')
		return nullptr;
	return from;
}	// getNextToken

/**
 * Converts a file name in the form name1_name2_name3 to Name1Name2Name3.
 *
 * @param filename		the lower-case file name
 *
 * @return the file name converted to upper case
 */
std::string filenameLowerToUpper(const std::string & filename)
{
	size_t count = filename.size();
	std::string newname;
	newname.reserve(count);

	for (size_t i = 0; i < count; ++i)
	{
		if (i > 0)
		{
			if (filename[i] != '_')
				newname += filename[i];
			else
				newname += static_cast<char>(toupper(filename[++i]));
		}
		else
			newname += static_cast<char>(toupper(filename[i]));
	}
	return newname;
}	// filenameLowerToUpper

/**
 * Converts a file name in the form Name1Name2Name3 to name1_name2_name3.
 *
 * @param filename		the upper-case file name
 *
 * @return the file name converted to lower case
 */
std::string filenameUpperToLower(const std::string & filename)
{
	size_t count = filename.size();
	std::string newname;
	newname.reserve(count + 5);		// add some padding for underscores

	for (size_t i = 0; i < count; ++i)
	{
		if (i > 0)
		{
			if (!isupper(filename[i]))
				newname += filename[i];
			else
			{
				newname += '_';
				newname += static_cast<char>(tolower(filename[i]));
			}
		}
		else
			newname += static_cast<char>(tolower(filename[i]));
	}
	return newname;
}	// filenameUpperToLower

/**
 * Concatinates two paths together.
 *
 * @param path1		original path
 * @param path2		path to concatinate to path2
 *
 * @return the concatinated path
 *
std::string concatPaths(const char *path1, const char *path2)
{
	// test for missing path
	if (path1 == nullptr || *path1 == '\0')
		return path2;
	if (path2 == nullptr || *path2 == '\0')
		return path1;

#ifdef WIN32
	// test if path2 is an absolute path
	if ((isalpha(*path2) && *(path2 + 1) == ':') || *path2 == PATH_SEPARATOR)
		return path2;
#else
	if (*path2 == PATH_SEPARATOR)
		return path2;
#endif

	std::string path = path1;
	if (path1[strlen(path1) - 1] != PATH_SEPARATOR)
		path += PATH_SEPARATOR;
	path += path2;
	return path;
}	// concatPaths
*/
/**
 * Finds the next highest path of a given path.
 *
 * @param path		the original path
 *
 * @return the new path
 *
std::string getNextHighestPath(const char *path)
{
	NOT_NULL(path);

	




	// find the two highest path separators
	const char *separator1 = strrchr(path, PATH_SEPARATOR);
	if (separator1 == nullptr)
	{
#ifdef WIN32
		if (isalpha(*path) && *(path + 1) == ':')
			return std::string(path, 2);
		else
			return std::string();
#else
		return std::string();
#endif
	}
	else if (separator1 == path)
		return std::string(path, 1);
#ifdef WIN32
	else if (isalpha(*path) && *(path + 1) == ':' && separator1 == path + 2)
		return std::string(path, 3);
#endif
	const char *separator2 = strrchr(separator1 - 1, PATH_SEPARATOR);
	if (separator2 == nullptr)
		return std::string(path, separator1 - path);

	return std::string(path, separator2 - path);
}	// getNextHighestPath
*/
