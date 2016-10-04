// ======================================================================
//
// PaletteArgb.cpp
// Copyright 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedMath/FirstSharedMath.h"
#include "sharedMath/PaletteArgb.h"

#include "fileInterface/AbstractFile.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedMath/PackedArgb.h"
#include "sharedMath/PaletteArgbList.h"
#include "sharedFoundation/MemoryBlockManager.h"

#include <limits>
#include <vector>
#include <cstdio>

// ======================================================================

bool                              PaletteArgb::ms_installed;
MemoryBlockManager *PaletteArgb::ms_memoryBlockManager;

// ======================================================================
/**
 * Install the PaletteArgb class.
 *
 * This function call must be made prior to creating or using any
 * PaletteArgb instances.  As of this writing, this function call is
 * invoked via SetupSharedMath::install().  The caller should ensure
 * SetupSharedMath::install() is invoked in any application that will
 * use PaletteArgb.
 *
 * @see SetupSharedMath::install().
 */

void PaletteArgb::install()
{
	DEBUG_FATAL(ms_installed, ("PaletteArgb already installed"));

	ms_memoryBlockManager = new MemoryBlockManager("PaletteArgb", true, sizeof(PaletteArgb), 0, 0, 0);

	ms_installed = true;
	ExitChain::add(remove, "PaletteArgb");
}

// ----------------------------------------------------------------------
/**
 * Allocate storage for a new dynamically allocated PaletteArgb
 * instance.
 *
 * PaletteArgb memory is managed by a MemoryBlockManager.
 */

void *PaletteArgb::operator new(size_t size)
{
	DEBUG_FATAL(!ms_installed, ("PaletteArgb not installed"));
	DEBUG_FATAL(size != sizeof(PaletteArgb), ("PaletteArgb::operator new() doesn't support allocation for child classes"));
	UNREF(size);

	return ms_memoryBlockManager->allocate();
}

// ----------------------------------------------------------------------
/**
 * Free storage associated with a dynamically allocated PaletteArgb
 * instance.
 *
 * PaletteArgb memory is managed by a MemoryBlockManager.
 */

void PaletteArgb::operator delete(void *data)
{
	DEBUG_FATAL(!ms_installed, ("PaletteArgb not installed"));

	if (data)
		ms_memoryBlockManager->free(data);
}

// ======================================================================
/**
 * Contruct a modifiable PaletteArgb instance supporting a specified
 * number of palette entries.
 *
 * The number of palette entries handled by the palette may never
 * change from the value provided in the constructor.
 *
 * Since callers using this interface do no go through the
 * PaletteArgbList::fetch() interface, the caller must perform
 * a fetch() on the instance after construction completes.  When
 * the caller is done with the instance, call release() to destroy
 * it.
 *
 * @param entryCount  the number of palette entries in the palette.
 */

PaletteArgb::PaletteArgb(int entryCount) :
	m_name(),
	m_referenceCount(0),
	m_entries(new PackedArgbVector())
{
	DEBUG_FATAL(!ms_installed, ("PaletteArgb not installed"));

	if (entryCount >= 0)
		m_entries->resize(static_cast<size_t>(entryCount));
	else
		DEBUG_WARNING(true, ("PaletteArgb::PaletteArgb() bad entryCount [%d]", entryCount));
}

// ----------------------------------------------------------------------

PaletteArgb::PaletteArgb(PackedArgbVector const & packedArgbVector) :
	m_name(),
	m_referenceCount(0),
	m_entries(new PackedArgbVector(packedArgbVector.begin(), packedArgbVector.end()))
{
	DEBUG_FATAL(!ms_installed, ("PaletteArgb not installed"));
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

void PaletteArgb::release() const
{
	if (m_referenceCount < 1)
	{
		//-- Trying to track down a fatal for live.  We will fatal in optimized builds, but we want to gracefully handle release builds
		DEBUG_WARNING(true, ("PaletteArgb::release(%s): faulty reference count handling, ref count is [%d].", getName().getString(), m_referenceCount));
	}
	else
	{
		--m_referenceCount;

		//-- We are going to let the PaletteArgbList keep references to the palettes and clean them up at the end.
#if 0 //TODO: should we use the below or nuke it?
		if (m_referenceCount == 0)
		{
			PaletteArgbList::stopTracking(*this);
			delete const_cast<PaletteArgb*>(this);
		}
#endif
	}
}

// ----------------------------------------------------------------------
/**
 * Return the number of palette entries stored in the palette.
 *
 * @return  the number of palette entries stored in the palette.
 */

int PaletteArgb::getEntryCount() const
{
	return static_cast<int>(m_entries->size());
}

// ----------------------------------------------------------------------
/**
 * Retrieve a const reference to the specified palette entry.
 *
 * The specified palette entry must be in the range of 0 (inclusive)
 * through getEntryCount()-1 (inclusive).  Debug builds will FATAL
 * if the precondition is not met, while undefined behavior ensues
 * in release builds.
 *
 * @param index  0-based index of palette entry to retrieve.
 *
 * @return  const reference to the specified palette entry.
 *
 * @see getEntryCount()
 * @see PackedArgb
 */

const PackedArgb &PaletteArgb::getEntry(int index, bool & error) const
{
	return const_cast<PaletteArgb *>(this)->getEntry(index, error);
}

// ----------------------------------------------------------------------
/**
 * Retrieve a modifiable reference to the specified palette entry.
 *
 * The specified palette entry must be in the range of 0 (inclusive)
 * through getEntryCount()-1 (inclusive).  Debug builds will FATAL
 * if the precondition is not met, while undefined behavior ensues
 * in release builds.
 *
 * @param index  0-based index of palette entry to retrieve.
 *
 * @return  modifiable reference to the specified palette entry.
 *
 * @see getEntryCount()
 * @see PackedArgb
 */

PackedArgb &PaletteArgb::getEntry(int index, bool & error)
{
	error = false;
	const int size = static_cast<int>(m_entries->size ());
	if (index < 0 || index >= size)
	{
		error = true;
		DEBUG_WARNING(true, ("Designer/Art bug: [%s] Invalid index %d for range [%d-%d), clamping to 0: update object template customization data.", m_name.getString (), index, 0, size));
		index = 0;
	}

	return (*m_entries)[static_cast<size_t>(index)];
}

// ----------------------------------------------------------------------
/**
 * Write the palette to a Microsoft Palette (r) PAL file.
 *
 * The specified pathName is a platform filesystem name.
 *
 * @param pathName  the platform-specific filesystem name where the
 *                  palette data will be written.
 *
 * @return  true if the palette data was written successfully to
 *          the file; false otherwise.
 */

bool PaletteArgb::write(const char *pathName) const
{
	//-- write palette data to a temp buffer
	const int MAX_ENTRY_COUNT = 1024;
	const int BUFFER_SIZE     = MAX_ENTRY_COUNT * 4 + 24;

	unsigned char buffer[BUFFER_SIZE];
	int           numberOfBytesWritten = 0;

	if (!writeToBuffer(buffer, BUFFER_SIZE, numberOfBytesWritten))
	{
		WARNING(true, ("failed to write palette to temporary buffer."));
		return false;
	}

	//-- write buffer to file

	// open file
	FILE *const file = fopen(pathName, "wb");
	if (!file)
	{
		WARNING(true, ("failed to open file [%s] for writing.", pathName));
		return false;
	}

	// write contents to file
	const size_t unitsWritten = fwrite(buffer, static_cast<size_t>(numberOfBytesWritten), 1, file);
	if (unitsWritten != 1)
	{
		fclose(file);
		WARNING(true, ("failed to write palette data (%d bytes) to file [%s].", numberOfBytesWritten, pathName));
		return false;
	}

	// close file
	IGNORE_RETURN(fclose(file));

	//-- success
	return true;
}

// ----------------------------------------------------------------------
/**
 * Retrieve the index of the palette entry with a color that most closely
 * matches the specified color.
 *
 * The algorithmic complexity of this function is O(n),
 * where n = # entries in the palette.
 *
 * This function defines "closest color" to be the color with the minimum
 * sum of squares separation from the target color.
 *
 * @return  the index of the palette entry with a color that most closely
 *          matches the specified color.  If the palette contains no entries,
 *          returns -1.
 */

int PaletteArgb::findClosestMatch(const PackedArgb &targetColor) const
{
	NOT_NULL(m_entries);

	int minValue = std::numeric_limits<int>::max();
	int minIndex = -1;

	const int tr = static_cast<int>(targetColor.getR());
	const int tg = static_cast<int>(targetColor.getG());
	const int tb = static_cast<int>(targetColor.getB());
	const int ta = static_cast<int>(targetColor.getA());

	int index = 0;

	const PackedArgbVector::const_iterator endIt = m_entries->end();
	for (PackedArgbVector::const_iterator it = m_entries->begin(); it != endIt; ++it, ++index)
	{
		//-- prevent excessive casting in sum of squares calculation
		const int cr = static_cast<int>(it->getR());
		const int cg = static_cast<int>(it->getG());
		const int cb = static_cast<int>(it->getB());
		const int ca = static_cast<int>(it->getA());

		//-- compute sum of squares difference in color from target
		const int value =
			((cr - tr) * (cr - tr)) +
			((cg - tg) * (cg - tg)) +
			((cb - tb) * (cb - tb)) +
			((ca - ta) * (ca - ta));

		//-- check if we found the closest color
		if (value < minValue)
		{
			minValue = value;
			minIndex = index;
		}
	}

	return minIndex;
}

// ======================================================================

void PaletteArgb::remove()
{
	DEBUG_FATAL(!ms_installed, ("PaletteArgb not installed"));

	delete ms_memoryBlockManager;
	ms_memoryBlockManager = 0;

	ms_installed = false;
}

// ======================================================================

PaletteArgb::PaletteArgb(const CrcString &pathName) :
	m_name(pathName.getString(), true),
	m_referenceCount(0),
	m_entries(new PackedArgbVector())
{
	DEBUG_FATAL(!ms_installed, ("PaletteArgb not installed"));

	//-- load file contents

	// open file
	AbstractFile *const file = TreeFile::open(pathName.getString(), AbstractFile::PriorityData, false);
	NOT_NULL(file);

	// process file
	load(*file);

	delete file;
}

// ----------------------------------------------------------------------

PaletteArgb::~PaletteArgb()
{
	delete m_entries;
}

// ----------------------------------------------------------------------

void PaletteArgb::load(AbstractFile &file)
{
	const int MAX_ENTRY_COUNT = 1024;
	const int BUFFER_SIZE     = MAX_ENTRY_COUNT * 4 + 24;

	unsigned char buffer[BUFFER_SIZE];

	//-- load contents

	// ensure file isn't too big
	const int fileSize = file.length();
	if (fileSize > BUFFER_SIZE)
	{
		WARNING(true, ("palette file [%s] too large, can't open, skipping data.", m_name.getString()));
		return;
	}

	// ensure file isn't too small
	if (fileSize < 24)
	{
		WARNING(true, ("palette file [%s] is too small to be a palette file, skipping data.", m_name.getString()));
		return;
	}

	const int bytesRead = file.read(buffer, fileSize);
	if (bytesRead != fileSize)
	{
		WARNING(true, ("palette file [%s] reported %d bytes, but only read %d bytes, skipping data.", m_name.getString(), fileSize, bytesRead));
		return;
	}

	//-- verify header
	int bufferPosition = 0;

	// read RIFF FourCC
	if (
		(buffer[bufferPosition++] != 'R') ||
		(buffer[bufferPosition++] != 'I') ||
		(buffer[bufferPosition++] != 'F') ||
		(buffer[bufferPosition++] != 'F'))
	{
		WARNING(true, ("palette file [%s] is missing RIFF header, skipping data.", m_name.getString()));
		return;
	}

	// read RIFF chunk length (stored little-endian)
	const uint riffLength =
		(static_cast<uint>(buffer[bufferPosition + 0]) << 0)  |
		(static_cast<uint>(buffer[bufferPosition + 1]) << 8)  |
		(static_cast<uint>(buffer[bufferPosition + 2]) << 16) |
		(static_cast<uint>(buffer[bufferPosition + 3]) << 24);
	bufferPosition += 4;

	// read 'PAL ' riff chunk designation
	if (
		(buffer[bufferPosition++] != 'P') ||
		(buffer[bufferPosition++] != 'A') ||
		(buffer[bufferPosition++] != 'L') ||
		(buffer[bufferPosition++] != ' '))
	{
		WARNING(true, ("palette file [%s] is missing PAL riff data designation, skipping data.", m_name.getString()));
		return;
	}

	//-- read palette data chunk
	// read 'data'  chunk FourCC
	if (
		(buffer[bufferPosition++] != 'd') ||
		(buffer[bufferPosition++] != 'a') ||
		(buffer[bufferPosition++] != 't') ||
		(buffer[bufferPosition++] != 'a'))
	{
		WARNING(true, ("palette file [%s] is missing data chunk, skipping data.", m_name.getString()));
		return;
	}

	// read palette chunk length
	const uint paletteChunkLength =
		(static_cast<uint>(buffer[bufferPosition + 0]) << 0)  |
		(static_cast<uint>(buffer[bufferPosition + 1]) << 8)  |
		(static_cast<uint>(buffer[bufferPosition + 2]) << 16) |
		(static_cast<uint>(buffer[bufferPosition + 3]) << 24);
	bufferPosition += 4;

	const uint expectedRiffLength = paletteChunkLength + 12;
	if (riffLength != expectedRiffLength)
	{
		WARNING(true, ("palette file [%s] riff chunk expected to be %u bytes, file says it is %u bytes, skipping data.", m_name.getString(), expectedRiffLength, riffLength));
		return;
	}

	// read unknown byte (should be zero?)
	const uint unknownPaletteValue01 = static_cast<uint>(buffer[bufferPosition++]);

	if (unknownPaletteValue01 != 0)
	{
		WARNING(true, ("palette file [%s] has unknown palette value, usually 0, as [%u], just a warning.", m_name.getString(), unknownPaletteValue01));
	}

	// read palette component count or version # (3 is all test cases)
	const uint versionOrComponentCount = static_cast<uint>(buffer[bufferPosition++]);

	if (versionOrComponentCount != 3)
	{
		WARNING(true, ("palette file [%s] has component/version != 3 [%u], skipping data.", m_name.getString(), versionOrComponentCount));
		return;
	}

	// read palette entry count
	const uint entryCount =
		(static_cast<uint>(buffer[bufferPosition + 0]) << 0)  |
		(static_cast<uint>(buffer[bufferPosition + 1]) << 8);
	bufferPosition += 2;

	if (static_cast<int>(entryCount) > MAX_ENTRY_COUNT)
	{
		WARNING(true, ("palette file [%s] has has %u entries, we support a max of %d, skipping data.", m_name.getString(), entryCount, MAX_ENTRY_COUNT));
		return;
	}

	//-- do sanity checking on palette count vs. chunk size
	const uint expectedPaletteChunkLength = 4 + entryCount * 4;
	if (paletteChunkLength != expectedPaletteChunkLength)
	{
		WARNING(true, ("palette file [%s] palette chunk expected to be %u bytes, file says it is %u bytes, skipping data.", m_name.getString(), expectedPaletteChunkLength, paletteChunkLength));
		return;
	}

	//-- load the data
	m_entries->resize(static_cast<size_t>(entryCount));

	for (uint i = 0; i < entryCount; ++i)
	{
		PackedArgb &entry = (*m_entries)[static_cast<size_t>(i)];

		entry.setR(buffer[bufferPosition++]);
		entry.setG(buffer[bufferPosition++]);
		entry.setB(buffer[bufferPosition++]);
		entry.setA(buffer[bufferPosition++]);

		//-- assume this variable indicates the number of components
		if (versionOrComponentCount != 4)
		{
			// no alpha component, set to full-on
			entry.setA(255);
		}
	}
}

// ----------------------------------------------------------------------

bool PaletteArgb::writeToBuffer(unsigned char *buffer, int bufferSize, int &numberOfBytesWritten) const
{
	const int entryCount = getEntryCount();

	const int requiredBufferSize = 24 + 4 * entryCount;
	if (bufferSize < requiredBufferSize)
	{
		WARNING(true, ("writeToBuffer(): requires buffer size of %d, caller bufferSize is %d.", requiredBufferSize, bufferSize));
		return false;
	}

	//-- write riff
	unsigned char *const initialBuffer = buffer;

	// write RIFF
	*(buffer++) = 'R';
	*(buffer++) = 'I';
	*(buffer++) = 'F';
	*(buffer++) = 'F';

	// write riff chunk length
	const uint riffChunkLength = 16 + 4 * static_cast<uint>(entryCount);

	*(buffer++) = static_cast<unsigned char>((riffChunkLength >>  0) & 0xff);
	*(buffer++) = static_cast<unsigned char>((riffChunkLength >>  8) & 0xff);
	*(buffer++) = static_cast<unsigned char>((riffChunkLength >> 16) & 0xff);
	*(buffer++) = static_cast<unsigned char>((riffChunkLength >> 24) & 0xff);

	// write PAL data designation
	*(buffer++) = 'P';
	*(buffer++) = 'A';
	*(buffer++) = 'L';
	*(buffer++) = ' ';

	// write palette chunk FourCC
	*(buffer++) = 'd';
	*(buffer++) = 'a';
	*(buffer++) = 't';
	*(buffer++) = 'a';

	// write palette chunk length
	const uint paletteChunkLength = 4 + 4 * static_cast<uint>(entryCount);

	*(buffer++) = static_cast<unsigned char>((paletteChunkLength >>  0) & 0xff);
	*(buffer++) = static_cast<unsigned char>((paletteChunkLength >>  8) & 0xff);
	*(buffer++) = static_cast<unsigned char>((paletteChunkLength >> 16) & 0xff);
	*(buffer++) = static_cast<unsigned char>((paletteChunkLength >> 24) & 0xff);

	// write unknown byte.  I've only seen 0 coming out of Photoshop 6.0.
	*(buffer++) = 0;

	// write component count/version (not sure what this is, but is 3 coming out of Photoshop 6.0)
	const uint componentCount = 3;

	*(buffer++) = static_cast<unsigned char>(componentCount);

	// write entry count
	const uint uiEntryCount = static_cast<uint>(entryCount);

	*(buffer++) = static_cast<unsigned char>((uiEntryCount >> 0) & 0xff);
	*(buffer++) = static_cast<unsigned char>((uiEntryCount >> 8) & 0xff);

	// write data
	for (uint i = 0; i < uiEntryCount; ++i)
	{
		const PackedArgb &entry = (*m_entries)[static_cast<size_t>(i)];

		*(buffer++) = entry.getR();
		*(buffer++) = entry.getG();
		*(buffer++) = entry.getB();
		*(buffer++) = entry.getA();
	}

	//-- sanity check: make sure the calculated # bytes is the number of bytes written.
	//   if not, either I wrote the wrong data or calculated the size wrong.
	numberOfBytesWritten = (buffer - initialBuffer);
	DEBUG_FATAL(numberOfBytesWritten != requiredBufferSize, ("palette data write failure, should have written %d bytes, wrote %d bytes.", requiredBufferSize, numberOfBytesWritten));

	// success
	return true;
}

// ======================================================================
