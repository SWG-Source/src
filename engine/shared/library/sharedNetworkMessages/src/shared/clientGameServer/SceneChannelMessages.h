// SceneChannelMessages.h
// copyright 2000 Verant Interactive
// Author: Justin Randall

#ifndef	_SCENE_CHANNEL_MESSAGES_H
#define	_SCENE_CHANNEL_MESSAGES_H

//-----------------------------------------------------------------------

namespace Archive
{
	class ByteStream;
}

#include "sharedFoundation/Tag.h"
#include "sharedMath/Transform.h"
#include "sharedMathArchive/TransformArchive.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

#include <string>

//-----------------------------------------------------------------------

class SceneCreateObjectByName : public GameNetworkMessage
{
public:
	SceneCreateObjectByName          (const NetworkId & networkId, const Transform & transform, const std::string & templateName, bool hyperspace = false);
	explicit SceneCreateObjectByName (Archive::ReadIterator & source);
	virtual ~SceneCreateObjectByName ();

	const NetworkId     getNetworkId    (void) const;
	const Transform &   getTransform    (void) const;
	const std::string & getTemplateName (void) const;
	bool                getHyperspace   (void) const;

	static void  install (void);
	static void  remove (void);
	static void* operator new (size_t size);
	static void  operator delete (void* pointer);

private:
	Archive::AutoVariable<NetworkId>   m_networkId;
	Archive::AutoVariable<Transform>   m_transform;
	Archive::AutoVariable<std::string> m_templateName;
	Archive::AutoVariable<bool>        m_hyperspace;

	SceneCreateObjectByName();
	SceneCreateObjectByName(const SceneCreateObjectByName&);
	SceneCreateObjectByName& operator= (const SceneCreateObjectByName&);
};

//-----------------------------------------------------------------------

class SceneCreateObjectByCrc : public GameNetworkMessage
{
public:
	SceneCreateObjectByCrc          (const NetworkId & networkId, const Transform & transform, uint32 templateCrc, bool hyperspace = false);
	explicit SceneCreateObjectByCrc (Archive::ReadIterator & source);
	virtual ~SceneCreateObjectByCrc ();

	const NetworkId     getNetworkId    (void) const;
	const Transform &   getTransform    (void) const;
	uint32              getTemplateCrc  (void) const;
	bool                getHyperspace   (void) const;

	static void  install (void);
	static void  remove (void);
	static void* operator new (size_t size);
	static void  operator delete (void* pointer);

private:
	Archive::AutoVariable<NetworkId>   m_networkId;
	Archive::AutoVariable<Transform>   m_transform;
	Archive::AutoVariable<uint32>      m_templateCrc;
	Archive::AutoVariable<bool>        m_hyperspace;

	SceneCreateObjectByCrc();
	SceneCreateObjectByCrc(const SceneCreateObjectByCrc&);
	SceneCreateObjectByCrc& operator= (const SceneCreateObjectByCrc&);
};

//-----------------------------------------------------------------------

class SceneDestroyObject : public GameNetworkMessage
{
public:
	explicit SceneDestroyObject(const NetworkId &id, bool hyperspace = false);
	explicit SceneDestroyObject(Archive::ReadIterator & source);
	virtual ~SceneDestroyObject();
	const NetworkId     getNetworkId (void) const;
	bool                getHyperspace(void) const;

	static void  install (void);
	static void  remove (void);
	static void* operator new (size_t size);
	static void  operator delete (void* pointer);

private:
	Archive::AutoVariable<NetworkId> m_networkId;
	Archive::AutoVariable<bool> m_hyperspace;

	SceneDestroyObject();
	SceneDestroyObject(const SceneDestroyObject&);
	SceneDestroyObject& operator= (const SceneDestroyObject&);
};

//-----------------------------------------------------------------------

class SceneEndBaselines : public GameNetworkMessage
{
public:
	explicit SceneEndBaselines(const NetworkId &id);
	explicit SceneEndBaselines(Archive::ReadIterator & source);

	virtual ~SceneEndBaselines() {};

	NetworkId getNetworkId() const;

	static void  install (void);
	static void  remove (void);
	static void* operator new (size_t size);
	static void  operator delete (void* pointer);

private:
	Archive::AutoVariable<NetworkId> networkId;

	SceneEndBaselines();
	SceneEndBaselines(const SceneEndBaselines &);
	SceneEndBaselines& operator= (const SceneEndBaselines &);
};

//-----------------------------------------------------------------------

#endif	// _SCENE_CHANNEL_MESSAGES_H

