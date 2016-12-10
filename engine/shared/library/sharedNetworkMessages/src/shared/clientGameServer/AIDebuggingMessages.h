// ======================================================================
//
// AIDebuggingMessages.h
// copyright(c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_AIDebuggingMessages_H
#define INCLUDED_AIDebuggingMessages_H

//-----------------------------------------------------------------------

#include "sharedMath/Vector.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedMath/Transform.h"
#include "sharedMathArchive/TransformArchive.h"

// ======================================================================

/**
 * This message is sent by the client to request the information about a object's path
 * This message will result in 1 AIObjectPathInfo message being sent down to the client.
 * This message will optionally result in 1 AIPathInfo messages being sent down to the client.
 * This message will optionally result in 1 or multiple AINodeInfo messages being sent down 
 * to the client (via the PathInfo message).
 */
class RequestWatchObjectPath : public GameNetworkMessage
{
public:
	static const char* const MESSAGE_TYPE;

public:
	RequestWatchObjectPath();
	explicit   RequestWatchObjectPath(Archive::ReadIterator& source);
	virtual   ~RequestWatchObjectPath();

	NetworkId  getObjectId() const;
	void       setObjectId(const NetworkId& pathId);
	NetworkId  getClientId() const;
	void       setClientId(const NetworkId &i_client);
	bool       getEnable() const;
	void       setEnable(const bool enable);

private:
	Archive::AutoVariable<NetworkId> m_objectId;
	Archive::AutoVariable<NetworkId> m_client;
	Archive::AutoVariable<bool> m_enable;

private:
	RequestWatchObjectPath(const RequestWatchObjectPath&);
	RequestWatchObjectPath& operator=(const RequestWatchObjectPath& rhs);
};

//-----------------------------------------------------------------------

inline NetworkId RequestWatchObjectPath::getObjectId() const
{
	return m_objectId.get();
}

//-----------------------------------------------------------------------

inline void RequestWatchObjectPath::setObjectId(const NetworkId& objectId)
{
	m_objectId.set(objectId);
}

//-----------------------------------------------------------------------

inline NetworkId RequestWatchObjectPath::getClientId() const
{
	return m_client.get();
}

//-----------------------------------------------------------------------

inline void RequestWatchObjectPath::setClientId(const NetworkId& objectId)
{
	m_client.set(objectId);
}

//-----------------------------------------------------------------------

inline bool RequestWatchObjectPath::getEnable() const
{
	return m_enable.get();
}

//-----------------------------------------------------------------------

inline void RequestWatchObjectPath::setEnable(const bool enable)
{
	m_enable.set(enable);
}

// ======================================================================

/**
 * This message is sent by the client to request the information about a object's path
 * This message will result in 1 AIObjectPathInfo message being sent down to the client.
 * This message will optionally result in 1 AIPathInfo messages being sent down to the client.
 * This message will optionally result in 1 or multiple AINodeInfo messages being sent down 
 * to the client (via the PathInfo message).
 */
class RequestWatchPathMap : public GameNetworkMessage
{
public:
	static const char* const MESSAGE_TYPE;

public:
	RequestWatchPathMap();
	explicit   RequestWatchPathMap(Archive::ReadIterator& source);
	virtual   ~RequestWatchPathMap();

	NetworkId  getClientId() const;
	void       setClientId(const NetworkId &i_client);
	bool       getEnable() const;
	void       setEnable(const bool enable);

private:
	Archive::AutoVariable<NetworkId> m_client;
	Archive::AutoVariable<bool> m_enable;

private:
	RequestWatchPathMap(const RequestWatchPathMap&);
	RequestWatchPathMap& operator=(const RequestWatchPathMap& rhs);
};

//-----------------------------------------------------------------------

inline NetworkId RequestWatchPathMap::getClientId() const
{
	return m_client.get();
}

//-----------------------------------------------------------------------

inline void RequestWatchPathMap::setClientId(const NetworkId& objectId)
{
	m_client.set(objectId);
}

//-----------------------------------------------------------------------

inline bool RequestWatchPathMap::getEnable() const
{
	return m_enable.get();
}

//-----------------------------------------------------------------------

inline void RequestWatchPathMap::setEnable(const bool enable)
{
	m_enable.set(enable);
}

// ======================================================================

class RequestUnstick : public GameNetworkMessage
{
public:
	static const char* const MESSAGE_TYPE;

public:
	RequestUnstick();
	explicit   RequestUnstick(Archive::ReadIterator& source);
	virtual   ~RequestUnstick();

	NetworkId  getClientId() const;
	void       setClientId(const NetworkId &i_client);

private:
	Archive::AutoVariable<NetworkId> m_client;

private:
	RequestUnstick(const RequestUnstick&);
	RequestUnstick& operator=(const RequestUnstick& rhs);
};

//-----------------------------------------------------------------------

inline NetworkId RequestUnstick::getClientId() const
{
	return m_client.get();
}

//-----------------------------------------------------------------------

inline void RequestUnstick::setClientId(const NetworkId& objectId)
{
	m_client.set(objectId);
}

//========================================================================
/**
 * This message is sent from the gameserver to the client, and represents the data about a given nodeId.
 */
class AINodeInfo : public GameNetworkMessage
{
public:
	static const char* const MESSAGE_TYPE;

public:
	AINodeInfo();
	explicit   AINodeInfo(Archive::ReadIterator& source);
	virtual   ~AINodeInfo();

	int                           getNodeId() const;
	void                          setNodeId(int nodeId);
	Vector                        getLocation() const;
	void                          setLocation(const Vector& v);
	int                           getParent() const;
	void                          setParent(int p);
	const std::vector<int>&    getChildren() const;
	void                          setChildren(const std::vector<int>& c);
	const std::vector<int>&    getSiblings() const;
	void                          setSiblings(const std::vector<int>& c);
	int                           getType() const;
	void                          setType(int t);
	int                           getLevel() const;
	void                          setLevel(int l);


private:
	Archive::AutoVariable<int>    m_nodeId;
	Archive::AutoVariable<float>  m_locationX;
	Archive::AutoVariable<float>  m_locationY;
	Archive::AutoVariable<float>  m_locationZ;
	Archive::AutoVariable<int>    m_parent;
	Archive::AutoArray<int>       m_children;
	Archive::AutoArray<int>       m_siblings;
	Archive::AutoVariable<int>    m_type;
	Archive::AutoVariable<int>    m_level;

private:
	AINodeInfo(const AINodeInfo&);
	AINodeInfo& operator=(const AINodeInfo& rhs);
};

//-----------------------------------------------------------------------

inline int AINodeInfo::getNodeId() const
{
	return m_nodeId.get();
}

//-----------------------------------------------------------------------

inline void AINodeInfo::setNodeId(int n)
{
	m_nodeId.set(n);
}

//-----------------------------------------------------------------------

inline Vector AINodeInfo::getLocation() const
{
	return Vector(m_locationX.get(), m_locationY.get(), m_locationZ.get());
}

//-----------------------------------------------------------------------

inline void AINodeInfo::setLocation(const Vector& loc)
{
	m_locationX.set(loc.x);
	m_locationY.set(loc.y);
	m_locationZ.set(loc.z);
}

//-----------------------------------------------------------------------

inline int AINodeInfo::getParent() const
{
	return m_parent.get();
}

//-----------------------------------------------------------------------

inline void AINodeInfo::setParent(int p)
{
	m_parent.set(p);
}

//-----------------------------------------------------------------------

inline int AINodeInfo::getType() const
{
	return m_type.get();
}

//-----------------------------------------------------------------------

inline void AINodeInfo::setType(int t)
{
	m_type.set(t);
}

//-----------------------------------------------------------------------

inline int AINodeInfo::getLevel() const
{
	return m_level.get();
}

//-----------------------------------------------------------------------

inline void AINodeInfo::setLevel(int l)
{
	m_level.set(l);
}

// ======================================================================

struct AIPathInfo_NodeInfo
{
	int node;
	enum eState
	{
		kReported=1,
		kCanMove=2,
		kTarget=4,
		kPassed=8,
		kFacingTarget=16,
		kInCone=32,
		kFinalTarget=64
	};
	eState state;
	bool operator==(AIPathInfo_NodeInfo const &other) const
	{
		return node == other.node && state == other.state;
	}
};

/**
 * This message is sent from the gameserver to the client, and represents the data about a given object's path
 */
class AIPathInfo : public GameNetworkMessage
{
public:
	static const char* const MESSAGE_TYPE;

public:
	AIPathInfo();
	explicit AIPathInfo(Archive::ReadIterator &source);
	virtual ~AIPathInfo();

	NetworkId const &getObjectId() const;
	void setObjectId(NetworkId const & pathId);

	std::vector<AIPathInfo_NodeInfo> const &getNodes() const;
	void setNodes(std::vector<AIPathInfo_NodeInfo> const &n);

private:
	Archive::AutoVariable<NetworkId> m_objectId;
	Archive::AutoArray<AIPathInfo_NodeInfo> m_nodes;

private:
	AIPathInfo(AIPathInfo const &);
	AIPathInfo &operator=(AIPathInfo const &);
};

//-----------------------------------------------------------------------

inline NetworkId const &AIPathInfo::getObjectId() const
{
	return m_objectId.get();
}

//-----------------------------------------------------------------------

inline void AIPathInfo::setObjectId(NetworkId const &objectId)
{
	m_objectId.set(objectId);
}

// ======================================================================

namespace Archive
{
	void put(ByteStream & target, AIPathInfo_NodeInfo const &n);
	void get(ReadIterator & source, AIPathInfo_NodeInfo &n);
};

// ======================================================================

class DebugTransformMessage : public GameNetworkMessage
{
public: //ctor/dtor
	DebugTransformMessage   (const Transform& transform, const NetworkId& cellId);
	explicit DebugTransformMessage   (Archive::ReadIterator & source);
	~DebugTransformMessage  ();

public: // methods
	const Transform &            getTransform       () const;
	const NetworkId &            getCellId          () const;

	void                         pack               (Archive::ByteStream & target) const;

private: 
	Archive::AutoVariable<Transform>           m_transform;
	Archive::AutoVariable<NetworkId>            m_cellId;
};

//-----------------------------------------------------------------------
inline const Transform & DebugTransformMessage::getTransform() const
{
	return m_transform.get();
}
//-----------------------------------------------------------------------
inline const NetworkId & DebugTransformMessage::getCellId() const
{
	return m_cellId.get();
}
//-----------------------------------------------------------------------

#endif
