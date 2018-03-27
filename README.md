# Star Wars Galaxies Source Code (C++) Repository

This is the main server code for SWGSource 1.2 as originally forked from the https://bitbucket.org/stellabellumswg/ repository.  Please see that repository for original publication and alteration credit.

# Works in progress
* testing-64 - fully 64 bit version thatbuilds but doesn't run, some typedefs and things are wrong

# Building

## Clang Versions

**Important**: For versions of clang <= 4 you'll probably have to remove/omit a deprecated CFLAG or two from the CMakelists.txt file

Only use the Debug and Release targets unless you want to work on 64 bit. For local testing, and non-live builds set MODE=Release or MODE=debug in build_linux.sh.

For production, user facing builds, set MODE=MINSIZEREL for profile built, heavily optimized versions of the binaries.

## Profiling and Using Profiles

To generate new profiles, build SWG with MODE=RELWITHDEBINFO. 

Add export LLVM_PROFILE_FILE="output-%p.profraw" to your startServer.sh file. 

WHILE THE SERVER IS RUNNING do a ps -a to get the pid's of each SWG executable. And take note of which ones are which.

After you cleanly exit (shutdown) the server, and ctrl+c the LoginServer, move each output-pid.profraw to a folder named for it's process.

Then, proceed to combine them into usable profiles for the compiler:

llvm-profdata merge -output=code.profdata output-*.profraw

Finally, then replace the profdata files with the updated versions, within the src/ tree.

See http://clang.llvm.org/docs/UsersManual.html#profiling-with-instrumentation for more information.

# More Information

See http://www.swgsource.com/ for more information on the SWG Source project.

Join the SWGSource Discord if you would like to contribute:  https://discord.gg/j53cMj9