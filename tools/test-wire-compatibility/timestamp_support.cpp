// Standalone serializer test support only. No serializer or clock is replaced.
#include "sharedFoundation/FirstSharedFoundation.h"
#include "sharedFoundation/MessageQueue.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedMessageDispatch/Message.h"
#include <cstdlib>
#include <map>

MessageQueue::Data::Data() {}
MessageQueue::Data::~Data() {}
// ChatLogEntry serializers share a TU with ChatOnRequestLog; full message construction
// is outside this test and must not silently succeed against an inert network base.
GameNetworkMessage::GameNetworkMessage(std::string const &) { std::abort(); }
GameNetworkMessage::~GameNetworkMessage() {}
MessageDispatch::MessageBase::MessageBase(char const *) { std::abort(); }
MessageDispatch::MessageBase::~MessageBase() {}

namespace { std::map<MemoryBlockManager const *, size_t> elementSizes; }
MemoryBlockManager::MemoryBlockManager(char const *name, bool shared, int elementSize, int, int, int)
: m_name(name), m_shared(shared), m_currentNumberOfElements(0), m_allocator(0) {
 elementSizes[this]=static_cast<size_t>(elementSize);
}
MemoryBlockManager::~MemoryBlockManager() { elementSizes.erase(this); }
void *MemoryBlockManager::allocate(bool) { return ::operator new(elementSizes.at(this)); }
void MemoryBlockManager::free(void *p) { ::operator delete(p); }
// Tests call the actual message install(), but need neither dispatch registration nor
// server shutdown infrastructure; process exit reclaims these test-only pools.
void ExitChain::add(Function, char const *, int, bool) {}
void ControllerMessageFactory::registerControllerMessageHandler(int32_t, ControllerMessagePackFunction, ControllerMessageUnpackFunction, bool) {}
