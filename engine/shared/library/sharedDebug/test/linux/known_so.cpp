// A function in a shared object for debughelp_test.cpp. On x86-64 the
// object is mapped above 4 GiB.
extern "C" __attribute__((noinline)) int knownSoFunction(int x)
{
	int y = x * 5;
	return y + 2;
}
