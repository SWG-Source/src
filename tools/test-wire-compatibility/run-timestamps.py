#!/usr/bin/env python3
"""Build real repository serializers; requires g++ and multilib for --bits 32."""
import argparse, os, pathlib, subprocess, tempfile
p=argparse.ArgumentParser(); p.add_argument('--bits',type=int,choices=[32,64],required=True); a=p.parse_args()
root=pathlib.Path(__file__).resolve().parents[2]
sources = [
    'tools/test-wire-compatibility/timestamps.cpp',
    'tools/test-wire-compatibility/timestamp_support.cpp',
    'tools/test-wire-compatibility/fatal.cpp',
    'external/ours/library/archive/src/shared/ByteStream.cpp',
    'external/ours/library/archive/src/shared/AutoByteStream.cpp',
    'external/ours/library/archive/src/shared/AutoDeltaByteStream.cpp',
    'external/ours/library/archive/src/linux/ArchiveMutex.cpp',
    'engine/shared/library/sharedFoundation/src/shared/NetworkId.cpp',
    'engine/shared/library/sharedFoundation/src/shared/NetworkIdArchive.cpp',
    'engine/shared/library/sharedNetworkMessages/src/shared/chat/ChatOnRequestLog.cpp',
    'engine/shared/library/sharedNetworkMessages/src/shared/clientGameServer/ImageDesignChangeMessage.cpp',
    'engine/shared/library/sharedNetworkMessages/src/shared/clientGameServer/BuffBuilderChangeMessage.cpp',
    'external/ours/library/unicodeArchive/src/shared/UnicodeArchive.cpp',
]
includes=[]
for base in (root/'engine/shared/library',root/'external/ours/library'):
 for lib in sorted(base.iterdir()):
  for suffix in ('include/public','include','src/shared','src/linux'):
   path=lib/suffix
   if path.is_dir(): includes+=['-I'+str(path)]
with tempfile.TemporaryDirectory(prefix='swg-wire-') as tmp:
 out=str(pathlib.Path(tmp)/'fixtures')
 cmd=[os.environ.get('CXX','g++'),'-std=c++17',f'-m{a.bits}','-DLINUX=1','-Dlinux=1','-D_USING_STL=1','-DDEBUG_LEVEL=0','-DPRODUCTION=1','-O1','-ffunction-sections','-fdata-sections','-Wl,--gc-sections','-pthread',*includes,*[str(root/s) for s in sources],'-o',out]
 # Other changed per-message writers receive syntax coverage, not runtime coverage.
 syntax_sources=[
  'clientGameServer/DroidCommandProgrammingMessage.cpp',
  'clientGameServer/MessageQueueCraftExperiment.cpp',
  'clientGameServer/MessageQueueDraftSlotsDataArchive.cpp',
  'customerService/CustomerServiceCategoryArchive.cpp',
 ]
 syntax_cmd=[os.environ.get('CXX','g++'),'-std=c++17',f'-m{a.bits}',
  '-DLINUX=1','-Dlinux=1','-D_USING_STL=1','-DDEBUG_LEVEL=0','-DPRODUCTION=1',
  '-fsyntax-only',*includes,*[str(root/'engine/shared/library/sharedNetworkMessages/src/shared'/s) for s in syntax_sources]]
 result=subprocess.run(syntax_cmd)
 if result.returncode: raise SystemExit(result.returncode)
 result=subprocess.run(cmd)
 if result.returncode: raise SystemExit(result.returncode)
 result=subprocess.run([out], stdout=subprocess.PIPE, text=True)
 print(result.stdout, end='')
 if result.returncode: raise SystemExit(result.returncode)
 expected=16 if a.bits == 32 else 22
 observed=sum(line.startswith('PASS:') for line in result.stdout.splitlines())
 if observed != expected:
  raise SystemExit(f'Expected {expected} passing checks, observed {observed}')
