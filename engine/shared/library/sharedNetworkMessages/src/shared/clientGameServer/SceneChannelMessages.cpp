// SceneChannelMessages.cpp
// copyright 2000 Verant Interactive
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/SceneChannelMessages.h"
#include "sharedNetwork/NetworkHandler.h"
#include "sharedMath/Quaternion.h"
#include "sharedMathArchive/TransformArchive.h"
#include "sharedFoundation/MemoryBlockManager.h"

#include <cstdio>

//-----------------------------------------------------------------------

namespace SceneChannelMessagesNamespace
{
	MemoryBlockManager* ms_sceneCreateObjectByNameBlockManager;
	MemoryBlockManager* ms_sceneCreateObjectByCrcBlockManager;
	MemoryBlockManager* ms_sceneDestroyBlockManager;
	MemoryBlockManager* ms_beginBaselinesBlockManager;
	MemoryBlockManager* ms_endBaselinesBlockManager;
}

using namespace SceneChannelMessagesNamespace;

//-----------------------------------------------------------------

SceneCreateObjectByName::SceneCreateObjectByName (const NetworkId & networkId, const Transform & transform, const std::string & templateName, const bool hyperspace) :
GameNetworkMessage ("SceneCreateObjectByName"),
m_networkId(networkId),
m_transform(transform),
m_templateName(templateName),
m_hyperspace(hyperspace)
{
	addVariable(m_networkId);
	addVariable(m_transform);
	addVariable(m_templateName);
	addVariable(m_hyperspace);
	NetworkHandler::reportMessage("send.SceneCreateObjectByName." + m_templateName.get(), sizeof(SceneCreateObjectByName) + m_templateName.get().length());
}

//-----------------------------------------------------------------

SceneCreateObjectByName::SceneCreateObjectByName(Archive::ReadIterator & source) :
GameNetworkMessage("SceneCreateObjectByName"),
m_networkId(NetworkId::cms_invalid),
m_transform (),
m_templateName(),
m_hyperspace(false)
{
	addVariable(m_networkId);
	addVariable(m_transform);
	addVariable(m_templateName);
	addVariable(m_hyperspace);

	AutoByteStream::unpack(source);
	NetworkHandler::reportMessage("recv.SceneCreateObjectByName." + m_templateName.get(), sizeof(SceneCreateObjectByName) + m_templateName.get().length());
}

//-----------------------------------------------------------------------

SceneCreateObjectByName::~SceneCreateObjectByName()
{
}

//-----------------------------------------------------------------------

const NetworkId SceneCreateObjectByName::getNetworkId(void) const
{
	return m_networkId.get();
}

//-----------------------------------------------------------------------

const Transform & SceneCreateObjectByName::getTransform(void) const
{
	return m_transform.get();
}

//-----------------------------------------------------------------------

const std::string & SceneCreateObjectByName::getTemplateName(void) const
{
	return m_templateName.get();
}

//-----------------------------------------------------------------------

bool SceneCreateObjectByName::getHyperspace() const
{
	return m_hyperspace.get();
}

//-----------------------------------------------------------------

SceneCreateObjectByCrc::SceneCreateObjectByCrc (const NetworkId & networkId, const Transform & transform, uint32 templateCrc, const bool hyperspace) :
GameNetworkMessage ("SceneCreateObjectByCrc"),
m_networkId(networkId),
m_transform(transform),
m_templateCrc(templateCrc),
m_hyperspace(hyperspace)
{
	addVariable(m_networkId);
	addVariable(m_transform);
	addVariable(m_templateCrc);
	addVariable(m_hyperspace);

	char buffer[64];
	sprintf(buffer, "send.SceneCreateObjectByCrc.%08x", static_cast<int>(templateCrc));
	NetworkHandler::reportMessage(buffer, sizeof(SceneCreateObjectByCrc));
}
//-----------------------------------------------------------------

SceneCreateObjectByCrc::SceneCreateObjectByCrc(Archive::ReadIterator & source) :
GameNetworkMessage("SceneCreateObjectByCrc"),
m_networkId(NetworkId::cms_invalid),
m_transform (),
m_templateCrc(),
m_hyperspace(false)
{
	addVariable(m_networkId);
	addVariable(m_transform);
	addVariable(m_templateCrc);
	addVariable(m_hyperspace);

	AutoByteStream::unpack(source);

	char buffer[64];
	sprintf(buffer, "recv.SceneCreateObjectByCrc.%08x", static_cast<int>(m_templateCrc.get()));
	NetworkHandler::reportMessage(buffer, sizeof(SceneCreateObjectByCrc));
}

//-----------------------------------------------------------------------

SceneCreateObjectByCrc::~SceneCreateObjectByCrc()
{
}

//-----------------------------------------------------------------------

const NetworkId SceneCreateObjectByCrc::getNetworkId(void) const
{
	return m_networkId.get();
}

//-----------------------------------------------------------------------

const Transform & SceneCreateObjectByCrc::getTransform(void) const
{
	return m_transform.get();
}

//-----------------------------------------------------------------------

uint32 SceneCreateObjectByCrc::getTemplateCrc(void) const
{
	return m_templateCrc.get();
}

//-----------------------------------------------------------------------

bool SceneCreateObjectByCrc::getHyperspace() const
{
	return m_hyperspace.get();
}

// ----------------------------------------------------------------------

SceneDestroyObject::SceneDestroyObject(const NetworkId &id, const bool hyperspace) :
GameNetworkMessage("SceneDestroyObject"),
m_networkId(id),
m_hyperspace(hyperspace)
{
	addVariable(m_networkId);
	addVariable(m_hyperspace);
}

//-----------------------------------------------------------------------

SceneDestroyObject::SceneDestroyObject(Archive::ReadIterator & source) :
GameNetworkMessage("SceneDestroyObject"),
m_networkId(NetworkId::cms_invalid),
m_hyperspace(false)
{
	addVariable(m_networkId);
	addVariable(m_hyperspace);
	AutoByteStream::unpack(source);
}

//-----------------------------------------------------------------------

SceneDestroyObject::~SceneDestroyObject()
{
}

//-----------------------------------------------------------------------

const NetworkId SceneDestroyObject::getNetworkId(void) const
{
	return m_networkId.get();
}

//-----------------------------------------------------------------------

bool SceneDestroyObject::getHyperspace(void) const
{
	return m_hyperspace.get();
}

//-----------------------------------------------------------------------

SceneEndBaselines::SceneEndBaselines(const NetworkId &id)
        : GameNetworkMessage("SceneEndBaselines"),
          networkId(id)
{
	addVariable(networkId);
}

//-----------------------------------------------------------------------

SceneEndBaselines::SceneEndBaselines(Archive::ReadIterator & source) 
        : GameNetworkMessage("SceneEndBaselines"),
          networkId(NetworkId::cms_invalid)
{
	addVariable(networkId);
	AutoByteStream::unpack(source);
}

//-----------------------------------------------------------------------

NetworkId SceneEndBaselines::getNetworkId() const
{
    return networkId.get();
}

//========================================================================

void SceneCreateObjectByName::install (void)
{
	DEBUG_FATAL (ms_sceneCreateObjectByNameBlockManager, ("SceneCreateObjectByName already installed"));
	ms_sceneCreateObjectByNameBlockManager = new MemoryBlockManager ("SceneCreateObjectByName::ms_sceneCreateObjectByNameBlockManager", true, sizeof(SceneCreateObjectByName), 0, 0, 0);
}

//-------------------------------------------------------------------

void SceneCreateObjectByName::remove (void)
{
	DEBUG_FATAL(!ms_sceneCreateObjectByNameBlockManager, ("SceneCreateObjectByName is not installed"));

	delete ms_sceneCreateObjectByNameBlockManager;
	ms_sceneCreateObjectByNameBlockManager = 0;
}

//-------------------------------------------------------------------

void *SceneCreateObjectByName::operator new (size_t size)
{
	UNREF (size);
	NOT_NULL (ms_sceneCreateObjectByNameBlockManager);
	DEBUG_FATAL (size != sizeof (SceneCreateObjectByName), ("bad size"));
	DEBUG_FATAL (size != static_cast<size_t> (ms_sceneCreateObjectByNameBlockManager->getElementSize()), ("SceneCreateObjectByName installed with bad size"));

	return ms_sceneCreateObjectByNameBlockManager->allocate ();
}

//-------------------------------------------------------------------

void SceneCreateObjectByName::operator delete (void *pointer)
{
	NOT_NULL (ms_sceneCreateObjectByNameBlockManager);

	ms_sceneCreateObjectByNameBlockManager->free (pointer);
}

//========================================================================

void SceneCreateObjectByCrc::install (void)
{
	DEBUG_FATAL (ms_sceneCreateObjectByCrcBlockManager, ("SceneCreateObjectByCrc already installed"));
	ms_sceneCreateObjectByCrcBlockManager = new MemoryBlockManager ("SceneCreateObjectByCrc::ms_sceneCreateObjectByCrcBlockManager", true, sizeof(SceneCreateObjectByCrc), 0, 0, 0);
}

//-------------------------------------------------------------------

void SceneCreateObjectByCrc::remove (void)
{
	DEBUG_FATAL(!ms_sceneCreateObjectByCrcBlockManager, ("SceneCreateObjectByCrc is not installed"));

	delete ms_sceneCreateObjectByCrcBlockManager;
	ms_sceneCreateObjectByCrcBlockManager = 0;
}

//-------------------------------------------------------------------

void *SceneCreateObjectByCrc::operator new (size_t size)
{
	UNREF (size);
	NOT_NULL (ms_sceneCreateObjectByCrcBlockManager);
	DEBUG_FATAL (size != sizeof (SceneCreateObjectByCrc), ("bad size"));
	DEBUG_FATAL (size != static_cast<size_t> (ms_sceneCreateObjectByCrcBlockManager->getElementSize()), ("SceneCreateObjectByCrc installed with bad size"));

	return ms_sceneCreateObjectByCrcBlockManager->allocate ();
}

//-------------------------------------------------------------------

void SceneCreateObjectByCrc::operator delete (void *pointer)
{
	NOT_NULL (ms_sceneCreateObjectByCrcBlockManager);

	ms_sceneCreateObjectByCrcBlockManager->free (pointer);
}

//===================================================================================

void SceneDestroyObject::install (void)
{
	DEBUG_FATAL (ms_sceneDestroyBlockManager, ("SceneDestroyObject already installed"));

	ms_sceneDestroyBlockManager = new MemoryBlockManager ("SceneDestroyObject::ms_sceneDestroyBlockManager", true, sizeof(SceneDestroyObject), 0, 0, 0);
}

//-------------------------------------------------------------------

void SceneDestroyObject::remove (void)
{
	DEBUG_FATAL(!ms_sceneDestroyBlockManager, ("SceneDestroyObject is not installed"));

	delete ms_sceneDestroyBlockManager;
	ms_sceneDestroyBlockManager = 0;
}

//-------------------------------------------------------------------

void *SceneDestroyObject::operator new (size_t size)
{
	UNREF (size);
	NOT_NULL (ms_sceneDestroyBlockManager);
	DEBUG_FATAL (size != sizeof (SceneDestroyObject), ("bad size"));
	DEBUG_FATAL (size != static_cast<size_t> (ms_sceneDestroyBlockManager->getElementSize()), ("SceneDestroyObject installed with bad size"));

	return ms_sceneDestroyBlockManager->allocate ();
}

//-------------------------------------------------------------------

void SceneDestroyObject::operator delete (void *pointer)
{
	NOT_NULL (ms_sceneDestroyBlockManager);

	ms_sceneDestroyBlockManager->free (pointer);
}

//======================================================================

void SceneEndBaselines::install (void)
{
	DEBUG_FATAL (ms_endBaselinesBlockManager, ("SceneEndBaselines already installed"));

	ms_endBaselinesBlockManager = new MemoryBlockManager ("SceneEndBaselines::ms_endBaselinesBlockManager", true, sizeof(SceneEndBaselines), 0, 0, 0);
}

//-------------------------------------------------------------------

void SceneEndBaselines::remove (void)
{
	DEBUG_FATAL(!ms_endBaselinesBlockManager, ("SceneEndBaselines is not installed"));

	delete ms_endBaselinesBlockManager;
	ms_endBaselinesBlockManager = 0;
}

//-------------------------------------------------------------------

void *SceneEndBaselines::operator new (size_t size)
{
	UNREF (size);
	NOT_NULL (ms_endBaselinesBlockManager);
	DEBUG_FATAL (size != sizeof (SceneEndBaselines), ("bad size"));
	DEBUG_FATAL (size != static_cast<size_t> (ms_endBaselinesBlockManager->getElementSize()), ("SceneEndBaselines installed with bad size"));

	return ms_endBaselinesBlockManager->allocate ();
}

//-------------------------------------------------------------------

void SceneEndBaselines::operator delete (void *pointer)
{
	NOT_NULL (ms_endBaselinesBlockManager);

	ms_endBaselinesBlockManager->free (pointer);
}

//-------------------------------------------------------------------

