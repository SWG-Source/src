// ======================================================================
//
// TriggerVolume.h
//
// Copyright 2000-04 Sony Online Entertainment
//
// ======================================================================

#ifndef	_INCLUDED_TriggerVolume_H
#define	_INCLUDED_TriggerVolume_H

// ======================================================================

#include "sharedFoundation/NetworkId.h"
#include "sharedUtility/PooledString.h"

#include <unordered_set>

// ======================================================================

class ServerObject;
class SpatialSubdivisionHandle;
class Vector;

// ======================================================================

class TriggerVolume
{
public:


	TriggerVolume(ServerObject &owner, float radius, std::string const &name, bool isPromiscuous);
	virtual ~TriggerVolume();

	struct ServerObjectPointerHash
	{
		size_t operator()(ServerObject const * const ptr) const
		{
			return (reinterpret_cast<const size_t>(ptr) >> 4);
		};
	};

	typedef std::unordered_set<ServerObject *, ServerObjectPointerHash> ContentsSet;

	void                              addObject(ServerObject &object);
	void                              addEventSource(NetworkId const &source);
	ContentsSet const &  getContents() const;
	std::string const &               getName() const;
	ServerObject &                    getOwner();
	ServerObject const &              getOwner() const;
	bool                              getPromiscuous() const;
	float                             getRadius() const;
	SpatialSubdivisionHandle *        getSpatialSubdivisionHandle();
	bool                              hasObject(ServerObject const &object) const;
	void                              moveObject(ServerObject &object, Vector const &start, Vector const &end);
	void                              moveTriggerVolume(ServerObject &object, Vector const &start, Vector const &end);
	void                              removeEventSource(NetworkId const &source);
	void                              removeObject(ServerObject &object);
	void                              setPromiscuous(bool isPromiscuous);
	void                              setSpatialSubdivisionHandle(SpatialSubdivisionHandle *newHandle);
	void                              objectMoved(ServerObject &object);
	void                              objectDestroyed(ServerObject &object);
	void                              contentsChanged(std::vector<ServerObject *> const &newSortedContents);
	static void                       runScriptTriggers();

	virtual bool                      isNetworkTriggerVolume() const;
	virtual bool                      isPortalTriggerVolume() const;

private:
	TriggerVolume(TriggerVolume const &);
	TriggerVolume &operator=(TriggerVolume const &);

	void onEnter(ServerObject &object);
	void onExit(ServerObject &object);
	bool intersectsExtent(ServerObject const &object) const;

	virtual void virtualOnEnter(ServerObject &object);
	virtual void virtualOnExit(ServerObject &object);

private:
	float                        m_radius;
	ContentsSet                  m_contents;
	std::set<NetworkId>          m_eventSources;
	PooledString                 m_name;
	ServerObject &               m_owner;
	SpatialSubdivisionHandle *   m_spatialSubdivisionHandle;
	bool                         m_promiscuous;
};

// ----------------------------------------------------------------------

inline TriggerVolume::ContentsSet const &TriggerVolume::getContents() const
{
	return m_contents;
}

// ----------------------------------------------------------------------

inline std::string const &TriggerVolume::getName() const
{
	return m_name;
}

// ----------------------------------------------------------------------

inline ServerObject const &TriggerVolume::getOwner() const
{
	return m_owner;
}

// ----------------------------------------------------------------------

inline ServerObject &TriggerVolume::getOwner()
{
	return m_owner;
}

// ----------------------------------------------------------------------

inline bool TriggerVolume::getPromiscuous() const
{
	return m_promiscuous;
}

// ----------------------------------------------------------------------

inline float TriggerVolume::getRadius() const
{
	return m_radius;
}

// ----------------------------------------------------------------------

inline SpatialSubdivisionHandle *TriggerVolume::getSpatialSubdivisionHandle()
{
	return m_spatialSubdivisionHandle;
}

// ----------------------------------------------------------------------

inline bool TriggerVolume::hasObject(ServerObject const &object) const
{
	return m_contents.count(const_cast<ServerObject*>(&object)) > 0;
}

// ======================================================================

#endif	// _INCLUDED_TriggerVolume_H

