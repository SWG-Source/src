// Regression test: the Linux DebugHelp symbolizer on the running binary.
//
// DebugHelp maps the running executable (and each loaded shared object) and
// reads its DWARF line table to turn a code address into file and line for
// crash reports. It used to parse every binary with Elf32 types and an int
// file size: on a 64-bit build it found nothing, and on 32-bit it could
// return wrong lines. The test compiles the real DebugHelp.cpp in and looks
// up addresses whose file and line it knows, in the executable and in a
// shared object. debughelp-test.sh builds and runs it, and compares with
// addr2line. It must print ALL PASS on -m32 and -m64.

#include DEBUGHELP_SOURCE

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dlfcn.h>

namespace
{
	int failures = 0;

	void check(bool ok, char const *what)
	{
		std::printf("%s  %s\n", ok ? "PASS" : "FAIL", what);
		if (!ok)
			++failures;
	}

	bool endsWith(char const *text, char const *suffix)
	{
		size_t const a = std::strlen(text), b = std::strlen(suffix);
		return a >= b && std::strcmp(text + a - b, suffix) == 0;
	}

	void expectLine(char const *what, void const *function, char const *fileSuffix, int firstLine, int lastLine)
	{
		char lib[512] = "", file[512] = "";
		int line = -1;
		uint64 const address = reinterpret_cast<uint64>(function) + 8;
		bool const found = DebugHelp::lookupAddress(address, lib, file, sizeof(file), line);
		std::printf("      %s: address %p -> %s file=%s line=%d\n", what, reinterpret_cast<void *>(address), found ? "found" : "not found", file, line);
		std::printf("ADDR2LINE %s %llx %s:%d\n", what, static_cast<unsigned long long>(address), file, line);
		check(found && endsWith(file, fileSuffix) && line >= firstLine && line <= lastLine, what);
	}
}

static int const cs_knownFunctionFirstLine = __LINE__ + 1;
extern "C" __attribute__((noinline)) int knownFunction(int x)
{
	int y = x * 3;
	return y + 1;
}
static int const cs_knownFunctionLastLine = __LINE__ - 1;

int main(int argc, char **argv)
{
	std::printf("sizeof(void*)=%u\n", unsigned(sizeof(void *)));

	expectLine("lookupAddress(knownFunction + 8) gives this file and line", reinterpret_cast<void const *>(&knownFunction), "debughelp_test.cpp", cs_knownFunctionFirstLine, cs_knownFunctionLastLine);

	uint64 stack[8] = {0};
	DebugHelp::getCallStack(stack, 8);
	{
		// frame 0 may be getCallStack itself; main must be one of the frames
		bool foundMain = false;
		for (int i = 0; i < 8 && stack[i] && !foundMain; ++i)
		{
			char lib[512] = "", file[512] = "";
			int line = -1;
			bool const found = DebugHelp::lookupAddress(stack[i], lib, file, sizeof(file), line);
			std::printf("      call stack frame %d %p -> %s file=%s line=%d\n", i, reinterpret_cast<void *>(stack[i]), found ? "found" : "not found", file, line);
			foundMain = found && endsWith(file, "debughelp_test.cpp") && line > cs_knownFunctionLastLine;
		}
		check(foundMain, "getCallStack: a frame resolves to main in this file");
	}

	// argv[1]: a shared object built from known_so.cpp; argv[2], argv[3]: its first and last line numbers
	if (argc >= 4)
	{
		void *const handle = dlopen(argv[1], RTLD_NOW);
		if (!handle)
			std::printf("      dlopen: %s\n", dlerror());
		check(handle != 0, "dlopen the test shared object");
		if (handle)
			expectLine("lookupAddress(knownSoFunction + 8) in a shared object", dlsym(handle, "knownSoFunction"), "known_so.cpp", std::atoi(argv[2]), std::atoi(argv[3]));
	}

#ifdef DEBUGHELP_HAS_ELF_CLASS_CHECK
	// argv[4]: an ELF file of the other class, which must be rejected rather than misparsed
	if (argc >= 5)
	{
		MappedElfFile const self("/proc/self/exe");
		MappedElfFile const foreign(argv[4]);
		check(self.getSectionByName(".debug_line") >= 0, "MappedElfFile reads its own binary");
		check(foreign.getSectionByName(".shstrtab") < 0, "MappedElfFile rejects an ELF file of the other class");
	}
#endif

	std::printf(failures ? "FAILED %d\n" : "ALL PASS\n", failures);
	return (failures || knownFunction(argc) <= 0) ? 1 : 0;
}
