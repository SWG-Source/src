// ======================================================================
//
// SharedBuildoutAreaManager.h
//
// Copyright 2005 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SharedBuildoutAreaManager_H
#define INCLUDED_SharedBuildoutAreaManager_H

// ======================================================================

#include "sharedMath/Quaternion.h"
#include "sharedMath/Vector.h"
#include "sharedMath/Vector2d.h"
#include "sharedMath/Rectangle2d.h"

// ======================================================================

class BuildoutArea
{
public:
	int getSharedBaseId() const;
	int getSharedTopId() const;
	int getServerBaseId() const;
	int getServerTopId() const;
	bool isLocationInside(float x, float z) const;
	float getXCenterPoint(bool allowComposite) const;
	float getZCenterPoint(bool allowComposite) const;
	Rectangle2d const & getRectangle(bool allowComposite) const;
	Vector2d getSize(bool allowComposite) const;

	std::string const & getReferenceDisplayName() const;

	bool getUseComposite() const;

	Vector getRelativePosition(Vector const & pos_w, bool allowComposite) const;

	std::string const & getRequiredEventName() const;
	std::string const & getRequiredLoadLevel() const;

	int areaIndex;
	std::string areaName;
	Rectangle2d rect;
	bool useClipRect;
	Rectangle2d clipRect;
	int clipEnvironmentFlags;

	std::string compositeName;
	Rectangle2d compositeRect;

	//-- if a buildout area is isolated, it means you can't walk from it into another one.
	//-- or, if you can it means that waypoints in the previous one will shutdown when you walk into the new area
	bool isolated;

	bool allowMap;

	//-- if the buildout area is internal, it is not revealed to the player

	bool internalBuildoutArea;

	bool allowRadarTerrain;

	// Required Event name for a particular buildout
	std::string requiredEventName;

	// Required Server level for a particular buildout
	std::string requiredLoadLevel;
};

// ----------------------------------------------------------------------

struct ServerBuildoutAreaRow
{
	int         id;
	int         container;
	uint32      serverTemplateCrc;
	int         cellIndex;
	Vector      position;
	Quaternion  orientation;
	std::string scripts;
	std::string objvars;
};

// ----------------------------------------------------------------------

struct ClientBuildoutAreaRow
{
	int          id;
	int          container;
	uint32       type;
	uint32       sharedTemplateCrc;
	int          cellIndex;
	Vector       position;
	Quaternion   orientation;
	float        radius;
	uint32       portalLayoutCrc;
};

// ----------------------------------------------------------------------

class SharedBuildoutAreaManager
{
public:

	typedef std::vector<BuildoutArea> BuildoutAreaVector;
	typedef std::pair<std::string, std::string> StringPair;

	static bool isBuildoutScene(std::string const & sceneName);
	static void install();
	static void remove();
	static void load(std::string const & sceneName);
	static BuildoutAreaVector const &getBuildoutAreasForCurrentScene();
	static std::string getBuildoutNameForPosition(std::string const & sceneName, Vector const & location_w, bool ignoreInternal, bool ignoreNonActiveEvents = true);
	static Vector getRelativePositionInArea(std::string const & buildoutName, Vector const & location_w, bool ignoreInternal, bool ignoreNonActiveEvents = true);
	static StringPair parseEncodedBuildoutName(std::string const & encodedBuildoutName);
	static std::string createEncodedBuildoutName(std::string const & sceneName, std::string const & zoneName);
	static Vector2d getBuildoutAreaSize(std::string const & encodedBuildoutName, bool allowComposite);
	static bool adjustMapCoordinatesForScene(std::string const & sceneName);
	static Vector getCenterPositionInArea(const char *buildoutName, Vector const & location_w);
	static Vector getRelativePositionFromWorldSpaceToPlayerSpace(const char *zoneName, Vector const & playerPosition, Vector const & worldPosition);
	static BuildoutArea const * findBuildoutAreaAtPosition(float x, float z, bool ignoreInternal, bool ignoreNonActiveEvents = true);
	static BuildoutArea const * findBuildoutAreaAtPosition(std::string const & sceneName, float x, float z, bool ignoreInternal, bool ignoreNonActiveEvents = true);
	static BuildoutArea const * findBuildoutAreaAtPosition(Vector const & pos, bool ignoreInternal, bool ignoreNonActiveEvents = true);
	static BuildoutAreaVector const * findBuildoutAreasForScene(std::string const & sceneName);
	static bool isTraversable(std::string const & sceneName, Vector const & location_w1, Vector const & location_w2);
};

// ----------------------------------------------------------------------

namespace Archive
{
	class ReadIterator;
	class ByteStream;

	void get(ReadIterator &source, ServerBuildoutAreaRow &target);
	void put(ByteStream &target, ServerBuildoutAreaRow const &source);
	void get(ReadIterator &source, ClientBuildoutAreaRow &target);
	void put(ByteStream &target, ClientBuildoutAreaRow const &source);
}

//----------------------------------------------------------------------

inline BuildoutArea const * SharedBuildoutAreaManager::findBuildoutAreaAtPosition(Vector const & pos, bool ignoreInternal, bool ignoreNonActiveEvents)
{
	return SharedBuildoutAreaManager::findBuildoutAreaAtPosition(pos.x, pos.z, ignoreInternal, ignoreNonActiveEvents);
}

// ======================================================================

#endif // INCLUDED_SharedBuildoutAreaManager_H

