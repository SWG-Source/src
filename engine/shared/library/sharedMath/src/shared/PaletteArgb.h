// ======================================================================
//
// PaletteArgb.h
// Copyright 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#ifndef INCLUDED_PaletteArgb_H
#define INCLUDED_PaletteArgb_H

// ======================================================================

class AbstractFile;
class PackedArgb;
class PaletteArgbList;
class MemoryBlockManager;

#include "sharedFoundation/PersistentCrcString.h"

// ======================================================================
/**
 * Provides for loading, manipulation and saving of color palette
 * data.
 *
 * Color palette data may be read from and written to the
 * Microsoft Palette (r) format, typically stored with the .PAL
 * file extension.  Adobe Photoshop can load and save .PAL files.
 *
 * The palette may have as few as zero entries or as many as
 * 1024 entries.  The upper limit on palette entry count is
 * fixed by the file format at 65535 entries (2^32-1); however,
 * for implementation efficiency this class only supports a
 * maximum of 1024 entries.  The size of palette data in the .PAL file
 * is linearly related to the number of palette entries,
 * 24 bytes + 4 * (# palette entries).
 *
 * PaletteArgb loading is accomplished via the PaletteArgbList
 * class.  The caller may not modify a loaded PaletteArgb instance.
 *
 * PaletteArgb instances may be created and modified via the public
 * constructor.  Currently the caller must specify the exact number
 * of palette entries at time of construction.  PaletteArgb supports
 * writing to a file via the PaletteArgb::write() function.
 */

class PaletteArgb
{
friend class PaletteArgbList;

public:

	static void install();

	static void *operator new(size_t size);
	static void  operator delete(void *data);

public:

	explicit PaletteArgb(int entryCount);
	explicit PaletteArgb(std::vector<PackedArgb> const & packedArgbVector);

	const CrcString      &getName() const;
	
	void                  fetch() const;
	void                  release() const;
	int                   getReferenceCount() const;

	int                   getEntryCount() const;
	const PackedArgb     &getEntry(int index, bool & error) const;
	PackedArgb           &getEntry(int index, bool & error);

	bool                  write(const char *pathName) const;


	int                   findClosestMatch(const PackedArgb &targetColor) const;

private:

	typedef std::vector<PackedArgb>  PackedArgbVector;

private:

	static void remove();

private:

	PaletteArgb(const CrcString &pathName);
	~PaletteArgb();

	void  load(AbstractFile &file);
	bool  writeToBuffer(unsigned char *buffer, int bufferSize, int &numberOfBytesWritten) const;

	// disabled
	PaletteArgb();
	PaletteArgb(const PaletteArgb&);
	PaletteArgb &operator =(const PaletteArgb&);

private:

	static bool                              ms_installed;
	static MemoryBlockManager *ms_memoryBlockManager;

private:

	PersistentCrcString     m_name;
	mutable int             m_referenceCount;
	PackedArgbVector *const m_entries;
};

// ======================================================================
/**
 * Return the pathname of the PaletteArgb instance.
 *
 * The pathname for the instance will be a non-zero-length string only
 * if the PaletteArgb instance was loaded via PaletteArgbList.
 *
 * @return  the pathname of the PaletteArgb if loaded; otherwise, 
 *          zero-length string.
 *
 * @see PaletteArgbList
 */

inline const CrcString &PaletteArgb::getName() const
{
	return m_name;
}

// ----------------------------------------------------------------------
/**
 * Release a reference to the PaletteArgb instance.
 *
 * PaletteArgb::fetch() should be called for each logical reference
 * the caller has for the instance.  When the logical reference is
 * no longer needed, it should be released with a call to PackedArgb::release().
 * When no more references exist to a given PaletteArgb, typically
 * it is destroyed.
 *
 * Failure to fetch() a reference 
 */

inline void PaletteArgb::fetch() const
{
	++m_referenceCount;
}

// ----------------------------------------------------------------------
/**
 * Return the number of logical references existing on this
 * PaletteArgb instance.
 *
 * @return  the number of logical references existing on this
 *          PaletteArgb instance.
 */

inline int PaletteArgb::getReferenceCount() const
{
	return m_referenceCount;
}

// ======================================================================

#endif
