#include "sharedFoundation/FirstSharedFoundation.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedNetworkMessages/ImageDesignChangeMessage.h"
#include "sharedNetworkMessages/BuffBuilderChangeMessage.h"
#include "sharedNetworkMessages/ChatOnRequestLog.h"
#include "sharedNetworkMessages/NetworkMessageTimestamp.h"
#include <cstdio>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <initializer_list>

static int failures;
static void check(bool ok, char const *name) {
 std::printf("%s: %s\n", ok ? "PASS" : "FAIL", name);
 failures += !ok;
}
// Legacy timestamps are signed 32-bit time_t (Win32 _USE_32BIT_TIME_T; Linux -m32).
// Encode T with the message's real pack(); the result must differ from the T=0 encoding only
// in the 4 little-endian bytes at the legacy offset, and unpack() must return T as signed.
template<class Msg> static bool timestampRoundTrip(int offset, long long t) {
 Msg zero, msg; zero.setStartingTime(0); msg.setStartingTime(static_cast<time_t>(t));
 Archive::ByteStream a, b; Msg::pack(&zero,a); Msg::pack(&msg,b);
 if (a.getSize()!=b.getSize() || b.getSize()<static_cast<unsigned>(offset+4)) return false;
 unsigned const u=static_cast<unsigned>(static_cast<int>(t));
 for (unsigned i=0;i<b.getSize();++i) {
  unsigned char const want = (i>=static_cast<unsigned>(offset) && i<static_cast<unsigned>(offset+4)) ? static_cast<unsigned char>(u>>(8*(i-offset))) : a.getBuffer()[i];
  if (b.getBuffer()[i]!=want) return false; }
 auto rr=b.begin(); MessageQueue::Data *d=Msg::unpack(rr);
 bool const ok = static_cast<long long>(static_cast<Msg*>(d)->getStartingTime())==t && rr.getSize()==0;
 delete d; return ok;
}
static bool chatTimeRoundTrip(long long t) {
 ChatLogEntry e(Unicode::String(),Unicode::String(),Unicode::String(),Unicode::String(),static_cast<time_t>(t));
 unsigned const u=static_cast<unsigned>(static_cast<int>(t));
 Archive::ByteStream bytes; Archive::put(bytes,e);
 unsigned char const want[20]={0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, (unsigned char)u,(unsigned char)(u>>8),(unsigned char)(u>>16),(unsigned char)(u>>24)};
 if (bytes.getSize()!=20 || std::memcmp(bytes.getBuffer(),want,20)) return false;
 ChatLogEntry back; auto rr=bytes.begin(); Archive::get(rr,back);
 return static_cast<long long>(back.m_time)==t && rr.getSize()==0;
}


int main() {
 ImageDesignChangeMessage::install();
 BuffBuilderChangeMessage::install();
 for (long long t : {-2147483647LL-1, -1LL, 0LL, 2147483647LL}) {
  char label[128];
  std::snprintf(label,sizeof(label),"image-design signed timestamp %lld",t);
  check(timestampRoundTrip<ImageDesignChangeMessage>(33,t),label);
  std::snprintf(label,sizeof(label),"buff-builder signed timestamp %lld",t);
  check(timestampRoundTrip<BuffBuilderChangeMessage>(16,t),label);
  std::snprintf(label,sizeof(label),"chat-log signed timestamp %lld",t);
  check(chatTimeRoundTrip(t),label);
 }
 if (sizeof(time_t) > sizeof(int32_t)) {
  for (long long t : {-2147483649LL, 2147483648LL}) {
   ImageDesignChangeMessage image;
   BuffBuilderChangeMessage buff;
   image.setStartingTime(-1); buff.setStartingTime(-1);
   bool imageRejected=false, buffRejected=false, chatRejected=false;
   try { image.setStartingTime(static_cast<time_t>(t)); } catch (std::out_of_range const &) { imageRejected=true; }
   try { buff.setStartingTime(static_cast<time_t>(t)); } catch (std::out_of_range const &) { buffRejected=true; }
   try { ChatLogEntry entry(Unicode::String(),Unicode::String(),Unicode::String(),Unicode::String(),static_cast<time_t>(t)); } catch (std::out_of_range const &) { chatRejected=true; }
   check(imageRejected && image.getStartingTime()==-1,"image-design rejects out-of-range time without mutation");
   check(buffRejected && buff.getStartingTime()==-1,"buff-builder rejects out-of-range time without mutation");
   check(chatRejected,"chat-log rejects out-of-range constructor time");
  }
 }
 return failures ? 1 : 0;
}
