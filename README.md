# Branch Information

* master - stable, but internal only
* testing - bleeding edge, works in progress, internal only

# Old/Deprecated - For Reference Only
* clang-profile - intenal, modified to profile the code for clang-profile-build
* clang-profile-build - "release" and tc branch, as we want to use our fast profiled code
* google-profiler - modified for use with gperf
* icc - deprecated, modified for building with Intel's ICC compiler

# Building Notes

Only use the Debug and Release targets for testing internally, but never push these to public facing servers. For public facing builds, use the MINSIZEREL target for flto profile built bins, and RELWITHDEBUGINFO target for bins that produce profdata (see below). 

## Profiling and Using Profiles

To use, add export LLVM_PROFILE_FILE="output-%p.profraw" to your startServer.sh file. WHILE THE SERVER IS RUNNING do a ps -a to get the pid's of each SWG executable. After you cleanly exit (shutdown) the server, and ctrl+c the LoginServer, move each output-pid.profraw to a folder named for it's process.

Then, proceed to combine them into usable profiles for the compiler:

llvm-profdata merge -output=code.profdata output-*.profraw

See http://clang.llvm.org/docs/UsersManual.html#profiling-with-instrumentation
