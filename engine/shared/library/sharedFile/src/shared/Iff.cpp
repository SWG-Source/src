// ======================================================================
//
// Iff.cpp
// jeff grills
//
// copyright 1998 Bootprint Entertainment
//
// ======================================================================

#include "sharedFile/FirstSharedFile.h"
#include "sharedFile/Iff.h"

#include "sharedFile/ConfigSharedFile.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/ByteOrder.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/Os.h"
#include "sharedMath/Quaternion.h"
#include "sharedMath/Transform.h"
#include "sharedMath/Vector.h"
#include "sharedMath/VectorArgb.h"

#include <cstdio>
#include <string>

// ======================================================================

namespace IffNamespace
{
	bool consumeUint32(byte const * & memory, int & length, uint32 & value);
	bool isValid(byte const *memory, int length);
}

using namespace IffNamespace;


#define IFF_FATAL(a, b) ((a) ? fatal(b) : NOP)

#ifdef _DEBUG
	#define IFF_DEBUG_FATAL(a, b)   IFF_FATAL(a, b)
#else
	#define IFF_DEBUG_FATAL(a, b)   NOP
#endif

// ======================================================================

void Iff::install()
{

}

// ----------------------------------------------------------------------
/**
 * calculate the number of bytes contained in the raw iff data buffer.
 * 
 * @return The number of bytes of raw iff data stored in the buffer returned
 * by Iff::getRawData().
 * 
 * Requires that trailing non-iff data in the buffer is set to zero.
 * @see Iff:getRawData()
 */

int Iff::calculateRawDataSize(void) const
{
	// calculate the number of data bytes actually used given the max data buffer length and the contents of the data.
	if (!data)
		return 0;

	// -TF- this assumes any extra non-iff data within the valid iff data buffer has been zeroed out
	int     offset   = 0;
	int     blockLength;
	uint32  tempLength;

	do
	{
		// get block length (including tag and length field)
		memcpy(&tempLength, data + offset + sizeof(Tag), sizeof(uint32));
		blockLength = static_cast<int>(ntohl(tempLength)) + isizeof(Tag) + isizeof(uint32);
		offset += blockLength;
	}
	while ((offset < length) && blockLength);

	return offset;
}

// ======================================================================

bool IffNamespace::consumeUint32(byte const * & memory, int & length, uint32 & value)
{
	if (length < 4)
		return false;

	memcpy(&value, memory, sizeof(value));
	memory += sizeof(value);
	length -= sizeof(value);
	value = ntohl(value);
	return true;
}

// ----------------------------------------------------------------------

bool IffNamespace::isValid(byte const *memory, int length)
{
	if (length <= 0)
		return false;

	while (length > 0)
	{
		// read the block's tag (FORM or the the chunk name)
		Tag blockTag(0);
		if (!consumeUint32(memory, length, blockTag))
			return false;
			
		// read the block length
		uint32 blockLength(0);
		if (!consumeUint32(memory, length, blockLength))
			return false;

		// verify the file length can contain the block's data
		if ((static_cast<int>(blockLength) < 0) || (static_cast<int>(blockLength) > length))
			return false;

		// verify sub-forms
		int subLength = blockLength-sizeof(Tag);
		if (blockTag == TAG_FORM && (subLength != 0 && !isValid(memory+sizeof(Tag), subLength )))
			return false;

		// skip the block data
		memory += blockLength;
		length -= blockLength; 
	}

	return true;
}

// ----------------------------------------------------------------------

bool Iff::isValid(char const * fileName)
{
	AbstractFile * file = TreeFile::open(fileName, AbstractFile::PriorityData, true);
	if (!file)
		return false;

	// read in the file
	int const fileLength = file->length();
	byte *data = file->readEntireFileAndClose();
	delete file;
	file = nullptr;

	bool const result = IffNamespace::isValid(data, fileLength);
	delete [] data;
	data = nullptr;

	return result;
}

// ======================================================================
// Construct an empty Iff
//
// Remarks:
//
//   The Iff can have a file loaded into it by calling open().
//
// See Also:
//
//   Iff::open()

Iff::Iff(void)
: fileName(nullptr),
	maxStackDepth(DEFAULT_STACK_DEPTH),
	stackDepth(0),
	stack(new Stack[DEFAULT_STACK_DEPTH]),
	length(0),
	data(nullptr),
	inChunk(false),
	growable(false),
	nonlinear(false),
	ownsData(true)
{
	// clear out the stack data
	memset(stack, 0, isizeof(*stack) * maxStackDepth);
}

// ----------------------------------------------------------------------
/**
 * Construct an Iff from a data buffer.
 * 
 * If iffOwnsData is true, this Iff object will take ownership of the
 * buffer (i.e. it will delete the buffer when the iff is destroyed).
 * 
 * Behavior of the Iff object is undefined if the given data buffer
 * does not contain valid Iff-format data.
 * 
 * The Iff object created is not growable.
 * 
 * Do not change the contents of the newData buffer while this Iff object
 * still exists.  The Iff object does not copy the newData buffer.
 * 
 * Any portion of the newData buffer that doesn't contain Iff data
 * should be zero-filled.  If it is not, we will not be able to
 * correctly determine the real size of the valid iff data being passed in.
 * 
 * @param newDataSize  [IN] length, in bytes, of the Iff data buffer pointed to by newData
 * @param newData  [IN] the buffer containing Iff data to be interpreted by this Iff object
 * @param iffOwnsData  [IN] if true, the Iff takes ownership of the buffer and deletes it during the destructor call; if false, the Iff does not attempt to delete the buffer during the destructor call
 */

Iff::Iff(int newDataSize, const byte *newData, bool iffOwnsData) :
	fileName(nullptr),
	maxStackDepth(DEFAULT_STACK_DEPTH),
	stackDepth(0),
	stack(new Stack[DEFAULT_STACK_DEPTH]),
	length(newDataSize),
	data(const_cast<byte*>(newData)),
	inChunk(false),
	growable(false),
	nonlinear(false),
	ownsData(iffOwnsData)
{
	// clear out the stack data
	memset(stack, 0, isizeof(*stack) * maxStackDepth);

	// setup the stack data to know about the data
	stack[0].start  = 0;
	stack[0].used   = 0;
	stack[0].length = calculateRawDataSize();
}

// ----------------------------------------------------------------------
/**
 * Construct an Iff from a specified file.
 * 
 * This routine will effectively create an empty Iff and then call open()
 * on that Iff.  Because a constructor has no return value, there is no
 * way for this routine to return a success indicator when the optional
 * flag was specified.
 * 
 * @param newFileName  The name of the file to open
 * @param optional  Optional flag to pass to open()
 * @see Iff::open()
 */

Iff::Iff(const char *newFileName, bool optional)
: fileName(nullptr),
	maxStackDepth(DEFAULT_STACK_DEPTH),
	stackDepth(0),
	stack(new Stack[DEFAULT_STACK_DEPTH]),
	length(0),
	data(nullptr),
	inChunk(false),
	growable(false),
	nonlinear(false),
	ownsData(true)
{
	// clear out the stack data
	memset(stack, 0, isizeof(*stack) * maxStackDepth);

	// open the data file
	static_cast<void>(open(newFileName, optional));
}

// ----------------------------------------------------------------------
/**
 * Construct an Iff for writing new data.
 * 
 * @param initialSize  [IN] Initial size of the Iff data
 * @param isGrowable  [IN] Flag to indicate if the Iff data may automatically expand as needed,
 * @param clearDataBuffer  [IN] if true, the iff data buffer is zeroed during construction. if false, the data buffer is uninitialized
 */

Iff::Iff(int initialSize, bool isGrowable, bool clearDataBuffer)
: fileName(nullptr),
	maxStackDepth(DEFAULT_STACK_DEPTH),
	stackDepth(0),
	stack(new Stack[DEFAULT_STACK_DEPTH]),
	length(initialSize),
	data(new byte[static_cast<size_t>(length)]),
	inChunk(false),
	growable(isGrowable),
	nonlinear(false),
	ownsData(true)
{
	// clear out the stack data
	memset(stack, 0, isizeof(Stack) * maxStackDepth);
	if (clearDataBuffer)
		memset(data, 0, static_cast<size_t>(length));
}

// ----------------------------------------------------------------------
/**
 * Destroy an Iff.
 * 
 * This routine releases all resources allocated by an instance of the Iff.
 */

Iff::~Iff(void)
{
	close();
	delete [] stack;
}

// ----------------------------------------------------------------------
/**
 * Calculate the CRC of the all the data in the Iff.
 *
 * @return The CRC of the contents of the iff.
 */

uint32 Iff::calculateCrc() const
{
	return Crc::calculate(data, stack[0].length);
}

// ----------------------------------------------------------------------
/**
 * Read an Iff in from the specified file.
 * 
 * This routine will load all the data from the Iff into memory.
 * 
 * If the specified file does not exist, the behavior of this routine is
 * determined by the value of the optional flag.  If the flag
 * is true, the routine will return false.  If the flag is false,
 * the routine will call Fatal in debug compiles, but will return false in
 * release compiles.
 * 
 * Calling open() on an Iff instance that already contains data will cause the
 * old data to be discarded and then will load the new data.
 * 
 * @param newFileName  Name of the file to load
 * @param optional  Whether to allow clean failure
 * @return True if the Iff was successfully opened, false otherwise.
 * @see Iff::close()
 */

bool Iff::open(const char *newFileName, bool optional)
{
	AbstractFile * const file = TreeFile::open(newFileName, AbstractFile::PriorityData, optional);
	if(!file)
	{
		DEBUG_FATAL(!optional, ("could not open file '%s'", newFileName));
		return false;
	}

	open(*file, newFileName);
	delete file;

	// copy the file name
	fileName = DuplicateString(newFileName);

	return true;
}

// ----------------------------------------------------------------------
/**
 * Read an Iff in from the specified file.
 * 
 * This routine will load all the data from the Iff into memory.
 * 
 * 
 * Calling open() on an Iff instance that already contains data will cause the
 * old data to be discarded and then will load the new data.
 * 
 * @param file the file to load
 * @return True if the Iff was successfully opened, false otherwise.
 * @see Iff::close()
 */

void Iff::open(AbstractFile & file)
{
	open(file, 0);
}

// ----------------------------------------------------------------------

void Iff::open(AbstractFile & file, char const * const newFileName)
{
	close();

	// get the data file length
	length = file.length();

	// allocate storage for the data
	DEBUG_FATAL(data, ("causing memory leak"));
	data = file.readEntireFileAndClose();

	FATAL(ConfigSharedFile::getValidateIff() && !IffNamespace::isValid(data, length), ("File corruption detected! Iff::isValid failed for %s (size=%d, crc=%08X). Please try a \"Full Scan\" from the LaunchPad.", newFileName ? newFileName : "nullptr", length, Crc::calculate(data, length)));

	// setup the stack data to know about the data
	stack[0].start = 0;
	stack[0].length = length;
	stack[0].used   = 0;
}

// ----------------------------------------------------------------------
/**
 * Release the data associated with the current Iff.
 * 
 * This routine will release the file data that is associated with the Iff,
 * whether it was loaded from a file or was being generated by code.  This Iff
 * can then be used to load another file.
 * 
 * @see Iff::open()
 */

void Iff::close(void)
{
	delete [] fileName;
	fileName = nullptr;

	if (ownsData)
		delete [] data;
	data = nullptr; //lint !e672 // possible memory leak in assignment to Iff::data // no, we only delete when we own it
	stackDepth = 0;
}


// ----------------------------------------------------------------------

bool Iff::write(const char *writeFileName, bool optional)
{
	// update the file name
	delete [] fileName;
	fileName = DuplicateString(writeFileName);

	const bool result = Os::writeFile(fileName, data, stack[0].length);
	FATAL(!result && !optional, ("file write failed for %s", fileName));

	return result;
}

// ----------------------------------------------------------------------

void Iff::fatal(const char *string) const
{
	char buffer[1024];

	NOT_NULL(string);

	formatLocation(buffer, sizeof(buffer));
	FATAL(true, ("%s: %s", buffer, string));
}

// ----------------------------------------------------------------------
/**
 * Print the current Iff location into the specified buffer.
 * 
 * The routine will fill the buffer with the file name and the names
 * of all enclosing forms or chunks into the buffer.  The routine
 * is primarily designed to be used by debugging and error message
 * routines.
 * 
 * @param buffer  Buffer to format the location into
 * @param bufferLength  Length of the buffer to avoid overwriting memory
 */

void Iff::formatLocation(char *buffer, int bufferLength) const
{
	int i, stringLength, totalLength;

	NOT_NULL(buffer);

	// calculate the string length
	if (fileName)
		stringLength = static_cast<int>(strlen(fileName));
	else
		stringLength = 0;

	// calculate the total length of the output buffer
	totalLength = stringLength + 1 + stackDepth * 5 + 1;

	// see if there is enough room in the output buffer
	if (totalLength > bufferLength)
	{
		memset(buffer, '*', bufferLength-1);
		buffer[bufferLength-1] = '\0';
		DEBUG_FATAL(true, ("format buffer not big enough %d/%d", totalLength, bufferLength));
		return; //lint !e527 // Warning -- Unreachable
	}

	// copy the file name into the buffer
	if (fileName)
	{
		strcpy(buffer, fileName);
		buffer[stringLength] = '/';
		buffer += stringLength + 1;
	}

	// put all the block names into the buffer
	for (i = 0; i < stackDepth; ++i)
	{
		// add the next tag
		ConvertTagToString(getBlockName(i), buffer);
		buffer += 4;

		if (i+1 < stackDepth)
		{
			*buffer = '/';
			++buffer;
		}
	}

	// terminate the buffer string
	*buffer = '\0';
}

// ----------------------------------------------------------------------

Tag Iff::getFirstTag(int depth) const
{
	Tag t;

	NOT_NULL(data);

	if ( stack[depth].length - stack[depth].used < isizeof(Tag) + isizeof(uint32) )
	{
		char buf[1024];
		sprintf( buf, "read overflow depth=[%d] length=[%d] used=[%d] want=[%d]", depth, stack[depth].length, stack[depth].used, isizeof(Tag) + isizeof(uint32) );
		IFF_FATAL(true, buf);
	}
	memcpy(&t, data + stack[depth].start + stack[depth].used, sizeof(t));

	return ntohl(t);
}

// ----------------------------------------------------------------------
/**
 * Get the length of data contained in a block.
 * 
 * @param depth  [IN] depth of target block containing the block for which the client wants length info
 * @param offset  [IN] offset of target block within containing block
 */

int Iff::getLength(int depth, int offset) const
{
	uint32 u;

	NOT_NULL(data);
	IFF_DEBUG_FATAL(stack[depth].length - stack[depth].used+offset < isizeof(Tag) + isizeof(uint32), ("read overflow"));
	memcpy(&u, data + stack[depth].start + stack[depth].used + offset + sizeof(Tag), sizeof(u));

	return static_cast<int>(ntohl(u));
}

// ----------------------------------------------------------------------

Tag Iff::getSecondTag(int depth) const
{
	Tag t;

	NOT_NULL(data);
	IFF_DEBUG_FATAL(stack[depth].length - stack[depth].used < isizeof(Tag) + isizeof(uint32) + isizeof(Tag), ("read overflow"));
	memcpy(&t, data + stack[depth].start + stack[depth].used + sizeof(Tag) + sizeof(uint32), sizeof(Tag));

	return ntohl(t);
}

// ----------------------------------------------------------------------
/**
 * Adjust the data array as necessary.
 * 
 * This routine will check if the data array needs to be expanded to hold
 * the specified amount of new data.  If it does, and the Iff is not growable,
 * it will call Fatal in debug compiles, but in release compiles the behavior
 * is undefined.
 * 
 * The data array will be doubled in size if it does need to be grown until it
 * will hold the specified amount of data.
 * 
 * This routine will also handle size being negative, in which case it will
 * remove the specified number of bytes from the current location in the Iff.
 * 
 * @param size  Delta number of bytes
 */

void Iff::adjustDataAsNeeded(int size)
{
	// calculate the final required size of the data array
	const int neededLength = stack[0].length + size;

	NOT_NULL(data);
	IFF_DEBUG_FATAL(neededLength < 0, ("data size underflow"));

	// check if we need to expand the data array
	if (neededLength > length)
	{
		int newLength;

		// make sure the iff was growable
		DEBUG_FATAL(!growable, ("data size overflow %d/%d", neededLength, length));
		DEBUG_FATAL(length < 0, ("current length negative %d\n", length));

		// handle when the iff is created with an initial size of 0, this fixes
		// an infinite looping problem
		if (length <= 0)
		{
			length = 1;
		}

		// double in size until it supports the needed length
		for (newLength = length * 2; newLength < neededLength; newLength *= 2)
			;

		// allocate the new memory
		DEBUG_FATAL(newLength < 0, ("negative array allocation"));
		byte *newData = new byte[static_cast<size_t>(newLength)];

		// copy the old data over to the new data
		memcpy(newData, data, stack[0].length);

		// replace the old data with the new data
		delete [] data;
		data = newData;
		length = newLength;
	}

	// move data around to either make room or remove data
	const int offset = stack[stackDepth].start + stack[stackDepth].used;
	const int lengthToEnd  = stack[0].length - offset;
	if (size > 0)
		memmove(data+offset+size, data+offset, lengthToEnd);
	else
		memmove(data+offset, data+offset-size, lengthToEnd+size);

	// make sure all the enclosing stack entries know about the changed size
	for (int i = 0; i <= stackDepth; ++i)
	{
		// update the stack's idea of the block length
		stack[i].length += size;

		// the length of level 0 is the file size, so we should not write it
		if (i)
		{
			// update the data's idea of the block length
			if (i == stackDepth && inChunk)
			{
				const int ui32 = static_cast<int>(htonl(static_cast<unsigned long>(stack[i].length)));
				memcpy(data+stack[i].start-sizeof(uint32), &ui32, sizeof(uint32));
			}
			else
			{
				// account for forms start beyond the first 4 data bytes, which is their real form name
				const int ui32 = static_cast<int>(htonl(static_cast<unsigned long>(stack[i].length) + sizeof(Tag)));
				memcpy(data+stack[i].start-sizeof(Tag)-sizeof(uint32), &ui32, sizeof(uint32));
			}
		}
	}
}

// ----------------------------------------------------------------------
/**
 * Insert one Iff into another.
 * 
 * The Iff read/write pointer will be placed after the inserted data.
 * 
 * If the Iff is already inside a chunk, this routine will call Fatal for
 * debug compiles, but will have undefined behavior in release compiles.
 * 
 * @param iff  The Iff to insert in this iff
 */

void Iff::insertIff(const Iff *iff)
{
	NOT_NULL(iff);
	NOT_NULL(data);
	IFF_DEBUG_FATAL(inChunk, "inside chunk");

	// make sure the data array can handle this addition
	adjustDataAsNeeded(iff->stack[0].length);

	// compute the offset to start inserting data at
	const int offset = stack[stackDepth].start + stack[stackDepth].used;

	// add the other iff
	memcpy(data+offset, iff->data, iff->stack[0].length);

	// advance past the data
	stack[stackDepth].used += iff->stack[0].length;
}

// ----------------------------------------------------------------------
/**
 * Insert a new form into the Iff at the current location.
 * 
 * This routine will handle adding a form into the middle of an existing
 * Iff instance.
 * 
 * If the Iff is already inside a chunk, this routine will call Fatal for
 * debug compiles, but will have undefined behavior in release compiles.
 * 
 * @param name  Name for the new form
 * @param shouldEnterForm  True to automatically enter the form
 */

void Iff::insertForm(Tag name, bool shouldEnterForm)
{
	const int FORM_OVERHEAD = sizeof(Tag) + sizeof(uint32) + sizeof(Tag);

	Tag    t;
	uint32 ui32;
	int    offset;

	NOT_NULL(data);
	IFF_DEBUG_FATAL(inChunk, "inside chunk");

	// make sure the data array can handle this addition
	adjustDataAsNeeded(FORM_OVERHEAD);

	// compute the offset to start inserting data at
	offset = stack[stackDepth].start + stack[stackDepth].used;

	// add the form header
	t = htonl(TAG_FORM);
	memcpy(data+offset, &t, sizeof(Tag));
	offset += isizeof(Tag);

	// add the size of the form
	ui32 = htonl(sizeof(uint32));
	memcpy(data+offset, &ui32, sizeof(uint32));
	offset += isizeof(uint32);

	// add the real form name
	t = htonl(name);
	memcpy(data+offset, &t, sizeof(Tag));

	// enter the form if requested
	if (shouldEnterForm)
		enterForm();
}

// ----------------------------------------------------------------------
/**
 * Insert a new chunk into the Iff at the current location.
 * 
 * This routine will handle adding a chunk into the middle of an existing
 * Iff instance.
 * 
 * If the Iff is already inside a chunk, this routine will call Fatal for
 * debug compiles, but will have undefined behavior in release compiles.
 * 
 * @param name  Name for the new form
 * @param shouldEnterChunk  True to automatically enter the chunk
 */

void Iff::insertChunk(Tag name, bool shouldEnterChunk)
{
	const int CHUNK_OVERHEAD = sizeof(Tag) + sizeof(uint32);

	Tag    t;
	int    offset;

	NOT_NULL(data);
	IFF_DEBUG_FATAL(inChunk, "inside chunk");

	// make sure the data array can handle this addition
	adjustDataAsNeeded(CHUNK_OVERHEAD);

	// compute the offset to start inserting data at
	offset = stack[stackDepth].start + stack[stackDepth].used;

	// add the form header
	t = htonl(name);
	memcpy(data+offset, &t, sizeof(Tag));
	offset += isizeof(Tag);

	// add the size of the chunk
	memset(data+offset, 0, sizeof(uint32));

	// enter the chunk if requested
	if (shouldEnterChunk)
		enterChunk();
}

// ----------------------------------------------------------------------
/**
 * Insert data into the current chunk at the current location.
 * 
 * This routine will handle adding data into the middle of an existing
 * chunk.  The current position pointer will be moved to the end of
 * the inserted data.
 * 
 * If the Iff is not inside a chunk, this routine will call Fatal for
 * debug compiles, but will have undefined behavior in release compiles.
 * 
 * @param newData  Data to put into the chunk
 * @param dataLength  Length of the data to copy into the chunk
 */

void Iff::insertChunkData(const void *newData, int dataLength)
{
	NOT_NULL(data);
	DEBUG_FATAL(dataLength < 0, ("dataLength < 0, %d", dataLength));
	IFF_DEBUG_FATAL(!inChunk, "not in chunk");

	// make sure there is some data to insert
	if (dataLength == 0)
		return;

	NOT_NULL(newData);

	// make sure the data array can handle this addition
	adjustDataAsNeeded(dataLength);

	// compute the offset to start inserting data at
	const int offset = stack[stackDepth].start + stack[stackDepth].used;


	// add the size of the chunk
	memcpy(data+offset, newData, dataLength);

	// move the current pointer to the end of the inserted text
	stack[stackDepth].used += dataLength;
}


// ----------------------------------------------------------------------
/**
 * Add a vector to the Iff chunk.
 * 
 * This routine will write the vector to the chunk such that it can
 * be read back using read_floatVector().
 * 
 * @param vector  Vector to add to the chunk data
 */

void Iff::insertChunkFloatVector(const Vector &vector)
{
	insertChunkData(static_cast<float>(vector.x));
	insertChunkData(static_cast<float>(vector.y));
	insertChunkData(static_cast<float>(vector.z));
}

// ----------------------------------------------------------------------
/**
 * Add a vector argb to the Iff chunk.
 * 
 * This routine will write the vector argb to the chunk such that it can
 * be read back using read_floatVectorArgb().
 * 
 * @param color  Vector to add to the chunk data
 */

void Iff::insertChunkFloatVectorArgb(const VectorArgb &color)
{
	insertChunkData(static_cast<float>(color.a));
	insertChunkData(static_cast<float>(color.r));
	insertChunkData(static_cast<float>(color.g));
	insertChunkData(static_cast<float>(color.b));
}

// ----------------------------------------------------------------------
/**
 * Add a transform to the Iff chunk.
 * 
 * This routine will write the transform to the chunk such that it can
 * be read back using read_floatTransform().
 * 
 * @param transform  Transform to add to the chunk data
 */

void Iff::insertChunkFloatTransform(const Transform &transform)
{
	for (int y = 0; y < 3; ++y)
		for (int x = 0; x < 4; ++x)
			insertChunkData(static_cast<float>(transform.matrix[y][x]));
}

// ----------------------------------------------------------------------
/**
 * Add a quaternion to the Iff chunk.
 * 
 * This routine will write the quaternion to the chunk such that it can
 * be read back using read_floatQuaternion().
 */

void Iff::insertChunkFloatQuaternion(const Quaternion &quaternion)
{
	insertChunkData(static_cast<float>(quaternion.w));
	insertChunkData(static_cast<float>(quaternion.x));
	insertChunkData(static_cast<float>(quaternion.y));
	insertChunkData(static_cast<float>(quaternion.z));
}

// ----------------------------------------------------------------------
/**
 * Insert a string into the current chunk at the current location.
 * 
 * This routine will call insertChunkData(const void *, int length)
 * with the string using its string length (plus one for the nullptr
 * terminator).
 */

void Iff::insertChunkString(const char *string)
{
	NOT_NULL(string);
	insertChunkData(string, istrlen(string)+1);
}

//----------------------------------------------------------------------
/**
* Write a Unicode string into the iff.
* The string may contain nulls within the data.  That is, this function writes the entire contents
* of basic_string::data (), rather than basic_string::c_str ()
*/

void Iff::insertChunkString(const Unicode::String & str)
{
	insertChunkData (static_cast<int32>(str.size ()));
	insertChunkArray(str.data (), str.size ());
}

// ----------------------------------------------------------------------
/**
 * Delete the specified number of bytes within the chunk.
 * 
 * This routine will delete the specified number of bytes within the chunk
 * at the current read position.  The read position will be unchanged.
 * 
 * Calling this routine while not in a chunk will cause Fatal to be called
 * in debug compiles, but its behavior is undefined in release builds.
 * 
 * Specifing a negative length will cause Fatal to be called in debug compiles,
 * but its behavior is undefined in release builds.
 * 
 * Trying to delete more bytes than is left in a chunk will cause Fatal to be
 * called in debug compiles, but its behavior is undefined in release builds.
 * 
 * @param dataLength  Number of bytes to delete
 */

void Iff::deleteChunkData(int dataLength)
{
	IFF_DEBUG_FATAL(!inChunk, "not in chunk");
	DEBUG_FATAL(dataLength < 0, ("dataLength to delete %d < 0", dataLength));
	DEBUG_FATAL(dataLength > getChunkLengthLeft(), ("deleting more data than exists %d/%d", dataLength, getChunkLengthLeft()));
	adjustDataAsNeeded(-dataLength);
}

// ----------------------------------------------------------------------
/**
 * Seek within a chunk.
 * 
 * This function is only allowed after nonlinear functions have been enabled.
 * 
 * This function behaves much like the normal file seek command, but it moves
 * the read pointer within the current chunk.
 * 
 * @param offset   Distance to move
 * @param seekType   Base locaiton of movement
 * @see Iff::allowNonlinearFunctions()
 */

void Iff::seekWithinChunk(int offset, SeekType seekType)
{
	DEBUG_FATAL(!nonlinear, ("nonlinear commands not permitted"));
	IFF_DEBUG_FATAL(!inChunk, "not in chunk");

	switch (seekType)
	{
		case SeekBegin:
			stack[stackDepth].used = offset;
			break;

		case SeekCurrent:
			stack[stackDepth].used += offset;
			break;

		case SeekEnd:
			stack[stackDepth].used = stack[stackDepth].length + offset;
			break;
	}

	DEBUG_FATAL(stack[stackDepth].used < 0, ("seek underflow %d", stack[stackDepth].used));
	DEBUG_FATAL(stack[stackDepth].used > stack[stackDepth].length, ("seek overflow %d/%d", stack[stackDepth].used, stack[stackDepth].length));
}

// ----------------------------------------------------------------------
/**
 * Return to the top of the enclosing form.
 * 
 * This function is only allowed after nonlinear functions have been enabled.
 * 
 * Calling this function will return the current read pointer to the beginning
 * of the enclosing form.  If this function is called while within a chunk, it
 * will cause Fatal to be called in a debug compile, but will have undefined
 * behavior in a release compile.
 * 
 * @see Iff::allowNonlinearFunctions()
 */

void Iff::goToTopOfForm(void)
{
	DEBUG_FATAL(!nonlinear, ("nonlinear commands not permitted"));
	IFF_DEBUG_FATAL(inChunk, "in chunk");
	stack[stackDepth].used = 0;
}


// ----------------------------------------------------------------------
/**
 * Determine whether this Iff is at the end of its currently enclosing form.
 * 
 * This routine indicates whether there are any more chunks or forms in the
 * current form or not.
 * 
 * @return True if at the end of the form, false otherwise
 */

bool Iff::atEndOfForm(void) const
{
	return (stack[stackDepth].used == stack[stackDepth].length);
}

// ----------------------------------------------------------------------
/**
 * Get the number of blocks left in the current enclosing form.
 * 
 * @return The number of blocks left in the currently enclosing form
 */

int Iff::getNumberOfBlocksLeft(void) const
{
	int result, offset;

	IFF_DEBUG_FATAL(inChunk, "in chunk");

	// count the number of times we can advance the used amount by the block size
	for (result = 0, offset = 0; stack[stackDepth].used+offset < stack[stackDepth].length; ++result, offset += (getLength(stackDepth, offset) + isizeof(Tag) + isizeof(uint32)))
		;

	return result;
}

// ----------------------------------------------------------------------
/**
 * Return the name of the specified block.
 * 
 * @return The tag for the specified block depth
 */

Tag Iff::getBlockName(int depth) const
{
	Tag t;

	t = getFirstTag(depth);
	if (t == TAG_FORM)
		t = getSecondTag(depth);

	return t;
}

// ----------------------------------------------------------------------
/**
 * Return the length of the current block.
 * 
 * @return The length of the current block
 */

int Iff::getCurrentLength(void) const
{
	return getLength(stackDepth);
}

// ----------------------------------------------------------------------
/**
 * Determine if the currently selected block is a chunk.
 * 
 * If at the end of the current form, so there is no currently selected block,
 * this routine will call Fatal for debug compiles, but its behavior will be
 * undefined for release compiles.
 * 
 * @return True if the currently selected block is a chunk, false otherwise.
 * @see Iff::atEndOfForm()
 */

bool Iff::isCurrentChunk(void) const
{
	return (getFirstTag(stackDepth) != TAG_FORM);
}

// ----------------------------------------------------------------------
/**
 * Determine if the currently selected block is a form.
 * 
 * If at the end of the current form, so there is no currently selected block,
 * this routine will call Fatal for debug compiles, but its behavior will be
 * undefined for release compiles.
 * 
 * @return True if the currently selected block is a form, false otherwise.
 * @see Iff::atEndOfForm()
 */

bool Iff::isCurrentForm(void) const
{
	return (getFirstTag(stackDepth) == TAG_FORM);
}

// ----------------------------------------------------------------------
/**
 * Increase the Iff stack depth if needed.
 * 
 * If adding another stack entry will cause the stack to overflow, this
 * routine will allocate a new larger stack and copy the data from the
 * old stack into the new stack.
 * 
 * Currently the stack will grow linearly by 16 elements.
 */

void Iff::growStackAsNeeded(void)
{
	FATAL(stackDepth >= maxStackDepth, ("stack already blown"));

	if (stackDepth+1 == maxStackDepth)
	{
		// add another 16 entries to the stack depth
		maxStackDepth += 16;

		// allocate a new stack
		Stack *newStack = new Stack[static_cast<size_t>(maxStackDepth)];

		// copy the data over
		memcpy(newStack, stack, isizeof(*stack) * (stackDepth + 1));

		// replace the old stack
		delete [] stack;
		stack = newStack;
	}
}

// ----------------------------------------------------------------------

bool Iff::enterForm(Tag name, bool validateName, bool optional)
{
	UNREF(optional);

	// make sure it's a form, and check to make sure it's named appropriately
	if (!inChunk && !atEndOfForm() && isCurrentForm() && (!validateName || getSecondTag(stackDepth) == name))
	{
		growStackAsNeeded();

		// setup the next stack frame
		Stack &s = stack[stackDepth+1];
		s.start  = stack[stackDepth].start + stack[stackDepth].used + isizeof(Tag) + isizeof(uint32) + isizeof(Tag);
		s.length = getLength(stackDepth) - isizeof(Tag);
		s.used   = 0;

		// advance the stack frame
		++stackDepth;

		return true;
	}

	if (!optional)
	{
		char tagname[5];
		ConvertTagToString(name, tagname);
		char buf[256];
		snprintf( buf, sizeof(buf), "enter form [%s] failed", tagname );

		IFF_FATAL(true, (buf));
	}
	return false;
}

// ----------------------------------------------------------------------
/**
 * Exit the current form.
 * 
 * It is illegal to attempt to exit the enclosing chunk while the Iff is not
 * already within a chunk.  Doing so in a debug compile will call Fatal, but
 * in a release compile the behavior is undefined.
 * 
 * This version of the routine will validate that the chunk name that it is
 * exiting is of the specified name.  If it is not, in a debug compile the
 * 
 * @see Iff::enterForm()
 */

void Iff::exitForm(Tag name, bool mayNotBeAtEndOfForm)
{
	UNREF(name);
	UNREF(mayNotBeAtEndOfForm);

	IFF_DEBUG_FATAL(stackDepth == 0, ("exiting root"));
	IFF_DEBUG_FATAL(inChunk, ("exitForm called within a chunk"));
	IFF_DEBUG_FATAL(!mayNotBeAtEndOfForm && !atEndOfForm(), ("exit form but not at the end of it"));
	IFF_FATAL(getSecondTag(stackDepth-1) != name, ("exit form with incorrect name"));
	exitForm(mayNotBeAtEndOfForm);
}

// ----------------------------------------------------------------------
/**
 * Exit the current form.
 * 
 * It is illegal to exit the "root" form.  Doing so in a debug compile will
 * call Fatal, but in a release compile the behavior is undefined.
 * 
 * It is also illegal to attempt to exit the enclosing form while the Iff has
 * entered but not exited a chunk.  Again, doing so in a debug compile will
 * call Fatal, but in a release compile the behavior is undefined.
 * 
 * Exiting the form will leave the Iff's current position just beyond that form
 * the enclosing form.
 * 
 * @see Iff::enterForm()
 */

void Iff::exitForm(bool mayNotBeAtEndOfForm)
{
	UNREF(mayNotBeAtEndOfForm);
	IFF_DEBUG_FATAL(stackDepth == 0, ("exiting root"));
	IFF_DEBUG_FATAL(!mayNotBeAtEndOfForm && !atEndOfForm(), ("exiting form but not at the end of it"));
	IFF_DEBUG_FATAL(inChunk, ("exit form called within a chunk"));

	// advance beyond the form we where in
	stack[stackDepth-1].used += stack[stackDepth].length  + isizeof(Tag) + isizeof(uint32) + isizeof(Tag);

	// pop the stack frame off
	--stackDepth;
}

// ----------------------------------------------------------------------

bool Iff::enterChunk(Tag name, bool validateName, bool optional)
{
	UNREF(optional);

	// make sure it's a chunk, and check to make sure it's named appropriately
	if (!inChunk && !atEndOfForm() && isCurrentChunk() && (!validateName || getFirstTag(stackDepth) == name))
	{
		growStackAsNeeded();

		// setup the next stack frame
		Stack &s = stack[stackDepth+1];
		s.start  = stack[stackDepth].start + stack[stackDepth].used + isizeof(Tag) + isizeof(uint32);
		s.length = getLength(stackDepth);
		s.used   = 0;

		// advance the stack frame
		++stackDepth;

		// mark that we are in a chunk
		inChunk = true;

		return true;
	}

	if (!optional)
	{
		char strName[8];
		char strFirstTag[8];
		ConvertTagToString(name, strName);
		ConvertTagToString(getFirstTag(stackDepth), strFirstTag);
		FATAL(true, ("enter chunk [%s] failed: inChunk=%d, atEndOfForm()=%d, isCurrentChunk()=%d, getFirstTag(stackDepth)=[%s]", 
			strName, inChunk, atEndOfForm(), isCurrentChunk(), strFirstTag));
	}

	return false;
}

// ----------------------------------------------------------------------
/**
 * Exit the current chunk.
 * 
 * It is illegal to attempt to exit the enclosing chunk while the Iff is not
 * already within a chunk.  Doing so in a debug compile will call Fatal, but
 * in a release compile the behavior is undefined.
 * 
 * This version of the routine will validate that the chunk name that it is
 * exiting is of the specified name.  If it is not, in a debug compile the
 * 
 * @see Iff::enterForm()
 */

void Iff::exitChunk(Tag name, bool mayNotBeAtEndOfChunk)
{
	UNREF(name);
	UNREF(mayNotBeAtEndOfChunk);
	IFF_DEBUG_FATAL(!inChunk, ("not in chunk"));
	IFF_DEBUG_FATAL(!mayNotBeAtEndOfChunk && getChunkLengthLeft(), ("exiting chunk but not at the end of it"));
	IFF_FATAL(getFirstTag(stackDepth-1) != name, ("exit with incorrect name"));
	exitChunk(mayNotBeAtEndOfChunk);
}

// ----------------------------------------------------------------------
/**
 * Exit the current chunk.
 * 
 * It is illegal to attempt to exit the enclosing chunk while the Iff is not
 * already within a chunk.  Doing so in a debug compile will call Fatal, but
 * in a release compile the behavior is undefined.
 * 
 * @see Iff::enterForm()
 */

void Iff::exitChunk(bool mayNotBeAtEndOfChunk)
{
	UNREF(mayNotBeAtEndOfChunk);
	IFF_DEBUG_FATAL(!mayNotBeAtEndOfChunk && getChunkLengthLeft(), ("exiting chunk but not at the end of it"));

	// leave the chunk
	DEBUG_FATAL(!inChunk, ("not in chunk"));
	inChunk = false;

	// advance beyond the chunk we where in
	stack[stackDepth-1].used += stack[stackDepth].length + isizeof(Tag) + isizeof(uint32);

	// pop the stack frame off
	--stackDepth;
}

// ----------------------------------------------------------------------
/**
 * Skip forward past the specified number of blocks.
 * 
 * It is illegal to call this function while inside a chunk.  Doing so in
 * a debug compile will call Fatal, but the behavior will be undefined in
 * a release compile.
 * 
 * If there are fewer blocks left in the current form that the number of
 * blocks requested to skip, the behavior is determined by the optional flag.
 * If the optional flag is true, the routine will return false.  If
 * that flag is not false, in debug compiles the Fatal routine will
 * be called, but in release compiles the routine will return false.
 * 
 * @param count  Number of blocks to advance
 * @param optional  Flag to determine behavior if the operation cannot succeed
 * @return True if the specified number of blocks were skipped, otherwise false
 */

bool Iff::goForward(int count, bool optional)
{
	UNREF(optional);
	DEBUG_FATAL(inChunk, ("in chunk"));

	for ( ; count; --count)
	{
		if (atEndOfForm())
		{
			DEBUG_FATAL(!optional, ("at end of form"));
			return false;
		}

		stack[stackDepth].used += (getLength(stackDepth) + isizeof(Tag) + isizeof(uint32));
	}

	return true;
}

// ----------------------------------------------------------------------

bool Iff::seek(Tag name, BlockType type)
{
	DEBUG_FATAL(inChunk, ("in chunk"));

	while (!atEndOfForm())
	{
		if ((getCurrentName() == name) && ((type == BT_either) || ((type == BT_form && isCurrentForm()) ||  (type == BT_chunk && isCurrentChunk()))))
			return true;

		// advance past the current block
		stack[stackDepth].used += (getLength(stackDepth) + isizeof(Tag) + isizeof(uint32));
	}

	return false;
}

// ----------------------------------------------------------------------
/**
 * Return the total number of elements of data in the chunk, read or unread.
 * 
 * It is illegal to call this routine if the Iff is not currently within
 * a chunk.
 * 
 * The routine will make sure the amount of data is an even multiple of
 * the element size.  If it is not, the routine will call Fatal in debug
 * compiles, and in release compiles the number of elements will be
 * truncated to the highest integer.
 * 
 * If the element size specified is 1, this routine will return the number
 * of bytes of data in the chunk.
 * 
 * @param elementSize  Number of bytes per element
 * @return The total number of elements of data in the chunk
 * @see Iff::getChunkLengthLeft()
 */

int Iff::getChunkLengthTotal(int elementSize) const
{
	DEBUG_FATAL(!inChunk, ("not in chunk"));
	DEBUG_FATAL(stack[stackDepth].length % elementSize != 0, ("%d not a multiple of %d", stack[stackDepth].length, elementSize));
	return (stack[stackDepth].length / elementSize);
}

// ----------------------------------------------------------------------
/**
 * Return the number of unread elements of data in the chunk.
 * 
 * It is illegal to call this routine if the Iff is not currently within
 * a chunk.
 * 
 * The routine will make sure the amount of data is an even multiple of
 * the element size.  If it is not, the routine will call Fatal in debug
 * compiles, and in release compiles the number of elements will be
 * truncated to the highest integer.
 * 
 * If the element size specified is 1, this routine will return the number
 * of bytes of data left in the chunk.
 * 
 * @param elementSize  Number of bytes per element
 * @return The number of unread elements of data in the chunk
 * @see Iff::getChunkLengthTotal()
 */

int Iff::getChunkLengthLeft(int elementSize) const
{
	DEBUG_FATAL(!inChunk, ("not in chunk"));
	const int left = stack[stackDepth].length - stack[stackDepth].used;
	DEBUG_FATAL(left % elementSize != 0, ("%d not a multiple of %d", left, elementSize));
	return (left / elementSize);
}

// ----------------------------------------------------------------------

void Iff::read_misc(void *readData, int readLength)
{
	NOT_NULL(readData);
	NOT_NULL(data);
	DEBUG_FATAL(!inChunk, ("not in chunk"));

	Stack &s = stack[stackDepth];

	DEBUG_FATAL(s.used+readLength > s.length, ("overflow %d/%d in file [%s]", s.used+readLength, s.length, getFileName()));
	memcpy(readData, data+s.start+s.used, readLength);

	s.used += readLength;
}

// ----------------------------------------------------------------------

void Iff::read_floatVector(int count, Vector *array)
{
	float  f;

	NOT_NULL(array);

	for (int i = 0; i < count; ++i)
	{
		read_misc(&f, isizeof(f));
		array[i].x = static_cast<real>(f);

		read_misc(&f, isizeof(f));
		array[i].y = static_cast<real>(f);

		read_misc(&f, isizeof(f));
		array[i].z = static_cast<real>(f);
	}
}

// ----------------------------------------------------------------------

void Iff::read_floatTransform(int count, Transform *array)
{
	float     f;

	NOT_NULL(array);

	for (int i = 0; i < count; ++i)
		for (int y = 0; y < 3; ++y)
			for (int x = 0; x < 4; ++x)
			{
				read_misc(&f, isizeof(f));
				array[i].matrix[y][x] = static_cast<real>(f);
			}
}

// ----------------------------------------------------------------------

void Iff::read_floatQuaternion(int count, Quaternion *array)
{
	NOT_NULL(array);

	for (int i = 0; i < count; ++i)
	{
		Quaternion &q = array[i];
		q.w = read_float();
		q.x = read_float();
		q.y = read_float();
		q.z = read_float();
	}
}

// ----------------------------------------------------------------------

Transform Iff::read_floatTransform(void)
{
	Transform t;
	float     f;

	for (int y = 0; y < 3; ++y)
		for (int x = 0; x < 4; ++x)
		{
			read_misc(&f, isizeof(f));
			t.matrix[y][x] = static_cast<real>(f);
		}

	return t;
}

// ----------------------------------------------------------------------

Quaternion Iff::read_floatQuaternion(void)
{
	const real w = read_float();
	const real x = read_float();
	const real y = read_float();
	const real z = read_float();

	return Quaternion(w, x, y, z);
}

// ----------------------------------------------------------------------
/**
 * Read a string from the Iff.
 * 
 * The string is an 8-bit character string with a 0-valued byte as a terminator.
 * 
 * If the string is longer than the max length specified, the routine will call
 * Fatal in debug compiles.
 * 
 * If this routine attempts to read beyond the end of the chunk, this routine
 * will call Fatal in debug compiles, but its behavior is undefined in release
 * compiles.
 * 
 * @param string  Buffer to read the string into
 * @param maxLength  Size of the buffer
 */

void Iff::read_string(char *string, int maxLength)
{
	NOT_NULL(string);
	NOT_NULL(data);
	DEBUG_FATAL(!inChunk, ("not in chunk"));

	Stack &s = stack[stackDepth];

	// get a pointer to the start of the source string
	char *source = reinterpret_cast<char *>(data + s.start + s.used);

	// copy the string
	for ( ; *source; ++string, ++source, ++s.used, --maxLength)
	{
		DEBUG_FATAL(s.used >= s.length, ("hit end of chunk before string terminator"));
		DEBUG_FATAL(maxLength <= 0, ("destination string too short"));
		*string = *source;
	}

	// step over the nullptr terminator on the input
	++s.used;

	// nullptr terminate the output string
	DEBUG_FATAL(maxLength <= 0, ("destination string too short"));
	*string = '\0';
}

// ----------------------------------------------------------------------
/**
 * Read a string from the Iff.
 * 
 * The caller is responsible for freeing the memory allocated by this routine.
 * 
 * The string is an 8-bit character string with a 0-valued byte as a terminator.
 * 
 * If this routine attempts to read beyond the end of the chunk, this routine
 * will call Fatal in debug compiles, but its behavior is undefined in release
 * compiles.
 * 
 * @return Dynamically allocated string
 */

char *Iff::read_string(void)
{
	NOT_NULL(data);
	DEBUG_FATAL(!inChunk, ("not in chunk"));

	Stack &s = stack[stackDepth];

	// get a pointer to the start of the source string
	char *source       = reinterpret_cast<char *>(data + s.start + s.used);
	int   maxLength    = s.length - s.used;
	int   sourceLength = 0;

	// search for the end of the string
	for ( ; sourceLength < maxLength && source[sourceLength]; ++sourceLength)
		;

	// verify that we found the nullptr terminator
	DEBUG_FATAL(sourceLength >= maxLength, ("hit end of chunk before string terminator"));

	// create and copy the string
	++sourceLength;
	char *string = new char[static_cast<size_t>(sourceLength)];
	memcpy(string, source, sourceLength);

	// update the amount used in the chunk
	s.used += sourceLength;

	return string;
}

// ----------------------------------------------------------------------

/**
* Read a std::string from the iff.
* The string may NOT contain nulls within the data.  That is, this function only reads the contents
* of basic_string::c_str (), rather than basic_string::data ().
*
* Note that his behavior is different from the unicode version of this function.
*/

void Iff::read_string(std::string &string)
{
	NOT_NULL(data);
	DEBUG_FATAL(!inChunk, ("not in chunk"));

	Stack &s = stack[stackDepth];

	// get a pointer to the start of the source string
	char *source       = reinterpret_cast<char *>(data + s.start + s.used);
	int   maxLength    = s.length - s.used;
	int   sourceLength = 0;

	// search for the end of the string
	for ( ; sourceLength < maxLength && source[sourceLength]; ++sourceLength)
		;

	// verify that we found the nullptr terminator
	DEBUG_FATAL(sourceLength >= maxLength, ("hit end of chunk before string terminator"));

	// account for the nullptr terminator
	++sourceLength;

	s.used += sourceLength;
	string.reserve(sourceLength);
	string = source;
}

// ----------------------------------------------------------------------

std::string Iff::read_stdstring()
{
	std::string str;
	read_string (str);
	return str;
}

//----------------------------------------------------------------------

/**
* Read a Unicode string from the iff.
* The string may contain nulls within the data.  That is, this function can read the entire contents
* of basic_string::data (), rather than basic_string::c_str ()
*/

//----------------------------------------------------------------------

void  Iff::read_string(Unicode::String &str)
{
	const int32 count = read_int32 ();
	unsigned short * data = new unsigned short [count];
	read_uint16 (count, data);
	str.assign (data, static_cast<size_t>(count));
	delete [] data;
}

/**
* Read a Unicode string from the iff.
* The string may contain nulls within the data.  That is, this function can read the entire contents
* of basic_string::data (), rather than basic_string::c_str ()
*/

//----------------------------------------------------------------------

Unicode::String Iff::read_unicodeString()
{
	Unicode::String str;
	read_string (str);
	return str;
}

// ----------------------------------------------------------------------
/**
 * Read a vector from the current chunk.
 * 
 * If this routine attempts to read beyond the end of the chunk, this routine
 * will call Fatal in debug compiles, but its behavior is undefined in release
 * compiles.
 * 
 * @return The value of the vector that was in the chunk
 * @see Iff::read_*()
 */

Vector Iff::read_floatVector(void)
{
	float  f;
	real   x, y, z;

	read_misc(&f, isizeof(f));
	x = static_cast<real>(f);

	read_misc(&f, isizeof(f));
	y = static_cast<real>(f);

	read_misc(&f, isizeof(f));
	z = static_cast<real>(f);

	return Vector(x, y, z);
}

// ----------------------------------------------------------------------
/**
 * Read a vector from the current chunk.
 * 
 * If this routine attempts to read beyond the end of the chunk, this routine
 * will call Fatal in debug compiles, but its behavior is undefined in release
 * compiles.
 * 
 * @return The value of the vector that was in the chunk
 * @see Iff::read_*()
 */

VectorArgb Iff::read_floatVectorArgb(void)
{
	float  f;
	real   a, r, g, b;

	read_misc(&f, isizeof(f));
	a = static_cast<real>(f);

	read_misc(&f, isizeof(f));
	r = static_cast<real>(f);

	read_misc(&f, isizeof(f));
	g = static_cast<real>(f);

	read_misc(&f, isizeof(f));
	b = static_cast<real>(f);

	return VectorArgb(a, r, g, b);
}

// ======================================================================

