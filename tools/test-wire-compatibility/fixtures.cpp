#include "sharedFoundation/FirstSharedFoundation.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedGame/PlayerQuestData.h"
#include "Archive/AutoDeltaPackedMap.h"
#include "Archive/AutoDeltaVector.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <initializer_list>
#ifdef WIRE_TEST_MISSIONS
#include "sharedNetworkMessages/MessageQueueMissionListResponse.h"
#include "sharedNetworkMessages/MessageQueueMissionListResponseArchive.h"
#endif

static int failures;
static void check(bool ok, char const *name) { std::printf("%s: %s\n", ok ? "PASS" : "FAIL", name); failures += !ok; }
static Archive::ByteStream literal(std::initializer_list<unsigned char> bytes) {
 Archive::ByteStream out; for (auto b : bytes) out.put(&b,1); return out;
}
static bool equal(Archive::ByteStream const &a, Archive::ByteStream const &b) {
 return a.getSize()==b.getSize() && (!a.getSize() || !std::memcmp(a.getBuffer(),b.getBuffer(),a.getSize()));
}
int main() {
 // First quest use in this process: pack's Command constructs age 1;
 // active and completed values receive ages 2 and 3. This is a legacy32
 // fixture, including the non-persisted relative-age field (not normalized).
 auto quests=literal({2,0,0,0, 0,0,0,0,
   0, 0,0,0,128, 42,0,0,0,0,0,0,0, 1,0, 2,0, 0, 2,0,0,0, 0,
   0, 255,255,255,255, 0,0,0,0,0,0,0,0, 0,0, 0,0, 1, 3,0,0,0, 1});
 Archive::ByteStream questBytes;
 Archive::AutoDeltaPackedMap<uint32_t,PlayerQuestData>::pack(questBytes,"2147483648 1 2 42:4294967295 1:");
 check(equal(questBytes,quests),"active and completed high-key quests match legacy32 literal bytes including ages");
 std::string questText; auto questRead=quests.begin();
 Archive::AutoDeltaPackedMap<uint32_t,PlayerQuestData>::unpack(questRead,questText);
 check(questText=="2147483648 1 2 42:4294967295 1:","legacy32 quests decode active tasks, completed reward and high-bit keys");
 // Legacy x86 layout: int32 count, uint32 baseline count, ADD=0, uint32 key/value.
 Archive::ByteStream expected=literal({2,0,0,0, 0,0,0,0, 0, 0,0,0,128, 255,255,255,255, 0, 255,255,255,255, 0,0,0,128});
 Archive::ByteStream actual;
 Archive::AutoDeltaPackedMap<uint32_t,uint32_t>::pack(actual,"2147483648 4294967295:4294967295 2147483648:");
 check(equal(actual,expected),"packed map high-bit keys and values match legacy32 literal bytes");
 std::string text; auto r=expected.begin();
 Archive::AutoDeltaPackedMap<uint32_t,uint32_t>::unpack(r,text);
 check(text=="2147483648 4294967295:4294967295 2147483648:","packed map legacy32 literal decodes to exact unsigned decimal values");
 // Empty vector, baseline=UINT32_MAX. Two INSERT commands reach target=1
 // after modulo-2^32 wrap; legacy32 must retain both values, not skip them.
 auto baseline=literal({0,0,0,0,255,255,255,255});
 auto delta=literal({2,0,0,0,1,0,0,0, 1,0,0,65,0,0,0, 1,1,0,66,0,0,0});
 Archive::AutoDeltaVector<uint32_t> v;
 r=baseline.begin(); v.unpack(r); r=delta.begin(); v.unpackDelta(r);
 check(v.size()==2 && v[0]==65 && v[1]==66,"legacy32 counter wrap decodes two inserts into [65,66]");
 auto finalExpected=literal({2,0,0,0,1,0,0,0,65,0,0,0,66,0,0,0});
 Archive::ByteStream finalBytes; v.pack(finalBytes);
 check(equal(finalBytes,finalExpected),"counter wrap resulting baseline matches legacy32 bytes");
#ifdef WIRE_TEST_MISSIONS
 MessageQueueMissionListResponse::DataVector missions;
 MessageQueueMissionListResponse empty(missions, 7, true);
 Archive::ByteStream emptyBytes; Archive::put(emptyBytes,empty);
 check(equal(emptyBytes,literal({7,1,0,0,0,0})),"empty mission response has six-byte legacy32 header");
 MessageQueueMissionListResponseData mission;
 mission.bond=7; mission.difficulty=8; mission.reward=9; mission.missionData=NetworkId(int64(42));
 missions.push_back(mission); missions.push_back(mission);
 MessageQueueMissionListResponse two(missions,7,true);
 // Fixed legacy32 element layout: int32 bond, two empty UTF16 strings,
 // empty StringId(table uint16 length,index uint32,text uint16 length),
 // difficulty, empty planet/region, int64 ID, empty type, reward,
 // empty planet/region and empty title StringId.
 auto element=literal({7,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,0,0,0,0,
   8,0,0,0, 0,0, 0,0,0,0, 42,0,0,0,0,0,0,0, 0,0,0,0, 9,0,0,0,
   0,0, 0,0,0,0, 0,0,0,0,0,0,0,0});
 auto missionExpected=literal({7,1,2,0,0,0});
 missionExpected.put(element.getBuffer(),element.getSize());
 missionExpected.put(element.getBuffer(),element.getSize());
 Archive::ByteStream missionBytes; Archive::put(missionBytes,two);
 check(equal(missionBytes,missionExpected),"two mission entries match legacy32 count and element bytes");
 MessageQueueMissionListResponse decoded;
 auto missionRead=missionExpected.begin(); Archive::get(missionRead,decoded);
 check(decoded.getSequenceId()==7 && decoded.getBountyTerminal() && decoded.getResponse().size()==2
   && decoded.getResponse()[0].bond==7 && decoded.getResponse()[1].difficulty==8
   && decoded.getResponse()[1].reward==9 && decoded.getResponse()[1].missionData==NetworkId(int64(42))
   && missionRead.getSize()==0,"legacy32 mission response decodes both entries with no trailing bytes");
#else
 std::puts("NOT RUN: MissionListResponse (pass --build-dir with same-ABI server libraries)");
#endif
 return failures ? 1 : 0;
}
