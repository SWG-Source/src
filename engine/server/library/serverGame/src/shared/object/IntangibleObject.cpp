//========================================================================
//
// IntangibleObject.cpp
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/IntangibleObject.h"

#include "serverGame/Client.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/ContainerInterface.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/GameServer.h"
#include "serverGame/MessageToQueue.h"
#include "serverGame/ObjectTracker.h"
#include "serverGame/ObserveTracker.h"
#include "serverGame/Region.h"
#include "serverGame/RegionMaster.h"
#include "serverGame/ServerIntangibleObjectTemplate.h"
#include "serverGame/ServerUniverse.h"
#include "serverGame/ServerWorld.h"
#include "serverGame/ServerWorldIntangibleNotification.h"
#include "serverGame/TangibleObject.h"
#include "serverNetworkMessages/UpdateObjectOnPlanetMessage.h"
#include "serverScript/GameScriptObject.h"
#include "serverScript/ScriptParameters.h"
#include "serverUtility/ServerClock.h"
#include "sharedFoundation/Clock.h"
#include "sharedFoundation/ConstCharCrcLowerString.h"
#include "sharedGame/SharedObjectTemplate.h"
#include "sharedLog/Log.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "sharedObject/ObjectTemplateList.h"
#include "sharedTerrain/TerrainModificationHelper.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"

#include "sharedFoundation/CrcConstexpr.hpp"

#include <climits>

// ======================================================================

const SharedObjectTemplate * IntangibleObject::m_defaultSharedTemplate = nullptr;
uint32 IntangibleObject::ms_lastFrame = 0;
uint32 IntangibleObject::ms_theaterTime = 0;

static const uint32 THEATER_DATATABLE_TAG = constcrc("THEATER");

static const std::string THEATER_TEMPLATE("object/intangible/theater/base_theater.iff");
static const std::string THEATER_FLATTEN_LAYER("terrain/poi_small.lay");

static const std::string OBJVAR_THEATER_BASE("theater");
static const std::string OBJVAR_THEATER_CENTER(OBJVAR_THEATER_BASE + ".center");
static const std::string OBJVAR_THEATER_RADIUS(OBJVAR_THEATER_BASE + ".radius");
static const std::string OBJVAR_THEATER_PLAYER(OBJVAR_THEATER_BASE + ".player");
static const std::string OBJVAR_THEATER_OBJECTS(OBJVAR_THEATER_BASE + ".objects");
static const std::string OBJVAR_THEATER_NAME(OBJVAR_THEATER_BASE + ".name");
static const std::string OBJVAR_THEATER_FLATTEN(OBJVAR_THEATER_BASE + ".flatten");


// ======================================================================

IntangibleObject::IntangibleObject(const ServerIntangibleObjectTemplate* newTemplate) :
	ServerObject(newTemplate, ServerWorldIntangibleNotification::getInstance()),
	m_count(newTemplate->getCount()),
	m_isTheater(false),
	m_crcs(),
	m_positions(),
	m_headings(),
	m_scripts(),
	m_player(),
	m_objects(),
	m_center(),
	m_radius(),
	m_creator(),
	m_theaterName()
#ifdef _DEBUG
	, m_theaterCreationTime(0)
#endif
{

	WARNING_STRICT_FATAL(!getSharedTemplate(), ("Shared template for %s is nullptr", getTemplateName()));
	addMembersToPackages();
	ObjectTracker::addIntangible();
}	

//-----------------------------------------------------------------------

IntangibleObject::~IntangibleObject()
{
	ObjectTracker::removeIntangible();
}	// IntangibleObject::~IntangibleObject

//-----------------------------------------------------------------------

/**
 * Returns a shared template if none was given for this object.
 *
 * @return the shared template
 */
const SharedObjectTemplate * IntangibleObject::getDefaultSharedTemplate(void) const
{
static const ConstCharCrcLowerString templateName("object/intangible/base/shared_intangible_default.iff");

	if (m_defaultSharedTemplate == nullptr)
	{
		m_defaultSharedTemplate = safe_cast<const SharedObjectTemplate *>(
			ObjectTemplateList::fetch(templateName));
		WARNING_STRICT_FATAL(m_defaultSharedTemplate == nullptr, ("Cannot create "
			"default shared object template %s", templateName.getString()));
		if (m_defaultSharedTemplate != nullptr)
			ExitChain::add (removeDefaultTemplate, "IntangibleObject::removeDefaultTemplate");
	}
	return m_defaultSharedTemplate;
}	// IntangibleObject::getDefaultSharedTemplate

//-----------------------------------------------------------------------

/**
 * Cleans up the default shared template.
 */
void IntangibleObject::removeDefaultTemplate(void)
{
	if (m_defaultSharedTemplate != nullptr)
	{
		m_defaultSharedTemplate->releaseReference();
		m_defaultSharedTemplate = nullptr;
	}
}	// IntangibleObject::removeDefaultTemplate

//-----------------------------------------------------------------------

void IntangibleObject::onLoadedFromDatabase()
{
	ServerObject::onLoadedFromDatabase();

	if (isAuthoritative())
	{
		if (getObjVars().hasItem(OBJVAR_THEATER_BASE))
		{
			Vector center;
			float radius;
			NetworkId player;
			std::string name;
			int flatten;

			m_isTheater = true;
			if (getObjVars().getItem(OBJVAR_THEATER_CENTER, center))
				m_center = center;
			if (getObjVars().getItem(OBJVAR_THEATER_RADIUS, radius))
				m_radius = radius;
			if (getObjVars().getItem(OBJVAR_THEATER_PLAYER, player))
				m_player = CachedNetworkId(player);
			if (getObjVars().getItem(OBJVAR_THEATER_NAME, name))
				setTheaterName(name);
			if (getObjVars().getItem(OBJVAR_THEATER_FLATTEN, flatten))
			{
				if (flatten != 0)
				{
					TerrainGenerator::Layer * layer = TerrainModificationHelper::importLayer(THEATER_FLATTEN_LAYER.c_str());
					if (layer != nullptr)
					{
						setLayer(layer);
					}
				}
			}

			std::vector<NetworkId> ids;
			DynamicVariableList::NestedList objects(getObjVars(), OBJVAR_THEATER_OBJECTS);
			for (DynamicVariableList::NestedList::const_iterator i = objects.begin();
				i != objects.end(); ++i)
			{
				ids.clear();
				if (i.getValue(ids))
				{
					for (std::vector<NetworkId>::const_iterator j = ids.begin();
						j != ids.end(); ++j)
					{
						m_objects.push_back(CachedNetworkId(*j));
					}
				}
			}
		}
	}
}

// ----------------------------------------------------------------------

void IntangibleObject::sendObjectSpecificBaselinesToClient(Client const &client) const
{
	ServerObject::sendObjectSpecificBaselinesToClient(client);
	if (getLayer() != nullptr)
	{
		client.send(GenericValueTypeMessage<std::pair<NetworkId, bool> >(
			"IsFlattenedTheaterMessage", std::make_pair(getNetworkId(), true)), true);
	}
}

// ----------------------------------------------------------------------

float IntangibleObject::alter(float time)
{
size_t i;

	float returnValue = ServerObject::alter(time);

	if (isAuthoritative() && isTheater() && !isBeingDestroyed() && !isPlacing())
	{
		uint32 currentFrame = ServerClock::getInstance().getServerFrame();
		if (currentFrame != ms_lastFrame)
		{
			ms_lastFrame = currentFrame;
			ms_theaterTime = 0;
		}
		if (!m_crcs.empty() && m_crcs.size() > m_objects.size())
		{
#ifdef _DEBUG
			int objectsCreated = 0;
#endif
			unsigned long totalTime = 0;
			unsigned long startTime = Clock::timeMs();
			const Vector myPos(getPosition_w());
			size_t count = m_crcs.size();
			for (i = m_objects.size(); i < count;)
			{
				if (ms_theaterTime + totalTime >= static_cast<unsigned long>
					(ConfigServerGame::getTheaterCreationLimitMilliseconds()))
				{
					break;
				}

				Transform tr;
				tr.setPosition_p(myPos+m_positions[i]);
				ServerObject * newObject = ServerWorld::createNewObject(m_crcs[i], tr, 0, false);
				if (newObject != nullptr)
				{
					if (newObject->asTangibleObject() != nullptr)
						newObject->asTangibleObject()->setVisible(false);
					newObject->addToWorld();
					m_objects.push_back(CachedNetworkId(*newObject));
					// set the object heading
					if (i < m_headings.size())
					{
						newObject->yaw_o(m_headings[i] - newObject->getObjectFrameK_p().theta());
					}
					// attach the appropriate script to the object if needed
					if (i < m_scripts.size() && !m_scripts[i].empty())
					{
						NOT_NULL(newObject->getScriptObject());
						newObject->getScriptObject()->attachScript(m_scripts[i], true);
					}
#ifdef _DEBUG
					++objectsCreated;
#endif
				}
				else
				{
					m_objects.push_back(CachedNetworkId::cms_cachedInvalid);
					WARNING(true, ("Theater object %s could not create object "
						"from template crc %ul", getNetworkId().getValueString().c_str(), 
						m_crcs[i]));
				}
				++i;

				unsigned long endTime = Clock::timeMs();
				if (endTime >= startTime)
				{
					totalTime = endTime - startTime;
				}
				else
				{
					// time wrapped
					totalTime = endTime + (ULONG_MAX - startTime);
				}
			}
			ms_theaterTime += totalTime;
#ifdef _DEBUG
			m_theaterCreationTime += totalTime;
			DEBUG_LOG("Theater", ("Theater %s created %d objects, time = %lu\n", 
				getNetworkId().getValueString().c_str(), objectsCreated, totalTime));
#endif
			if (i == count)
			{
				// we created all the objects
#ifdef _DEBUG
				DEBUG_LOG("Theater", ("Theater %s total creation time = %lu\n", 
					getNetworkId().getValueString().c_str(), m_theaterCreationTime));
				startTime = Clock::timeMs();
#endif
				std::vector<ServerObject *> players;
				Vector sphereCenter(m_center.get());
				sphereCenter.y = findPosition_w().y;
				ServerWorld::findPlayerCreaturesInRange(sphereCenter, m_radius.get() + 64.0f, players);

				// make the objects visible
				count = m_objects.size();
				for (i = 0; i < count; ++i)
				{
					ServerObject * object = safe_cast<ServerObject *>(m_objects[i].getObject());
					if (object != nullptr && object->asTangibleObject() != nullptr)
					{
						const ServerObjectTemplate * objTemplate = safe_cast<const ServerObjectTemplate *>(object->getObjectTemplate());
						for (size_t j = 0; j < objTemplate->getVisibleFlagsCount(); ++j)
						{
							if (objTemplate->getVisibleFlags(j) == ServerObjectTemplate::VF_player)
							{
								object->asTangibleObject()->setVisible(true);
								ObserveTracker::onObjectMadeVisibleTo(*object, players);
								break;
							}
						}
					}
				}

				// clear out data we don't need anymore
				m_crcs.clear();
				m_positions.clear();
				m_headings.clear();
				m_scripts.clear();
#ifdef _DEBUG
				unsigned long endTime = Clock::timeMs();
				if (endTime >= startTime)
				{
					totalTime = endTime - startTime;
				}
				else
				{
					// time wrapped
					totalTime = endTime + (ULONG_MAX - startTime);
				}
				DEBUG_LOG("Theater", ("Theater %s make visible time = %lu\n", 
					getNetworkId().getValueString().c_str(), totalTime));
#endif

				// tell scripts that we're done
				NOT_NULL(getScriptObject());
				ScriptParams params;
				params.addParam(m_objects.get());
				params.addParam(m_player.get());
				params.addParam(m_creator.get());
				IGNORE_RETURN(getScriptObject()->trigAllScripts(
					Scripting::TRIG_THEATER_CREATED, params));
			}
			else
			{
				scheduleForAlter();
			}
		}
	}

	return returnValue;
}

//-----------------------------------------------------------------------

bool IntangibleObject::isVisibleOnClient (const Client & client) const
{
	if (isTheater())
	{
		return getLayer() != nullptr;
	}
	return true;
}

//------------------------------------------------------------------------------------------

/**
 * Called when we are being destroyed. We will destroy any objects we've created.
 */
void IntangibleObject::onPermanentlyDestroyed()
{
	if (isTheater())
	{
		DEBUG_LOG("Theater", ("IntangibleObject::onPermanentlyDestroyed destroying "
			"theater %s", getNetworkId().getValueString().c_str()));

		if (m_player.get() != CachedNetworkId::cms_invalid)
		{
			MessageToQueue::getInstance().sendMessageToC(m_player.get(), 
				"C++TheaterDestroyed", getNetworkId().getValueString(), 0, 
				false);
		}

		size_t count = m_objects.size();
		for (size_t i = 0; i < count; ++i)
		{
			ServerObject * object = safe_cast<ServerObject *>(m_objects[i].getObject());
			if (object != nullptr)
				object->permanentlyDestroy(DeleteReasons::Consumed);
		}
		if (!m_theaterName.get().empty())
			ServerUniverse::getInstance().clearTheater(m_theaterName.get());
	}

	ServerObject::onPermanentlyDestroyed();
}	// IntangibleObject::onPermanentlyDestroyed

//------------------------------------------------------------------------------------------

/**
 * Persists this object in the database. Will also persist all the objects it 
 * is keeping track of.
 */
void IntangibleObject::persist()
{
	if (isAuthoritative() && isTheater())
	{
		if (!m_crcs.empty() || m_objects.empty())
		{
			// we are not finished creating our objects, prevent persistance
			WARNING(true, ("IntangibleObject::persist called before all objects created"));
			return;
		}

		if (isPersisted())
			return;

		ServerObject::persist();

		// We want to store our objects' ids in an objvar, but we can't store
		// more than 1000 chars of data (which comes to about 47 network ids). 
		// So we split our objects into separate lists for storage
		static const int MAX_IDS = 45;
		std::vector<std::vector<NetworkId> > splitObjects;
		splitObjects.reserve(1000 / MAX_IDS + 1);
		int splitCount = MAX_IDS + 1;

		// persist the objects in the theater
		for (std::vector<CachedNetworkId>::const_iterator i = m_objects.begin();
			i != m_objects.end(); ++i, ++splitCount)
		{
			if (splitCount > MAX_IDS)
			{
				splitObjects.push_back(std::vector<NetworkId>());
				splitObjects.back().reserve(MAX_IDS);
				splitCount = 0;
			}
			ServerObject * o = safe_cast<ServerObject *>((*i).getObject());
			if (o != nullptr)
			{
				o->persist();
				splitObjects.back().push_back(*i);
			}
		}

		// since we don't have a database table, store our persisted data in objvars
		setObjVarItem(OBJVAR_THEATER_CENTER, m_center.get());
		setObjVarItem(OBJVAR_THEATER_RADIUS, m_radius.get());
		if (m_player.get() != CachedNetworkId::cms_cachedInvalid)
			setObjVarItem(OBJVAR_THEATER_PLAYER, m_player.get());
		setObjVarItem(OBJVAR_THEATER_NAME, m_theaterName.get());
		if (getLayer() != nullptr)
			setObjVarItem(OBJVAR_THEATER_FLATTEN, 1);
		
		char buffer[32];
		int k = 0;
		for (std::vector<std::vector<NetworkId> >::const_iterator j = splitObjects.begin(); 
			j != splitObjects.end(); ++j, ++k)
		{
			int stored = snprintf(buffer, sizeof(buffer), "%s.%d", OBJVAR_THEATER_OBJECTS.c_str(), k);
			if (static_cast<size_t>(stored) < sizeof(buffer))
				setObjVarItem(buffer, *j);
			else
			{
				WARNING_STRICT_FATAL(true, ("IntangibleObject::persist buffer "
					"overflow, %d %d %u", k, stored, sizeof(buffer)));
			}
		}

	}
	else
		ServerObject::persist();
}	// IntangibleObject::persist

//------------------------------------------------------------------------------------------

bool IntangibleObject::onContainerAboutToTransfer(ServerObject * destination, ServerObject* transferer)
{
	if (transferer && !destination)
	{
		//You cannot drop an intangible into the world.
		DEBUG_REPORT_LOG(true, ("Player tried to drop an intangible object\n"));
		return false;
	}
	return ServerObject::onContainerAboutToTransfer(destination, transferer);
}

//------------------------------------------------------------------------------------------

/**
 * Sends a message to the Planet Server to update the position of the object.
 */
void IntangibleObject::updatePlanetServerInternal(const bool forceUpdate) const
{
	// Don't send updates for contained intangibles, and only send when forced or moving
	if ((forceUpdate || getPositionChanged()) && !ContainerInterface::getContainedByObject(*this))
	{
		Vector const &position = getPosition_p();
	
		UpdateObjectOnPlanetMessage const msg(
			getNetworkId(),
			NetworkId::cms_invalid,
			static_cast<int>(position.x),
			static_cast<int>(position.y),
			static_cast<int>(position.z),
			0,
			0,
			false,
			false,
			static_cast<int>(getObjectType()),
			0,
			false,
			getTemplateCrc(),
			-1,
			-1);
		GameServer::getInstance().sendToPlanetServer(msg);
	}
}

// ----------------------------------------------------------------------

/**
 * Returns the approximate number of objects we can create per frame.
 *
 * @return the objects per frame
 */
int IntangibleObject::getObjectsCreatedPerFrame()
{
	return static_cast<int>((ConfigServerGame::getTheaterCreationLimitMilliseconds() / 
		1000.0f) / 5.5f) + 1;
}	// IntangibleObject::getObjectsCreatedPerFrame

//-----------------------------------------------------------------------

/**
 * Returns the number of objects defined in a theater datatable.
 * 
 * @param datatable		the datatable containing the theater objects
 *
 * @return the theater's object count
 */
int IntangibleObject::getNumObjects(const std::string & datatable)
{
	DataTable * dt = DataTableManager::getTable(datatable, true);
	if (dt == nullptr)
	{
		WARNING(true, ("IntangibleObject::getNumObjects could not open "
			"datatable %s", datatable.c_str()));
		return 0;
	}
	int rows = dt->getNumRows();
	if (rows <= 0)
	{
		WARNING(true, ("IntangibleObject::getNumObjects datatable %s has no "
			"rows", datatable.c_str()));
		return 0;
	}

	int templateColumn = dt->findColumnNumber("template");

	// if the 1st row's template is "THEATER", ignore it
	if (static_cast<uint32>(dt->getIntValue(templateColumn, 0)) == THEATER_DATATABLE_TAG)
		--rows;
	return rows;
}	// IntangibleObject::getNumObjects

//-----------------------------------------------------------------------

/**
 * Determines the radius for the area taken up by a theater. Note that the radius
 * is only determined by objects' positions, not size.
 * 
 * @param datatable		the datatable containing the theater objects
 *
 * @return the theater's radius
 */
float IntangibleObject::getRadius(const std::string & datatable)
{
	DataTable * dt = DataTableManager::getTable(datatable, true);
	if (dt == nullptr)
	{
		WARNING(true, ("IntangibleObject::getRadius could not open "
			"datatable %s", datatable.c_str()));
		return 0;
	}
	int rows = dt->getNumRows();
	if (rows <= 0)
	{
		WARNING(true, ("IntangibleObject::getRadius datatable %s has no "
			"rows", datatable.c_str()));
		return 0;
	}

	int templateColumn = dt->findColumnNumber("template");
	int xColumn        = dt->findColumnNumber("x");
	int zColumn        = dt->findColumnNumber("z");

	float minx = FLT_MAX;
	float maxx = -FLT_MIN;
	float minz = FLT_MAX;
	float maxz = -FLT_MIN;
	for (int i = 0; i < rows; ++i)
	{
		if (i == 0)
		{
			// if the 1st row's template is "THEATER", ignore it
			if (static_cast<uint32>(dt->getIntValue(templateColumn, i)) == THEATER_DATATABLE_TAG)
				continue;
		}
		float x = dt->getFloatValue(xColumn, i);
		float z = dt->getFloatValue(zColumn, i);
		if (x < minx)
			minx = x;
		else if (x > maxx)
			maxx = x;
		if (z < minz)
			minz = z;
		else if (z > maxz)
			maxz = z;
	}
	float xrad = (maxx - minx) / 2.0f;
	float zrad = (maxz - minz) / 2.0f;
	return sqrt(xrad * xrad + zrad * zrad);	
}	// IntangibleObject::getRadius

//-----------------------------------------------------------------------

/**
 * Creates a new theater from a datatable.
 *
 * @param datatable		the theater datatable
 * @param postion		the location of the theater
 * @param script		a script to be attached to the theater
 * @param locationType	how to create the theater
 *
 * @return the theater object
 */
IntangibleObject * IntangibleObject::spawnTheater(const std::string & datatable, 
	const Vector & position, const std::string & script, TheaterLocationType locationType)
{
	DataTable * dt = DataTableManager::getTable(datatable, true);
	if (dt == nullptr)
	{
		WARNING(true, ("IntangibleObject::spawnTheater could not open "
			"datatable %s", datatable.c_str()));
		return nullptr;
	}
	int rows = dt->getNumRows();
	if (rows <= 0)
	{
		WARNING(true, ("IntangibleObject::spawnTheater datatable %s has no "
			"rows", datatable.c_str()));
		return nullptr;
	}

	int templateColumn = dt->findColumnNumber("template");
	int xColumn        = dt->findColumnNumber("x");
	int yColumn        = dt->findColumnNumber("y");
	int zColumn        = dt->findColumnNumber("z");
	int headingColumn  = dt->findColumnNumber("heading");
	int scriptColumn   = dt->findColumnNumber("script");
	
	std::vector<int32> objectCrcs;
	std::vector<Vector> objectPositions;
	std::vector<float> objectHeadings;
	std::vector<std::string> objectScripts;

	dt->getIntColumn(templateColumn, objectCrcs);
	dt->getFloatColumn(headingColumn, objectHeadings);

	bool skipFirstRow = false;
	if (static_cast<uint32>(objectCrcs[0]) == THEATER_DATATABLE_TAG)
	{
		if (rows == 1)
		{
			WARNING(true, ("IntangibleObject::spawnTheater datatable %s has no "
				"objects", datatable.c_str()));
			return nullptr;
		}
		skipFirstRow = true;
		objectCrcs.erase(objectCrcs.begin());
		objectHeadings.erase(objectHeadings.begin());
		objectPositions.resize(rows-1);
		objectScripts.resize(rows-1);
	}
	else
	{
		objectPositions.resize(rows);
		objectScripts.resize(rows);
	}

	std::vector<float> tempPos;
	int i, j;
	int startIndex = skipFirstRow ? 1 : 0;
	float minx = FLT_MAX;
	float maxx = -FLT_MIN;
	float minz = FLT_MAX;
	float maxz = -FLT_MIN;

	dt->getFloatColumn(xColumn, tempPos);
	for (i = startIndex, j = 0; i < rows; ++i, ++j)
	{
		objectPositions[j].x = tempPos[i];
		if (tempPos[i] < minx)
			minx = tempPos[i];
		else if (tempPos[i] > maxx)
			maxx = tempPos[i];
	}
	dt->getFloatColumn(yColumn, tempPos);
	for (i = startIndex, j = 0; i < rows; ++i, ++j)
		objectPositions[j].y = tempPos[i];
	dt->getFloatColumn(zColumn, tempPos);
	for (i = startIndex, j = 0; i < rows; ++i, ++j)
	{
		objectPositions[j].z = tempPos[i];
		if (tempPos[i] < minz)
			minz = tempPos[i];
		else if (tempPos[i] > maxz)
			maxz = tempPos[i];
		objectScripts[j] = dt->getStringValue(scriptColumn, i);
	}

	// Find out the area the theater will take up and use getGoodLocation
	// to try and find somewhere where we can place the theater. Our search
	// area is the size of the theater * 3.
	float dx = maxx - minx;
	float dz = maxz - minz;
	float centerX = minx + dx / 2.0f + position.x;
	float centerZ = minz + dz / 2.0f + position.z;

	TerrainGenerator::Layer * layer = nullptr;
	if (locationType == TLT_flatten)
	{
		layer = TerrainModificationHelper::importLayer(THEATER_FLATTEN_LAYER.c_str());
		if (layer == nullptr)
		{
			WARNING (true, ("Layer %s not found for theater %s, using getGoodLocation "
				"instead", THEATER_FLATTEN_LAYER.c_str(), datatable.c_str()));
			locationType = TLT_getGoodLocation;
		}
	}

	Vector goodLoc(centerX, 0, centerZ);
	if (locationType == TLT_getGoodLocation)
	{
		goodLoc = ServerWorld::getGoodLocation(dx, dz, Vector(minx - dx + position.x, 
			0, minz - dz + position.z), Vector(maxx + dx + position.x, 0, maxz + dz + position.z), 
			false, false);
	}
	if (goodLoc.x == 0 && goodLoc.z == 0)
	{
		WARNING(true, ("IntangibleObject::spawnTheater could not find good location "
			"of size %f %f around point %f %f", dx, dz, centerX, centerZ));
		return 0;
	}
	
	// check the location region - don't spawn in cities or battlefields
	std::vector<const Region *> regions;
	RegionMaster::getRegionsAtPoint(ServerWorld::getSceneId(), goodLoc.x, goodLoc.z, regions);
	for (std::vector<const Region *>::const_iterator iter = regions.begin();
		iter != regions.end(); ++iter)
	{
		const Region * region = *iter;
		if (region != nullptr)
		{
			if (region->getPvp() == RegionNamespace::RP_pvpBattlefield ||
				region->getPvp() == RegionNamespace::RP_pveBattlefield ||
				region->getGeography() == RegionNamespace::RG_ocean ||
				region->getGeography() == RegionNamespace::RG_underwater ||
				region->getGeography() == RegionNamespace::RG_city)
			{
				WARNING(true, ("IntangibleObject::spawnTheater found good location "
					"at %f %f in bad region %s", goodLoc.x, goodLoc.z, 
					Unicode::wideToNarrow(region->getName()).c_str()));
				return 0;
			}
		}
	}

	DEBUG_LOG("Theater", ("IntangibleObject::spawnTheater found goodloc at %f %f",
		goodLoc.x, goodLoc.z));

	// create an object at the center of the theater that will be used to represent 
	// the entire theater
	Transform tr;
	tr.setPosition_p(goodLoc.x + position.x - centerX, 0, goodLoc.z + position.z - centerZ);
	IntangibleObject * theater = safe_cast<IntangibleObject *>(ServerWorld::createNewObject(THEATER_TEMPLATE, tr, 0, false));
	NOT_NULL(theater);

	if (layer != nullptr)
	{
		theater->setLayer(layer);
	}
	theater->setTheater();
	theater->setObjects(objectCrcs, objectPositions, objectHeadings, objectScripts);
	theater->addToWorld();
	theater->scheduleForAlter();

	if (!script.empty())
	{
		NOT_NULL(theater->getScriptObject());
		theater->getScriptObject()->attachScript(script, true);
	}

	return theater;
}	// IntangibleObject::spawnTheater

//-----------------------------------------------------------------------

/**
 * Sets the data for the objects that will be created for the theater.
 *
 * @param crcs			the object template crcs for the objects
 * @param positions		the positions where to create the objects, relative to 
 *						our position
 * @param headings		the orientation of the objects
 * @param scripts       list scripts to be attached to the objects
 */
void IntangibleObject::setObjects(const std::vector<int32> & crcs, 
	const std::vector<Vector> & positions, const std::vector<float> & headings, 
	const std::vector<std::string> & scripts)
{
	if (!isTheater())
		return;

	if (crcs.empty() || crcs.size() != positions.size())
		return;

	if (!headings.empty() && crcs.size() != headings.size())
		return;

	if (!scripts.empty() && crcs.size() != scripts.size())
		return;

	if (!isAuthoritative())
	{
		WARNING(true, ("IntangibleObject::setObjects called on non-authoritative "
			"object"));
		return;
	}

	float minx = FLT_MAX;
	float maxx = -FLT_MIN;
	float minz = FLT_MAX;
	float maxz = -FLT_MIN;
	size_t count = crcs.size();
	for (size_t i = 0; i < count; ++i)
	{
		m_crcs.push_back(crcs[i]);
		m_positions.push_back(positions[i]);
		if (positions[i].x < minx)
			minx = positions[i].x;
		else if (positions[i].x > maxx)
			maxx = positions[i].x;
		if (positions[i].z < minz)
			minz = positions[i].z;
		else if (positions[i].z > maxz)
			maxz = positions[i].z;

		if (!headings.empty())
			m_headings.push_back(convertDegreesToRadians(headings[i]));

		if (!scripts.empty())
			m_scripts.push_back(scripts[i]);
	}
	float xrad = (maxx - minx) / 2.0f;
	float zrad = (maxz - minz) / 2.0f;
	Vector mypos(getPosition_w());
	m_center = Vector(minx + xrad + mypos.x, 0, minz + zrad + mypos.z);
	m_radius = sqrt(xrad * xrad + zrad * zrad);
	DEBUG_LOG("Theater", ("Theater %s center = %f %f, rad = %f", 
		getNetworkId().getValueString().c_str(), m_center.get().x,
		m_center.get().z, m_radius.get()));
}	// IntangibleObject::setObjects

//-----------------------------------------------------------------------

/**
 * Sets the player this theater is created for.
 *
 * @param player		the theater's player
 */
void IntangibleObject::setPlayer(const CreatureObject & player)
{
	if (isTheater())
		m_player = CachedNetworkId(player);
}	// IntangibleObject::setPlayer

// ----------------------------------------------------------------------

/**
 * Sets the name of this theater.
 *
 * @param name		the theater's name
 *
 * @return true on success, false if there is already a theater with this name
 */
bool IntangibleObject::setTheaterName(const std::string & name)
{
	if (!isAuthoritative())
		return false;

	if (!isTheater())
		return false;

	// don't allow renaming theaters
	if (!m_theaterName.get().empty())
		return false;

	if (!name.empty())
	{
		const NetworkId & oldTheater = ServerUniverse::getInstance().findTheaterId(name);
		if (oldTheater != NetworkId::cms_invalid)
			return false;
		ServerUniverse::getInstance().setTheater(name, getNetworkId());
	}
	m_theaterName = name;
	return true;
}	// IntangibleObject::setTheaterName

// ----------------------------------------------------------------------

void IntangibleObject::getAttributes(std::vector<std::pair<std::string, Unicode::String> > &data) const
{
	ServerObject::getAttributes(data);
}

// ----------------------------------------------------------------------

IntangibleObject *IntangibleObject::asIntangibleObject()
{
	return this;
}

// ----------------------------------------------------------------------

IntangibleObject const *IntangibleObject::asIntangibleObject() const
{
	return this;
}

// ======================================================================

