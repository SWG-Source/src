// Compile coverage only: instantiate every generic count-writing overload.
// This is not an oversized-container rejection test and is not called at runtime.
#include "sharedFoundation/FirstSharedFoundation.h"
#include "Archive/Archive.h"
#include "Archive/AutoDeltaMap.h"
#include "Archive/AutoDeltaQueue.h"
#include "Archive/AutoDeltaSet.h"
#include "Archive/AutoDeltaVector.h"

void compileCountSites(Archive::ByteStream &target)
{
 Archive::put(target, std::string());
 Archive::put(target, std::vector<int>());
 Archive::put(target, std::set<int>());
 Archive::put(target, std::deque<int>());
 Archive::put(target, std::map<int, int>());
 Archive::AutoDeltaMap<int, int> map;
 map.pack(target);
 map.packDelta(target);
 map.pack(target, std::vector<Archive::AutoDeltaMap<int, int>::Command>());
 Archive::AutoDeltaQueue<int> queue;
 queue.pack(target);
 queue.packDelta(target);
 Archive::AutoDeltaSet<int> set;
 set.pack(target);
 set.packDelta(target);
 set.pack(target, std::set<int>());
 Archive::AutoDeltaVector<int> vector;
 vector.pack(target);
 vector.packDelta(target);
 vector.pack(target, std::vector<int>());
}
