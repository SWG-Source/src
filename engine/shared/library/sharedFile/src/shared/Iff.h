// ======================================================================
//
// Iff.h
//
// copyright 1998 Bootprint Entertainment
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_Iff_H
#define INCLUDED_Iff_H

// ======================================================================

#include "sharedFoundation/Tag.h"

class AbstractFile;
class Quaternion;
class Transform;
class Vector;
class VectorArgb;

// ======================================================================

class Iff
{
public:

	enum SeekType
	{
		SeekBegin,
		SeekCurrent,
		SeekEnd
	};

private:

	// explicitly disable these routines
	Iff(const Iff &);
	Iff &operator =(const Iff &);

private:

	enum
	{
		DEFAULT_STACK_DEPTH = 64
	};

	enum BlockType
	{
		BT_either,
		BT_form,
		BT_chunk
	};

	struct Stack
	{
		int start;
		int length;
		int used;
	};

private:

	char  *fileName;

	int    maxStackDepth;
	int    stackDepth;
	Stack *stack;

	int    length;
	byte  *data;

	bool   inChunk;
	bool   growable;
	bool   nonlinear;
	bool   ownsData;

private:

	Tag  getFirstTag(int depth) const;
	int  getLength(int depth, int offset=0) const;
	Tag  getSecondTag(int depth) const;
	Tag  getBlockName(int depth) const;
	
	void fatal(const char *string) const;
	void read_misc(void *data, int length);
	void growStackAsNeeded(void);
	void adjustDataAsNeeded(int size);
	int  calculateRawDataSize(void) const;

	bool enterForm(Tag name, bool validateName, bool optional);
	bool enterChunk(Tag name, bool validateName, bool optional);

	bool seek(Tag name, BlockType blockType);

public:

	static void install();
	static bool isValid(char const * fileName);

public:

	Iff(void);
	Iff(int newDataSize, const byte *newData, bool iffOwnsData=true);
	explicit Iff(const char *fileName, bool optional=false);
	explicit Iff(int initialSize, bool growable=true, bool clearDataBuffer=false);
	~Iff(void);

	const char* getFileName (void) const;

	uint32 calculateCrc() const;

	// open and close operations
	bool open(const char *filename, bool optional=false);
	void open(AbstractFile & file);
	void open(AbstractFile & file, char const * fileName);
	void close(void);
	bool write(const char *filename, bool optional=false);

	// raw data retrieval
	const byte *getRawData(void) const;
	int         getRawDataSize(void) const;

	void formatLocation(char *buffer, int bufferLength) const;

	// creating forms and chunk
	void insertIff(const Iff *iff);
	void insertForm(Tag name, bool shouldEnterForm=true);
	void insertChunk(Tag name, bool shouldEnterChunk=true);
	void insertChunkData(const void *data, int length);
	void insertChunkString(const char *string);
	void insertChunkString(const Unicode::String & str);
	void insertChunkFloatVector(const Vector &vector);
	void insertChunkFloatVectorArgb (const VectorArgb& color);
	void insertChunkFloatTransform(const Transform &transform);
	void insertChunkFloatQuaternion(const Quaternion &quaternion);

	template <class T> void insertChunkArray(T const * array, int size)
	{
		insertChunkData(array,sizeof(T) * size);
	}
	template <class T> void insertChunkData(const T &object)
	{
		// -qq- I really don't want to put the body here, but it's the only way it works in MSVC 5.0
		insertChunkData(&object, sizeof(object));
	}

	// delete data from the chunk data
	void deleteChunkData(int length);

	// nonlinear functions
	void allowNonlinearFunctions(void);
	void seekWithinChunk(int offset, SeekType seekType);
	void goToTopOfForm(void);

	// get information on current block	
	Tag  getCurrentName(void) const;
	int  getCurrentLength(void) const;
	bool isCurrentChunk(void) const;
	bool isCurrentForm(void) const;
	bool atEndOfForm(void) const;

	// get the number of blocks left in the current enclosing form	
	int getNumberOfBlocksLeft(void) const;

	// get information about the number of bytes in the current chunk
	int getChunkLengthTotal(int elementSize=1) const;
	int getChunkLengthLeft(int elementSize=1) const;

	// enter/exit forms
	void enterForm(void);
	void enterForm(Tag name);
	bool enterForm(bool optional);
	bool enterForm(Tag name, bool optional);
	void exitForm(Tag name, bool mayNotBeAtEndOfForm=false);
	void exitForm(bool mayNotBeAtEndOfForm=false);

	// enter/exit chunks
	void enterChunk(void);
	void enterChunk(Tag name);
	bool enterChunk(bool optional);
	bool enterChunk(Tag name, bool optional);
	void exitChunk(Tag name, bool mayNotBeAtEndOfChunk=false);
	void exitChunk(bool mayNotBeAtEndOfChunk=false);

	// skip forward blocks
	bool goForward(int count=1, bool optional=false);
	bool seek(Tag name);
	bool seekForm(Tag name);
	bool seekChunk(Tag name);

	// read single entities
	bool        read_bool8(void);
	int8        read_int8(void);
	int16       read_int16(void);
	int32       read_int32(void);
	uint8       read_uint8(void);
	uint16      read_uint16(void);
	uint32      read_uint32(void);
	real        read_float(void);
	Vector      read_floatVector(void);
	VectorArgb  read_floatVectorArgb(void);
	Transform   read_floatTransform(void);
	Quaternion  read_floatQuaternion(void);

	// read arrays
	void    read_int8  (int count, int8   *array);
	void    read_int16 (int count, int16  *array);
	void    read_int32 (int count, int32  *array);
	void    read_uint8 (int count, uint8  *array);
	void    read_uint16(int count, uint16 *array);
	void    read_uint32(int count, uint32 *array);
	void    read_char  (int count, char   *array);
	void    read_floatVector(int count, Vector *array);
	void    read_floatTransform(int count, Transform *array);
	void    read_floatQuaternion(int count, Quaternion *array);

	int8   *read_int8  (int count);
	int16  *read_int16 (int count);
	int32  *read_int32 (int count);
	uint8  *read_uint8 (int count);
	uint16 *read_uint16(int count);
	uint32 *read_uint32(int count);
	char   *read_char  (int count);

	void    readRest_int8  (int8   *array, int numberOfElements);
	void    readRest_int16 (int16  *array, int numberOfElements);
	void    readRest_int32 (int32  *array, int numberOfElements);
	void    readRest_uint8 (uint8  *array, int numberOfElements);
	void    readRest_uint16(uint16 *array, int numberOfElements);
	void    readRest_uint32(uint32 *array, int numberOfElements);
	void    readRest_char  (char   *array, int numberOfElements);

	int8   *readRest_int8  (void);
	int16  *readRest_int16 (void);
	int32  *readRest_int32 (void);
	uint8  *readRest_uint8 (void);
	uint16 *readRest_uint16(void);
	uint32 *readRest_uint32(void);
	char   *readRest_char  (void);

	void  read_string(char *string, int maxLength);
	char *read_string(void);

	void        read_string(std::string &string);
	std::string read_stdstring();

	void            read_string(Unicode::String &str);
	Unicode::String read_unicodeString();
};

// ----------------------------------------------------------------------
/**
 * retrieve name of the iff file
 * 
 * @return iff name
 */

inline const char* Iff::getFileName (void) const
{
	return fileName;
}

// ======================================================================
// retrieve the number of bytes contained in the raw iff data buffer
//
// Return Value:
//
//   The number of bytes of raw iff data stored in the buffer returned
//   by Iff::getRawData().
//
// See Also:
//
//   Iff:getRawData()

inline int Iff::getRawDataSize(void) const
{
	if (!data || !stack)
		return 0;
	else
		return stack[0].length;
}

// ----------------------------------------------------------------------
/**
 * retrieve a read-only pointer to the raw data managed by the Iff.
 * 
 * This routine can be used by utilities that need the ability
 * to store raw iff data via a mechanism other than Iff::write().
 * 
 * @return A read-only pointer to the data buffer containing the raw Iff data
 * managed by this Iff object.  It may be nullptr if no data is currently
 * managed by the Iff.
 * @see Iff::getRawDataSize(), Iff::write()
 */

inline const byte *Iff::getRawData(void) const
{
	return data;
}

// ----------------------------------------------------------------------
/**
 * Allow use of nonlinear functions.
 * 
 * This routine will enable the Iff to perform the nonlinear movement
 * functions, listed in the "see also" section.
 * 
 * @see Iff::goToTopOfForm(), Iff::seekWithinChunk()
 */

inline void Iff::allowNonlinearFunctions(void)
{
	nonlinear = true;
}

// ----------------------------------------------------------------------
/**
 * Return the name of the currently selected block.
 * 
 * If at the end of the current form, so there is no currently selected block,
 * this routine will call Fatal for debug compiles, but its behavior will be
 * undefined for release compiles.
 * 
 * @return The tag for the currently selected block
 * @see Iff::atEndOfForm()
 */

inline Tag Iff::getCurrentName(void) const
{
	return getBlockName(stackDepth);
}

// ----------------------------------------------------------------------
/**
 * Enter the next form.
 * 
 * This routine will enter the next form in the Iff regardless of its name.
 * If the Iff is not positioned to enter a new form, it will call Fatal.
 */

inline void Iff::enterForm(void)
{
	static_cast<void>(enterForm(0, false, false));
}

// ----------------------------------------------------------------------
/**
 * Enter the next form.
 * 
 * This routine will check to make sure that the Iff is positioned to enter
 * a new form with the specified name, and enter it.  If the Iff is not pointing
 * at a form, or the name isn't correct, Fatal will be called.
 * 
 * @param name  Name of the next form
 */

inline void Iff::enterForm(Tag name)
{
	static_cast<void>(enterForm(name, true, false));
}

// ----------------------------------------------------------------------
/**
 * Enter the next form.
 * 
 * This routine will enter the next form in the Iff regardless of its name.
 * If the Iff is not positioned to enter a new form, the behavior depends upon
 * the optional flag.  If the optional flag is true, the routine will return
 * false, otherwise it will call Fatal.
 */

inline bool Iff::enterForm(bool optional)
{
	return enterForm(0, false, optional);
}

// ----------------------------------------------------------------------
/**
 * Enter the next form.
 * 
 * This routine will check to make sure that the Iff is positioned to enter
 * a new form with the specified name, and enter it.  If the Iff is not pointing
 * at a form, or the name isn't correct, the behavior depends upon
 * the optional flag.  If the optional flag is true, the routine will return
 * false, otherwise it will call Fatal.
 */

inline bool Iff::enterForm(Tag name, bool optional)
{
	return enterForm(name, true, optional);
}

// ----------------------------------------------------------------------
/**
 * Enter the next chunk.
 * 
 * This routine will enter the next chunk in the Iff regardless of its name.
 * If the Iff is not positioned to enter a new chunk, it will call Fatal.
 */

inline void Iff::enterChunk(void)
{
	static_cast<void>(enterChunk(0, false, false));
}

// ----------------------------------------------------------------------
/**
 * Enter the next chunk.
 * 
 * This routine will check to make sure that the Iff is positioned to enter
 * a new chunk with the specified name, and enter it.  If the Iff is not pointing
 * at a chunk, or the name isn't correct, Fatal will be called.
 */

inline void Iff::enterChunk(Tag name)
{
	static_cast<void>(enterChunk(name, true, false));
}

// ----------------------------------------------------------------------
/**
 * Enter the next chunk.
 * 
 * This routine will enter the next chunk in the Iff regardless of its name.
 * If the Iff is not positioned to enter a new chunk, the behavior depends upon
 * the optional flag.  If the optional flag is true, the routine will return
 * false, otherwise it will call Fatal.
 */

inline bool Iff::enterChunk(bool optional)
{
	return enterChunk(0, false, optional);
}

// ----------------------------------------------------------------------
/**
 * Enter the next chunk.
 * 
 * This routine will check to make sure that the Iff is positioned to enter
 * a new chunk with the specified name, and enter it.  If the Iff is not pointing
 * at a chunk, or the name isn't correct, the behavior depends upon
 * the optional flag.  If the optional flag is true, the routine will return
 * false, otherwise it will call Fatal.
 */

inline bool Iff::enterChunk(Tag name, bool optional)
{
	return enterChunk(name, true, optional);
}

// ----------------------------------------------------------------------
/**
 * Seek forward through the current enclosing form looking for the named block.
 * 
 * The block may be either a chunk or a form.  If the block is not found,
 * the iff read position will be left at the end of the block.
 * 
 * @param name The tag of the block to seek for.
 * @return True if the named block was found, otherwise false.
 */

inline bool Iff::seek(Tag name)
{
	return seek(name, BT_either);
}

// ----------------------------------------------------------------------
/**
 * Seek forward through the current enclosing form looking for the named form.
 * 
 * If the block is not found, the iff read position will be left at the end of the enclosing form.
 * 
 * @param name The tag of the form to seek for.
 * @return True if the named form was found, otherwise false.
 */

inline bool Iff::seekForm(Tag name)
{
	return seek(name, BT_form);
}

// ----------------------------------------------------------------------
/**
 * Seek forward through the current enclosing form looking for the named chunk.
 * 
 * If the block is not found, the iff read position will be left at the end of the enclosing form.
 * 
 * @param name The tag of the chunk to seek for.
 * @return True if the named chunk was found, otherwise false.
 */

inline bool Iff::seekChunk(Tag name)
{
	return seek(name, BT_chunk);
}

// ----------------------------------------------------------------------
/**
 * Read a bool from the current chunk.
 * 
 * The bool is read as an 8 bit value.  If the value is 0, the result is false,
 * otherwise it is true.
 * 
 * If this routine attempts to read beyond the end of the chunk, this routine
 * will call Fatal in debug compiles, but its behavior is undefined in release
 * compiles.
 * 
 * @return The value of the bool that was in the chunk.
 * @see    Iff::read_*()
 */

inline bool Iff::read_bool8(void)
{
	int8 i;
	read_misc(&i, isizeof(i));
	return (i != 0);
}

// ----------------------------------------------------------------------
/**
 * Read an int8 from the current chunk.
 * 
 * If this routine attempts to read beyond the end of the chunk, this routine
 * will call Fatal in debug compiles, but its behavior is undefined in release
 * compiles.
 * 
 * @return The value of the int8 that was in the chunk
 * @see Iff::read_*()
 */

inline int8 Iff::read_int8(void)
{
	int8 i;
	read_misc(&i, isizeof(i));
	return i;
}

// ----------------------------------------------------------------------
/**
 * Read an int16 from the current chunk.
 * 
 * If this routine attempts to read beyond the end of the chunk, this routine
 * will call Fatal in debug compiles, but its behavior is undefined in release
 * compiles.
 * 
 * @return The value of the int16 that was in the chunk
 * @see Iff::read_*()
 */

inline int16 Iff::read_int16(void)
{
	int16 i;
	read_misc(&i, isizeof(i));
	return i;
}

// ----------------------------------------------------------------------
/**
 * Read an int32 from the current chunk.
 * 
 * If this routine attempts to read beyond the end of the chunk, this routine
 * will call Fatal in debug compiles, but its behavior is undefined in release
 * compiles.
 * 
 * @return The value of the int32 that was in the chunk
 * @see Iff::read_*()
 */

inline int32 Iff::read_int32(void)
{
	int32 i;
	read_misc(&i, isizeof(i));
	return i;
}

// ----------------------------------------------------------------------
/**
 * Read an uint8 from the current chunk.
 * 
 * If this routine attempts to read beyond the end of the chunk, this routine
 * will call Fatal in debug compiles, but its behavior is undefined in release
 * compiles.
 * 
 * @return The value of the uint8 that was in the chunk
 * @see Iff::read_*()
 */

inline uint8 Iff::read_uint8(void)
{
	uint8 i;
	read_misc(&i, isizeof(i));
	return i;
}

// ----------------------------------------------------------------------
/**
 * Read an uint16 from the current chunk.
 * 
 * If this routine attempts to read beyond the end of the chunk, this routine
 * will call Fatal in debug compiles, but its behavior is undefined in release
 * compiles.
 * 
 * @return The value of the uint16 that was in the chunk
 * @see Iff::read_*()
 */

inline uint16 Iff::read_uint16(void)
{
	uint16 i;
	read_misc(&i, isizeof(i));
	return i;
}

// ----------------------------------------------------------------------
/**
 * Read an uint32 from the current chunk.
 * 
 * If this routine attempts to read beyond the end of the chunk, this routine
 * will call Fatal in debug compiles, but its behavior is undefined in release
 * compiles.
 * 
 * @return The value of the uint32 that was in the chunk
 * @see Iff::read_*()
 */

inline uint32 Iff::read_uint32(void)
{
	uint32 i;
	read_misc(&i, isizeof(i));
	return i;
}

// ----------------------------------------------------------------------
/**
 * Read an float from the current chunk.
 * 
 * If this routine attempts to read beyond the end of the chunk, this routine
 * will call Fatal in debug compiles, but its behavior is undefined in release
 * compiles.
 * 
 * @return The value of the float that was in the chunk
 * @see Iff::read_*()
 */

inline real Iff::read_float(void)
{
	float  f;
	read_misc(&f, isizeof(f));
	return static_cast<real>(f);
}

// ----------------------------------------------------------------------
/**
 * Read an array of int8's from the current chunk into a specified array.
 * 
 * If this routine attempts to read beyond the end of the chunk, this routine
 * will call Fatal in debug compiles, but its behavior is undefined in release
 * compiles.
 * 
 * @param count  Number of elements to read
 * @param array  Array to store the entries
 * @see Iff::read_*()
 */

inline void Iff::read_int8(int count, int8 *array)
{
	read_misc(array, count * isizeof(*array));
}

// ----------------------------------------------------------------------
/**
 * Read an array of int16's from the current chunk into a specified array.
 * 
 * If this routine attempts to read beyond the end of the chunk, this routine
 * will call Fatal in debug compiles, but its behavior is undefined in release
 * compiles.
 * 
 * @param count  Number of elements to read
 * @param array  Array to store the entries
 * @see Iff::read_*()
 */

inline void Iff::read_int16(int count, int16 *array)
{
	read_misc(array, count * isizeof(*array));
}

// ----------------------------------------------------------------------
/**
 * Read an array of int32's from the current chunk into a specified array.
 * 
 * If this routine attempts to read beyond the end of the chunk, this routine
 * will call Fatal in debug compiles, but its behavior is undefined in release
 * compiles.
 * 
 * @param count  Number of elements to read
 * @param array  Array to store the entries
 * @see Iff::read_*()
 */

inline void Iff::read_int32(int count, int32 *array)
{
	read_misc(array, count * isizeof(*array));
}

// ----------------------------------------------------------------------
/**
 * Read an array of int8's from the current chunk into a specified array.
 * 
 * If this routine attempts to read beyond the end of the chunk, this routine
 * will call Fatal in debug compiles, but its behavior is undefined in release
 * compiles.
 * 
 * @param count  Number of elements to read
 * @param array  Array to store the entries
 * @see Iff::read_*()
 */

inline void Iff::read_uint8(int count, uint8 *array)
{
	read_misc(array, count * isizeof(*array));
}

// ----------------------------------------------------------------------
/**
 * Read an array of int16's from the current chunk into a specified array.
 * 
 * If this routine attempts to read beyond the end of the chunk, this routine
 * will call Fatal in debug compiles, but its behavior is undefined in release
 * compiles.
 * 
 * @param count  Number of elements to read
 * @param array  Array to store the entries
 * @see Iff::read_*()
 */

inline void Iff::read_uint16(int count, uint16 *array)
{
	read_misc(array, count * isizeof(*array));
}

// ----------------------------------------------------------------------
/**
 * Read an array of int32's from the current chunk into a specified array.
 * 
 * If this routine attempts to read beyond the end of the chunk, this routine
 * will call Fatal in debug compiles, but its behavior is undefined in release
 * compiles.
 * 
 * @param count  Number of elements to read
 * @param array  Array to store the entries
 * @see Iff::read_*()
 */

inline void Iff::read_uint32(int count, uint32 *array)
{
	read_misc(array, count * isizeof(*array));
}

// ----------------------------------------------------------------------
/**
 * Read an array of int32's from the current chunk into a specified array.
 * 
 * If this routine attempts to read beyond the end of the chunk, this routine
 * will call Fatal in debug compiles, but its behavior is undefined in release
 * compiles.
 * 
 * @param count  Number of elements to read
 * @param array  Array to store the entries
 * @see Iff::read_*()
 */

inline void Iff::read_char(int count, char *array)
{
	read_misc(array, count * isizeof(*array));
}

// ----------------------------------------------------------------------
/**
 * Read an array of int8's from the current chunk into a dynamically allocated array.
 * 
 * The caller is responsible for freeing the memory allocated by this routine.
 * 
 * If this routine attempts to read beyond the end of the chunk, this routine
 * will call Fatal in debug compiles, but its behavior is undefined in release
 * compiles.
 * 
 * @param count  Number of elements to read
 * @return Dyanmically allocated array containing the data
 * @see Iff::read_*()
 */

inline int8 *Iff::read_int8(int count)
{
	int8 *array = new int8[static_cast<size_t>(count)];
	read_int8(count, array);
	return array;
}

// ----------------------------------------------------------------------
/**
 * Read an array of int16's from the current chunk into a dynamically allocated array.
 * 
 * The caller is responsible for freeing the memory allocated by this routine.
 * 
 * If this routine attempts to read beyond the end of the chunk, this routine
 * will call Fatal in debug compiles, but its behavior is undefined in release
 * compiles.
 * 
 * @param count  Number of elements to read
 * @return Dyanmically allocated array containing the data
 * @see Iff::read_*()
 */

inline int16 *Iff::read_int16(int count)
{
	int16 *array = new int16[static_cast<size_t>(count)];
	read_int16(count, array);
	return array;
}

// ----------------------------------------------------------------------
/**
 * Read an array of int32's from the current chunk into a dynamically allocated array.
 * 
 * The caller is responsible for freeing the memory allocated by this routine.
 * 
 * If this routine attempts to read beyond the end of the chunk, this routine
 * will call Fatal in debug compiles, but its behavior is undefined in release
 * compiles.
 * 
 * @param count  Number of elements to read
 * @return Dyanmically allocated array containing the data
 * @see Iff::read_*()
 */

inline int32 *Iff::read_int32(int count)
{
	int32 *array = new int32[static_cast<size_t>(count)];
	read_int32(count, array);
	return array;
}

// ----------------------------------------------------------------------
/**
 * Read an array of uint8's from the current chunk into a dynamically allocated array.
 * 
 * The caller is responsible for freeing the memory allocated by this routine.
 * 
 * If this routine attempts to read beyond the end of the chunk, this routine
 * will call Fatal in debug compiles, but its behavior is undefined in release
 * compiles.
 * 
 * @param count  Number of elements to read
 * @return Dyanmically allocated array containing the data
 * @see Iff::read_*()
 */

inline uint8 *Iff::read_uint8(int count)
{
	uint8 *array = new uint8[static_cast<size_t>(count)];
	read_uint8(count, array);
	return array;
}

// ----------------------------------------------------------------------
/**
 * Read an array of uint16's from the current chunk into a dynamically allocated array.
 * 
 * The caller is responsible for freeing the memory allocated by this routine.
 * 
 * If this routine attempts to read beyond the end of the chunk, this routine
 * will call Fatal in debug compiles, but its behavior is undefined in release
 * compiles.
 * 
 * @param count  Number of elements to read
 * @return Dyanmically allocated array containing the data
 * @see Iff::read_*()
 */

inline uint16 *Iff::read_uint16(int count)
{
	uint16 *array = new uint16[static_cast<size_t>(count)];
	read_uint16(count, array);
	return array;
}

// ----------------------------------------------------------------------
/**
 * Read an array of uint32's from the current chunk into a dynamically allocated array.
 * 
 * The caller is responsible for freeing the memory allocated by this routine.
 * 
 * If this routine attempts to read beyond the end of the chunk, this routine
 * will call Fatal in debug compiles, but its behavior is undefined in release
 * compiles.
 * 
 * @param count  Number of elements to read
 * @return Dyanmically allocated array containing the data
 * @see Iff::read_*()
 */

inline uint32 *Iff::read_uint32(int count)
{
	uint32 *array = new uint32[static_cast<size_t>(count)];
	read_uint32(count, array);
	return array;
}

// ----------------------------------------------------------------------
/**
 * Read an array of char's from the current chunk into a dynamically allocated array.
 * 
 * The caller is responsible for freeing the memory allocated by this routine.
 * 
 * If this routine attempts to read beyond the end of the chunk, this routine
 * will call Fatal in debug compiles, but its behavior is undefined in release
 * compiles.
 * 
 * @param count  Number of elements to read
 * @return Dyanmically allocated array containing the data
 * @see Iff::read_*()
 */

inline char *Iff::read_char(int count)
{
	char *array = new char[static_cast<size_t>(count)];
	read_char(count, array);
	return array;
}

// ----------------------------------------------------------------------
/**
 * Read an array of int8's from the current chunk into a pre-allocated array.
 * 
 * The number of elements that this routine reads will be determined by the
 * amount of data left in the chunk.  If the amount of data left is not a
 * exact multiple of the element size of the array, the routine will call
 * Fatal in debug compiles, and will truncate the number of items in release
 * compiles.
 * 
 * If the read will overwrite the specified array, in debug compiles the
 * routine will call Fatal, but in release compiles the effect will be
 * undefined.
 * 
 * If this routine attempts to read beyond the end of the chunk, this routine
 * will call Fatal in debug compiles, but its behavior is undefined in release
 * compiles.
 * 
 * @param array  Array to read the data into
 * @param numberOfElements  Maximum number of elements inthe array
 * @see Iff::read_*()
 */

inline void Iff::readRest_int8(int8 *array, int numberOfElements)
{
	UNREF(numberOfElements);
	const int count = getChunkLengthLeft(isizeof(*array));
	DEBUG_FATAL(count > numberOfElements, ("Iff::ReadRest_int8 overflow array %d/%d", count, numberOfElements));
	read_int8(count, array);
}

// ----------------------------------------------------------------------
/**
 * Read an array of int16's from the current chunk into a pre-allocated array.
 * 
 * The number of elements that this routine reads will be determined by the
 * amount of data left in the chunk.  If the amount of data left is not a
 * exact multiple of the element size of the array, the routine will call
 * Fatal in debug compiles, and will truncate the number of items in release
 * compiles.
 * 
 * If the read will overwrite the specified array, in debug compiles the
 * routine will call Fatal, but in release compiles the effect will be
 * undefined.
 * 
 * If this routine attempts to read beyond the end of the chunk, this routine
 * will call Fatal in debug compiles, but its behavior is undefined in release
 * compiles.
 * 
 * @param array  Array to read the data into
 * @param numberOfElements  Maximum number of elements inthe array
 * @see Iff::read_*()
 */

inline void Iff::readRest_int16(int16 *array, int numberOfElements)
{
	UNREF(numberOfElements);
	const int count = getChunkLengthLeft(isizeof(*array));
	DEBUG_FATAL(count > numberOfElements, ("Iff::ReadRest_int8 overflow array %d/%d", count, numberOfElements));
	read_int16(count, array);
}

// ----------------------------------------------------------------------
/**
 * Read an array of int32's from the current chunk into a pre-allocated array.
 * 
 * The number of elements that this routine reads will be determined by the
 * amount of data left in the chunk.  If the amount of data left is not a
 * exact multiple of the element size of the array, the routine will call
 * Fatal in debug compiles, and will truncate the number of items in release
 * compiles.
 * 
 * If the read will overwrite the specified array, in debug compiles the
 * routine will call Fatal, but in release compiles the effect will be
 * undefined.
 * 
 * If this routine attempts to read beyond the end of the chunk, this routine
 * will call Fatal in debug compiles, but its behavior is undefined in release
 * compiles.
 * 
 * @param array  Array to read the data into
 * @param numberOfElements  Maximum number of elements inthe array
 * @see Iff::read_*()
 */

inline void Iff::readRest_int32(int32 *array, int numberOfElements)
{
	UNREF(numberOfElements);
	const int count = getChunkLengthLeft(isizeof(*array));
	DEBUG_FATAL(count > numberOfElements, ("Iff::ReadRest_int8 overflow array %d/%d", count, numberOfElements));
	read_int32(count, array);
}

// ----------------------------------------------------------------------
/**
 * Read an array of uint8's from the current chunk into a pre-allocated array.
 * 
 * The number of elements that this routine reads will be determined by the
 * amount of data left in the chunk.  If the amount of data left is not a
 * exact multiple of the element size of the array, the routine will call
 * Fatal in debug compiles, and will truncate the number of items in release
 * compiles.
 * 
 * If the read will overwrite the specified array, in debug compiles the
 * routine will call Fatal, but in release compiles the effect will be
 * undefined.
 * 
 * If this routine attempts to read beyond the end of the chunk, this routine
 * will call Fatal in debug compiles, but its behavior is undefined in release
 * compiles.
 * 
 * @param array  Array to read the data into
 * @param numberOfElements  Maximum number of elements inthe array
 * @see Iff::read_*()
 */

inline void Iff::readRest_uint8(uint8 *array, int numberOfElements)
{
	UNREF(numberOfElements);
	const int count = getChunkLengthLeft(isizeof(*array));
	DEBUG_FATAL(count > numberOfElements, ("Iff::ReadRest_int8 overflow array %d/%d", count, numberOfElements));
	read_uint8(count, array);
}

// ----------------------------------------------------------------------
/**
 * Read an array of uint16's from the current chunk into a pre-allocated array.
 * 
 * The number of elements that this routine reads will be determined by the
 * amount of data left in the chunk.  If the amount of data left is not a
 * exact multiple of the element size of the array, the routine will call
 * Fatal in debug compiles, and will truncate the number of items in release
 * compiles.
 * 
 * If the read will overwrite the specified array, in debug compiles the
 * routine will call Fatal, but in release compiles the effect will be
 * undefined.
 * 
 * If this routine attempts to read beyond the end of the chunk, this routine
 * will call Fatal in debug compiles, but its behavior is undefined in release
 * compiles.
 * 
 * @param array  Array to read the data into
 * @param numberOfElements  Maximum number of elements inthe array
 * @see Iff::read_*()
 */

inline void Iff::readRest_uint16(uint16 *array, int numberOfElements)
{
	UNREF(numberOfElements);
	const int count = getChunkLengthLeft(isizeof(*array));
	DEBUG_FATAL(count > numberOfElements, ("Iff::ReadRest_int8 overflow array %d/%d", count, numberOfElements));
	read_uint16(count, array);
}

// ----------------------------------------------------------------------
/**
 * Read an array of uint32's from the current chunk into a pre-allocated array.
 * 
 * The number of elements that this routine reads will be determined by the
 * amount of data left in the chunk.  If the amount of data left is not a
 * exact multiple of the element size of the array, the routine will call
 * Fatal in debug compiles, and will truncate the number of items in release
 * compiles.
 * 
 * If the read will overwrite the specified array, in debug compiles the
 * routine will call Fatal, but in release compiles the effect will be
 * undefined.
 * 
 * If this routine attempts to read beyond the end of the chunk, this routine
 * will call Fatal in debug compiles, but its behavior is undefined in release
 * compiles.
 * 
 * @param array  Array to read the data into
 * @param numberOfElements  Maximum number of elements inthe array
 * @see Iff::read_*()
 */

inline void Iff::readRest_uint32(uint32 *array, int numberOfElements)
{
	UNREF(numberOfElements);
	const int count = getChunkLengthLeft(isizeof(*array));
	DEBUG_FATAL(count > numberOfElements, ("Iff::ReadRest_int8 overflow array %d/%d", count, numberOfElements));
	read_uint32(count, array);
}

// ----------------------------------------------------------------------
/**
 * Read an array of char's from the current chunk into a pre-allocated array.
 * 
 * The number of elements that this routine reads will be determined by the
 * amount of data left in the chunk.  If the amount of data left is not a
 * exact multiple of the element size of the array, the routine will call
 * Fatal in debug compiles, and will truncate the number of items in release
 * compiles.
 * 
 * If the read will overwrite the specified array, in debug compiles the
 * routine will call Fatal, but in release compiles the effect will be
 * undefined.
 * 
 * If this routine attempts to read beyond the end of the chunk, this routine
 * will call Fatal in debug compiles, but its behavior is undefined in release
 * compiles.
 * 
 * @param array  Array to read the data into
 * @param numberOfElements  Maximum number of elements inthe array
 * @see Iff::read_*()
 */

inline void Iff::readRest_char(char *array, int numberOfElements)
{
	UNREF(numberOfElements);
	const int count = getChunkLengthLeft(isizeof(*array));
	DEBUG_FATAL(count > numberOfElements, ("Iff::ReadRest_int8 overflow array %d/%d", count, numberOfElements));
	read_char(count, array);
}

// ----------------------------------------------------------------------
/**
 * Read an array of int8's from the current chunk into a dynamically allocated array.
 * 
 * The caller is responsible for freeing the memory allocated by this routine.
 * 
 * The number of elements that this routine reads will be determined by the
 * amount of data left in the chunk.  If the amount of data left is not a
 * exact multiple of the element size of the array, the routine will call
 * Fatal in debug compiles, and will truncate the number of items in release
 * compiles.
 * 
 * If this routine attempts to read beyond the end of the chunk, this routine
 * will call Fatal in debug compiles, but its behavior is undefined in release
 * compiles.
 * 
 * @return Dyanmically allocated array containing the data
 * @see Iff::read_*()
 */

inline int8 *Iff::readRest_int8(void)
{
	const int    count = getChunkLengthLeft(isizeof(int8));
	int8 *const  array = new int8[static_cast<size_t>(count)];
	read_int8(count, array);
	return array;
}

// ----------------------------------------------------------------------
/**
 * Read an array of int16's from the current chunk into a dynamically allocated array.
 * 
 * The caller is responsible for freeing the memory allocated by this routine.
 * 
 * The number of elements that this routine reads will be determined by the
 * amount of data left in the chunk.  If the amount of data left is not a
 * exact multiple of the element size of the array, the routine will call
 * Fatal in debug compiles, and will truncate the number of items in release
 * compiles.
 * 
 * If this routine attempts to read beyond the end of the chunk, this routine
 * will call Fatal in debug compiles, but its behavior is undefined in release
 * compiles.
 * 
 * @return Dyanmically allocated array containing the data
 * @see Iff::read_*()
 */

inline int16 *Iff::readRest_int16(void)
{
	const int     count = getChunkLengthLeft(isizeof(int16));
	int16 * const array = new int16[static_cast<size_t>(count)];
	read_int16(count, array);
	return array;
}

// ----------------------------------------------------------------------
/**
 * Read an array of int32's from the current chunk into a dynamically allocated array.
 * 
 * The caller is responsible for freeing the memory allocated by this routine.
 * 
 * The number of elements that this routine reads will be determined by the
 * amount of data left in the chunk.  If the amount of data left is not a
 * exact multiple of the element size of the array, the routine will call
 * Fatal in debug compiles, and will truncate the number of items in release
 * compiles.
 * 
 * If this routine attempts to read beyond the end of the chunk, this routine
 * will call Fatal in debug compiles, but its behavior is undefined in release
 * compiles.
 * 
 * @return Dyanmically allocated array containing the data
 * @see Iff::read_*()
 */

inline int32 *Iff::readRest_int32(void)
{
	const int     count = getChunkLengthLeft(isizeof(int32));
	int32 * const array = new int32[static_cast<size_t>(count)];
	read_int32(count, array);
	return array;
}

// ----------------------------------------------------------------------
/**
 * Read an array of uint8's from the current chunk into a dynamically allocated array.
 * 
 * The caller is responsible for freeing the memory allocated by this routine.
 * 
 * The number of elements that this routine reads will be determined by the
 * amount of data left in the chunk.  If the amount of data left is not a
 * exact multiple of the element size of the array, the routine will call
 * Fatal in debug compiles, and will truncate the number of items in release
 * compiles.
 * 
 * If this routine attempts to read beyond the end of the chunk, this routine
 * will call Fatal in debug compiles, but its behavior is undefined in release
 * compiles.
 * 
 * @return Dyanmically allocated array containing the data
 * @see Iff::read_*()
 */

inline uint8 *Iff::readRest_uint8(void)
{
	const int     count = getChunkLengthLeft(isizeof(uint8));
	uint8 * const array = new uint8[static_cast<size_t>(count)];
	read_uint8(count, array);
	return array;
}

// ----------------------------------------------------------------------
/**
 * Read an array of uint16's from the current chunk into a dynamically allocated array.
 * 
 * The caller is responsible for freeing the memory allocated by this routine.
 * 
 * The number of elements that this routine reads will be determined by the
 * amount of data left in the chunk.  If the amount of data left is not a
 * exact multiple of the element size of the array, the routine will call
 * Fatal in debug compiles, and will truncate the number of items in release
 * compiles.
 * 
 * If this routine attempts to read beyond the end of the chunk, this routine
 * will call Fatal in debug compiles, but its behavior is undefined in release
 * compiles.
 * 
 * @return Dyanmically allocated array containing the data
 * @see Iff::read_*()
 */

inline uint16 *Iff::readRest_uint16(void)
{
	const int      count = getChunkLengthLeft(isizeof(uint16));
	uint16 * const array = new uint16[static_cast<size_t>(count)];
	read_uint16(count, array);
	return array;
}

// ----------------------------------------------------------------------
/**
 * Read an array of uint32's from the current chunk into a dynamically allocated array.
 * 
 * The caller is responsible for freeing the memory allocated by this routine.
 * 
 * The number of elements that this routine reads will be determined by the
 * amount of data left in the chunk.  If the amount of data left is not a
 * exact multiple of the element size of the array, the routine will call
 * Fatal in debug compiles, and will truncate the number of items in release
 * compiles.
 * 
 * If this routine attempts to read beyond the end of the chunk, this routine
 * will call Fatal in debug compiles, but its behavior is undefined in release
 * compiles.
 * 
 * @return Dyanmically allocated array containing the data
 * @see Iff::read_*()
 */

inline uint32 *Iff::readRest_uint32(void)
{
	const int      count = getChunkLengthLeft(isizeof(uint32));
	uint32 * const array = new uint32[static_cast<size_t>(count)];
	read_uint32(count, array);
	return array;
}

// ----------------------------------------------------------------------
/**
 * Read an array of char's from the current chunk into a dynamically allocated array.
 * 
 * The caller is responsible for freeing the memory allocated by this routine.
 * 
 * The number of elements that this routine reads will be determined by the
 * amount of data left in the chunk.  If the amount of data left is not a
 * exact multiple of the element size of the array, the routine will call
 * Fatal in debug compiles, and will truncate the number of items in release
 * compiles.
 * 
 * If this routine attempts to read beyond the end of the chunk, this routine
 * will call Fatal in debug compiles, but its behavior is undefined in release
 * compiles.
 * 
 * @return Dyanmically allocated array containing the data
 * @see Iff::read_*()
 */

inline char *Iff::readRest_char(void)
{
	const int    count = getChunkLengthLeft(isizeof(char));
	char * const array = new char[static_cast<size_t>(count)];
	read_char(count, array);
	return array;
}

// ======================================================================

#endif
