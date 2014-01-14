// ============================================================================
//
// FileNameUtils.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_FileNameUtils_H
#define INCLUDED_FileNameUtils_H

//-----------------------------------------------------------------------------
class FileNameUtils
{
public:

	enum
	{
		drive =     1L << 0,
		directory = 1L << 1,
		fileName =  1L << 2,
		extension = 1L << 3
	};

	// Returns a combination of items depending on the enums passed in above.
	// Ex: get(FileNameUtils::fileName | FileNameUtils::extension) returns
	//     the "filename.extension", notice the dot is automatically appended.

	static std::string get(std::string const &path, int const flags);
	
	// Returns only the local or network drive

	static std::string getDrive(std::string const &path);

	// Returns only the diretory

	static std::string getDirectory(std::string const &path);

	// Returns only the filename, no extension or dot

	static std::string getFileName(std::string const &path);
	
	// Returns just the extension, no dot

	static std::string getExtension(std::string const &path);

	static bool        isReadable(std::string const &path);
	static bool        isWritable(std::string const &path);
	static bool        isIff(std::string const &path);

	static void        swapChar(std::string &path, char const sourceCharacter, char const destinationCharacter);

private:

	// Disabled

	FileNameUtils();
	~FileNameUtils();
	FileNameUtils(FileNameUtils const &);
	FileNameUtils &operator =(FileNameUtils const &);
};

// ============================================================================

#endif // INCLUDED_FileNameUtils_H
