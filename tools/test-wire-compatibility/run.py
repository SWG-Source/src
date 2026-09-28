#!/usr/bin/env python3
"""Build real repository serializers; requires g++ and multilib for --bits 32."""
import argparse, os, pathlib, subprocess, tempfile
p=argparse.ArgumentParser(); p.add_argument('--bits',type=int,choices=[32,64],required=True); p.add_argument('--build-dir',type=pathlib.Path); a=p.parse_args()
root=pathlib.Path(__file__).resolve().parents[2]
sources=['tools/test-wire-compatibility/fixtures.cpp',
 'tools/test-wire-compatibility/fatal.cpp',
'external/ours/library/archive/src/shared/ByteStream.cpp',
'external/ours/library/archive/src/shared/AutoByteStream.cpp',
'external/ours/library/archive/src/shared/AutoDeltaByteStream.cpp',
'external/ours/library/archive/src/shared/AutoDeltaPackedMap.cpp',
'external/ours/library/archive/src/linux/ArchiveMutex.cpp',
'engine/shared/library/sharedFoundation/src/shared/NetworkId.cpp',
'engine/shared/library/sharedFoundation/src/shared/NetworkIdArchive.cpp',
'engine/shared/library/sharedGame/src/shared/quest/PlayerQuestData.cpp']
if a.build_dir:
 sources.remove('tools/test-wire-compatibility/fatal.cpp')
includes=[]
for base in (root/'engine/shared/library',root/'external/ours/library'):
 for lib in sorted(base.iterdir()):
  for suffix in ('include/public','include','src/shared','src/linux'):
   path=lib/suffix
   if path.is_dir(): includes+=['-I'+str(path)]
with tempfile.TemporaryDirectory(prefix='swg-wire-') as tmp:
 out=str(pathlib.Path(tmp)/'fixtures')
 cmd=[os.environ.get('CXX','g++'),'-std=c++17',f'-m{a.bits}','-DLINUX=1','-Dlinux=1','-D_USING_STL=1','-DDEBUG_LEVEL=0','-DPRODUCTION=1','-O1','-ffunction-sections','-fdata-sections','-Wl,--gc-sections','-pthread',*includes,*[str(root/s) for s in sources],'-o',out]
 if a.build_dir:
  libraries=sorted(str(x) for x in a.build_dir.resolve().rglob('lib*.a'))
  if not any(pathlib.Path(x).name=='libsharedNetworkMessages.a' for x in libraries):
   raise SystemExit('--build-dir must contain completed same-ABI server libraries')
  cmd+=['-DWIRE_TEST_MISSIONS=1','-Wl,--start-group',*libraries,'-Wl,--end-group','-ldl','-lz']
 result=subprocess.run(cmd)
 if result.returncode: raise SystemExit(result.returncode)
 raise SystemExit(subprocess.run([out]).returncode)
