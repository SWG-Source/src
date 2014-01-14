// ======================================================================
//
// StdioFile.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_StdioFile_H
#define INCLUDED_StdioFile_H

#include "fileInterface/AbstractFile.h"

#include <cstdio>

// ======================================================================
/**
 * Implements access to stdio functions using the AbstractFile interface.
 * Instances of this class are always created using the StdioFileFactory and
 * always referenced through an AbstractFile pointer, to maintain the AbstractFactory pattern
 * (see "Design Patterns").
 * @see AbstractFile
 */
class StdioFile : public AbstractFile
{
public:
	StdioFile(const char *filename, const char *open_type);
	virtual ~StdioFile();

	virtual void close();

	virtual bool isOpen() const;
	virtual int  length() const;
	virtual int  tell() const;

	virtual bool seek(SeekType seekType, int offset);
	virtual int  read(void* dest_buffer, int num_bytes);
	virtual int  write(int num_bytes, const void* source_buffer);
	virtual void flush();

private:
	///disabled
	StdioFile();
	StdioFile(const StdioFile& copy);
	StdioFile &operator =(const StdioFile& rhs);

private:
	///pointer to the stdio file
	FILE *m_file;
	///indicates that we need to resync the stream before reading using resyncStream
	bool m_justWrote;

	void resyncStream();
};

// ======================================================================

class StdioFileFactory : public AbstractFileFactory
{
public:
	virtual AbstractFile* createFile(const char *filename, const char *openType);
};

// ======================================================================

#endif
