//========================================================================
//
// Filename.cpp - a fully qualified file name
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================


#include "sharedTemplateDefinition/FirstSharedTemplateDefinition.h"
#include "Filename.h"

#include "TemplateGlobals.h"
#include "UnicodeUtils.h"

#if defined(PLATFORM_LINUX)
#include <libgen.h>
#endif

#include <climits>

#if defined(WIN32)
const char PATH_SEPARATOR = '\\';
#elif defined(linux)
const char PATH_SEPARATOR = '/';
#else
#error unknown OS
#endif


/**
 * Sets the path of the file name.
 * If the path has a drive letter, the drive will be set (Winodws only)
 *
 * @param path		the path
 */
void Filename::setPath(const char *path)
{
	if (path == nullptr || *path == '\0')
		m_path.clear();
	else
	{
#ifdef WIN32
		if (isalpha(*path) && *(path + 1) == ':')
		{
			setDrive(path);
			path += 2;
		}
#endif
		m_path = path;
		convertToSystemPath(m_path);
		if (m_path[strlen(path) - 1] != PATH_SEPARATOR)
			m_path += PATH_SEPARATOR;
	}

	makeFullPath();
}	// Filename::setPath

/**
 * Sets the name of the file name.
 * If the name has an extension, the extension will be set
 * If the name has a full path (starts with drive or separator), the path will be set
 * If the name has a relative path, the path will be appended to the current path
 *
 * @param name		the name
 */
void Filename::setName(const char *name)
{
	if (name == nullptr || *name == '\0')
		m_name.clear();
	else
	{
#ifdef WIN32
		if (isalpha(*name) && *(name + 1) == ':')
		{
			setDrive(name);
			name += 2;
		}
#endif
		std::string localname = name;
		convertToSystemPath(localname);
		const char *dot = strrchr(localname.c_str(), '.');
		const char *firstSeparator = strchr(localname.c_str(), PATH_SEPARATOR);
		const char *lastSeparator = strrchr(localname.c_str(), PATH_SEPARATOR);
		if (firstSeparator != nullptr)
		{
			// name has a path
			if (firstSeparator == localname)
			{
				// set path
				m_path = std::string(firstSeparator, lastSeparator - firstSeparator + 1);
			}
			else
			{
				// append path
				m_path += std::string(localname.c_str(), lastSeparator - 
					localname.c_str() + 1);
			}
		}
		if (dot != nullptr && (lastSeparator == nullptr || dot > lastSeparator))
		{
			// name has an extension
			setExtension(dot);
			if (lastSeparator == nullptr)
				m_name = std::string(localname.c_str(), dot - localname.c_str());
			else
				m_name = std::string(lastSeparator + 1, dot - (lastSeparator + 1));
		}
		else
		{
			// name doesn't have an extension
			if (lastSeparator == nullptr)
				m_name = localname;
			else
				m_name = std::string(lastSeparator + 1);
		}
	}

	makeFullPath();
}	// Filename::setName

/**
 * Sets the extension of the file name.
 *
 * @param path		the path
 */
void Filename::setExtension(const char *extension)
{
	if (extension == nullptr || *extension == '\0')
		m_extension.clear();
	else
	{
		if (*extension == '.')
			m_extension = extension;
		else
		{
			m_extension = ".";
			m_extension += extension;
		}
	}

	makeFullPath();
}	// Filename::setExtension

/**
 * Converts a path name to use the system path seperator.
 *
 * @param path		the path to convert
 */
void Filename::convertToSystemPath(std::string & path)
{
	size_t i;
	std::string newpath;
	size_t count = path.size();

	for (i = 0; i < count; ++i)
	{
		if (path[i] == '/' || path[i] == '\\')
		{
			if (newpath.empty() || newpath[newpath.size() - 1] != PATH_SEPARATOR)
				newpath += PATH_SEPARATOR;
		}
		else
			newpath += path[i];
	}
	path = newpath;
}	// Filename::convertToSystemPath

/**
 * Prepends a path to our path, unless our path is an an absolute path.
 *
 * @param path		the path to prepend
 */
void Filename::prependPath(const Filename &path)
{
	if (m_path.size() != 0 && m_path[0] != PATH_SEPARATOR)
	{
		m_path = path.getPath() + m_path;
		convertToSystemPath(m_path);
		makeFullPath();
	}
}	// Filename::prependPath

/**
 * Appends a path to our path. If the path is an absolute path, replaces our path.
 *
 * @param path		the path to append
 */
void Filename::appendPath(const Filename &path)
{
	std::string localpath = path.getPath();
	convertToSystemPath(localpath);
	if (localpath.size() != 0 && localpath[0] == PATH_SEPARATOR)
		m_path = localpath;
	else
		m_path += localpath;
	makeFullPath();
}	// Filename::appendPath

/**
 * Converts paths like A/./B to A/B and A/B/../C to A/C.
 */
void Filename::normalizePath(void)
{
static const char PATH_SEPARATOR_BUFF[] = {PATH_SEPARATOR, '\0'};
static const std::string PATH_SEPARATOR_STRING(PATH_SEPARATOR_BUFF);

	// get rid of "/./"
	std::string mask(PATH_SEPARATOR_STRING + "." + PATH_SEPARATOR_STRING);
	std::string::size_type pos;
	for (pos = m_path.find(mask); pos != std::string::npos; pos = m_path.find(mask))
	{
		m_path.erase(pos, mask.size() - 1);
	}

	// get rid of "/../"
	mask = PATH_SEPARATOR_STRING + ".." + PATH_SEPARATOR_STRING;
	std::string::size_type startPos = 0;
	for (pos = m_path.find(mask, startPos); pos != std::string::npos; 
		pos = m_path.find(mask, startPos))
	{
		if (pos != 0)
		{
			std::string::size_type prevPathPos = m_path.rfind(PATH_SEPARATOR, pos - 1);
			if (prevPathPos == 0)
			{
				m_path.erase(0, pos + mask.size() - 1);
			}
			else if (prevPathPos == std::string::npos || m_path.substr(prevPathPos, 
				pos - prevPathPos + 1) == mask)
			{
				startPos = pos + 1;
			}
			else
			{
				m_path.erase(prevPathPos + 1, (pos + mask.size() - 1) - prevPathPos);
			}
		}
		else
		{
			m_path.erase(pos, mask.size() - 1);
		}
	}
}	// Filename::normalizePath

/**
 * Sets the drive letter of the file name.
 *
 * @param drive		the drive
 */
void Filename::setDrive(const char *drive)
{
	if (drive != nullptr && isalpha(*drive))
		m_drive = std::string(drive, 1) + ":";
	else
		m_drive.clear();

	makeFullPath();
}	// Filename::setDrive

/**
 * Determines the absolute path (including drive letter for Windows) for the 
 * current path.
 *
 * @return the absolute path
 */
const std::string Filename::getAbsolutePath(void) const
{
std::string path;

#if defined(WIN32)
	char *pathBuf;
	DWORD bufsize = GetFullPathName(getFullFilename().c_str(), 0, nullptr, nullptr);
	if (bufsize != 0)
	{
		pathBuf = new char[bufsize + 1];
		char * filenamePart;
		GetFullPathName(getFullFilename().c_str(), bufsize + 1, pathBuf, &filenamePart);
		*filenamePart = '\0';
		path = pathBuf;
		delete[] pathBuf;
	}
#elif defined(linux)
	char pathBuf[PATH_MAX];
	if (getcwd(pathBuf, PATH_MAX) != nullptr)
	{
		strcat(pathBuf, "/");
		strcat(pathBuf, getFullFilename().c_str());
		dirname(pathBuf);
		strcat(pathBuf, "/");
		path = pathBuf;
	}
#endif

	return path;
}	// Filename::getAbsolutePath

/**
 * Makes sure the path of the filename exists. If it doesn't, create the path.
 */
void Filename::verifyAndCreatePath(void) const
{
#if defined(WIN32)
	if (WindowsUnicode)
	{
		char *buffer = nullptr;
		DWORD buflen;
		// get our current path
		buflen = GetFullPathName(".", 0, buffer, nullptr);
		buffer = new char[buflen + 1];
		buflen = GetFullPathName(".", buflen + 1, buffer, nullptr);
		Unicode::String srcPath = Unicode::narrowToWide(buffer);
		delete[] buffer;
		// get the destination path
		std::string correctPath(getDrive() + getPath());
		buflen = GetFullPathName(correctPath.c_str(), 0, buffer, nullptr);
		buffer = new char[buflen + 1];
		buflen = GetFullPathName(correctPath.c_str(), buflen + 1, buffer, nullptr);
		Unicode::String destPath = Unicode::narrowToWide(buffer);
		delete[] buffer;

		std::vector<Unicode::String> splitDestPath;
		splitPath(destPath, splitDestPath,
			static_cast<Unicode::unicode_char_t>(WIN32_PATH_SEPARATOR));

		Unicode::String prefix = L"\\\\?\\";
		destPath = prefix + splitDestPath[0];
		
		for (size_t i = 1; i < splitDestPath.size(); ++i)
		{
			destPath += WIN32_PATH_SEPARATOR;
			destPath += splitDestPath[i];
			if (SetCurrentDirectoryW((LPCWSTR)destPath.c_str()) == 0)
			{
				if (CreateDirectoryW((LPCWSTR)destPath.c_str(), nullptr) == 0)
					return;
				if (SetCurrentDirectoryW((LPCWSTR)destPath.c_str()) == 0)
					return;
			}
		}

		// set the directory back to the working directory
		SetCurrentDirectoryW(srcPath.c_str());
	}
	else
	{
		char *buffer = nullptr;
		DWORD buflen;
		// get our current path
		buflen = GetFullPathName(".", 0, buffer, nullptr);
		buffer = new char[buflen + 1];
		buflen = GetFullPathName(".", buflen + 1, buffer, nullptr);
		std::string srcPath = buffer;
		delete[] buffer;
		// get the destination path
		std::string correctPath(getDrive() + getPath());
		buflen = GetFullPathName(correctPath.c_str(), 0, buffer, nullptr);
		buffer = new char[buflen + 1];
		buflen = GetFullPathName(correctPath.c_str(), buflen + 1, buffer, nullptr);
		std::string destPath = buffer;
		delete[] buffer;

		// change to the destination path
		if (destPath.size() >= MAX_PATH)
			return;
		
		std::vector<std::string> splitDestPath;
		splitPath(destPath, splitDestPath, WIN32_PATH_SEPARATOR);

		destPath = splitDestPath[0];
		
		for (size_t i = 1; i < splitDestPath.size(); ++i)
		{
			destPath += WIN32_PATH_SEPARATOR;
			destPath += splitDestPath[i];
			if (SetCurrentDirectory(destPath.c_str()) == 0)
			{
				if (CreateDirectory(destPath.c_str(), nullptr) == 0)
					return;
				if (SetCurrentDirectory(destPath.c_str()) == 0)
					return;
			}
		}

		// set the directory back to the working directory
		SetCurrentDirectory(srcPath.c_str());
	}
#elif defined(linux)
	// @todo: implement for linux
#endif
}	// Filename::verifyAndCreatePath
