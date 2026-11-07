#include "sharedFoundation/FirstSharedFoundation.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedGame/PlayerQuestData.h"
#include "Archive/AutoDeltaPackedMap.h"
#include "Archive/AutoDeltaVector.h"
#include "Archive/AutoDeltaMap.h"
#include "Archive/AutoDeltaQueue.h"
#include "Archive/ArchiveCount.h"
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
template<typename Count> static bool rejectsCount(size_t value) {
 try { (void)ArchiveCount::fromSize<Count>(value); }
 catch (std::out_of_range const &) { return true; }
 catch (...) { return false; }
 return false;
}
int main() {
 check(ArchiveCount::fromSize<uint32_t>(UINT32_MAX)==UINT32_MAX,"unsigned count UINT32_MAX fits");
 check(ArchiveCount::fromSize<int32_t>(INT32_MAX)==INT32_MAX,"signed count INT32_MAX fits");
 check(rejectsCount<int32_t>(static_cast<size_t>(INT32_MAX)+1),"signed count INT32_MAX + 1 throws out_of_range");
 if (sizeof(size_t)>4)
  check(rejectsCount<uint32_t>(static_cast<size_t>(UINT64_C(0x100000000))),"unsigned count UINT32_MAX + 1 throws out_of_range");
 else
  std::puts("SKIP: size_t cannot represent UINT32_MAX + 1");
 // Test both sides of the actual string encoder's short/long marker boundary.
 for (size_t length : {size_t(65534), size_t(65535)}) {
  std::string input(length, 'x'), decoded;
  Archive::ByteStream encoded;
  Archive::put(encoded, input);
  auto expectedHeader = length == 65534 ? literal({254,255}) : literal({255,255,255,255,0,0});
  bool const headerMatches = encoded.getSize() == expectedHeader.getSize() + length &&
   std::memcmp(encoded.getBuffer(), expectedHeader.getBuffer(), expectedHeader.getSize()) == 0;
  auto reader = encoded.begin(); Archive::get(reader, decoded);
  check(headerMatches && decoded == input && reader.getSize() == 0,
   length == 65534 ? "string 65534 uses legacy short header" : "string 65535 uses legacy long header");
 }
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
 // Legacy x86 layout: uint32 count, uint32 baseline count, ADD=0, uint32 key/value.
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
 // AutoDeltaMap counters are legacy 32-bit unsigned (size_t on Win32): all delta
 // arithmetic is modulo 2^32. A client behind by commands that cross 2^31 must apply
 // the pending ADD and catch up, not treat the target as negative and drop it.
 {
  auto mapBaseline=literal({0,0,0,0, 0xf0,0xff,0xff,0x7f});
  auto mapDelta=literal({1,0,0,0, 5,0,0,0x80, 0, 1,0,0,0, 10,0,0,0});
  Archive::AutoDeltaMap<uint32_t,uint32_t> m;
  r=mapBaseline.begin(); m.unpack(r); r=mapDelta.begin(); m.unpackDelta(r);
  check(m.size()==1 && m.find(1)!=m.end() && m.find(1)->second==10,"map behind across 2^31 applies the pending ADD");
  Archive::ByteStream mapBytes; m.pack(mapBytes);
  check(equal(mapBytes,literal({1,0,0,0, 5,0,0,0x80, 0,1,0,0,0,10,0,0,0})),"map catches up to baseline 0x80000005 in legacy32 bytes");
 }
 // Wrap through zero: baseline 0xfffffffe, three commands, target 0. The first command
 // is already reflected (skip one); the last two apply and the counter wraps to 0.
 {
  auto mapBaseline=literal({0,0,0,0, 0xfe,0xff,0xff,0xff});
  auto mapDelta=literal({3,0,0,0, 0,0,0,0, 0,1,0,0,0,1,0,0,0, 0,2,0,0,0,2,0,0,0, 0,3,0,0,0,3,0,0,0});
  Archive::AutoDeltaMap<uint32_t,uint32_t> m;
  r=mapBaseline.begin(); m.unpack(r); r=mapDelta.begin(); m.unpackDelta(r);
  check(m.size()==2 && m.find(1)==m.end() && m.find(2)!=m.end() && m.find(3)!=m.end(),"map wrap through zero skips the one already-applied command");
  Archive::ByteStream mapBytes; m.pack(mapBytes);
  check(equal(mapBytes,literal({2,0,0,0, 0,0,0,0, 0,2,0,0,0,2,0,0,0, 0,3,0,0,0,3,0,0,0})),"map wrap resulting baseline 0 matches legacy32 bytes");
 }
 // Queue uses unsigned subtraction and clamps the skip count, without map catch-up.
 // baseline=0, one PUSH, target=2 => difference UINT32_MAX: skip the whole delta.
 {
  auto queueBaseline=literal({0,0,0,0, 0,0,0,0});
  auto queueDelta=literal({1,0,0,0, 2,0,0,0, 0, 65,0,0,0});
  Archive::AutoDeltaQueue<uint32_t> q;
  r=queueBaseline.begin(); q.unpack(r); r=queueDelta.begin(); q.unpackDelta(r);
  check(q.empty() && r.getSize()==0,"queue unsigned skip clamps and consumes an ahead delta");
  Archive::ByteStream queueBytes; q.pack(queueBytes);
  check(equal(queueBytes,queueBaseline),"queue skipped delta preserves legacy baseline zero");
 }
 // Two PUSHes wrap UINT32_MAX to 1; applying the same delta twice must not duplicate them.
 {
  auto queueBaseline=literal({0,0,0,0, 255,255,255,255});
  auto queueDelta=literal({2,0,0,0, 1,0,0,0, 0,65,0,0,0, 0,66,0,0,0});
  auto queueExpected=literal({2,0,0,0, 1,0,0,0, 0,65,0,0,0, 0,66,0,0,0});
  Archive::AutoDeltaQueue<uint32_t> q;
  r=queueBaseline.begin(); q.unpack(r); r=queueDelta.begin(); q.unpackDelta(r);
  Archive::ByteStream queueBytes; q.pack(queueBytes);
  check(equal(queueBytes,queueExpected) && r.getSize()==0,"queue wrap preserves both PUSHes and baseline one");
  r=queueDelta.begin(); q.unpackDelta(r);
  Archive::ByteStream repeatedBytes; q.pack(repeatedBytes);
  check(equal(repeatedBytes,queueExpected) && r.getSize()==0,"queue duplicate delta is consumed without reapplying PUSHes");
 }
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
