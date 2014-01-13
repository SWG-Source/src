// ======================================================================
//
// AIDebuggingMessages.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/AIDebuggingMessages.h"

//-----------------------------------------------------------------------

const char* const RequestWatchObjectPath::MESSAGE_TYPE  = "RequestWatchObjectPath::MESSAGE_TYPE";
const char* const RequestWatchPathMap::MESSAGE_TYPE     = "RequestWatchPathMap::MESSAGE_TYPE";
const char* const RequestUnstick::MESSAGE_TYPE          = "RequestUnstick::MESSAGE_TYPE";
const char* const AINodeInfo::MESSAGE_TYPE              = "AINodeInfo::MESSAGE_TYPE";
const char* const AIPathInfo::MESSAGE_TYPE              = "AIPathInfo::MESSAGE_TYPE";

//=======================================================================

RequestWatchObjectPath::RequestWatchObjectPath()
: GameNetworkMessage (MESSAGE_TYPE),
  m_objectId(),
  m_client(),
	m_enable()
{
	addVariable(m_objectId);
	addVariable(m_client);
	addVariable(m_enable);
}

//-----------------------------------------------------------------------

RequestWatchObjectPath::RequestWatchObjectPath(Archive::ReadIterator& source)
: GameNetworkMessage (MESSAGE_TYPE),
  m_objectId(),
  m_client(),
	m_enable()
{
	addVariable(m_objectId);
	addVariable(m_client);
	addVariable(m_enable);
	unpack(source);
}

//-----------------------------------------------------------------------

RequestWatchObjectPath::~RequestWatchObjectPath()
{
}

//=======================================================================

RequestWatchPathMap::RequestWatchPathMap()
: GameNetworkMessage (MESSAGE_TYPE),
  m_client(),
	m_enable()
{
	addVariable(m_client);
	addVariable(m_enable);
}

//-----------------------------------------------------------------------

RequestWatchPathMap::RequestWatchPathMap(Archive::ReadIterator& source)
: GameNetworkMessage (MESSAGE_TYPE),
  m_client(),
	m_enable()
{
	addVariable(m_client);
	addVariable(m_enable);
	unpack(source);
}

//-----------------------------------------------------------------------

RequestWatchPathMap::~RequestWatchPathMap()
{
}

//=======================================================================

RequestUnstick::RequestUnstick()
: GameNetworkMessage (MESSAGE_TYPE),
  m_client()
{
	addVariable(m_client);
}

//-----------------------------------------------------------------------

RequestUnstick::RequestUnstick(Archive::ReadIterator& source)
: GameNetworkMessage (MESSAGE_TYPE),
  m_client()
{
	addVariable(m_client);
	unpack(source);
}

//-----------------------------------------------------------------------

RequestUnstick::~RequestUnstick()
{
}

//=======================================================================

AINodeInfo::AINodeInfo()
: GameNetworkMessage (MESSAGE_TYPE),
  m_nodeId(),
  m_locationX(),
  m_locationY(),
  m_locationZ(),
  m_parent(),
  m_children(),
  m_siblings(),
  m_type(),
  m_level()
{
	addVariable(m_nodeId);
	addVariable(m_locationX);
	addVariable(m_locationY);
	addVariable(m_locationZ);
	addVariable(m_parent);
	addVariable(m_children);
	addVariable(m_siblings);
	addVariable(m_type);
	addVariable(m_level);
}

//-----------------------------------------------------------------------

AINodeInfo::AINodeInfo(Archive::ReadIterator& source)
: GameNetworkMessage (MESSAGE_TYPE),
  m_nodeId(),
  m_locationX(),
  m_locationY(),
  m_locationZ(),
  m_parent(),
  m_children(),
  m_siblings(),
  m_type(),
  m_level()
{
	addVariable(m_nodeId);
	addVariable(m_locationX);
	addVariable(m_locationY);
	addVariable(m_locationZ);
	addVariable(m_parent);
	addVariable(m_children);
	addVariable(m_siblings);
	addVariable(m_type);
	addVariable(m_level);
	unpack(source);
}

//-----------------------------------------------------------------------

const std::vector<int>& AINodeInfo::getChildren() const
{
	return m_children.get();
}

//-----------------------------------------------------------------------

void AINodeInfo::setChildren(const std::vector<int>& c)
{
	m_children.set(c);
}

//-----------------------------------------------------------------------

const std::vector<int>&  AINodeInfo::getSiblings() const
{
	return m_siblings.get();
}

//-----------------------------------------------------------------------

void AINodeInfo::setSiblings(const std::vector<int>& s)
{
	m_siblings.set(s);
}

//----------------------------------------------------------------------

AINodeInfo::~AINodeInfo()
{
}

//=======================================================================

AIPathInfo::AIPathInfo()
: GameNetworkMessage (MESSAGE_TYPE),
  m_objectId(),
  m_nodes()
{
	addVariable(m_objectId);
	addVariable(m_nodes);
}

//-----------------------------------------------------------------------

AIPathInfo::AIPathInfo(Archive::ReadIterator& source)
: GameNetworkMessage (MESSAGE_TYPE),
  m_objectId(),
  m_nodes()
{
	addVariable(m_objectId);
	addVariable(m_nodes);
	unpack(source);
}

//-----------------------------------------------------------------------

std::vector<AIPathInfo_NodeInfo> const &AIPathInfo::getNodes() const
{
	return m_nodes.get();
}

//-----------------------------------------------------------------------

void AIPathInfo::setNodes(std::vector<AIPathInfo_NodeInfo> const &n)
{
	m_nodes.set(n);
}

//----------------------------------------------------------------------

AIPathInfo::~AIPathInfo()
{
}


//----------------------------------------------------------------------

namespace Archive
{
	void put(ByteStream & target, AIPathInfo_NodeInfo const &n)
	{
		put(target, n.node);
		unsigned char state = static_cast<unsigned char>(n.state);
		put(target, state);
	}
	void get(ReadIterator & source, AIPathInfo_NodeInfo &n)
	{
		get(source, n.node);
		unsigned char state;
		get(source, state);
		n.state = static_cast<AIPathInfo_NodeInfo::eState>(state);
	}
};

//-----------------------------------------------------------------------

DebugTransformMessage::DebugTransformMessage(const Transform& transform, const NetworkId& cellId) :
GameNetworkMessage("DebugTransformMessage"),
m_transform(transform),
m_cellId(cellId)
{
	addVariable(m_transform);
	addVariable(m_cellId);
}

//-----------------------------------------------------------------------

DebugTransformMessage::DebugTransformMessage(Archive::ReadIterator & source) :
GameNetworkMessage("DebugTransformMessage"),
m_transform(),
m_cellId(NetworkId::cms_invalid)
{
	addVariable(m_transform);
	addVariable(m_cellId);
	unpack(source);
}

//-----------------------------------------------------------------------

DebugTransformMessage::~DebugTransformMessage  ()
{
}

//-----------------------------------------------------------------------

void DebugTransformMessage::pack(Archive::ByteStream & target) const
{
	AutoByteStream::pack(target);
}

//=======================================================================
