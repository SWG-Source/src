// ======================================================================
//
// BitStream.cpp
// ala diaz
//
// copyright 1999 Bootprint Entertainment
//
// ======================================================================

#include "sharedCompression/FirstSharedCompression.h"
#include "sharedCompression/BitStream.h"

const uint32 BitStream::INITIAL_MASK_VALUE = 0x80;

// ======================================================================
// Construct a BitStream
//
// Remarks:
//
//   Initializes the components and sets the stream state.

BitStream::BitStream(bool inputStream)
: isInput(inputStream),
	mask(INITIAL_MASK_VALUE),
	rack(0)
{
}

// ----------------------------------------------------------------------
/**
 * Destroy a BitStream.
 */

BitStream::~BitStream(void)
{
}

// ======================================================================
// Construct a BitBuffer
//
// Remarks:
//
//   Initializes the buffer stream.

BitBuffer::BitBuffer(void *buffer, int size, bool inputStream)
: BitStream(inputStream),
	base(static_cast<byte*>(NON_NULL(buffer))),
	current(base),
	bufSize(size)
{
	DEBUG_FATAL(!buffer, ("buffer nullptr"));
	DEBUG_FATAL(!current, ("buffer nullptr"));
}

// ----------------------------------------------------------------------
/**
 * Destroy a BitBuffer.
 */

BitBuffer::~BitBuffer(void)
{
	base = nullptr;
	current = nullptr;
}

// ----------------------------------------------------------------------

int BitBuffer::getOffset(void) const
{
	return current - base;
}

// ----------------------------------------------------------------------
/**
 * Output bits to the stream.
 * 
 * Outputs the lower count bits of the code to the stream.
 * Count defaults to 1 bit.
 */

void BitBuffer::outputBits(uint32 code, uint32 count)
{
	DEBUG_FATAL(isInput,("BitBuffer::outputBits called by input stream."));

	uint32 codeMask = 1U << (count - 1);

	while (codeMask)
	{
		if (codeMask & code)
		{
			rack |= mask;
		}

		mask >>= 1;

		if (!mask)
		{
			DEBUG_FATAL(((current - base) * isizeof(rack)) >= bufSize, ("BitBuffer::outputBits attempting to write past end of buffer."));
			*current++ = rack;
			rack = 0;
			mask = INITIAL_MASK_VALUE;
		}

		codeMask >>= 1;
	}
}

// ----------------------------------------------------------------------
/**
 * Flush the rack to the stream.
 * 
 * If the current rack has not been written to, nothing happens.
 * Otherwise, the rack is output to the stream and reset.
 */

void BitBuffer::outputRack(void)
{
	if (mask != INITIAL_MASK_VALUE)
	{
		DEBUG_FATAL(((current - base) * isizeof(rack)) >= bufSize, ("BitBuffer::outputRack attempting to write past end of buffer."));
		*current++ = rack;
		rack = 0;
		mask = INITIAL_MASK_VALUE;
	}
}

// ----------------------------------------------------------------------
/**
 * Input bits from the stream.
 * 
 * Input count bits from the stream into the return value.
 * Count defaults to 1 bit.
 * 
 * @return The bits input from the stream.
 */

uint32 BitBuffer::inputBits(uint32 count)
{
	DEBUG_FATAL(count > 32,("BitBuffer::inputBits can only input up to 32 bits at once, requested %d", count));
	DEBUG_FATAL(!isInput,("BitBuffer::inputBits called by output stream."));

	uint32 inputMask = 1U << (count - 1);
	uint32 retval = 0;

	while (inputMask)
	{
		if (mask == INITIAL_MASK_VALUE)
		{
			DEBUG_FATAL(((current - base) * isizeof(rack)) >= bufSize, ("BitBuffer::inputBits attempting to read past end of buffer."));
			rack = *current++;
		}

		if (rack & mask)
		{
			retval |= inputMask;
		}

		inputMask >>= 1;
		mask      >>= 1;

		if (!mask)
		{
			mask = INITIAL_MASK_VALUE;
		}
	}

	return retval;
}

// ======================================================================
// Construct a BitFile
//
// Remarks:
//
//   Initializes the file stream.

BitFile::BitFile(const char *fileName, bool inputStream)
: BitStream(inputStream),
	hFile(INVALID_HANDLE_VALUE),
	name(DuplicateString(fileName))
{
	if (isInput)
	{
		hFile = CreateFile(fileName, GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	}
	else
	{
		// try to truncate it first
		hFile = CreateFile(fileName, GENERIC_READ | GENERIC_WRITE, 0, nullptr, TRUNCATE_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

		if (hFile == INVALID_HANDLE_VALUE)
		{
			//if that fails, create it
			hFile = CreateFile(fileName, GENERIC_READ | GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
		}
	}

	DEBUG_FATAL(hFile == INVALID_HANDLE_VALUE,("BitFile::BitFile - unable to open %s", fileName));
}

// ----------------------------------------------------------------------
/**
 * Destroy a BitFile.
 * 
 * Closes the file stream.
 */

BitFile::~BitFile(void)
{
	if (hFile != INVALID_HANDLE_VALUE)
	{
		if (!CloseHandle(hFile))
		{
			DEBUG_FATAL(true,("BitFile::~BitFile - failed to close HANDLE for file %s - Error %d", name, GetLastError()));
		}
	}

	hFile = nullptr;

	delete [] name;
}

// ----------------------------------------------------------------------

int BitFile::getOffset(void) const
{
	return static_cast<int>(SetFilePointer(hFile, 0, nullptr, FILE_CURRENT));
}

// ----------------------------------------------------------------------
/**
 * Output bits to the stream.
 * 
 * Outputs the lower count bits of the code to the stream.
 * Count defaults to 1 bit.
 */

void BitFile::outputBits(uint32 code, uint32 count)
{

	DEBUG_FATAL(isInput,("BitFile::outputBits called on input stream."));

	uint32 codeMask = 1U << (count - 1);

	while (codeMask)
	{
		if (codeMask & code)
		{
			rack |= mask;
		}

		mask >>= 1;

		if (!mask)
		{
			uint32 bytesWritten;
			if (!WriteFile(hFile, &rack, sizeof(rack), &bytesWritten, nullptr))
			{
				DEBUG_FATAL(true,("BitFile::outputBits error %d.", GetLastError()));
			}
			rack = 0;
			mask = INITIAL_MASK_VALUE;
		}

		codeMask >>= 1;
	}
}

// ----------------------------------------------------------------------
/**
 * Flush the rack to the stream.
 * 
 * If the current rack has not been written to, nothing happens.
 * Otherwise, the rack is output to the stream and reset.
 */

void BitFile::outputRack(void)
{
	uint32 bytesWritten;

	if (mask != INITIAL_MASK_VALUE)
	{
		if (!WriteFile(hFile, &rack, sizeof(rack), &bytesWritten, nullptr))
		{
			DEBUG_FATAL(true,("BitFile::outputRack writing error %d.", GetLastError()));
		}

		rack = 0;
		mask = INITIAL_MASK_VALUE;
	}
}

// ----------------------------------------------------------------------
/**
 * Input bits from the stream.
 * 
 * Input count bits from the stream into the return value.
 * Count defaults to 1 bit.
 */

uint32 BitFile::inputBits(uint32 count)
{
	DEBUG_FATAL(count > 32,("BitFile::inputBits can only input up to 32 bits at once - requested %d", count));
	DEBUG_FATAL(!isInput,("BitFile::inputBits called on output stream."));

	uint32 inputMask = 1U << (count - 1);
	uint32 retval = 0;

	while (inputMask)
	{
		if (mask == INITIAL_MASK_VALUE )
		{
			uint32 bytesRead;

			const BOOL result = ReadFile(hFile, &rack, sizeof(rack), &bytesRead, nullptr);

			UNREF(result);
			DEBUG_FATAL(result && !bytesRead,("BitFile::inputBits hit EOF"));
		}

		if (rack & mask)
		{
			retval |= inputMask;
		}

		inputMask >>= 1;
		mask      >>= 1;

		if (!mask)
		{
			mask = INITIAL_MASK_VALUE;
		}
	}

	return retval;
}

// ======================================================================
// ======================================================================
// ======================================================================


// ======================================================================
// Construct a ByteStream
//
// Remarks:
//
//   Initializes the stream state.

ByteStream::ByteStream(bool inputStream)
: isInput(inputStream)
{
}

// ----------------------------------------------------------------------
/**
 * Destroy a ByteStream.
 */

ByteStream::~ByteStream(void)
{
}

// ======================================================================
// Construct a ByteBuffer
//
// Remarks:
//
//   Initializes the buffer stream.

ByteBuffer::ByteBuffer(void *buffer, int size, bool inputStream)
: ByteStream(inputStream),
	base(static_cast<byte*>(NON_NULL(buffer))),
	current(base),
	bufSize(size)
{
}

// ----------------------------------------------------------------------
/**
 * Destroy a ByteBuffer.
 */

ByteBuffer::~ByteBuffer(void)
{
	base = nullptr;
	current = nullptr;
}

// ----------------------------------------------------------------------
/**
 * Return the offset of the current position of the buffer.
 */

int ByteBuffer::getOffset(void) const
{
	return current - base;
}

// ----------------------------------------------------------------------
/**
 * Output a byte to the stream.
 */

void ByteBuffer::output(byte b)
{
	DEBUG_FATAL((current - base) >= bufSize,("ByteBuffer::output attempting to write past end of buffer."));
	DEBUG_FATAL(isInput,("ByteBuffer::output called on input stream"));

	*current++ = b;
}

// ----------------------------------------------------------------------
/**
 * Input a byte from the stream.
 * 
 * Stores the next byte in the stream in b.  Check the return value
 * to make sure the stream is still valid.
 * 
 * @return Returns true on end-of-stream, otherwise false.
 */

bool ByteBuffer::input(byte *b)
{
	DEBUG_FATAL(!b, ("nullptr pointer"));
	DEBUG_FATAL(!isInput,("ByteBuffer::input called on output stream"));

	if ((current - base) >= bufSize)
		return true;

	*b = *current++;
	return false;
}

// ======================================================================
// Construct a ByteFile
//
// Remarks:
//
//   Initializes the file stream.

ByteFile::ByteFile(const char *fileName, bool inputStream)
: ByteStream(inputStream),
	hFile(INVALID_HANDLE_VALUE),
	name(DuplicateString(fileName))
{
	if (isInput)
	{
		hFile = CreateFile(fileName, GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	}
	else
	{
		hFile = CreateFile(fileName, GENERIC_READ | GENERIC_WRITE, 0, nullptr, TRUNCATE_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

		if (hFile == INVALID_HANDLE_VALUE)
		{
			hFile = CreateFile(fileName, GENERIC_READ | GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
		}
	}

	DEBUG_FATAL(hFile == INVALID_HANDLE_VALUE,("BitFile::BitFile - unable to open %s", fileName));
}

// ----------------------------------------------------------------------
/**
 * Destroy a ByteFile.
 * 
 * Closes the file stream.
 */

ByteFile::~ByteFile(void)
{
	if (hFile != INVALID_HANDLE_VALUE && !CloseHandle(hFile))
		DEBUG_FATAL(true,("ByteFile::~ByteFile - Unable to close HANDLE for file %s - Error %d", name, GetLastError()));

	hFile = nullptr;

	delete [] name;
}

// ----------------------------------------------------------------------

int ByteFile::getOffset(void) const
{
	return static_cast<int>(SetFilePointer(hFile, 0, nullptr, FILE_CURRENT));
}

// ----------------------------------------------------------------------
/**
 * Output a byte to the stream.
 */

void ByteFile::output(byte b)
{
	DEBUG_FATAL(isInput,("ByteFile::output called on input stream."));

	byte	out = b;
	uint32  bytesWritten;

	if (!WriteFile(hFile, &out, sizeof(byte), &bytesWritten, nullptr))
	{
		DEBUG_FATAL(true,("ByteFile::output error %d.", GetLastError()));
	}
}

// ----------------------------------------------------------------------
/**
 * Input a byte from the stream.
 * 
 * Stores the next byte in the stream in b.  Check the return value
 * to make sure the stream is still valid.
 * 
 * @return Returns true on end-of-stream, otherwise false.
 */

bool ByteFile::input(byte *b)
{
	DEBUG_FATAL(!isInput,("ByteFile::input called on output stream."));

	uint32 bytesRead;
	BOOL result = ReadFile(hFile, b, sizeof(byte), &bytesRead, nullptr);

	// check for EOS
	return (result && !bytesRead);
}

// ======================================================================
