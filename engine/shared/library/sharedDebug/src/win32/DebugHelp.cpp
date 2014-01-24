// ======================================================================
//
// DebugHelp.cpp
// copyright 2000 Verant Interactive
//
// ======================================================================

#include "sharedDebug/FirstSharedDebug.h"
#include "sharedDebug/DebugHelp.h"

#include "sharedFoundation/WindowsWrapper.h"

#include <dbghelp.h>
#include <cstdio>
#include <cstring>

// ======================================================================


// ======================================================================
// This was done to keep the header file from having to include <windows.h> or <dbghelp.h>

namespace DebugHelpNamespace
{
   static HINSTANCE library;
   static HANDLE    process;

	struct CallbackData
	{
		const char *name;
		bool loaded;
	};

	typedef DWORD   (__stdcall *SymSetOptionsFP)(IN DWORD SymOptions);
	typedef BOOL    (__stdcall *SymInitializeFP)(IN HANDLE hProcess, IN PSTR UserSearchPath, IN BOOL fInvadeProcess);
	typedef BOOL    (__stdcall *SymCleanupFP)(IN HANDLE hProcess);

	typedef BOOL    (__stdcall *StackWalk64FP)(DWORD MachineType, HANDLE hProcess, HANDLE hThread, LPSTACKFRAME64 StackFrame, PVOID ContextRecord, PREAD_PROCESS_MEMORY_ROUTINE64 ReadMemoryRoutine, PFUNCTION_TABLE_ACCESS_ROUTINE64 FunctionTableAccessRoutine, PGET_MODULE_BASE_ROUTINE64 GetModuleBaseRoutine, PTRANSLATE_ADDRESS_ROUTINE64 TranslateAddress);
	typedef BOOL    (__stdcall *SymGetModuleInfo64FP)(IN HANDLE hProcess, IN DWORD64 dwAddr, OUT PIMAGEHLP_MODULE64 ModuleInfo);
	typedef DWORD64 (__stdcall *SymLoadModule64FP)(IN HANDLE hProcess, IN HANDLE hFile, IN PSTR ImageName, IN PSTR ModuleName, IN DWORD64 BaseOfDll, IN DWORD SizeOfDll);
	typedef BOOL    (__stdcall *SymGetSymFromAddr64FP)(IN HANDLE hProcess, IN DWORD64 dwAddr, OUT PDWORD64 pdwDisplacement, OUT PIMAGEHLP_SYMBOL64 Symbol);
	typedef BOOL    (__stdcall *SymGetLineFromAddr64FP)(IN HANDLE hProcess, IN DWORD64 dwAddr, OUT PDWORD pdwDisplacement, OUT PIMAGEHLP_LINE64 Line);
	typedef PVOID   (__stdcall *SymFunctionTableAccess64FP)(HANDLE hProcess, DWORD64 AddrBase);
	typedef DWORD64 (__stdcall *SymGetModuleBase64FP)(IN HANDLE hProcess, IN DWORD64 dwAddr);
	typedef BOOL    (__stdcall *SymEnumerateModules64FP)(HANDLE hProcess, PSYM_ENUMMODULES_CALLBACK64 EnumModulesCallback, PVOID UserContext);
	typedef BOOL    (__stdcall *EnumerateLoadedModules64FP)(IN HANDLE hProcess, IN PENUMLOADED_MODULES_CALLBACK64 EnumLoadedModulesCallback, IN PVOID UserContext);
	typedef BOOL    (__stdcall *MiniDumpWriteDumpFP)(HANDLE hProcess, DWORD ProcessId, HANDLE hFile, MINIDUMP_TYPE DumpType, PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam, PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam, PMINIDUMP_CALLBACK_INFORMATION CallbackParam);

   static SymSetOptionsFP            symSetOptions;
   static SymInitializeFP            symInitialize;
   static SymCleanupFP               symCleanup;
   static StackWalk64FP              stackWalk64;
   static SymGetModuleInfo64FP       symGetModuleInfo64;
   static SymLoadModule64FP          symLoadModule64;
   static SymGetSymFromAddr64FP      symGetSymFromAddr64;
   static SymGetLineFromAddr64FP     symGetLineFromAddr64;
   static SymFunctionTableAccess64FP symFunctionTableAccess64;
   static SymGetModuleBase64FP       symGetModuleBase64;
   static SymEnumerateModules64FP    symEnumerateModules64;
   static EnumerateLoadedModules64FP enumerateLoadedModules64;
   static MiniDumpWriteDumpFP        miniDumpWriteDump;
   static CRITICAL_SECTION           criticalSection; 

   // ----------------------------------------------------------------------

	//BOOL CALLBACK loadSymbolsForDllCallback(PTSTR ModuleName, DWORD64 ModuleBase, ULONG ModuleSize, PVOID UserContext);

   // ----------------------------------------------------------------------


   typedef  unsigned long  int  ub4;   /* unsigned 4-byte quantities */
   typedef  unsigned       char ub1;   /* unsigned 1-byte quantities */

   #define hashsize(n) ((ub4)1<<(n))
   #define hashmask(n) (hashsize(n)-1)

   /*
   --------------------------------------------------------------------
   mix -- mix 3 32-bit values reversibly.
   For every delta with one or two bits set, and the deltas of all three
     high bits or all three low bits, whether the original value of a,b,c
     is almost all zero or is uniformly distributed,
   * If mix() is run forward or backward, at least 32 bits in a,b,c
     have at least 1/4 probability of changing.
   * If mix() is run forward, every bit of c will change between 1/3 and
     2/3 of the time.  (Well, 22/100 and 78/100 for some 2-bit deltas.)
   mix() was built out of 36 single-cycle latency instructions in a 
     structure that could supported 2x parallelism, like so:
         a -= b; 
         a -= c; x = (c>>13);
         b -= c; a ^= x;
         b -= a; x = (a<<8);
         c -= a; b ^= x;
         c -= b; x = (b>>13);
         ...
     Unfortunately, superscalar Pentiums and Sparcs can't take advantage 
     of that parallelism.  They've also turned some of those single-cycle
     latency instructions into multi-cycle latency instructions.  Still,
     this is the fastest good hash I could find.  There were about 2^^68
     to choose from.  I only looked at a billion or so.
   --------------------------------------------------------------------
   */
   #define mix(a,b,c) \
   { \
     a -= b; a -= c; a ^= (c>>13); \
     b -= c; b -= a; b ^= (a<<8); \
     c -= a; c -= b; c ^= (b>>13); \
     a -= b; a -= c; a ^= (c>>12);  \
     b -= c; b -= a; b ^= (a<<16); \
     c -= a; c -= b; c ^= (b>>5); \
     a -= b; a -= c; a ^= (c>>3);  \
     b -= c; b -= a; b ^= (a<<10); \
     c -= a; c -= b; c ^= (b>>15); \
   }

   /*
   --------------------------------------------------------------------
   hash() -- hash a variable-length key into a 32-bit value
     k       : the key (the unaligned variable-length array of bytes)
     len     : the length of the key, counting by bytes
     initval : can be any 4-byte value
   Returns a 32-bit value.  Every bit of the key affects every bit of
   the return value.  Every 1-bit and 2-bit delta achieves avalanche.
   About 6*len+35 instructions.

   The best hash table sizes are powers of 2.  There is no need to do
   mod a prime (mod is sooo slow!).  If you need less than 32 bits,
   use a bitmask.  For example, if you need only 10 bits, do
     h = (h & hashmask(10));
   In which case, the hash table should have hashsize(10) elements.

   If you are hashing n strings (ub1 **)k, do it like this:
     for (i=0, h=0; i<n; ++i) h = hash( k[i], len[i], h);

   By Bob Jenkins, 1996.  bob_jenkins@burtleburtle.net.  You may use this
   code any way you wish, private, educational, or commercial.  It's free.

   See http://burtleburtle.net/bob/hash/evahash.html
   Use for hash table lookup, or anything where one collision in 2^^32 is
   acceptable.  Do NOT use for cryptographic purposes.
   --------------------------------------------------------------------
   */

   // k;        /* the key */
   // length;   /* the length of the key */
   // initval;  /* the previous hash, or an arbitrary value */
#if 0
   static ub4 hash(ub1 *k, const ub4 length, const ub4 initval)
   {
      ub4 a, b, c, len;

      /* Set up the internal state */
      len = length;
      a = b = 0x9e3779b9;  /* the golden ratio; an arbitrary value */
      c = initval;         /* the previous hash value */

      /*---------------------------------------- handle most of the key */
      while (len >= 12)
      {
         a += (k[0] +((ub4)k[1]<<8) +((ub4)k[2]<<16) +((ub4)k[3]<<24));
         b += (k[4] +((ub4)k[5]<<8) +((ub4)k[6]<<16) +((ub4)k[7]<<24));
         c += (k[8] +((ub4)k[9]<<8) +((ub4)k[10]<<16)+((ub4)k[11]<<24));
         mix(a,b,c);
         k += 12; len -= 12;
      }

      /*------------------------------------- handle the last 11 bytes */
      c += length;
      switch(len)              /* all the case statements fall through */
      {
      case 11: c+=((ub4)k[10]<<24);
      case 10: c+=((ub4)k[9]<<16);
      case 9 : c+=((ub4)k[8]<<8);
         /* the first byte of c is reserved for the length */
      case 8 : b+=((ub4)k[7]<<24);
      case 7 : b+=((ub4)k[6]<<16);
      case 6 : b+=((ub4)k[5]<<8);
      case 5 : b+=k[4];
      case 4 : a+=((ub4)k[3]<<24);
      case 3 : a+=((ub4)k[2]<<16);
      case 2 : a+=((ub4)k[1]<<8);
      case 1 : a+=k[0];
        /* case 0: nothing left to add */
      }
      mix(a,b,c);
      /*-------------------------------------------- report the result */
      return c;
   }
#endif 

   // version optimized for 4 byte input.
   static ub4 hash_DWORD(const DWORD in, const ub4 initval)
   {
      ub1 *const k = (ub1 *)&in;
      ub4 a, b, c, len;

      /* Set up the internal state */
      len = 4;
      a = b = 0x9e3779b9;  /* the golden ratio; an arbitrary value */
      c = initval;         /* the previous hash value */

      /*------------------------------------- handle the last 11 bytes */
      c += 4;
      a+=((ub4)k[3]<<24);
      a+=((ub4)k[2]<<16);
      a+=((ub4)k[1]<<8);
      a+=k[0];

      mix(a,b,c);

      /*-------------------------------------------- report the result */
      return c;
   }


   // ----------------------------------------------------------------------

	struct BaseAddressLookup
	{
		DWORD64 keyAddress;   // key
		DWORD64 baseAddress;  // value
	};

	static BaseAddressLookup *      s_baseAddressCache;
	static inline unsigned          _baseAddressCachePageBits()  { return 8; }
	static inline unsigned          _baseAddressCacheBits()      { return _baseAddressCachePageBits() + 12; }
	static inline unsigned          _baseAddressCacheSize()      { return 1 << (_baseAddressCacheBits()); }
	static inline unsigned          _baseAddressCacheElements()  { return _baseAddressCacheSize() / sizeof(*s_baseAddressCache); }
	static inline unsigned          _baseAddressCacheMask()      { return _baseAddressCacheElements() - 1; }

	static int s_baseAddressCacheMisses;
	static int s_baseAddressCacheHits;

	static int s_baseAddressElements;
	static int s_baseAddressUsed;

	/*
	static void _baseAddressCacheAnalyze()
	{
		s_baseAddressElements=0;
		s_baseAddressUsed=0;

		unsigned i;
		const unsigned count = _baseAddressCacheElements();
		for (i=0;i<count;i++)
		{
			const BaseAddressLookup *lookup = s_baseAddressCache + i;
			s_baseAddressElements++;
			if (lookup->baseAddress)
			{
				s_baseAddressUsed++;
			}
		}
	}
	*/

	static DWORD64 _baseAddressLookup(DWORD64 addr)
	{
		BaseAddressLookup *lookup;

		unsigned long bits = _baseAddressCacheBits();

		DWORD *addr32 = (DWORD *)&addr;
		unsigned long hash32 = hash_DWORD(addr32[0], addr32[1]);
		unsigned long hash   = (hash32>>(32-bits)) ^ hash32;

		unsigned long mask = _baseAddressCacheMask();

		unsigned long index = hash & mask;
		lookup = s_baseAddressCache + index;
		if (lookup->keyAddress==addr)
		{
			s_baseAddressCacheHits++;
			//DEBUG_FATAL(symGetModuleBase64(process, addr) != lookup->baseAddress, ("Cache failure.\n"));
			return lookup->baseAddress;
		}
		else
		{
			s_baseAddressCacheMisses++;

			DWORD64 baseAddress = symGetModuleBase64(process, addr);

			lookup->keyAddress=addr;
			lookup->baseAddress=baseAddress;

			return baseAddress;
		}
	}	

   static DWORD64 __stdcall getModuleBase(HANDLE hProcess, DWORD64 dwAddr)
	{
		UNREF(hProcess);
		DEBUG_FATAL(hProcess!=process, ("Wrong process handle for module base lookup.\n"));
		return _baseAddressLookup(dwAddr);
	}

   // ----------------------------------------------------------------------
	struct FunctionTableLookup
	{
		DWORD64 keyAddress;   // key
		PVOID   functionTable;  // value
	};

	static FunctionTableLookup *      s_functionTableCache;
	static inline unsigned          _functionTableCachePageBits()  { return 4; }
	static inline unsigned          _functionTableCacheBits()      { return _functionTableCachePageBits() + 12; }
	static inline unsigned          _functionTableCacheSize()      { return 1 << (_functionTableCacheBits()); }
	static inline unsigned          _functionTableCacheElements()  { return _functionTableCacheSize() / sizeof(*s_functionTableCache); }
	static inline unsigned          _functionTableCacheMask()      { return _functionTableCacheElements() - 1; }

	static int s_functionTableCacheMisses;
	static int s_functionTableCacheHits;


	static PVOID _functionTableLookup(DWORD64 addr)
	{
		FunctionTableLookup *lookup;

		unsigned long bits = _functionTableCacheBits();

		DWORD *addr32 = (DWORD *)&addr;
		unsigned long hash32 = hash_DWORD(addr32[0], addr32[1]);
		unsigned long hash   = (hash32>>(32-bits)) ^ hash32;

		unsigned long mask = _functionTableCacheMask();

		unsigned long index = hash & mask;
		lookup = s_functionTableCache + index;
		if (lookup->keyAddress==addr)
		{
			s_functionTableCacheHits++;
			//DEBUG_FATAL(symFunctionTableAccess64(process, addr) != lookup->functionTable, ("Cache failure.\n"));
			return lookup->functionTable;
		}
		else
		{
			s_functionTableCacheMisses++;
			PVOID functionTable = symFunctionTableAccess64(process, addr);
			lookup->keyAddress=addr;
			lookup->functionTable=functionTable;

			return functionTable;
		}
	}	

   static PVOID __stdcall functionTableAccess(HANDLE hProcess, DWORD64 dwAddr)
	{
		UNREF(hProcess);
		DEBUG_FATAL(hProcess!=process, ("Wrong process handle for module base lookup.\n"));
		return _functionTableLookup(DWORD(dwAddr));
	}

   // ----------------------------------------------------------------------
}
using namespace DebugHelpNamespace;

// ----------------------------------------------------------------------

BOOL CALLBACK loadSymbolsForDllCallback(PSTR ModuleName, DWORD64 ModuleBase, ULONG ModuleSize, PVOID UserContext)
{
	if (!library)
		return false;

	CallbackData *callbackData = reinterpret_cast<CallbackData *>(UserContext);

	// see if this is the right file module and if we can load its symbol information
	if (_stricmp(ModuleName, callbackData->name) == 0 && symLoadModule64(process, NULL, ModuleName, 0, ModuleBase, ModuleSize) != 0)
	{
		callbackData->loaded = true;
		return FALSE;
	}

	return TRUE;
}

// ======================================================================

void DebugHelp::install()
{
	DEBUG_FATAL(library, ("DebugHelp already installed"));

	library = LoadLibrary("dbghelp_6.3.17.0.dll");
	if (library)
	{
		process = GetCurrentProcess();

#define GPA(a, b) a = reinterpret_cast<b##FP>(GetProcAddress(library, #b)); DEBUG_FATAL(!a, ("GetProcAddress failed for " #b))
		GPA(symSetOptions,            SymSetOptions);
		GPA(symInitialize,            SymInitialize);
		GPA(symCleanup,               SymCleanup);	
		GPA(stackWalk64,              StackWalk64);
		GPA(symGetModuleInfo64,       SymGetModuleInfo64);
		GPA(symLoadModule64,          SymLoadModule64);
		GPA(symGetSymFromAddr64,      SymGetSymFromAddr64);
		GPA(symGetLineFromAddr64,     SymGetLineFromAddr64);
		GPA(symFunctionTableAccess64, SymFunctionTableAccess64);
		GPA(symGetModuleBase64,       SymGetModuleBase64);
		GPA(symEnumerateModules64,    SymEnumerateModules64);
		GPA(enumerateLoadedModules64, EnumerateLoadedModules64);
		GPA(miniDumpWriteDump,        MiniDumpWriteDump);
#undef GPA

		IGNORE_RETURN(symSetOptions(SYMOPT_DEFERRED_LOADS | SYMOPT_UNDNAME | SYMOPT_LOAD_LINES));

		// get the path to the executable
		char executableDirectory[MAX_PATH * 2];
		const DWORD result = GetModuleFileName(NULL, executableDirectory, sizeof(executableDirectory));
		FATAL(result == 0, ("GetModuleFileName failed"));
		char * const slash = strrchr(executableDirectory, '\\');
		DEBUG_FATAL(!slash, ("Executable path does not contain a slash"));
		*slash = '\0';

		const BOOL result1 = symInitialize(process,  executableDirectory, TRUE);
		UNREF(result1);
		DEBUG_FATAL(!result1, ("SymInitialize failed"));

      // -----------------------------------------------------------------------

      s_baseAddressCache = (BaseAddressLookup *)VirtualAlloc(0, _baseAddressCacheSize(), MEM_COMMIT, PAGE_READWRITE);

      s_functionTableCache = (FunctionTableLookup *)VirtualAlloc(0, _functionTableCacheSize(), MEM_COMMIT, PAGE_READWRITE);

      // -----------------------------------------------------------------------
	}

	// Initialize the critical section one time only.
	InitializeCriticalSection(&criticalSection); 
}

// ----------------------------------------------------------------------

void DebugHelp::remove()
{
   if (s_baseAddressCache)
   {
      VirtualFree(s_baseAddressCache, 0, MEM_RELEASE);
      s_baseAddressCache=0;
   }

   if (s_functionTableCache)
   {
      VirtualFree(s_functionTableCache, 0, MEM_RELEASE);
      s_functionTableCache=0;
   }

	if (library)
	{
		IGNORE_RETURN(symCleanup(process));
		IGNORE_RETURN(FreeLibrary(library));
		library                  = NULL;
		process                  = NULL;
		symSetOptions            = NULL;
		symInitialize            = NULL;
		symCleanup               = NULL;
		stackWalk64              = NULL;
		symGetModuleInfo64       = NULL;
		symLoadModule64          = NULL;
		symGetSymFromAddr64      = NULL;
		symGetLineFromAddr64     = NULL;
		symFunctionTableAccess64 = NULL;
		symGetModuleBase64       = NULL;
		symEnumerateModules64    = NULL;
		enumerateLoadedModules64 = NULL;
	}

	// Release resources used by the critical section object.
	DeleteCriticalSection(&criticalSection);
}

// ----------------------------------------------------------------------

bool DebugHelp::loadSymbolsForDll(const char *name)
{
	if (!library)
		return false;

	CallbackData callbackData = { name, false };
	enumerateLoadedModules64(process, (PENUMLOADED_MODULES_CALLBACK64)loadSymbolsForDllCallback, reinterpret_cast<void *>(&callbackData));
	return callbackData.loaded;
}

// ----------------------------------------------------------------------
#pragma warning (disable: 4740 4748)
void DebugHelp::getCallStack(uint32 *callStack, int sizeOfCallStack)
{
	{
		for (int i = 0; i < sizeOfCallStack; ++i)
			callStack[i] = 0;
	}

	if (!library)
		return;

	CONTEXT context;
	Zero(context);
	context.ContextFlags = CONTEXT_FULL;

	// GetThreadContext returns invalid data when called from within the same thread
	//if (!GetThreadContext(GetCurrentThread(), &context))
	//	return;

	EnterCriticalSection(&criticalSection);
	__asm
	{
		call GetEIP
		GetEIP:
		pop eax
		mov context.Eip, eax
		mov context.Esp, esp
		mov context.Ebp, ebp
	}
	LeaveCriticalSection(&criticalSection);

	STACKFRAME64 stackFrame;
	Zero(stackFrame);
	stackFrame.AddrPC.Mode      = AddrModeFlat;
	stackFrame.AddrPC.Offset    = context.Eip;
	stackFrame.AddrStack.Offset = context.Esp;
	stackFrame.AddrStack.Mode   = AddrModeFlat;
	stackFrame.AddrFrame.Offset = context.Ebp;
	stackFrame.AddrFrame.Mode   = AddrModeFlat;

	for (int i = 0; i < sizeOfCallStack; ++i, ++callStack)
	{
		if (stackWalk64(IMAGE_FILE_MACHINE_I386, process, process, &stackFrame, &context, NULL, functionTableAccess, getModuleBase, NULL))
		{
			const DWORD64 Offset = stackFrame.AddrPC.Offset;
			*callStack = DWORD(Offset);
		}
	}
}

// ----------------------------------------------------------------------

void DebugHelp::reportCallStack(int const maxStackDepth)
{
	// look up the call stack information
	int const callStackOffset = 2;
	int const callStackSize = callStackOffset + maxStackDepth;
	uint32 * callStack = static_cast<uint32 *>(_alloca((callStackOffset + maxStackDepth) * sizeof(uint32)));
	getCallStack(callStack, callStackOffset + maxStackDepth);

	// look up the caller's file and line
	if (callStack[callStackOffset])
	{
		char lib[4 * 1024] = { '\0' };
		char file[4 * 1024] = { '\0' };
		int line = 0;
		for (int i = callStackOffset; i < callStackSize; ++i)
		{
			if (callStack[i])
			{
				if (lookupAddress(callStack[i], lib, file, sizeof(file), line))
					REPORT_LOG(true, ("\t%s(%d) : caller %d\n", file, line, i-callStackOffset));
				else
					REPORT_LOG(true, ("\tunknown(0x%08X) : caller %d\n", static_cast<int>(callStack[i]), i-callStackOffset));
			}
		}
	}
}

// ----------------------------------------------------------------------

bool DebugHelp::lookupAddress(uint32 address, char *libName, char *fileName, int fileNameLength, int &line)
{
	UNREF(libName);

	if (!library)
		return false;

	// make sure the image is loaded
	IMAGEHLP_MODULE64 imageHelpModule;
	Zero(imageHelpModule);
	imageHelpModule.SizeOfStruct = sizeof(imageHelpModule);
	if (!symGetModuleInfo64(process, address, &imageHelpModule))
		return false;

	// look up the symbol
	const int MaxNameLength = 256;
	char buffer[sizeof(IMAGEHLP_SYMBOL64) + MaxNameLength];
	memset(buffer, 0, sizeof(buffer));
	IMAGEHLP_SYMBOL64 *imageHelpSymbol = reinterpret_cast<IMAGEHLP_SYMBOL64*>(buffer);
	imageHelpSymbol->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL64);
	imageHelpSymbol->Address = address;
	imageHelpSymbol->MaxNameLength = MaxNameLength;
	{
		DWORD64 displacement = 0;
		if (!symGetSymFromAddr64(process, address, &displacement, imageHelpSymbol))
		{
			return false;
		}
	}

	// look up the source file name and line number
	IMAGEHLP_LINE64 imageHelpLine;
	Zero(imageHelpLine);
	imageHelpLine.SizeOfStruct = sizeof(imageHelpLine);
	{
		DWORD displacement = 0;
		if (!symGetLineFromAddr64(process, address, &displacement, &imageHelpLine))
		{
			return false;
		}
	}

	// return the results
	strncpy(fileName, imageHelpLine.FileName, static_cast<uint>(fileNameLength));
	line = static_cast<int>(imageHelpLine.LineNumber);
	return true;
}

// ----------------------------------------------------------------------

bool DebugHelp::writeMiniDump(char const *miniDumpFileName, PEXCEPTION_POINTERS exceptionPointers)
{
	if (!miniDumpWriteDump)
		return false;

	char buffer[256];
	if (!miniDumpFileName)
	{
		// get the program name
		char programName[512];
		DWORD result = GetModuleFileName(NULL, programName, sizeof(programName));
		if (result == 0)
			return false;

		// get the file name without the path
		const char *shortProgramName = strrchr(programName, '\\');
		if (shortProgramName)
			++shortProgramName;
		else
			shortProgramName = programName;

		// lop off the extension
		char *dot = const_cast<char *>(strchr(shortProgramName, '.'));
		if (dot)
			*dot = '\0';

		// create a reasonable minidump filename
		snprintf(buffer, sizeof(buffer), "%s_%d.mdmp", shortProgramName, static_cast<int>(GetCurrentProcessId()));
		miniDumpFileName = buffer;
	}

	// create the file
	HANDLE const file = CreateFile(miniDumpFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_ARCHIVE, NULL);
	if (file == INVALID_HANDLE_VALUE)
		return false;

	// create the exception information
	MINIDUMP_EXCEPTION_INFORMATION exceptionInformationData;
	MINIDUMP_EXCEPTION_INFORMATION *exceptionInformation = 0;
	if (exceptionPointers)
	{
		exceptionInformationData.ThreadId = GetCurrentThreadId();
		exceptionInformationData.ExceptionPointers = exceptionPointers;
		exceptionInformationData.ClientPointers = true;
		exceptionInformation = &exceptionInformationData;
	}

	// @todo make the minidump style modifiable
  BOOL const result = miniDumpWriteDump(process, GetCurrentProcessId(), file, MiniDumpNormal, exceptionInformation, NULL, NULL);

	// close the file
	CloseHandle(file);

	return result ? true : false;
}

// ======================================================================
