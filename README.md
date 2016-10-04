This branch is to be used with profiles generated with clang and the cmake-profile branch. The profile branch includes changes to cause the applications to exit(0), which causes the clang profiler to flush it's stats to the profile files.


Prebuilt profilers for some (maybe all later) applications are included but as the code changes this may change.

I (darth) will probably use this branch for building release/tc builds.

See http://clang.llvm.org/docs/UsersManual.html#profiling-with-instrumentation 

It is suggested to strip -s the binaries before deployment.
