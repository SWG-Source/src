/* Differential test: the parser.yac integer helpers against a real ILP32 long.
   Build and run with arith-diff.sh (needs bison and gcc -m32). */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
int line_num; static int errors;
void yyerror(char *e) { (void)e; errors++; }
int yylex(void) { return 0; }
void MIFFMessage(char *m) { (void)m; }
char *MIFFallocString(int n) { return malloc(n); }
void MIFFfreeString(char *p) { free(p); }
int validateTargetFilename(char *f, int n) { (void)f; (void)n; return 0; }
void setCurrentLineNumber(int a, char *b, int c) { (void)a; (void)b; (void)c; }
void MIFFSetIFFName(const char *n) { (void)n; }
void MIFFinsertForm(const char *n) { (void)n; }
void MIFFinsertChunk(const char *n) { (void)n; }
static unsigned char last[8]; static unsigned lastSize;
void MIFFinsertChunkData(void *b, unsigned s) { lastSize = s; for (unsigned i = 0; i < s; i++) last[i] = ((unsigned char*)b)[i]; }
int MIFFloadRawData(char *f, void *b, unsigned m) { (void)f; (void)b; (void)m; return 0; }
void MIFFexitChunk(void) {}
void MIFFexitForm(void) {}
uint32_t MIFFgetLabelHash(char *s) { (void)s; return 0; }
int32_t intAdd(int32_t, int32_t); int32_t intSub(int32_t, int32_t); int32_t intMul(int32_t, int32_t);
int32_t intDiv(int32_t, int32_t); int32_t intMod(int32_t, int32_t); int32_t intShiftLeft(int32_t, int32_t);
int32_t intShiftRight(int32_t, int32_t); int32_t intAnd(int32_t, int32_t); int32_t intOr(int32_t, int32_t);
int32_t intXor(int32_t, int32_t); int32_t intNot(int32_t); int32_t intNegate(int32_t);
void writeInt8Value(int32_t); void writeUint8Value(int32_t); void writeInt16Value(int32_t); void writeUint16Value(int32_t);
void writeInt32Value(int32_t); void writeUint32Value(int32_t);

static long legacy(int op, long a, long b, int *defined)
{
	long long e; *defined = 1;
	switch (op) {
	case 0: e = (long long)a + b; if (e < LONG_MIN || e > LONG_MAX) *defined = 0; return (long)e;
	case 1: e = (long long)a - b; if (e < LONG_MIN || e > LONG_MAX) *defined = 0; return (long)e;
	case 2: e = (long long)a * b; if (e < LONG_MIN || e > LONG_MAX) *defined = 0; return (long)e;
	case 3: if (b == 0 || (a == LONG_MIN && b == -1)) { *defined = 0; return 0; } return a / b;
	case 4: if (b == 0 || (a == LONG_MIN && b == -1)) { *defined = 0; return 0; } return a % b;
	case 5: if (b < 0 || b > 31 || a < 0 || ((long long)a << b) > LONG_MAX) { *defined = 0; return 0; } return a << b;
	case 6: if (b < 0 || b > 31) { *defined = 0; return 0; } return a >> b; /* gcc: arithmetic */
	case 7: return a & b; case 8: return a | b; case 9: return a ^ b;
	case 10: return ~a;
	case 11: if (a == LONG_MIN) { *defined = 0; return 0; } return -a;
	}
	return 0;
}
static int32_t mine(int op, int32_t a, int32_t b)
{
	switch (op) {
	case 0: return intAdd(a, b); case 1: return intSub(a, b); case 2: return intMul(a, b);
	case 3: return intDiv(a, b); case 4: return intMod(a, b); case 5: return intShiftLeft(a, b);
	case 6: return intShiftRight(a, b); case 7: return intAnd(a, b); case 8: return intOr(a, b);
	case 9: return intXor(a, b); case 10: return intNot(a); default: return intNegate(a);
	}
}
static uint32_t rnd(void)
{
	static const uint32_t edge[] = {0, 1, 2, 3, 4, 31, 32, 0x7FFFFFFF, 0x80000000, 0x80000001, 0xFFFFFFFF, 0xFFFFFFFE, 0xFFFF, 0x10000, 0xFF, 0x100, 0xFFFF8000, 0xFFFFFF80};
	if (rand() % 3 == 0) return edge[rand() % (sizeof(edge) / sizeof(edge[0]))];
	if (rand() % 3 == 0) return (uint32_t)(rand() % 64);
	return ((uint32_t)rand() << 16) ^ (uint32_t)rand() ^ ((uint32_t)rand() << 31);
}
int main(void)
{
	long checked = 0, mismatches = 0, falseRejects = 0, undefinedAccepted = 0;
	if (sizeof(long) != 4) { puts("build with -m32"); return 2; }
	srand(12345);
	for (long i = 0; i < 3000000; i++) {
		int op = (int)(i % 12); uint32_t ua = rnd(), ub = rnd();
		long la = (long)ua, lb = (long)ub; /* ILP32: literal -> long takes the 32-bit pattern */
		int defined; long want = legacy(op, la, lb, &defined);
		errors = 0; int32_t got = mine(op, (int32_t)la, (int32_t)lb);
		checked++;
		if (defined && errors) { falseRejects++; if (falseRejects < 5) printf("false reject op %d %ld %ld\n", op, la, lb); }
		else if (defined && got != want) { mismatches++; if (mismatches < 5) printf("mismatch op %d %ld %ld: %ld vs %d\n", op, la, lb, want, got); }
		else if (!defined && !errors) undefinedAccepted++;
	}
	/* the reviewer's three expressions */
	errors = 0;
	printf("-1 ^ 0xFFFFFFFF = %d, ~0xFFFFFFFF = %d, ~0x80000000 = 0x%x, errors=%d\n", intXor(-1, (int32_t)(long)0xFFFFFFFFul), intNot((int32_t)(long)0xFFFFFFFFul), (unsigned)intNot((int32_t)(long)0x80000000ul), errors);
	/* writes: sample the int32 range in steps of 997; 8/16 bit accept exactly [-2^(N-1), 2^N-1] and write the legacy cast bytes */
	long writeBad = 0;
	for (long long v = INT32_MIN; v <= INT32_MAX; v += 997) {
		int32_t x = (int32_t)v; errors = 0; writeInt16Value(x);
		int fits = v >= -32768 && v <= 65535;
		if (fits != (errors == 0) || (fits && (last[0] != (unsigned char)(short)x || last[1] != (unsigned char)((unsigned short)(short)x >> 8)))) writeBad++;
		errors = 0; writeInt8Value(x); fits = v >= -128 && v <= 255;
		if (fits != (errors == 0) || (fits && last[0] != (unsigned char)(char)x)) writeBad++;
		errors = 0; writeUint32Value(x);
		uint32_t stored = 0; memcpy(&stored, last, sizeof(stored));
		if (errors || lastSize != sizeof(stored) || stored != (uint32_t)(unsigned long)(long)x) writeBad++;
	}
	printf("ops checked=%ld mismatches=%ld falseRejects=%ld undefinedRejected-only=%s writeBad=%ld\n", checked, mismatches, falseRejects, undefinedAccepted ? "NO" : "yes", writeBad);
	return (mismatches || falseRejects || undefinedAccepted || writeBad) ? 1 : 0;
}
