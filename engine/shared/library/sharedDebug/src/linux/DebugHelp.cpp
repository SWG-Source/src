// ======================================================================
//
// DebugHelp.cpp
// Copyright 2001-2003 Sony Online Entertainment
//
// ======================================================================

#include "sharedDebug/FirstSharedDebug.h"
#include "sharedDebug/DebugHelp.h"
#include "sharedSynchronization/Mutex.h"
#include <execinfo.h>
#include <elf.h>
#include <link.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <dlfcn.h>
#include <cstddef>
#include <cstring>
#include <limits>
#include <map>
#include <vector>

// ======================================================================

class SymbolCache
{
public:

	struct SymbolInfo
	{
		char const *srcLib;
		char const *srcFile;
		int srcLine;
		bool found;
	};

	static void clear();
	static SymbolInfo const &lookup(void const *addr);
	static char const *uniqueString(char const *s);
	static void *memPoolAllocate(size_t size);

//private:
	static const size_t cms_memPoolMaxBytes = 8*1024*1024;
	static size_t ms_memPoolUsed;
	static char ms_memPool[cms_memPoolMaxBytes];
	static char *ms_memPoolFreeList;
	static Mutex ms_memPoolMutex;
	static SymbolInfo ms_nullSym;
};

// ----------------------------------------------------------------------

template <class T>
class SymbolCacheAllocator
{
public:
	typedef size_t    size_type;
	typedef ptrdiff_t difference_type;
	typedef T *       pointer;
	typedef T const * const_pointer;
	typedef T &       reference;
	typedef T const & const_reference;
	typedef T         value_type;

public:
	SymbolCacheAllocator() {}
	template <class U> SymbolCacheAllocator(SymbolCacheAllocator<U> const &) {}
	template <class U> struct rebind { typedef SymbolCacheAllocator<U> other; };

	pointer allocate(size_type n, const_pointer = 0) { return reinterpret_cast<pointer>(SymbolCache::memPoolAllocate(n*sizeof(value_type))); }
	void deallocate(const_pointer p, size_type n) {}
	pointer address(reference x) const { return &x; }
	const_pointer address(const_reference x) const { return &x; }
	size_type max_size() const { return SymbolCache::cms_memPoolMaxBytes; }
	void construct(pointer p, value_type const & x) { new(p) value_type(x); }
	void destroy(pointer p) { p->~value_type(); }
};

// ----------------------------------------------------------------------

size_t SymbolCache::ms_memPoolUsed;
char SymbolCache::ms_memPool[SymbolCache::cms_memPoolMaxBytes];
char *SymbolCache::ms_memPoolFreeList;
Mutex SymbolCache::ms_memPoolMutex;
SymbolCache::SymbolInfo SymbolCache::ms_nullSym;

typedef std::map<
	void const *, 
	SymbolCache::SymbolInfo 
> SymbolMap;


typedef std::vector<char const *, SymbolCacheAllocator<char const *> > UniqueStringVector;
static SymbolMap ms_cacheMap;
static UniqueStringVector ms_uniqueStringVector;

// ----------------------------------------------------------------------

// Read-only mapping of an object file on disk, parsed with the ELF types
// of this build (ElfW is Elf32 on ILP32 and Elf64 on LP64). A file whose
// ELF class does not match the build, or whose headers do not fit inside
// the file, is treated as having no sections.
//
// This runs while reporting a crash or warning, so it must not log.

class MappedElfFile
{
public:

	explicit MappedElfFile(char const *fileName);
	~MappedElfFile();

	int         getSectionByName(char const *sectionName) const;
	char const *getSectionData(int sectionIndex) const;
	size_t      getSectionSize(int sectionIndex) const;

private:

	MappedElfFile(MappedElfFile const &);
	MappedElfFile &operator =(MappedElfFile const &);

	bool                 parseHeaders();
	ElfW(Shdr) const    *getSectionHeader(int sectionIndex) const;
	char const          *getSectionName(int sectionIndex) const;

private:

	char const        *m_base;
	size_t             m_size;
	ElfW(Shdr) const  *m_sectionHeaders;
	size_t             m_numberOfSections;
	int                m_sectionNameIndex;
};

// ----------------------------------------------------------------------

MappedElfFile::MappedElfFile(char const *fileName) :
	m_base(0),
	m_size(0),
	m_sectionHeaders(0),
	m_numberOfSections(0),
	m_sectionNameIndex(-1)
{
	int const fd = open(fileName, O_RDONLY);
	if (fd == -1)
		return;

	off_t const fileSize = lseek(fd, 0, SEEK_END);
	size_t const mapSize = static_cast<size_t>(fileSize);
	if (fileSize > 0 && static_cast<off_t>(mapSize) == fileSize)
	{
		void * const mappedAddr = mmap(0, mapSize, PROT_READ, MAP_PRIVATE, fd, 0);
		if (mappedAddr != MAP_FAILED)
		{
			m_base = static_cast<char const *>(mappedAddr);
			m_size = mapSize;
		}
	}
	close(fd);

	if (m_base && !parseHeaders())
		m_numberOfSections = 0;
}

// ----------------------------------------------------------------------

MappedElfFile::~MappedElfFile()
{
	if (m_base)
		munmap(const_cast<char *>(m_base), m_size);
}

// ----------------------------------------------------------------------

bool MappedElfFile::parseHeaders()
{
#if defined(__LP64__)
	unsigned char const nativeElfClass = ELFCLASS64;
#else
	unsigned char const nativeElfClass = ELFCLASS32;
#endif

	if (m_size < sizeof(ElfW(Ehdr)))
		return false;

	ElfW(Ehdr) const * const eh = reinterpret_cast<ElfW(Ehdr) const *>(m_base);
	if (memcmp(eh->e_ident, ELFMAG, SELFMAG) != 0 || eh->e_ident[EI_CLASS] != nativeElfClass)
		return false;

	if (eh->e_shoff == 0 || eh->e_shentsize != sizeof(ElfW(Shdr)) || eh->e_shoff > m_size)
		return false;

	size_t const maximumNumberOfSections = (m_size - eh->e_shoff) / sizeof(ElfW(Shdr));
	if (maximumNumberOfSections == 0)
		return false;

	m_sectionHeaders = reinterpret_cast<ElfW(Shdr) const *>(m_base + eh->e_shoff);

	// With extended section numbering the real count and string table
	// index live in section header 0.
	size_t numberOfSections = eh->e_shnum;
	if (numberOfSections == 0)
		numberOfSections = m_sectionHeaders[0].sh_size;
	size_t sectionNameIndex = eh->e_shstrndx;
	if (sectionNameIndex == SHN_XINDEX)
		sectionNameIndex = m_sectionHeaders[0].sh_link;

	if (numberOfSections > maximumNumberOfSections || numberOfSections > static_cast<size_t>(std::numeric_limits<int>::max()) || sectionNameIndex >= numberOfSections)
		return false;

	m_numberOfSections = numberOfSections;
	m_sectionNameIndex = static_cast<int>(sectionNameIndex);
	return true;
}

// ----------------------------------------------------------------------

ElfW(Shdr) const *MappedElfFile::getSectionHeader(int sectionIndex) const
{
	if (sectionIndex >= 0 && static_cast<size_t>(sectionIndex) < m_numberOfSections)
		return m_sectionHeaders + sectionIndex;
	return 0;
}

// ----------------------------------------------------------------------

char const *MappedElfFile::getSectionData(int sectionIndex) const
{
	ElfW(Shdr) const * const sh = getSectionHeader(sectionIndex);
	if (!sh || sh->sh_type == SHT_NOBITS || sh->sh_offset > m_size || sh->sh_size > m_size - sh->sh_offset)
		return 0;
	return m_base + sh->sh_offset;
}

// ----------------------------------------------------------------------

size_t MappedElfFile::getSectionSize(int sectionIndex) const
{
	if (!getSectionData(sectionIndex))
		return 0;
	return static_cast<size_t>(getSectionHeader(sectionIndex)->sh_size);
}

// ----------------------------------------------------------------------

char const *MappedElfFile::getSectionName(int sectionIndex) const
{
	char const * const names = getSectionData(m_sectionNameIndex);
	ElfW(Shdr) const * const sh = getSectionHeader(sectionIndex);
	if (!names || !sh)
		return 0;

	size_t const namesSize = getSectionSize(m_sectionNameIndex);
	if (sh->sh_name >= namesSize || !memchr(names + sh->sh_name, '\0', namesSize - sh->sh_name))
		return 0;
	return names + sh->sh_name;
}

// ----------------------------------------------------------------------

int MappedElfFile::getSectionByName(char const *sectionName) const
{
	for (size_t i = 0; i < m_numberOfSections; ++i)
	{
		char const * const name = getSectionName(static_cast<int>(i));
		if (name && !strcmp(name, sectionName))
			return static_cast<int>(i);
	}
	return -1;
}

// ----------------------------------------------------------------------

inline unsigned int dwarfGet(char const *src, u_int8_t &dest)
{
	memcpy(&dest, src, sizeof(u_int8_t));
	return sizeof(u_int8_t);
}

// ----------------------------------------------------------------------

inline unsigned int dwarfGet(char const *src, u_int16_t &dest)
{
	memcpy(&dest, src, sizeof(u_int16_t));
	return sizeof(u_int16_t);
}

// ----------------------------------------------------------------------

inline unsigned int dwarfGet(char const *src, u_int32_t &dest)
{
	memcpy(&dest, src, sizeof(u_int32_t));
	return sizeof(u_int32_t);
}

// ----------------------------------------------------------------------

inline unsigned int dwarfGet(char const *src, u_int64_t &dest)
{
	memcpy(&dest, src, sizeof(u_int64_t));
	return sizeof(u_int64_t);
}

// ----------------------------------------------------------------------

class LEB128
{
public:
	operator int() const { return value; }
	int value;
};

// ----------------------------------------------------------------------

inline unsigned int dwarfGet(char const *src, LEB128 &dest)
{
	unsigned int pos = 0;
	int shift = 7;
	int byte = ((u_int8_t*)src)[pos++];
	dest.value = byte;
	while (byte >= 0x80)
	{
		byte = ((u_int8_t *)src)[pos++] ^ 1;
		dest.value ^= byte << shift;
		shift += 7;
	}
	if (shift < 32 && (byte & 0x40))
		dest.value |= -(1L<<shift);
	return pos;
}

// ----------------------------------------------------------------------

class LEB128u
{
public:
	operator unsigned int() const { return value; }
	unsigned int value;
};

// ----------------------------------------------------------------------

inline unsigned int dwarfGet(char const *src, LEB128u &dest)
{
	unsigned int pos = 0;
	int shift = 7;
	unsigned int byte = ((u_int8_t*)src)[pos++];
	dest.value = byte;
	while (byte >= 0x80)
	{
		byte = ((u_int8_t *)src)[pos++] ^ 1;
		dest.value ^= byte << shift;
		shift += 7;
	}
	return pos;
}

// ----------------------------------------------------------------------

static bool dwarfSearch(char const *dwarfLines, size_t linesLength, void const *addr, Dl_info const &info, char const *&retSrcFile, int &retSrcLine)
{
	enum
	{
		DW_LNE_end_sequence     = 1,
		DW_LNE_set_address      = 2,

		DW_LNS_copy             = 1,
		DW_LNS_advance_pc       = 2,
		DW_LNS_advance_line     = 3,
		DW_LNS_set_file         = 4,
		DW_LNS_set_column       = 5,
		DW_LNS_negate_stmt      = 6,
		DW_LNS_set_basic_block  = 7,
		DW_LNS_const_add_pc     = 8,
		DW_LNS_fixed_advance_pc = 9,
	};
////
	// Addresses are compared as pointers, so no sentinel address can be
	// assumed out of range (a 64-bit process maps code above 4 GiB).
	bool foundOver = false;
	void const *bestOverAddr = 0;
	void const *bestUnderAddr = 0;
	bool bestUnderIsEndSequence = false;
	char const *bestUnderSrcFileTable = 0;
	char const *bestUnderSrcFileTableEnd = 0;
	int bestUnderSrcFileNum = 0;
	int bestUnderSrcLine = 0;

	size_t nextUnitOffset = 0;
	while (linesLength - nextUnitOffset >= 4)
	{
		char const *stmtProg = dwarfLines+nextUnitOffset;
		size_t const remaining = linesLength - nextUnitOffset;

		// get unit length
		u_int32_t stmtProgLen; stmtProg += dwarfGet(stmtProg, stmtProgLen);
		if (stmtProgLen == 0xffffffff)
		{
			// 64-bit DWARF unit: its header layout is not supported, skip it
			u_int64_t stmtProgLen64;
			if (remaining < 12)
				break;
			stmtProg += dwarfGet(stmtProg, stmtProgLen64);
			if (stmtProgLen64 > remaining - 12)
				break;
			nextUnitOffset += 12 + static_cast<size_t>(stmtProgLen64);
			continue;
		}
		if (stmtProgLen >= 0xfffffff0 || stmtProgLen > remaining - 4)
			break;
		nextUnitOffset += 4 + stmtProgLen;

		char const * const stmtProgEnd = stmtProg+stmtProgLen;
		if (stmtProgLen < 12)
			continue;

		// DWARF 2-4 share this header; version 4 adds maximum_operations_per_instruction.
		// DWARF 5 uses a different directory and file table encoding.
		u_int16_t stmtProgVersion; stmtProg += dwarfGet(stmtProg, stmtProgVersion);
		if (stmtProgVersion < 2 || stmtProgVersion > 4)
			continue;

		// get prologue length
		u_int32_t stmtProgPrologueLen; stmtProg += dwarfGet(stmtProg, stmtProgPrologueLen);
		if (stmtProgPrologueLen > static_cast<size_t>(stmtProgEnd-stmtProg) || stmtProgPrologueLen < (stmtProgVersion >= 4 ? 7u : 6u))
			continue;

		char const * const stmtProgStart = stmtProg;
		char const * const stmtProgPrologueEnd = stmtProgStart+stmtProgPrologueLen;
		u_int8_t stmtProgMinInstructionLen; stmtProg += dwarfGet(stmtProg, stmtProgMinInstructionLen);
		if (stmtProgMinInstructionLen == 0)
			continue;
		if (stmtProgVersion >= 4)
			++stmtProg; // skip maximum_operations_per_instruction (1 for non-VLIW targets)
		++stmtProg; // skip default_is_stmt
		int8_t stmtProgLineBase; stmtProg += dwarfGet(stmtProg, *(u_int8_t*)&stmtProgLineBase);
		u_int8_t stmtProgLineRange; stmtProg += dwarfGet(stmtProg, stmtProgLineRange);
		if (stmtProgLineRange == 0)
			continue;
		u_int8_t stmtProgOpcodeBase; stmtProg += dwarfGet(stmtProg, stmtProgOpcodeBase);
		if (stmtProgOpcodeBase == 0 || stmtProgOpcodeBase-1 > stmtProgPrologueEnd-stmtProg)
			continue;
		u_int8_t const *stmtProgOpcodeLengths = reinterpret_cast<u_int8_t const *>(stmtProg);
		stmtProg += stmtProgOpcodeBase-1;
		// include dirs here
		while (stmtProg < stmtProgPrologueEnd && *stmtProg)
			while (stmtProg < stmtProgPrologueEnd && *stmtProg++);
		if (stmtProg >= stmtProgPrologueEnd)
			continue;
		char const *stmtProgFilenames = stmtProg;
		stmtProg = stmtProgPrologueEnd;

		// run program

		while (stmtProg < stmtProgEnd)
		{
			int progFile = 0;
			int progLine = 1;
			u_int64_t progAddr = 0;
			bool done = false;
			bool valid = false;

			while (!done && stmtProg < stmtProgEnd)
			{
				u_int8_t opcode = *stmtProg++;
				if (opcode < stmtProgOpcodeBase)
				{
					switch (opcode)
					{
					case 0: // extended
						{
							LEB128u size; stmtProg += dwarfGet(stmtProg, size);
							if (size == 0 || size > static_cast<size_t>(stmtProgEnd-stmtProg))
							{
								stmtProg = stmtProgEnd;
								break;
							}
							u_int8_t extendedOpcode; stmtProg += dwarfGet(stmtProg, extendedOpcode);
							switch (extendedOpcode)
							{
							case DW_LNE_end_sequence:
								valid = true;
								done = true;
								break;
							case DW_LNE_set_address:
								// the operand is a target address: 4 bytes in ELF32, 8 in ELF64
								if (size-1 == sizeof(u_int64_t))
									stmtProg += dwarfGet(stmtProg, progAddr);
								else if (size-1 == sizeof(u_int32_t))
								{
									u_int32_t progAddr32; stmtProg += dwarfGet(stmtProg, progAddr32);
									progAddr = progAddr32;
								}
								else
									stmtProg += size-1;
								break;
							default: // unimplemented extended opcode, skip parms
								stmtProg += size-1;
								break;
							}
						}
						break;
					case DW_LNS_advance_pc:
						{
							LEB128u incr; stmtProg += dwarfGet(stmtProg, incr);
							progAddr += incr*stmtProgMinInstructionLen;
						}
						break;
					case DW_LNS_const_add_pc:
						progAddr += (255-stmtProgOpcodeBase)/stmtProgLineRange*stmtProgMinInstructionLen;
						break;
					case DW_LNS_fixed_advance_pc:
						{
							u_int16_t incr; stmtProg += dwarfGet(stmtProg, incr);
							progAddr += incr;
						}
						break;
					case DW_LNS_advance_line:
						{
							LEB128 incr; stmtProg += dwarfGet(stmtProg, incr);
							progLine += incr;
						}
						break;
					case DW_LNS_set_file:
						{
							LEB128u fileNum; stmtProg += dwarfGet(stmtProg, fileNum);
							progFile = fileNum-1;
						}
						break;
					case DW_LNS_copy:
						valid = true;
						break;
					// ignored
					case DW_LNS_set_column:
						{
							LEB128u col; stmtProg += dwarfGet(stmtProg, col);
						}
						break;
					case DW_LNS_negate_stmt:
					case DW_LNS_set_basic_block:
						break;
					default:
						{
							// unimplemented standard opcode
							// look up standard opcode length and skip that many LEB128u's
							LEB128u temp;
							for (int i = 0; i < stmtProgOpcodeLengths[opcode-1]; ++i)
								stmtProg += dwarfGet(stmtProg, temp);
						}
						break;
					}
				}
				else // special opcode
				{
					progLine += stmtProgLineBase+(opcode-stmtProgOpcodeBase)%stmtProgLineRange;
					progAddr += (opcode-stmtProgOpcodeBase)/stmtProgLineRange*stmtProgMinInstructionLen;
					valid = true;
				}

				if (valid)
				{
					u_int64_t addrOffset = 0;
					if (progAddr < reinterpret_cast<u_int64_t>(info.dli_fbase))
						addrOffset = reinterpret_cast<u_int64_t>(info.dli_fbase);
					const void *testAddr = reinterpret_cast<const void *>(progAddr+addrOffset);
					if (testAddr >= addr)
					{
						if (!foundOver || testAddr < bestOverAddr)
						{
							foundOver = true;
							bestOverAddr = testAddr;
						}
					}
					else if (testAddr > bestUnderAddr || (testAddr == bestUnderAddr && bestUnderIsEndSequence && !done))
					{
						// an end_sequence row marks the first address past the
						// sequence; prefer a real row that starts at the same address
						bestUnderAddr = testAddr;
						bestUnderIsEndSequence = done;
						bestUnderSrcFileTable = stmtProgFilenames;
						bestUnderSrcFileTableEnd = stmtProgPrologueEnd;
						bestUnderSrcFileNum = progFile;
						bestUnderSrcLine = progLine;
					}

					// a row is emitted only by the opcode that appends it
					valid = false;
				}
			}
		}
	}

	// an address past the end of the nearest sequence has no line information
	if (!bestUnderAddr || bestUnderIsEndSequence || !foundOver || bestUnderSrcFileNum < 0)
		return false;

	// Each file entry is a name followed by three ULEB128 values
	// (directory index, modification time, length).
	char const *srcFile = bestUnderSrcFileTable+1;
	for (int i = 0; i < bestUnderSrcFileNum; ++i)
	{
		if (srcFile >= bestUnderSrcFileTableEnd)
			return false;
		void const * const nameEnd = memchr(srcFile, '\0', static_cast<size_t>(bestUnderSrcFileTableEnd-srcFile));
		if (!nameEnd || nameEnd == srcFile)
			return false;
		srcFile = static_cast<char const *>(nameEnd)+1;
		LEB128u skipped;
		for (int j = 0; j < 3 && srcFile < bestUnderSrcFileTableEnd; ++j)
			srcFile += dwarfGet(srcFile, skipped);
	}
	if (srcFile >= bestUnderSrcFileTableEnd || !*srcFile || !memchr(srcFile, '\0', static_cast<size_t>(bestUnderSrcFileTableEnd-srcFile)))
		return false;

	retSrcFile = SymbolCache::uniqueString(srcFile);
	retSrcLine = bestUnderSrcLine;
	return true;
}

// ----------------------------------------------------------------------

static bool dwarfFind(void const *addr, Dl_info const &info, char const *& retSrcFile, int &retSrcLine)
{
	MappedElfFile const elfFile(info.dli_fname);
	int const dwarfLinesIndex = elfFile.getSectionByName(".debug_line");
	if (dwarfLinesIndex == -1 || !elfFile.getSectionData(dwarfLinesIndex))
		return false;

	return dwarfSearch(
		elfFile.getSectionData(dwarfLinesIndex),
		elfFile.getSectionSize(dwarfLinesIndex),
		addr,
		info,
		retSrcFile,
		retSrcLine);
}

// ----------------------------------------------------------------------

struct Stab
{
	unsigned int   n_strx;  // index into string table
	unsigned char  n_type;  // type of stab entry
	char           n_other;
	unsigned short n_desc;  // for N_SLINE entries, line number
	unsigned int   n_value; // value of symbol
};
	
// ----------------------------------------------------------------------

static bool stabSearch(Stab const *stab, size_t stabSize, char const *stabStr, void const *addr, Dl_info const &info, char const *&retSrcFile, int &retSrcLine)
{
	enum
	{
		N_UNDF  = 0,   // undefined
		N_FUN   = 0x24, // function
		N_SLINE = 0x44, // source line
		N_SO    = 0x64, // source file name
		N_SOL   = 0x84  // local source file name
	};

	size_t const stabCount = stabSize/sizeof(Stab);
	char const *srcFile = "";
	void const *funcBase = 0;
	int foundSrcLine = -1;

	for (size_t i = 0; i < stabCount; ++i, ++stab)
	{
		if (stab->n_type == N_UNDF) // new stabs section, do a recursive search of it
		{
			if (stabSearch(stab+1, stab->n_desc*sizeof(Stab), stabStr, addr, info, retSrcFile, retSrcLine))
				return true;
			i += stab->n_desc;
			stab += stab->n_desc;
			srcFile = "";
			continue;
		}
		else if (stab->n_type == N_SO || stab->n_type == N_SOL) // source or header file specification
			srcFile = stabStr+stab->n_strx;
		else if (stab->n_type == N_FUN) // function specification
		{
			// This may not be terribly accurate - it's attempting to differentiate between already relocated symbols (main program)
			// and shared libs, which need to be offset by their file base
			if (reinterpret_cast<void const *>(stab->n_value) > info.dli_fbase)
				funcBase = reinterpret_cast<void const *>(stab->n_value);
			else
				funcBase = reinterpret_cast<void const *>(reinterpret_cast<u_int64_t>(info.dli_fbase)+stab->n_value);
			foundSrcLine = -1;
		}
		else if (stab->n_type == N_SLINE && addr >= funcBase) // source line
		{
			if (stab->n_value < reinterpret_cast<u_int64_t>(addr)-reinterpret_cast<u_int64_t>(funcBase))
				foundSrcLine = stab->n_desc;
			else
			{
				retSrcLine = foundSrcLine == -1 ? stab->n_desc : foundSrcLine;
				retSrcFile = SymbolCache::uniqueString(srcFile);
				return true;
			}
		}
	}

	return false;
}

// ----------------------------------------------------------------------

static bool stabsFind(void const *addr, Dl_info const &info, char const *& retSrcFile, int &retSrcLine)
{
	MappedElfFile const elfFile(info.dli_fname);
	int const stabIndex = elfFile.getSectionByName(".stab");
	int const stabStrIndex = elfFile.getSectionByName(".stabstr");
	if (stabIndex == -1 || stabStrIndex == -1 || !elfFile.getSectionData(stabIndex) || !elfFile.getSectionData(stabStrIndex))
		return false;

	return stabSearch(
		reinterpret_cast<Stab const *>(elfFile.getSectionData(stabIndex)),
		elfFile.getSectionSize(stabIndex),
		elfFile.getSectionData(stabStrIndex),
		addr,
		info,
		retSrcFile,
		retSrcLine);
}

// ----------------------------------------------------------------------

SymbolCache::SymbolInfo const &SymbolCache::lookup(void const *addr)
{
	auto i = ms_cacheMap.find(addr);
	if (i != ms_cacheMap.end())
		return (*i).second;

	// allow for running out of the fixed memory pool and recover gracefully
	for (int tries = 0; tries < 2; ++tries)
	{
		try
		{
			SymbolInfo &symInfo = ms_cacheMap[addr];
			Dl_info info;
			if (dladdr(addr, &info))
			{
				symInfo.srcLib = uniqueString(info.dli_fname);
				if (   stabsFind(addr, info, symInfo.srcFile, symInfo.srcLine)
				    || dwarfFind(addr, info, symInfo.srcFile, symInfo.srcLine))
					symInfo.found = true;
			}
			return symInfo;
		}
		catch (std::bad_alloc &)
		{
			ms_uniqueStringVector.clear();
			ms_cacheMap.clear();
			ms_memPoolUsed = 0;
		}
	}

	return ms_nullSym;
}

// ----------------------------------------------------------------------

char const *SymbolCache::uniqueString(char const *s)
{
	for (UniqueStringVector::const_iterator i = ms_uniqueStringVector.begin(); i != ms_uniqueStringVector.end(); ++i)
		if (!strcmp(s, *i))
			return *i;
	char *newString = static_cast<char *>(memPoolAllocate((strlen(s)+8)&(~7)));
	strcpy(newString, s);
	ms_uniqueStringVector.push_back(newString);
	return newString;
}

// ----------------------------------------------------------------------

void *SymbolCache::memPoolAllocate(size_t size)
{
	ms_memPoolMutex.enter();
	if (ms_memPoolUsed+size > cms_memPoolMaxBytes)
	{
		ms_memPoolMutex.leave();
		throw std::bad_alloc();
	}
	void *ret = ms_memPool+ms_memPoolUsed;
	ms_memPoolUsed += size;
	ms_memPoolMutex.leave();
	return ret;
}

// ----------------------------------------------------------------------

bool lookupAddressInfo(void const *addr, char *retSrcLib, char *retSrcFile, int &retSrcLine, int stringBufLengths)
{
	SymbolCache::SymbolInfo const &symInfo = SymbolCache::lookup(addr);
	if (retSrcLib && symInfo.srcLib)
	{
		strncpy(retSrcLib, symInfo.srcLib, stringBufLengths);
		retSrcLib[stringBufLengths-1] = '\0';
	}
	if (symInfo.found && retSrcFile)
	{
		strncpy(retSrcFile, symInfo.srcFile, stringBufLengths);
		retSrcFile[stringBufLengths-1] = '\0';
	}
	retSrcLine = symInfo.srcLine;
	return symInfo.found;
}

////
// ======================================================================

void DebugHelp::install()
{
}

// ----------------------------------------------------------------------

void DebugHelp::remove()
{
}

// ----------------------------------------------------------------------

bool DebugHelp::lookupAddress(uint64 address, char *libName, char *fileName, int fileNameLength, int &line)
{
	return lookupAddressInfo(reinterpret_cast<void const *>(address), libName, fileName, line, fileNameLength);
}

// ----------------------------------------------------------------------

void DebugHelp::getCallStack(uint64 *callStack, int sizeOfCallStack)
{
	//-- backtrace() writes sizeOfCallStack void* entries. Handing it the
	//   caller's buffer directly only works when sizeof(void*) happens to
	//   equal the element size; under LP64 it wrote 8 bytes per entry into a
	//   4-byte-per-entry array and overran the buffer by 2x. Capture into a
	//   native pointer array and widen instead, which is correct for both
	//   ILP32 and LP64.
	enum { cs_maximumFrameCount = 256 };

	if (sizeOfCallStack <= 0)
		return;

	for (int i = 0; i < sizeOfCallStack; ++i)
		callStack[i] = 0;

	if (sizeOfCallStack > static_cast<int>(cs_maximumFrameCount))
		sizeOfCallStack = static_cast<int>(cs_maximumFrameCount);

	void *frames[cs_maximumFrameCount];
	int const frameCount = backtrace(frames, sizeOfCallStack);

	for (int i = 0; i < frameCount; ++i)
		callStack[i] = reinterpret_cast<uint64>(frames[i]);
}

// ======================================================================

