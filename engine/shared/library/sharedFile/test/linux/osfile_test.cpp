// Regression test: the Linux OsFile honours the 32-bit file API on every ABI.
//
// AbstractFile::length()/tell() return int and TRE/TOC offsets are 32-bit, so
// a file of 2 GiB or more cannot be used. On ILP32 stat()/open() fail with
// EOVERFLOW for such a file; on LP64 they succeed and the old code wrapped its
// length into an int. A missing file also reported size 0, which
// TreeFile::getFileSize() reads as "found".
//
// The test makes sparse files of 12 KiB, 2 GiB - 1, 2 GiB and 3 GiB in a
// scratch directory, checks exists()/getFileSize()/open() on each, and adds
// the 2 GiB file as a search tree in a child process, so a crash is reported
// as a failure. Build and run with osfile-test.sh.

#include "sharedFile/FirstSharedFile.h"
#include "sharedFile/OsFile.h"
#include "sharedFile/TreeFile.h"

#include <climits>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>

namespace
{
	int failures = 0;

	void check(bool ok, std::string const &what)
	{
		std::printf("%s  %s\n", ok ? "PASS" : "FAIL", what.c_str());
		if (!ok)
			++failures;
	}

	bool makeSparseFile(std::string const &name, long long size)
	{
		// the scratch files are made with the 64-bit file API on every ABI
		int const fd = open64(name.c_str(), O_CREAT | O_TRUNC | O_WRONLY, 0600);
		if (fd < 0)
			return false;
		bool const ok = ftruncate64(fd, size) == 0;
		close(fd);
		return ok;
	}

	void usable(std::string const &name, int size)
	{
		check(OsFile::exists(name.c_str()), name + ": exists");
		check(OsFile::getFileSize(name.c_str()) == size, name + ": getFileSize is its size");
		OsFile *const file = OsFile::open(name.c_str());
		check(file != 0 && file->length() == size, name + ": opens with its length");
		delete file;
	}

	void refused(std::string const &name)
	{
		check(OsFile::exists(name.c_str()), name + ": exists");
		check(OsFile::getFileSize(name.c_str()) == -1, name + ": getFileSize is -1 (unusable)");
		OsFile *const file = OsFile::open(name.c_str());
		if (file)
			std::printf("      open returned a file with length %d\n", file->length());
		check(file == 0, name + ": open refuses it");
		delete file;
	}
}

int main()
{
	std::printf("sizeof(long)=%u sizeof(off_t)=%u\n", unsigned(sizeof(long)), unsigned(sizeof(off_t)));

	char directory[] = "/tmp/osfile_test.XXXXXX";
	if (!mkdtemp(directory))
		return 2;
	std::string const dir(directory);
	std::string const small = dir + "/small.bin";
	std::string const intMax = dir + "/2GiB-1.bin";
	std::string const twoGiB = dir + "/2GiB.bin";
	std::string const threeGiB = dir + "/3GiB.bin";
	std::string const missing = dir + "/missing.bin";

	if (!makeSparseFile(small, 12288) || !makeSparseFile(intMax, INT_MAX) || !makeSparseFile(twoGiB, 2147483648LL) || !makeSparseFile(threeGiB, 3221225472LL))
	{
		std::printf("cannot create sparse files in %s\n", directory);
		return 2;
	}

	usable(small, 12288);
	usable(intMax, INT_MAX);
	refused(twoGiB);
	refused(threeGiB);

	check(!OsFile::exists(missing.c_str()), "missing file: does not exist");
	check(OsFile::getFileSize(missing.c_str()) == -1, "missing file: getFileSize is -1");

	// A configured tree the file API refuses must leave the node empty in a
	// release build, not dereference the failed open.
	std::fflush(stdout);
	pid_t const child = fork();
	if (child == 0)
	{
		TreeFile::addSearchTree(twoGiB.c_str(), 1);
		std::printf("      addSearchTree returned; exists(\"foo.iff\")=%d\n", TreeFile::exists("foo.iff") ? 1 : 0);
		std::fflush(stdout);
		_exit(TreeFile::exists("foo.iff") ? 1 : 0);
	}
	int status = 0;
	waitpid(child, &status, 0);
	check(WIFEXITED(status) && WEXITSTATUS(status) == 0, "TreeFile::addSearchTree on a refused file: no crash, nothing found");

	unlink(small.c_str());
	unlink(intMax.c_str());
	unlink(twoGiB.c_str());
	unlink(threeGiB.c_str());
	rmdir(directory);

	std::printf(failures ? "FAILED %d\n" : "ALL PASS\n", failures);
	return failures ? 1 : 0;
}
