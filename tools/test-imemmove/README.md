# Int-length memory move regression

Run with Python 3 and g++ (including multilib for 32-bit):

```
python3 tools/test-imemmove/run-linux.py --checkout . --bits 32 --consumers --out /tmp/imemmove32
python3 tools/test-imemmove/run-linux.py --checkout . --bits 64 --consumers --out /tmp/imemmove64
```

Outputs must be new directories. The probe includes actual FirstSharedFoundation.h/Misc.h and platform headers; no replacement headers or fatal functions. Each architecture executes 30 checks: overlapping moves in both directions, disjoint and identical regions, and zero length with valid pointers. A separate pre-copy byte snapshot supplies expected output. Checks cover the int helper, CRT size_t function pointer and ptrdiff_t expression, including return identity and the entire buffer. Negative sizes and null pointers are not executed. `--consumers` additionally compiles the actual changed Md5.cpp and Iff.cpp translation units; it does not link or run their complete libraries.

Use the same runner with `--checkout <clean-upstream-master> --baseline` for the original implementation. On tested GCC, upstream 7d2159a3 Linux32 passes the same 30 checks. Linux64 with `--baseline --expect-ambiguity` must fail compilation with exactly the two expected memmove ambiguity error locations: helper delegation and probe ptrdiff_t call. Missing, duplicate or unrelated errors fail the control; no failed binary is executed. This expectation is compiler/platform-specific, not a promise that every original 32-bit compiler accepts the overload.

The Linux runner records exact commands, compiler version, primary input hashes, dependency files, logs and results. It uses Release header policy; it is not initialized-engine Debug runtime evidence. Existing unrelated compiler warnings are retained, not suppressed.

For native MSVC use a matching VS2013 developer prompt:

```
python tools/test-imemmove/run.py --checkout . --bits 32 --configuration Release --out C:/temp/imemmove32
python tools/test-imemmove/run.py --checkout . --bits 64 --configuration Release --out C:/temp/imemmove64
```

This adapted client runner records included-header hashes. Debug mode only compiles the real Debug header; Release executes the same 30 checks. The strict MSVC baseline64 control uses `--baseline --expect-ambiguity`. Both native Release commands passed 30 checks with VS2013 on the candidate source. This native run used an exact Git archive of the three required header libraries and this test directory; it was not a full Windows server build. `python tools/test-imemmove/test_diagnostics.py` runs safe text-only tests of the MSVC diagnostic classifier.

Candidate production scope is exactly the distinct `imemmove` helper name and three calls in Md5/Iff. No buffer policy, platform type, SDK, build configuration or wire-format change is included.

Validated production commit `abdd4e94`: Linux32/64 each passed30 probe checks and compiled actual Md5.cpp/Iff.cpp. Linux64 Iff retains two warnings at unchanged Iff.cpp lines638/644: memcpy reads8 bytes from a4-byte region; Linux32 caller compilation emitted none. Upstream `7d2159a3` Linux32 passed30 and Linux64 matched the strict two-error control. Fourteen MSVC classifier tests passed. No full server link or gameplay run was performed for this isolated candidate.
