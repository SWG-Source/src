//========================================================================
//
// Filename.h - a fully qualified file name
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef _INCLUDED_Filename_H
#define _INCLUDED_Filename_H

class Filename
{
public:
	Filename(void);
	Filename(const char *drive, const char *path, const char *name, const char *extension);

	operator const char *(void) const;
	
	void clear(void);

	const std::string & getPath(void) const;
	const std::string & getName(void) const;
	const std::string & getExtension(void) const;
	const std::string & getFullFilename(void) const;
	const std::string   getAbsolutePath(void) const;

	void setPath(const char *path);
	void setName(const char *name);
	void setExtension(const char *extension);

	void prependPath(const Filename &path);
	void appendPath(const Filename &path);
	void normalizePath(void);

	void verifyAndCreatePath(void) const;

	const std::string & getDrive(void) const;
	void setDrive(const char *drive);

private:
	
	void convertToSystemPath(std::string & path);
	void makeFullPath(void);

	std::string m_path;				// path (not including drive in windows)
	std::string m_name;				// file name sans path & extension
	std::string m_extension;		// file extension (plus '.')
	std::string m_fullFilename;		// fully qualified filename
	std::string m_drive;			// drive letter (plus ':')
};


inline Filename::Filename(void)
{
}	// Filename::Filename

inline Filename::Filename(const char *drive, const char *path, const char *name, const char *extension)
{
	if (drive != nullptr)
		setDrive(drive);
	if (path != nullptr)
		setPath(path);
	if (name != nullptr)
		setName(name);
	if (extension != nullptr)
		setExtension(extension);
}	// Filename::Filename

inline Filename::operator const char *(void) const
{
	return m_fullFilename.c_str();
}	// Filename::operator const char *

inline void Filename::clear(void)
{
	m_path.clear();
	m_name.clear();
	m_extension.clear();
	m_fullFilename.clear();
	m_drive.clear();
}	// Filename::clear

inline const std::string & Filename::getPath(void) const
{
	return m_path;
}	// Filename::getPath

inline const std::string & Filename::getName(void) const
{
	return m_name;
}	// Filename::getName

inline const std::string & Filename::getExtension(void) const
{
	return m_extension;
}	// Filename::getExtension

inline const std::string & Filename::getFullFilename(void) const
{
	return m_fullFilename;
}	// Filename::getFullFilename

inline const std::string & Filename::getDrive(void) const
{
	return m_drive;
}	// Filename::getDrive

inline void Filename::makeFullPath(void)
{
	normalizePath();
	m_fullFilename = m_drive + m_path + m_name + m_extension;
}	// Filename::makeFullPath


//========================================================================

const Filename NEXT_HIGHER_PATH(nullptr, "..", nullptr, nullptr);


#endif	// _INCLUDED_Filename_H
