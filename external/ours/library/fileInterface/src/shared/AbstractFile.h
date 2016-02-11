// ======================================================================
//
// AbstractFile.h
// copyright (c) 2001,2002 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_AbstractFile_H
#define INCLUDED_AbstractFile_H

// ======================================================================

/**
 * This class represents the abstract interface for a file.  It is a pure virtual class and must be
 * inherited from.
 */

class AbstractFile
{
public:
	
	typedef void (*AudioServeFunction) ();
	static void setAudioServe(AudioServeFunction audioServeFunction);

	/**
	 * An enum used to specify the vaarious seek origins.
	 */
	enum SeekType
	{
		SeekBegin,
		SeekCurrent,
		SeekEnd
	};

	/**
	 * An enum used to specify how time-sensitive the data begin read is.
	 */
	enum PriorityType
	{
		PriorityLow,
		PriorityData,
		PriorityAudioVideo
	};

	AbstractFile(PriorityType priority);
	virtual ~AbstractFile();

	/**
	 * Is the file open
	 * @return true if this object is holding an open file, else false
	 */
	virtual bool isOpen() const = 0;

	/**
	 * Retrieve the length of the current file.  Returns 0 if there is no open file.
	 *
	 * @return the length of the file
	 */
	virtual int length() const = 0;
	
	/**
	 * Retrieve the current file position.  Does nothing if there is no open file.
	 *
	 * @return the current file position
	 */
	virtual int tell() const = 0;

	/**
	 * Move the file pointer to a new location.  Does nothing if there is no open file
	 *
	 * @param seekType where the origin of the seek is, i.e. the beginning, end, or current file position
	 * @param offset the offset from the origin to move
	 * @return true on successful movement of the file pointer, else false
	 * @see AbstractFile::SeekType
	 */
	virtual bool seek(SeekType seekType, int offset) = 0;

	/**
	 * Read data from a file.  Does nothing if there is no open file.
	 * @param num_bytes the number of bytes to attempt to read
	 * @param dest_buffer the memory location to store the read data, must be preallocated
	 * @return the number of bytes read
	 */
	virtual int read(void *destinationBuffer, int numberOfBytes) = 0;

	/**
	 * Write data to a file.  Does nothing if there is no open file.
	 *
	 * @param numberOfBytes the number of bytes to attempt to write
	 * @param source_buffer the memory location to use to write data from
	 * @return the number of bytes written
	 */
	virtual int write(int numberOfBytes, const void *sourceBuffer) = 0;

	/**
	 * Flush pending write data to the file.  This is simply a hint, and
	 * may not force a flush depending on the particular file type's
	 * implementation.
	 */
	virtual void flush();

	/**
	 * Close an open file.  Returns trivially if there is no currently open file.
	 * This allows the user to manually close the file when they can't control the scope
	 * of the AbstractFile-derived object.
	 */
	virtual void close() = 0;

	/**
	 * Read the entire file into a memory buffer.  The client is responsible for deleting the buffer
	 * using operator delete().  The file will be closed after the read completes.
	 *
	 * @return nullptr if an error occured
	 */
	virtual unsigned char *readEntireFileAndClose();

	virtual bool isZlibCompressed() const;
	virtual int  getZlibCompressedLength() const;
	virtual void getZlibCompressedDataAndClose(unsigned char * & compressedBuffer, int & compressedBufferLength);

private:

	/**
	 * Disabled.
	 */
	AbstractFile();

	/**
	 * Disabled.
	 */
	AbstractFile(const AbstractFile &);

	/**
	 * Disabled.
	 */
	AbstractFile &operator =(const AbstractFile &);

protected:

	const PriorityType m_priority;
};

// ======================================================================

class AbstractFileFactory
{
public:
	virtual AbstractFile* createFile(const char *filename, const char *openType) = 0;
	virtual ~AbstractFileFactory() {};
};

// ======================================================================

#endif
