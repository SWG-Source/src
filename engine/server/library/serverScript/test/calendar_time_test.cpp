// Regression test for getCalendarTime(year, month, day, hour, minute, second).
//
// The script API returns calendar time as a 32-bit jint. Where time_t is
// 32-bit, mktime() fails (-1) for a date it cannot represent; on LP64 it
// succeeds and the old code truncated the result to a plausible wrong time.
// The function must return the same answer on both ABIs: the time when it
// fits in 32 bits, -1 otherwise.
//
// calendar-time-test.sh compiles this file together with the function body
// taken verbatim from ScriptMethodsScript.cpp, so the real code is tested.

#include <climits>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <limits>

typedef int jint;
struct _JNIEnv;
typedef _JNIEnv JNIEnv;
typedef void *jobject;
#define JNICALL

namespace ScriptMethodsScriptNamespace
{
	jint JNICALL getCalendarTimeSeconds2(JNIEnv * env, jobject self, jint year, jint month, jint day, jint hour, jint minute, jint second);
}

#include CALENDAR_FUNCTION_SOURCE

namespace
{
	int failures = 0;

	void expect(char const *label, jint year, jint month, jint day, jint hour, jint minute, jint second, jint expected)
	{
		JNIEnv *const env = reinterpret_cast<JNIEnv *>(&failures); // any non-null pointer
		jint const got = ScriptMethodsScriptNamespace::getCalendarTimeSeconds2(env, 0, year, month, day, hour, minute, second);
		bool const ok = (got == expected);
		std::printf("%s  %-34s expected %11d got %11d\n", ok ? "PASS" : "FAIL", label, expected, got);
		if (!ok)
			++failures;
	}
}

int main()
{
	setenv("TZ", "UTC", 1);
	tzset();
	std::printf("sizeof(time_t)=%u sizeof(long)=%u\n", unsigned(sizeof(time_t)), unsigned(sizeof(long)));

	expect("2020-06-15 12:00:00",               2020,  6, 15, 12,  0,  0, 1592222400);
	expect("2038-01-19 03:14:07 (INT_MAX)",     2038,  1, 19,  3, 14,  7, INT_MAX);
	expect("2038-01-19 03:14:08 (INT_MAX + 1)", 2038,  1, 19,  3, 14,  8, -1);
	expect("2107-01-01 00:00:00",               2107,  1,  1,  0,  0,  0, -1);
	expect("year INT_MIN (year - 1900 overflows)", INT_MIN, 1, 1, 0, 0, 0, -1);

	std::printf(failures ? "FAILED %d\n" : "ALL PASS\n", failures);
	return failures ? 1 : 0;
}
