// ============================================================================
//
// FileNameUtils.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "sharedFile/FirstSharedFile.h"
#include "sharedFile/FileNameUtils.h"

#include <stdio.h>
#include <string>

// ============================================================================
//
// FileNameUtils
//
// ============================================================================

//-----------------------------------------------------------------------------
std::string FileNameUtils::get(std::string const &path, int const flags)
{
	std::string result;

	if (flags & drive)
	{
		result += getDrive(path);
	}
	if (flags & directory)
	{
		result += getDirectory(path);
	}
	if (flags & fileName)
	{
		result += getFileName(path);
	}
	if (flags & extension)
	{
		std::string const & extension = getExtension(path);

		if (   !extension.empty()
		    && (flags & fileName))
		{
			result += ".";
		}

		result += extension;
	}

	return result;
}

//-----------------------------------------------------------------------------
bool FileNameUtils::isReadable(std::string const &path)
{
	bool result = false;

	if (!path.empty())
	{
		FILE *fp = fopen(path.c_str(), "r");

		result = (fp != nullptr);

		if (fp != nullptr)
		{
			fclose(fp);
		}
	}

	return result;
}

//-----------------------------------------------------------------------------
bool FileNameUtils::isWritable(std::string const &path)
{
	bool result = false;

	if (!path.empty())
	{
		FILE *fp = fopen(path.c_str(), "w");

		result = (fp != nullptr);

		if (fp != nullptr)
		{
			fclose(fp);
		}
	}

	return result;
}

//-----------------------------------------------------------------------------
bool FileNameUtils::isIff(std::string const &path)
{
	bool result = false;
	FILE *fp = fopen(path.c_str(), "rt");

	if ((fp) &&
	    (getc(fp) == 'F') &&
	    (getc(fp) == 'O') &&
	    (getc(fp) == 'R') &&
	    (getc(fp) == 'M'))
	{
		result = true;
		fclose(fp);
	}

	return result;
}

//-----------------------------------------------------------------------------
std::string FileNameUtils::getDrive(std::string const &path)
{
	std::string drive;

	// Check if local drive
	
	size_t const semicolonIndex = path.find(':');
	
	if (semicolonIndex != std::string::npos)
	{
		drive = path.substr(0, semicolonIndex + 1);
	}
	else
	{
		std::string fixedUpPath(path);
		swapChar(fixedUpPath, '\\', '/');

		// Check if network drive

		size_t const doubleSlashIndex = fixedUpPath.find("//");
	
		if (doubleSlashIndex != std::string::npos)
		{
			size_t const firstSlashIndex = fixedUpPath.find('/', doubleSlashIndex + 2);

			if (firstSlashIndex != std::string::npos)
			{
				drive = fixedUpPath.substr(doubleSlashIndex, firstSlashIndex - doubleSlashIndex);
			}
		}
	}

	return drive;
}

//-----------------------------------------------------------------------------
std::string FileNameUtils::getDirectory(std::string const &path)
{
	std::string fixedUpPath(path);
	swapChar(fixedUpPath, '\\', '/');

	std::string directory;

	// Extract the filename position
	
	size_t const lastSlashIndex = fixedUpPath.rfind('/');
	
	// Extract network drive position

	size_t const doubleSlashIndex = fixedUpPath.find("//");

	// Extract the directory position
	
	size_t const firstSlashIndex = fixedUpPath.find('/', (doubleSlashIndex != std::string::npos) ? (doubleSlashIndex + 2) : 0);
	
	// Extract the directory

	if (firstSlashIndex != std::string::npos)
	{
		directory = fixedUpPath.substr(firstSlashIndex, lastSlashIndex - firstSlashIndex + 1);
	}

	return directory;
}

//-----------------------------------------------------------------------------
std::string FileNameUtils::getFileName(std::string const &path)
{
	std::string fixedUpPath(path);
	swapChar(fixedUpPath, '\\', '/');

	std::string fileName;

	// Extract the extension position

	size_t const dotIndex = fixedUpPath.rfind('.');

	// Extract the filename position

	size_t const lastSlashIndex = fixedUpPath.rfind('/');

	// Extract the filename

	if (lastSlashIndex != std::string::npos)
	{
		if (dotIndex == std::string::npos)
		{
			// Extension was not found

			fileName = fixedUpPath.substr(lastSlashIndex + 1);
		}
		else
		{
			// Extension was found

			fileName = fixedUpPath.substr(lastSlashIndex + 1, dotIndex - lastSlashIndex - 1);
		}
	}
	else
	{
		// There is no path

		if (dotIndex == std::string::npos)
		{
			// Extension was not found

			fileName = fixedUpPath;
		}
		else
		{
			// Extension was found

			fileName = fixedUpPath.substr(0, dotIndex);
		}
	}

	return fileName;
}

//-----------------------------------------------------------------------------
std::string FileNameUtils::getExtension(std::string const &path)
{
	std::string extension;

	// Extract the extension

	size_t const dotIndex = path.rfind('.');

	if (dotIndex != std::string::npos)
	{
		extension = path.substr(dotIndex + 1);
	}

	return extension;
}

//-----------------------------------------------------------------------------
void FileNameUtils::swapChar(std::string &path, char const sourceCharacter, char const destinationCharacter)
{
	std::string::iterator iterPath = path.begin();

	for (; iterPath != path.end(); ++iterPath)
	{
		if (*iterPath == sourceCharacter)
		{
			*iterPath = destinationCharacter;
		}
	}
}

// ============================================================================
