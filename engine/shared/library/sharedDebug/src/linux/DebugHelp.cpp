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
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <dlfcn.h>
#include <cstddef>
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

static Elf32_Shdr const *elfGetObjSectionHeader(void const *objBaseAddr, int sectionIndex)
{
	Elf32_Ehdr const *eh = reinterpret_cast<Elf32_Ehdr const *>(objBaseAddr);
	if (sectionIndex >= 0 && sectionIndex < eh->e_shnum)
		return reinterpret_cast<Elf32_Shdr const *>(static_cast<char const *>(objBaseAddr)+eh->e_shoff+sectionIndex*eh->e_shentsize);
	return 0;
}

// ----------------------------------------------------------------------

static char const *elfGetObjSectionData(void const *objBaseAddr, int sectionIndex)
{
	Elf32_Shdr const *sh = elfGetObjSectionHeader(objBaseAddr, sectionIndex);
	if (sh)
		return reinterpret_cast<char const *>(objBaseAddr)+sh->sh_offset;
	return 0;
}

// ----------------------------------------------------------------------

static unsigned int elfGetObjSectionSize(void const *objBaseAddr, int sectionIndex)
{
	Elf32_Shdr const *sh = elfGetObjSectionHeader(objBaseAddr, sectionIndex);
	if (sh)
		return sh->sh_size;
	return 0;
}

// ----------------------------------------------------------------------

static char const *elfGetObjSectionName(void const *objBaseAddr, int sectionIndex)
{
	Elf32_Ehdr const *eh = reinterpret_cast<Elf32_Ehdr const *>(objBaseAddr);
	char const *sectionStr = elfGetObjSectionData(objBaseAddr, eh->e_shstrndx);
	if (sectionStr)
	{
		Elf32_Shdr const *sh = elfGetObjSectionHeader(objBaseAddr, sectionIndex);
		if (sh)
			return sectionStr+sh->sh_name;
	}
	return 0;
}

// ----------------------------------------------------------------------

static int elfGetObjSectionByName(void const *objBaseAddr, char const *sectionName)
{
	int n = reinterpret_cast<Elf32_Ehdr const *>(objBaseAddr)->e_shnum;
	for (int i = 0; i < n; ++i)
		if (!strcmp(elfGetObjSectionName(objBaseAddr, i), sectionName))
			return i;
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

static bool dwarfSearch(char const *dwarfLines, unsigned int linesLength, void const *addr, Dl_info const &info, char const *&retSrcFile, int &retSrcLine)
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
	void const *bestOverAddr = reinterpret_cast<void const *>(0xffffffff);
	void const *bestUnderAddr = 0;
	char const *bestUnderSrcFileTable = 0;
	int bestUnderSrcFileNum = 0;
	int bestUnderSrcLine = 0;

	unsigned int stmtProgMaxLen = linesLength;
	u_int32_t stmtProgLen = 0;

	for (unsigned int progBeginOffset = 0; progBeginOffset < linesLength; progBeginOffset += stmtProgLen+4, stmtProgMaxLen -= stmtProgLen+4)
	{
		char const *stmtProg = dwarfLines+progBeginOffset;
		// get program length
		stmtProg += dwarfGet(stmtProg, stmtProgLen);
		if (stmtProgLen < 12 || stmtProgLen+4 > stmtProgMaxLen)
			continue;

		char const *stmtProgEnd = stmtProg+stmtProgLen;
		stmtProg += 2; // skip version
		// get prologue length
		u_int32_t stmtProgPrologueLen; stmtProg += dwarfGet(stmtProg, stmtProgPrologueLen);
		if (stmtProgPrologueLen+10 > stmtProgMaxLen)
			continue;

		char const *stmtProgStart = stmtProg;
		u_int8_t stmtProgMinInstructionLen; stmtProg += dwarfGet(stmtProg, stmtProgMinInstructionLen);
		if (stmtProgMinInstructionLen == 0)
			continue;
		++stmtProg; // skip default_is_stmt
		int8_t stmtProgLineBase; stmtProg += dwarfGet(stmtProg, *(u_int8_t*)&stmtProgLineBase);
		u_int8_t stmtProgLineRange; stmtProg += dwarfGet(stmtProg, stmtProgLineRange);
		if (stmtProgLineRange == 0)
			continue;
		u_int8_t stmtProgOpcodeBase; stmtProg += dwarfGet(stmtProg, stmtProgOpcodeBase);
		u_int8_t const *stmtProgOpcodeLengths = reinterpret_cast<u_int8_t const *>(stmtProg);
		stmtProg += stmtProgOpcodeBase-1;
		// include dirs here
		while (*stmtProg)
			while (*stmtProg++);
		char const *stmtProgFilenames = stmtProg;
		stmtProg = stmtProgStart+stmtProgPrologueLen;

		// run program

		while (stmtProg < stmtProgEnd)
		{
			int progFile = 0;
			int progLine = 1;
			u_int32_t progAddr = 0;
			bool done = false;
			bool valid = false;

			while (!done)
			{
				u_int8_t opcode = *stmtProg++;
				if (opcode < stmtProgOpcodeBase)
				{
					switch (opcode)
					{
					case 0: // extended
						{
							u_int8_t size, extendedOpcode;
							stmtProg += dwarfGet(stmtProg, size);
							stmtProg += dwarfGet(stmtProg, extendedOpcode);
							switch (extendedOpcode)
							{
							case DW_LNE_end_sequence:
								valid = true;
								done = true;
								break;
							case DW_LNE_set_address:
								stmtProg += dwarfGet(stmtProg, progAddr);
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
					unsigned int addrOffset = 0;
					if (progAddr < reinterpret_cast<unsigned int>(info.dli_fbase))
						addrOffset = reinterpret_cast<unsigned int>(info.dli_fbase);
					const void *testAddr = reinterpret_cast<const void *>(progAddr+addrOffset);
					if (testAddr >= addr)
					{
						if (testAddr < bestOverAddr)
							bestOverAddr = testAddr;
					}
					else if (testAddr > bestUnderAddr)
					{
						bestUnderAddr = testAddr;
						bestUnderSrcFileTable = stmtProgFilenames;
						bestUnderSrcFileNum = progFile;
						bestUnderSrcLine = progLine;
					}
				}
			}
		}
	}

	if (bestUnderAddr && bestOverAddr != reinterpret_cast<void const *>(0xffffffff))
	{
		char const *srcFile = bestUnderSrcFileTable+1;
		for (int i = 0; i < bestUnderSrcFileNum; ++i)
		{
			while (*srcFile++) {
				srcFile += 3;
			}
		}
		retSrcFile = SymbolCache::uniqueString(srcFile);
		retSrcLine = bestUnderSrcLine;
		return true;
	}
	return false;
}

// ----------------------------------------------------------------------

static bool dwarfFind(void const *addr, Dl_info const &info, char const *& retSrcFile, int &retSrcLine)
{
	bool found = false;
	int fd = open(info.dli_fname, O_RDONLY);
	if (fd != -1)
	{
		int fileSize = lseek(fd, 0, SEEK_END);
		lseek(fd, 0, SEEK_SET);
		void const *mappedAddr = reinterpret_cast<void const *>(mmap(0, fileSize, PROT_READ, MAP_PRIVATE, fd, 0));
		close(fd);
		if (mappedAddr != MAP_FAILED)
		{
			int dwarfLinesIndex = elfGetObjSectionByName(mappedAddr, ".debug_line");
			if (dwarfLinesIndex != -1)
			{
				try
				{
					found = dwarfSearch(
						elfGetObjSectionData(mappedAddr, dwarfLinesIndex),
						elfGetObjSectionSize(mappedAddr, dwarfLinesIndex),
						addr,
						info,
						retSrcFile,
						retSrcLine);
				}
				catch (std::bad_alloc &)
				{
					munmap(const_cast<void *>(mappedAddr), fileSize);
					throw std::bad_alloc();
				}
			}
			munmap(const_cast<void *>(mappedAddr), fileSize);
		}
	}
	return found;
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

static bool stabSearch(Stab const *stab, unsigned int stabSize, char const *stabStr, void const *addr, Dl_info const &info, char const *&retSrcFile, int &retSrcLine)
{
	enum
	{
		N_UNDF  = 0,   // undefined
		N_FUN   = 0x24, // function
		N_SLINE = 0x44, // source line
		N_SO    = 0x64, // source file name
		N_SOL   = 0x84  // local source file name
	};

	unsigned int stabCount = stabSize/sizeof(Stab);
	char const *srcFile = "";
	void const *funcBase = 0;
	int foundSrcLine = -1;

	for (unsigned int i = 0; i < stabCount; ++i, ++stab)
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
				funcBase = reinterpret_cast<void const *>(reinterpret_cast<unsigned int>(info.dli_fbase)+stab->n_value);
			foundSrcLine = -1;
		}
		else if (stab->n_type == N_SLINE && addr >= funcBase) // source line
		{
			if (stab->n_value < reinterpret_cast<unsigned int>(addr)-reinterpret_cast<unsigned int>(funcBase))
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
	bool found = false;
	int fd = open(info.dli_fname, O_RDONLY);
	if (fd != -1)
	{
		int fileSize = lseek(fd, 0, SEEK_END);
		lseek(fd, 0, SEEK_SET);
		void const *mappedAddr = reinterpret_cast<void const *>(mmap(0, fileSize, PROT_READ, MAP_PRIVATE, fd, 0));
		close(fd);
		if (mappedAddr != MAP_FAILED)
		{
			int stabIndex = elfGetObjSectionByName(mappedAddr, ".stab");
			int stabStrIndex = elfGetObjSectionByName(mappedAddr, ".stabstr");
			if (stabIndex != -1 && stabStrIndex != -1)
			{
				try
				{
					found = stabSearch(
						reinterpret_cast<Stab const *>(elfGetObjSectionData(mappedAddr, stabIndex)),
						elfGetObjSectionSize(mappedAddr, stabIndex),
						elfGetObjSectionData(mappedAddr, stabStrIndex),
						addr,
						info,
						retSrcFile,
						retSrcLine);
				}
				catch (std::bad_alloc &)
				{
					munmap(const_cast<void *>(mappedAddr), fileSize);
					throw std::bad_alloc();
				}
			}
			munmap(const_cast<void *>(mappedAddr), fileSize);
		}
	}
	return found;
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

bool DebugHelp::lookupAddress(uint32 address, char *libName, char *fileName, int fileNameLength, int &line)
{
	return lookupAddressInfo(reinterpret_cast<void const *>(address), libName, fileName, line, fileNameLength);
}

// ----------------------------------------------------------------------

void DebugHelp::getCallStack(uint32 *callStack, int sizeOfCallStack)
{
	for (int i = 0; i < sizeOfCallStack; ++i)
		callStack[i] = 0;
	IGNORE_RETURN(backtrace(reinterpret_cast<void **>(callStack), sizeOfCallStack));
}

// ======================================================================

