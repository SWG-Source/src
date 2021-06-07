// ======================================================================
//
// ServerBuildoutManager.cpp
//
// Copyright 2005 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ServerBuildoutManager.h"

#include "fileInterface/StdioFile.h"
#include "serverGame/BuildingObject.h"
#include "serverGame/CellObject.h"
#include "serverGame/Client.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/ContainerInterface.h"
#include "serverGame/GameServer.h"
#include "serverGame/PlanetObject.h"
#include "serverGame/ServerCellObjectTemplate.h"
#include "serverGame/ServerObject.h"
#include "serverGame/ServerStaticObjectTemplate.h"
#include "serverGame/ServerTangibleObjectTemplate.h"
#include "serverGame/ServerUniverse.h"
#include "serverGame/ServerWorld.h"
#include "serverScript/GameScriptObject.h"
#include "serverScript/ScriptParameters.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedGame/SharedBuildoutAreaManager.h"
#include "sharedGame/SharedObjectTemplate.h"
#include "sharedLog/Log.h"
#include "sharedMath/Quaternion.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "sharedObject/ContainedByProperty.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/ObjectTemplateList.h"
#include "sharedRandom/Random.h"
#include "sharedUtility/DataTable.h"
#include "UnicodeUtils.h"
#include <algorithm>

// ======================================================================

namespace ServerBuildoutManagerNamespace
{
	const int cs_buildingObjIdOffset = 2000;

	// ----------------------------------------------------------------------

	struct BuildoutRow
	{
		BuildoutRow();
		BuildoutRow(int64 id, int64 containerId, int cellIndex, Transform const &transform_p, ServerObjectTemplate const *serverTemplate, std::string const &scripts, std::string const &objvars, std::string const & eventRequired, std::string const & requiredLoadLevel);
		BuildoutRow(BuildoutRow const &rhs);
		~BuildoutRow();
		BuildoutRow &operator=(BuildoutRow const &rhs);

		int64 m_id;
		int64 m_containerId;
		Transform m_transform_p;
		ServerObjectTemplate const *m_serverTemplate;
		std::string m_scripts;
		std::string m_objvars;
		int m_cellIndex;
		std::string m_eventRequired;
		std::string m_requiredLoadLevel;
	};

	// ----------------------------------------------------------------------

	struct AreaInfo
	{
		BuildoutArea buildoutArea;
		bool loaded;
		bool editing;
		std::vector<BuildoutRow> buildoutRows;
	};

	struct ServerEventAreaInfo
	{
		ServerEventAreaInfo() :
			buildOut(nullptr),
			loadedObject(nullptr)
		{
		}
		ServerEventAreaInfo(BuildoutRow const * _buildOut, ServerObject* _loadedObject) :
			buildOut(_buildOut),
			loadedObject(_loadedObject)
		{
		}

		BuildoutRow const * buildOut;
		ServerObject* loadedObject;
	};

	typedef std::map< std::string, std::list< ServerEventAreaInfo > > ServerEventMap;
	// ----------------------------------------------------------------------

	int const s_buildoutCacheVersion = 2;
	bool s_installed = false;
	std::vector<AreaInfo> s_areas;
	ServerEventMap s_eventObjects;
	// ----------------------------------------------------------------------

	void remove();
	void generateBuildoutData(float x1, float z1, float x2, float z2, std::vector<ServerBuildoutAreaRow> &serverRows, std::vector<ClientBuildoutAreaRow> &clientRows);
	void loadArea(AreaInfo &areaInfo);
	void buildObjectsToSave(std::vector<ServerObject const *> &objectsToSave, std::vector<ServerObject const *> &potentialObjects);
	bool isNotObjectForBuildout(ServerObject const *obj);
	bool buildoutObjectLessComparator(ServerObject const *lhs, ServerObject const *rhs);
	void instantiateAreaNode(AreaInfo const &areaInfo, int nodeX, int nodeZ);

	// ----------------------------------------------------------------------
#ifdef _WIN32
	const int64 cs_dataBitStripMask = 0x80000000ffffffff;
#else
	const int64 cs_dataBitStripMask = 0x80000000ffffffffLL;
#endif
}
using namespace ServerBuildoutManagerNamespace;

// ======================================================================

void ServerBuildoutManager::install()
{
	FATAL(s_installed, ("ServerBuildoutManagerNamespace::install multiple install"));
	SharedBuildoutAreaManager::load(ConfigServerGame::getSceneID());

	std::vector<BuildoutArea> const &buildoutAreas = SharedBuildoutAreaManager::getBuildoutAreasForCurrentScene();

	for (std::vector<BuildoutArea>::const_iterator i = buildoutAreas.begin(); i != buildoutAreas.end(); ++i)
	{
		s_areas.push_back(AreaInfo());
		AreaInfo &areaInfo = s_areas.back();
		areaInfo.buildoutArea = *i;
		areaInfo.loaded = false;
		areaInfo.editing = false;
	}
	s_installed = true;
	ExitChain::add(remove, "ServerBuildoutManager::remove");
}

// ----------------------------------------------------------------------

void ServerBuildoutManager::onChunkComplete(int nodeX, int nodeZ)
{
    std::string configuredLoadLevel = ConfigServerGame::getServerLoadLevel();
	for (std::vector<AreaInfo>::iterator i = s_areas.begin(); i != s_areas.end(); ++i)
	{
		AreaInfo &areaInfo = *i;
		if (nodeX < areaInfo.buildoutArea.rect.x1
			&& nodeX + 100 > areaInfo.buildoutArea.rect.x0
			&& nodeZ < areaInfo.buildoutArea.rect.y1
			&& nodeZ + 100 > areaInfo.buildoutArea.rect.y0)
		{
			if (!areaInfo.loaded)
				loadArea(areaInfo);

		    // Before we decide to instantiate the area, make sure we need to actually load it based on the configured server load level.
			std::string areaRequiredLevel = areaInfo.buildoutArea.requiredLoadLevel;
			if (configuredLoadLevel != "heavy" && !areaRequiredLevel.empty()) {
			    if(configuredLoadLevel == "medium"){
                    // If our configured level is medium, then our area level must be medium or light.
                    if(areaRequiredLevel != "medium" && areaRequiredLevel != "light") {
                        DEBUG_REPORT_LOG(true,("Skipping Area %s (index %d) because its load level is %s and the configured load level is %s\n", areaInfo.buildoutArea.areaName.c_str(), areaInfo.buildoutArea.areaIndex, areaRequiredLevel.c_str(), configuredLoadLevel.c_str()));
                        continue;
                    }
			    }
			    else if(configuredLoadLevel == "light"){
                    if(areaRequiredLevel != "light") {
                        DEBUG_REPORT_LOG(true,("Skipping Area %s (index %d) because its load level is %s and the configured load level is %s\n", areaInfo.buildoutArea.areaName.c_str(), areaInfo.buildoutArea.areaIndex, areaRequiredLevel.c_str(), configuredLoadLevel.c_str()));
                        continue;
                    }
			    }
			}
			instantiateAreaNode(areaInfo, nodeX, nodeZ);
		}
	}
}

// ----------------------------------------------------------------------

void ServerBuildoutManager::saveArea(std::string const &serverFileName, std::string const &clientFileName, float x1, float z1, float x2, float z2)
{
	FATAL(true, ("ServerBuildoutManager::saveArea\n"));

	if (!ConfigServerGame::getBuildoutAreaEditingEnabled())
	{
		WARNING(true, ("Tried to save a buildout area with buildout area editing disabled?"));
		return;
	}

	StdioFile serverOutputFile(serverFileName.c_str(), "w");
	if (!serverOutputFile.isOpen())
	{
		WARNING(true, ("Failed to open area buildout save table '%s'.", serverFileName.c_str()));
		return;
	}

	StdioFile clientOutputFile(clientFileName.c_str(), "w");
	if (!clientOutputFile.isOpen())
	{
		WARNING(true, ("Failed to open area buildout save table '%s'.", clientFileName.c_str()));
		return;
	}

	// save the table headers
	{
		std::string const serverHeader(
			"server_template_crc\tcell_index\tpx\tpy\tpz\tqw\tqx\tqy\tqz\tscripts\tobjvars\n"
			"h\ti\tf\tf\tf\tf\tf\tf\tf\ts\tp\n");
		serverOutputFile.write(serverHeader.length(), serverHeader.c_str());

		std::string const clientHeader(
			"shared_template_crc\tcell_index\tpx\tpy\tpz\tqw\tqx\tqy\tqz\tradius\tportal_layout_crc\n"
			"h\ti\tf\tf\tf\tf\tf\tf\tf\tf\ti\n");
		clientOutputFile.write(clientHeader.length(), clientHeader.c_str());
	}

	// generate the rows
	std::vector<ServerBuildoutAreaRow> serverRows;
	std::vector<ClientBuildoutAreaRow> clientRows;
	generateBuildoutData(x1, z1, x2, z2, serverRows, clientRows);

	// save the server rows
	{
		for (std::vector<ServerBuildoutAreaRow>::const_iterator i = serverRows.begin(); i != serverRows.end(); ++i)
		{
			ConstCharCrcString const &serverTemplateName = ObjectTemplateList::lookUp((*i).serverTemplateCrc);
			char buf[512];
			IGNORE_RETURN(snprintf(buf, sizeof(buf) - 1, "%s\t%d\t%g\t%g\t%g\t%g\t%g\t%g\t%g\t",
				serverTemplateName.getString(),
				(*i).cellIndex,
				(*i).position.x, (*i).position.y, (*i).position.z,
				(*i).orientation.w, (*i).orientation.x, (*i).orientation.y, (*i).orientation.z));
			buf[sizeof(buf) - 1] = '\0';
			serverOutputFile.write(strlen(buf), buf);
			serverOutputFile.write((*i).scripts.length(), (*i).scripts.c_str());
			serverOutputFile.write(1, "\t");
			serverOutputFile.write((*i).objvars.length(), (*i).objvars.c_str());
			serverOutputFile.write(1, "\n");
		}
	}

	// save the client rows
	{
		for (std::vector<ClientBuildoutAreaRow>::const_iterator i = clientRows.begin(); i != clientRows.end(); ++i)
		{
			ConstCharCrcString const &sharedTemplateName = ObjectTemplateList::lookUp((*i).sharedTemplateCrc);
			char buf[512];
			IGNORE_RETURN(snprintf(buf, sizeof(buf) - 1, "%s\t%d\t%g\t%g\t%g\t%g\t%g\t%g\t%g\t%g\t%d\n",
				sharedTemplateName.getString(),
				(*i).cellIndex,
				(*i).position.x, (*i).position.y, (*i).position.z,
				(*i).orientation.w, (*i).orientation.x, (*i).orientation.y, (*i).orientation.z,
				(*i).radius,
				static_cast<int>((*i).portalLayoutCrc)));
			buf[sizeof(buf) - 1] = '\0';
			clientOutputFile.write(strlen(buf), buf);
		}
	}
}

// ----------------------------------------------------------------------

void ServerBuildoutManager::clientSaveArea(Client &client, std::string const &areaName, float x1, float z1, float x2, float z2)
{
	if (!ConfigServerGame::getBuildoutAreaEditingEnabled())
	{
		WARNING(true, ("Tried to save a buildout area to a client with buildout area editing disabled?"));
		return;
	}

	// generate the rows
	std::vector<ServerBuildoutAreaRow> serverRows;
	std::vector<ClientBuildoutAreaRow> clientRows;
	generateBuildoutData(x1, z1, x2, z2, serverRows, clientRows);

	// pass them to the client
	GenericValueTypeMessage<
		std::pair<
		std::pair<std::string, std::string>, // scene, area
		std::pair<std::vector<ServerBuildoutAreaRow>, std::vector<ClientBuildoutAreaRow> > // serverRows, clientRows
		>
	> const message(
		"GodClientSaveBuildoutArea",
		std::make_pair(
			std::make_pair(std::string(ConfigServerGame::getSceneID()), areaName),
			std::make_pair(serverRows, clientRows)));

	client.send(message, true);
}

// ----------------------------------------------------------------------

void ServerBuildoutManager::destroyPersistedDuplicates()
{
	if (!ConfigServerGame::getBuildoutAreaEditingEnabled())
	{
		WARNING(true, ("Tried to destroy persisted duplicates of buildout area objects with buildout area editing disabled?"));
		return;
	}

	for (std::vector<AreaInfo>::const_iterator i = s_areas.begin(); i != s_areas.end(); ++i)
	{
		for (std::vector<BuildoutRow>::const_iterator j = (*i).buildoutRows.begin(); j != (*i).buildoutRows.end(); ++j)
		{
			BuildoutRow const &buildoutRow = *j;

			// Find objects very close to the buildout object, and destroy if they match.
			std::vector<ServerObject *> objectsToTest;
			ServerObject *container = 0;

			if (buildoutRow.m_containerId == 0)
				ServerWorld::findObjectsInRange(buildoutRow.m_transform_p.getPosition_p(), 0.1f, objectsToTest);
			else
			{
				container = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(NetworkId(static_cast<NetworkId::NetworkIdType>(buildoutRow.m_containerId))));
				if (container && container->asCellObject())
				{
					Container * const containerProperty = container->getContainerProperty();
					if (containerProperty)
					{
						for (ContainerIterator c = containerProperty->begin(); c != containerProperty->end(); ++c)
							objectsToTest.push_back(safe_cast<ServerObject *>((*c).getObject()));
					}
				}
			}

			for (std::vector<ServerObject *>::const_iterator k = objectsToTest.begin(); k != objectsToTest.end(); ++k)
			{
				ServerObject * const so = *k;
				if (so
					&& so->getNetworkId().getValue() != buildoutRow.m_id
					&& so->isPersisted()
					&& so->getTemplateCrc() == buildoutRow.m_serverTemplate->getCrcName().getCrc()
					&& so->getPosition_p().magnitudeBetweenSquared(buildoutRow.m_transform_p.getPosition_p()) < 0.005f
					&& ContainerInterface::getContainedByObject(*so) == container)
					so->permanentlyDestroy(DeleteReasons::Replaced);
			}
		}
	}
}

// ----------------------------------------------------------------------

void ServerBuildoutManager::editBuildoutArea(std::string const &areaName)
{
	if (!ConfigServerGame::getBuildoutAreaEditingEnabled())
	{
		WARNING(true, ("Tried to edit a buildout area with buildout area editing disabled?"));
		return;
	}

	for (std::vector<AreaInfo>::iterator i = s_areas.begin(); i != s_areas.end(); ++i)
	{
		if (!(*i).editing && (*i).buildoutArea.areaName == areaName)
		{
			(*i).editing = true;
			// run through all objects belonging to the specified area and mark them as not client-cached
			for (std::vector<BuildoutRow>::const_iterator j = (*i).buildoutRows.begin(); j != (*i).buildoutRows.end(); ++j)
			{
				ServerObject * const obj = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(NetworkId(static_cast<NetworkId::NetworkIdType>((*j).m_id))));
				if (obj && obj->getCacheVersion() == s_buildoutCacheVersion)
					obj->setCacheVersion(0);
			}
			// tell the planetserver that we want this area to stay loaded
			GenericValueTypeMessage<std::pair<uint32, std::pair<std::pair<float, float>, std::pair<float, float> > > > const msg(
				"ForceLoadArea",
				std::make_pair(
					GameServer::getInstance().getProcessId(),
					std::make_pair(
						std::make_pair((*i).buildoutArea.rect.x0, (*i).buildoutArea.rect.y0),
						std::make_pair((*i).buildoutArea.rect.x1, (*i).buildoutArea.rect.y1))));
			GameServer::getInstance().sendToPlanetServer(msg);
		}
	}
}

// ----------------------------------------------------------------------

void ServerBuildoutManagerNamespace::remove()
{
	s_areas.clear();
	s_installed = false;
	s_eventObjects.clear();
}

// ----------------------------------------------------------------------

void ServerBuildoutManagerNamespace::generateBuildoutData(float x1, float z1, float x2, float z2, std::vector<ServerBuildoutAreaRow> &serverRows, std::vector<ClientBuildoutAreaRow> &clientRows)
{
	//	int objIdBase         = -1;

	typedef std::map<NetworkId::NetworkIdType, int> ObjIdMap;

	ObjIdMap objIdMap;

	std::vector<ServerObject const *> objectsToSave;

	// build vector of objects we need to save, in the appropriate order
	{
		std::vector<ServerObject const *> topmostObjects;
		int const objectCount = ServerWorld::getNumObjects();
		for (int i = 0; i < objectCount; ++i)
		{
			ServerObject const * const obj = ServerWorld::getObject(i);
			if (obj->isInWorld() && !ContainerInterface::getContainedByObject(*obj))
			{
				Vector const &pos = obj->getPosition_p();
				if (pos.x >= x1 && pos.x < x2 && pos.z >= z1 && pos.z < z2)
					topmostObjects.push_back(obj);
			}
		}
		buildObjectsToSave(objectsToSave, topmostObjects);
	}

	// run through the objects to save, generating their rows
	{
		for (std::vector<ServerObject const *>::const_iterator i = objectsToSave.begin(); i != objectsToSave.end(); ++i)
		{
			ServerObject const * const obj = *i;

			// check for unexported gold data
			if (obj->getIncludeInBuildout() == false && obj->getNetworkId().getValue() < 10000000)
			{
				continue;
			}

			// save template
			ServerObjectTemplate const * const serverTemplate = safe_cast<ServerObjectTemplate const *>(obj->getObjectTemplate());

			bool serverOnly = false;
			if (!obj->getPortalProperty() && dynamic_cast<ServerTangibleObjectTemplate const *>(serverTemplate))
			{
				// Tangibles are server only if they do not have the VF_player visible flag
				serverOnly = true;
				for (int vfIndex = 0; vfIndex < static_cast<int>(serverTemplate->getVisibleFlagsCount()); ++vfIndex)
					if (serverTemplate->getVisibleFlags(vfIndex) == ServerObjectTemplate::VF_player)
						serverOnly = false;
			}

			Quaternion const q(obj->getTransform_o2p());
			Vector p(obj->getPosition_p());
			int cellIndex = 0;

			const ServerObject * const containingObject = safe_cast<ServerObject const *>(ContainerInterface::getContainedByObject(*obj));

			if (containingObject)
			{
				CellObject const *objAsCell = obj->asCellObject();
				if (!objAsCell)
					objAsCell = containingObject->asCellObject();
				FATAL(!objAsCell, ("Unable to determine cell index to save for buildout object %s", obj->getDebugInformation().c_str()));
				cellIndex = objAsCell->getCell();
			}
			else
			{
				p.x -= x1;
				p.z -= z1;
			}

			std::string packedScriptList;
			ScriptList const &scriptList = obj->getScriptObject()->getScripts();
			for (ScriptList::const_iterator j = scriptList.begin(); j != scriptList.end(); ++j)
			{
				if (!packedScriptList.empty())
					packedScriptList += ':';
				packedScriptList += (*j).getScriptName();
			}

			serverRows.push_back(ServerBuildoutAreaRow());
			ServerBuildoutAreaRow &serverRow = serverRows.back();

			serverRow.id = static_cast<int>(obj->getNetworkId().getValue());
			serverRow.container = containingObject ? static_cast<int>(containingObject->getNetworkId().getValue()) : 0;
			serverRow.serverTemplateCrc = obj->getTemplateCrc();
			serverRow.cellIndex = cellIndex;
			serverRow.position = p;
			serverRow.orientation = q;
			serverRow.scripts = packedScriptList;
			serverRow.objvars = obj->getPackedObjVars(std::string());

			if (serverRow.id >= 10000000)
			{
				serverRow.id = Random::random() | 0x80000000; // generate a negative random number.
				objIdMap[obj->getNetworkId().getValue()] = serverRow.id;
			}
			else
			{
				serverRow.id &= cs_dataBitStripMask; // strip data bits
			}

			if (serverRow.container >= 10000000)
			{
				ObjIdMap::const_iterator it = objIdMap.find(serverRow.container);
				FATAL(it == objIdMap.end(), ("Unable to find the container relative objid for object %d\n", serverRow.id));
				serverRow.container = it->second;
			}
			else
			{
				serverRow.id &= cs_dataBitStripMask; // strip data bits
			}

			if (!serverOnly)
			{
				clientRows.push_back(ClientBuildoutAreaRow());
				ClientBuildoutAreaRow &clientRow = clientRows.back();

				clientRow.id = serverRow.id;
				clientRow.container = serverRow.container;
				clientRow.type = serverTemplate ? serverTemplate->getId() : 0;
				clientRow.sharedTemplateCrc = ConstCharCrcString(obj->getSharedTemplateName()).getCrc();
				clientRow.cellIndex = cellIndex;
				clientRow.position = p;
				clientRow.orientation = q;
				clientRow.radius = serverTemplate->getUpdateRanges(ServerObjectTemplate::UR_far);
				int portalLayoutCrc = 0;
				obj->getObjVars().getItem("portalProperty.crc", portalLayoutCrc);
				clientRow.portalLayoutCrc = static_cast<uint32>(portalLayoutCrc);
			}
		}
	}
}

// ----------------------------------------------------------------------

void ServerBuildoutManagerNamespace::buildObjectsToSave(std::vector<ServerObject const *> &objectsToSave, std::vector<ServerObject const *> &potentialObjects)
{
	// remove any non-buildout objects from the potential object vector
	potentialObjects.erase(std::remove_if(potentialObjects.begin(), potentialObjects.end(), isNotObjectForBuildout), potentialObjects.end());

	// sort by networkId for non-cells, cell index for cells
	std::sort(potentialObjects.begin(), potentialObjects.end(), buildoutObjectLessComparator);

	// push object on the stack and recurse for contents
	for (std::vector<ServerObject const *>::const_iterator i = potentialObjects.begin(); i != potentialObjects.end(); ++i)
	{
		ServerObject const * const obj = *i;
		objectsToSave.push_back(obj);

		Container const * const container = ContainerInterface::getContainer(*obj);

		if (container)
		{
			std::vector<ServerObject const *> contents;

			for (ContainerConstIterator j = container->begin(); j != container->end(); ++j)
			{
				contents.push_back(safe_cast<ServerObject const *>((*j).getObject()));
			}

			buildObjectsToSave(objectsToSave, contents);
		}
	}
}

// ----------------------------------------------------------------------

bool ServerBuildoutManagerNamespace::isNotObjectForBuildout(ServerObject const *obj)
{
	if (obj)
	{
		const ServerObject * const containingObject = safe_cast<const ServerObject  *>(ContainerInterface::getContainedByObject(*obj));

		// make sure that cells that should be included in the buildout have all their
		// children included too!
		if (obj->getObjectTemplate()->getId() == ServerCellObjectTemplate::ServerCellObjectTemplate_tag && containingObject && containingObject->getIncludeInBuildout())
		{
			ServerObject *tmpObj = const_cast<ServerObject *>(obj);
			tmpObj->setIncludeInBuildout(true);
		}

		if (!obj->isInWorld() || (!obj->isPersisted() && !obj->getIncludeInBuildout()) || obj->isPlayerControlled())
		{
			return true;
		}
	}
	return false;
}

// ----------------------------------------------------------------------

void ServerBuildoutManagerNamespace::loadArea(AreaInfo &areaInfo)
{
	int64 buildingObjId = areaInfo.buildoutArea.getSharedBaseId();
	int64 objIdBase = buildingObjId + cs_buildingObjIdOffset;
	int64 currentBuilding = 0;
	int64 currentCell = 0;

	int serverIdBase = areaInfo.buildoutArea.getServerBaseId();

	const bool isBuildoutEditingEnabled = ConfigServerGame::getBuildoutAreaEditingEnabled();
	UNREF(isBuildoutEditingEnabled);

	char filename[256];
	snprintf(filename, sizeof(filename) - 1, "datatables/buildout/%s/%s.iff", ConfigServerGame::getSceneID(), areaInfo.buildoutArea.areaName.c_str());
	filename[sizeof(filename) - 1] = '\0';

	std::string const & eventRequired = areaInfo.buildoutArea.getRequiredEventName();
	std::string const & requiredLoadLevel = areaInfo.buildoutArea.getRequiredLoadLevel();

	if (!eventRequired.empty())
	{
		ServerEventMap::iterator iter = s_eventObjects.find(eventRequired);
		if (iter != s_eventObjects.end())
		{
			// Nothing to do here. We've already setup a list for this event.
			//eventObjList = &(*iter).second;
		}
		else
		{
			s_eventObjects.insert(std::pair<std::string, std::list<ServerEventAreaInfo> >(eventRequired, std::list<ServerEventAreaInfo>()));
		}
	}

	Iff iff;
	if (!areaInfo.buildoutArea.areaName.empty() && iff.open(filename, true) && !iff.atEndOfForm())
	{
		DataTable areaBuildoutTable;
		areaBuildoutTable.load(iff);

		int const buildoutRowCount = areaBuildoutTable.getNumRows();
		if (buildoutRowCount > 0)
		{
			areaInfo.buildoutRows.reserve(buildoutRowCount);

			int const objIdColumn = areaBuildoutTable.findColumnNumber("objid");
			int const containerColumn = areaBuildoutTable.findColumnNumber("container");
			int const serverTemplateCrcColumn = areaBuildoutTable.findColumnNumber("server_template_crc");
			int const cellIndexColumn = areaBuildoutTable.findColumnNumber("cell_index");
			int const pxColumn = areaBuildoutTable.findColumnNumber("px");
			int const pyColumn = areaBuildoutTable.findColumnNumber("py");
			int const pzColumn = areaBuildoutTable.findColumnNumber("pz");
			int const qwColumn = areaBuildoutTable.findColumnNumber("qw");
			int const qxColumn = areaBuildoutTable.findColumnNumber("qx");
			int const qyColumn = areaBuildoutTable.findColumnNumber("qy");
			int const qzColumn = areaBuildoutTable.findColumnNumber("qz");
			int const scriptsColumn = areaBuildoutTable.findColumnNumber("scripts");
			int const objvarsColumn = areaBuildoutTable.findColumnNumber("objvars");

			FATAL(serverTemplateCrcColumn < 0, ("Unable to find serverTemplateCrcColumn in [%s]", filename));
			FATAL(cellIndexColumn < 0, ("Unable to find cellIndexColumn in [%s]", filename));
			FATAL(pxColumn < 0, ("Unable to find pxColumn in [%s]", filename));
			FATAL(pyColumn < 0, ("Unable to find pyColumn in [%s]", filename));
			FATAL(pzColumn < 0, ("Unable to find pzColumn in [%s]", filename));
			FATAL(qwColumn < 0, ("Unable to find qwColumn in [%s]", filename));
			FATAL(qxColumn < 0, ("Unable to find qxColumn in [%s]", filename));
			FATAL(qyColumn < 0, ("Unable to find qyColumn in [%s]", filename));
			FATAL(qzColumn < 0, ("Unable to find qzColumn in [%s]", filename));
			FATAL(scriptsColumn < 0, ("Unable to find scriptsColumn in [%s]", filename));
			FATAL(objvarsColumn < 0, ("Unable to find objvarsColumn in [%s]", filename));

			int buildOutFileVersion = 1;

			if (objIdColumn != -1)
			{
				buildOutFileVersion = 2;
			}

			std::set< int64 > objects;

			int64 originalObjId = 0;
			for (int buildoutRow = 0; buildoutRow < buildoutRowCount; ++buildoutRow)
			{
				int64 objId = 0;
				int64 containerId = 0;

				uint32 const serverTemplateCrc = static_cast<uint32>(areaBuildoutTable.getIntValue(serverTemplateCrcColumn, buildoutRow));
				unsigned int const cellIndex = areaBuildoutTable.getIntValue(cellIndexColumn, buildoutRow);

				ObjectTemplate const * const serverTemplateBase = ObjectTemplateList::fetch(serverTemplateCrc);
				FATAL(!serverTemplateBase, ("Nonexistant server template 0x%08x in buildout table %s, row %d (line %d) - rebuild your templates and reimport them into the database!", serverTemplateCrc, areaInfo.buildoutArea.areaName.c_str(), buildoutRow, buildoutRow + 3));

				ServerObjectTemplate const * const serverTemplate = serverTemplateBase->asServerObjectTemplate();
				FATAL(!serverTemplate, ("Bad server template 0x%08x [%s] in buildout table %s, row %d (line %d)", serverTemplateCrc, serverTemplateBase->getName(), areaInfo.buildoutArea.areaName.c_str(), buildoutRow, buildoutRow + 3));

				std::string const &sharedTemplateName = serverTemplate->getSharedTemplate();
				ObjectTemplate const * const sharedTemplateBase = ObjectTemplateList::fetch(sharedTemplateName);
				FATAL(!sharedTemplateBase, ("Nonexistant shared template [%s] for [%s] in buildout table %s, row %d (line %d)", sharedTemplateName.c_str(), serverTemplateBase->getName(), areaInfo.buildoutArea.areaName.c_str(), buildoutRow, buildoutRow + 3));

				SharedObjectTemplate const * const sharedTemplate = sharedTemplateBase->asSharedObjectTemplate();
				FATAL(!sharedTemplate, ("Bad shared template [%s] for [%s] in buildout table %s, row %d (line %d)", sharedTemplateName.c_str(), serverTemplateBase->getName(), areaInfo.buildoutArea.areaName.c_str(), buildoutRow, buildoutRow + 3));
				bool const isPob = !sharedTemplate->getPortalLayoutFilename().empty();
				sharedTemplateBase->releaseReference();

				bool serverOnly = false;
				if (!isPob && dynamic_cast<ServerTangibleObjectTemplate const *>(serverTemplate))
				{
					// Tangibles are server only if they do not have the VF_player visible flag
					serverOnly = true;
					for (int vfIndex = 0; vfIndex < static_cast<int>(serverTemplate->getVisibleFlagsCount()); ++vfIndex)
						if (serverTemplate->getVisibleFlags(vfIndex) == ServerObjectTemplate::VF_player)
							serverOnly = false;
				}

				if (buildOutFileVersion == 1)
				{
					if (isPob) // if ( is a building )
					{
						objId = buildingObjId++;
						currentBuilding = objId;
					}
					else if (serverTemplate->getId() == ServerCellObjectTemplate::ServerCellObjectTemplate_tag) // if ( is a cell )
					{
						objId = buildingObjId++;
						currentCell = objId;
						containerId = currentBuilding;
					}
					else if (cellIndex > 0) // if ( is an object in a cell )
					{
						objId = objIdBase++;
						containerId = currentCell;
					}
					else if (serverOnly)
					{
						objId = serverIdBase++;
					}
					else // is an object not in a cell
					{
						objId = objIdBase++;
					}

					FATAL(buildingObjId >= areaInfo.buildoutArea.getSharedBaseId() + cs_buildingObjIdOffset, ("building object id overflow"));
				}
				else
				{
					originalObjId = areaBuildoutTable.getIntValue(objIdColumn, buildoutRow);
					objId = originalObjId;
					containerId = areaBuildoutTable.getIntValue(containerColumn, buildoutRow);

					// with new buildout files, the object id is a random 31-bit negative value
					// then we give the area index some bits in the upper part of the number
					// by shifting the area index value left 48 bits.

					const int64 areaIndex = areaInfo.buildoutArea.areaIndex + 1;

					if (objId < 0)
					{
						objId ^= areaIndex << 48;
					}

					if (containerId < 0)
					{
						containerId ^= areaIndex << 48;
					}
				}

				FATAL(isPob && (cellIndex != 0 || containerId != 0), ("tried to add a pob to a cell or other container. %s (objId=%d cellIndex=%d containerId=%d)",
					serverTemplateBase->getName(), objId, cellIndex, containerId));

				Quaternion const q(
					areaBuildoutTable.getFloatValue(qwColumn, buildoutRow),
					areaBuildoutTable.getFloatValue(qxColumn, buildoutRow),
					areaBuildoutTable.getFloatValue(qyColumn, buildoutRow),
					areaBuildoutTable.getFloatValue(qzColumn, buildoutRow));

				Transform transform_p;

				q.getTransform(&transform_p);

				if (cellIndex == 0)
				{
					transform_p.setPosition_p(
						areaBuildoutTable.getFloatValue(pxColumn, buildoutRow) + areaInfo.buildoutArea.rect.x0,
						areaBuildoutTable.getFloatValue(pyColumn, buildoutRow),
						areaBuildoutTable.getFloatValue(pzColumn, buildoutRow) + areaInfo.buildoutArea.rect.y0);
				}
				else
				{
					transform_p.setPosition_p(
						areaBuildoutTable.getFloatValue(pxColumn, buildoutRow),
						areaBuildoutTable.getFloatValue(pyColumn, buildoutRow),
						areaBuildoutTable.getFloatValue(pzColumn, buildoutRow));
				}

				if (containerId == 0 || objects.find(containerId) != objects.end())
				{
					// store original ObjId from buildout table as ObjVar on item for reference in-game if needed
					std::string objVars = areaBuildoutTable.getStringValue(objvarsColumn, buildoutRow);
					objVars.insert(0, "buildoutObjectId|0|" + std::to_string(originalObjId) + "|");

					areaInfo.buildoutRows.push_back(
						BuildoutRow(
							objId,
							containerId,
							cellIndex,
							transform_p,
							serverTemplate,
							areaBuildoutTable.getStringValue(scriptsColumn, buildoutRow),
							objVars,
							eventRequired,
							requiredLoadLevel));
				}

				objects.insert(objId);

				serverTemplateBase->releaseReference();
			}
		}
	}

	areaInfo.loaded = true;
}

// ----------------------------------------------------------------------

bool ServerBuildoutManagerNamespace::buildoutObjectLessComparator(ServerObject const *lhs, ServerObject const *rhs)
{
	CellObject const * const lhsAsCell = lhs->asCellObject();
	CellObject const * const rhsAsCell = rhs->asCellObject();

	if (lhsAsCell && rhsAsCell)
		return lhsAsCell->getCell() < rhsAsCell->getCell();

	return lhs->getNetworkId() < rhs->getNetworkId();
}

// ----------------------------------------------------------------------

void ServerBuildoutManagerNamespace::instantiateAreaNode(AreaInfo const &areaInfo, int nodeX, int nodeZ)
{
	UNREF(nodeZ);
	UNREF(nodeX);

	std::vector<ServerObject *> newObjects;
	int buildingId = 0;
	int cellIndex = 0;
	UNREF(cellIndex);

	UNREF(buildingId);

	NetworkId controllerObjectId;

	bool discardContainedObjects = false;

	PlanetObject * const planetObject = ServerUniverse::getInstance().getTatooinePlanet();
	std::vector<std::string> events;

	if (planetObject)
		planetObject->parseCurrentEventsList(events);

	for (std::vector<BuildoutRow>::const_iterator rowIter = areaInfo.buildoutRows.begin(); rowIter != areaInfo.buildoutRows.end(); ++rowIter)
	{
		BuildoutRow const &buildoutRow = *rowIter;

		NetworkId const objectId(static_cast<NetworkId::NetworkIdType>(buildoutRow.m_id));

		int64 const containerId = buildoutRow.m_containerId;

		if (containerId && discardContainedObjects)
		{
			continue;
		}

		discardContainedObjects = false;

		if (containerId == 0)
		{
			//-- check to see if this is the controller object
			//-- if it is, it gets created regardless of its position relative to the node
			if (!controllerObjectId.isValid())
			{
				if (!buildoutRow.m_objvars.empty())
				{
					if (buildoutRow.m_objvars.find("isControllerObject|") != std::string::npos)
					{
						controllerObjectId = objectId;
					}
				}
			}
		}

		// reject object if it is out of bounds and is not the controller
		if (!containerId && objectId != controllerObjectId)
		{
			const Vector &position_w = buildoutRow.m_transform_p.getPosition_p();

			if (position_w.x < nodeX || position_w.x >= nodeX + 100 || position_w.z < nodeZ || position_w.z >= nodeZ + 100)
			{
				discardContainedObjects = true;
				continue;
			}
		}

		if (!buildoutRow.m_eventRequired.empty())
		{
			ServerEventMap::iterator searchIter = s_eventObjects.find(buildoutRow.m_eventRequired);
			if (searchIter != s_eventObjects.end())
			{
				ServerEventAreaInfo newEventObj(&buildoutRow, nullptr);
				(*searchIter).second.push_back(newEventObj);
			}
			else
			{
				DEBUG_REPORT_LOG(true, ("ServerBuildoutManager: Tried to load an unknown event object. Event name: %s\n", buildoutRow.m_eventRequired.c_str()));
			}

			if (planetObject) // We have a valid Tatooine Planet Object which keeps track of our universe wide events.
			{
				bool eventCurrentlyRunning = false;

				std::vector<std::string>::size_type i = 0;
				for (; i < events.size(); ++i)
				{
					if (events[i] == buildoutRow.m_eventRequired) // Is our required event already running?
						eventCurrentlyRunning = true;
				}

				if (!eventCurrentlyRunning)
					continue; // We found an event object but their event isn't started yet. Hold off on Object creation.
			}
			else
				continue; // No planet object. This should NEVER happen. Better hold off on any event object creation for now.
		}

		ServerObject * const containingObject = containerId ? safe_cast<ServerObject *>(NetworkIdManager::getObjectById(NetworkId(static_cast<NetworkId::NetworkIdType>(containerId)))) : 0;
		FATAL(containerId && !containingObject, ("could not find containing object obj=%d container=%d", (int)objectId.getValue(), containerId));

		//-- object has already been created
		//-- this must be the controller object or one of its contents.
		if (NetworkIdManager::getObjectById(objectId))
			continue;

		ServerObject * const newObject = safe_cast<ServerObject *>(buildoutRow.m_serverTemplate->createObject());
		FATAL(!newObject, ("could not create buildout object"));

		newObject->setNetworkId(objectId);
		newObject->setIncludeInBuildout(true);

		CellObject * const newCellObject = newObject->asCellObject();

		if (newCellObject)
		{
			newCellObject->setCell(buildoutRow.m_cellIndex);
		}

		if (!newObject->getController())
		{
			newObject->createDefaultController();
		}

		newObject->setAuthority();

		if (newObject->asTangibleObject() != nullptr)
			newObject->asTangibleObject()->initializeVisibility();

		//
		// if we're NOT building editing we want to set some of the objects
		// to be client cached.  If we ARE building editing we do not want
		// to client-cache any objects otherwise we will not be able to
		// move them around or delete them, etc.
		//

		const bool isEditing = ConfigServerGame::getBuildoutAreaEditingEnabled();

		if (isEditing == false)
		{
			//
			// cells and their parents should NOT BE CLIENT CACHED
			//
			if (newObject->asCellObject())
			{
				// by the time we get here the cache version on the POB has
				// already been set to non-zero, so we need to reset it back
				// to zero
				containingObject->setCacheVersion(0);
			}
			else
			{
				//
				// ok so we're here because this object is not a cell.
				// what needs to be done now
				// is make sure that objects that are player visible
				// are client cached.
				//
				// the reason for this is that there are server only objects,
				// like SPAWNERS and WAYPOINTS. These objects are NOT
				// in the client buildout files, so the MUST NOT be client cached.
				// If you make them client cached, you will not be able to see
				// them in god mode!!!
				//
				const int vfCount = buildoutRow.m_serverTemplate->getVisibleFlagsCount();

				for (int index = 0; index < vfCount; ++index)
				{
					const ServerObjectTemplate::VisibleFlags vf = buildoutRow.m_serverTemplate->getVisibleFlags(index);

					if (vf == ServerObjectTemplate::VF_player && buildoutRow.m_eventRequired.empty()) // Event objects can never be client cached currently.
					{
						newObject->setCacheVersion(s_buildoutCacheVersion);
						break;
					}
				}
			}
		}

		if (buildoutRow.m_transform_p != Transform::identity)
		{
			newObject->setTransform_o2p(buildoutRow.m_transform_p);
		}

		newObject->getContainedByProperty()->setContainedBy(NetworkId(static_cast<NetworkId::NetworkIdType>(containerId)));

		DynamicVariableList objVars;

		buildoutRow.m_serverTemplate->getObjvars(objVars);

		for (DynamicVariableList::MapType::const_iterator i = objVars.begin(); i != objVars.end(); ++i)
		{
			newObject->setObjVarItem(i->first, i->second);
		}

		if (!buildoutRow.m_objvars.empty())
		{
			newObject->setPackedObjVars(buildoutRow.m_objvars);
		}

		int const count = buildoutRow.m_serverTemplate->getScriptsCount();

		for (int j = 0; j < count; ++j)
		{
			newObject->getScriptObject()->attachScript(buildoutRow.m_serverTemplate->getScripts(j), false);
		}

		if (!buildoutRow.m_scripts.empty())
		{
			GameScriptObject * const gso = newObject->getScriptObject();

			if (nullptr != gso)
			{
				std::string const &scripts = buildoutRow.m_scripts;
				unsigned int pos = 0;
				while (pos < scripts.size())
				{
					unsigned int const oldPos = pos;
					while (scripts[pos] && scripts[pos] != ':')
						++pos;

					std::string const & scriptName = scripts.substr(oldPos, pos - oldPos);
					if (!gso->hasScript(scriptName))
						IGNORE_RETURN(gso->attachScript(scriptName, false));

					if (scripts[pos] == ':')
						++pos;
				}
			}
		}
		newObjects.push_back(newObject);

		if (!buildoutRow.m_eventRequired.empty())
		{
			ServerEventMap::iterator searchIter = s_eventObjects.find(buildoutRow.m_eventRequired);
			if (searchIter != s_eventObjects.end())
			{
				ServerEventAreaInfo & newEventObj = (*searchIter).second.back();
				newEventObj.loadedObject = newObject;
			}
		}
	}

	// run through newly instantiated objects and initialize them
	{
		for (std::vector<ServerObject *>::reverse_iterator it = newObjects.rbegin(); it != newObjects.rend(); ++it)
		{
			(*it)->serverObjectEndBaselines(false);
		}
	}

	// run through newly instantiated objects and notify the controller object
	if (controllerObjectId.isValid())
	{
		ServerObject * const controllerObject = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(controllerObjectId));
		if (nullptr != controllerObject)
		{
			GameScriptObject * const script = controllerObject->getScriptObject();

			for (std::vector<ServerObject *>::reverse_iterator it = newObjects.rbegin(); it != newObjects.rend(); ++it)
			{
				ServerObject * const obj = *it;
				if (obj == controllerObject)
					continue;

				int registerWithController = 0;
				if (obj->getObjVars().getItem("registerWithController", registerWithController))
				{
					if (registerWithController)
					{
						if (nullptr != script)
						{
							ScriptParams p;
							p.addParam(obj->getNetworkId());
							IGNORE_RETURN(script->trigAllScripts(Scripting::TRIG_BUILDOUT_OBJECT_REGISTER_WITH_CONTROLLER, p));
						}
					}
				}
			}
		}
	}

	// initialize any theaters
	{
		for (std::vector<ServerObject *>::reverse_iterator it = newObjects.rbegin(); it != newObjects.rend(); ++it)
		{
			ServerObject *obj = *it;

			if (obj  && obj->isAuthoritative())
			{
				GameScriptObject *scripts = obj->getScriptObject();

				if (scripts) //&& scripts->hasScript( "poi.template.scene.base.base_theater" ) )
				{
					scripts->handleMessage("startTheaterFromBuildout", std::vector<int8>());
				}
			}
		}
	}
}

// ----------------------------------------------------------------------

ServerBuildoutManagerNamespace::BuildoutRow::BuildoutRow() :
	m_id(0),
	m_containerId(0),
	m_transform_p(),
	m_serverTemplate(0),
	m_scripts(),
	m_objvars(),
	m_cellIndex(0)
{
}

// ----------------------------------------------------------------------

ServerBuildoutManagerNamespace::BuildoutRow::BuildoutRow(int64 id, int64 containerId, int cellIndex, Transform const &transform_p, ServerObjectTemplate const *serverTemplate, std::string const &scripts, std::string const &objvars, std::string const & eventRequired, std::string const & requiredLoadLevel) :
	m_id(id),
	m_containerId(containerId),
	m_transform_p(transform_p),
	m_serverTemplate(serverTemplate),
	m_scripts(scripts),
	m_objvars(objvars),
	m_cellIndex(cellIndex),
	m_eventRequired(eventRequired),
	m_requiredLoadLevel(requiredLoadLevel)
{
	if (serverTemplate)
		serverTemplate->addReference();
}

// ----------------------------------------------------------------------

ServerBuildoutManagerNamespace::BuildoutRow::BuildoutRow(BuildoutRow const &rhs) :
	m_id(rhs.m_id),
	m_containerId(rhs.m_containerId),
	m_transform_p(rhs.m_transform_p),
	m_serverTemplate(rhs.m_serverTemplate),
	m_scripts(rhs.m_scripts),
	m_objvars(rhs.m_objvars),
	m_cellIndex(rhs.m_cellIndex),
	m_eventRequired(rhs.m_eventRequired),
	m_requiredLoadLevel(rhs.m_requiredLoadLevel)
{
	if (m_serverTemplate)
		m_serverTemplate->addReference();
}

// ----------------------------------------------------------------------

ServerBuildoutManagerNamespace::BuildoutRow::~BuildoutRow()
{
	if (m_serverTemplate)
		m_serverTemplate->releaseReference();
}

// ----------------------------------------------------------------------

ServerBuildoutManagerNamespace::BuildoutRow &ServerBuildoutManagerNamespace::BuildoutRow::operator=(BuildoutRow const &rhs)
{
	if (&rhs != this)
	{
		m_id = rhs.m_id;
		m_containerId = rhs.m_containerId;
		m_transform_p = rhs.m_transform_p;
		m_scripts = rhs.m_scripts;
		m_objvars = rhs.m_objvars;
		m_cellIndex = rhs.m_cellIndex;
		m_eventRequired = rhs.m_eventRequired;
		m_requiredLoadLevel = rhs.m_requiredLoadLevel;

		if (m_serverTemplate)
			m_serverTemplate->releaseReference();
		m_serverTemplate = rhs.m_serverTemplate;
		if (m_serverTemplate)
			m_serverTemplate->addReference();
	}
	return *this;
}

// ----------------------------------------------------------------------

void ServerBuildoutManager::onEventStarted(std::string const & eventName)
{
	if (s_eventObjects.empty())
		return;

	ServerEventMap::iterator iter = s_eventObjects.find(eventName);

	if (iter == s_eventObjects.end())
		return; // Perhaps we should print a warning here? Would we ever have an event with no server side event objects?

	std::list< ServerEventAreaInfo >* eventList = &(*iter).second;

	std::vector<ServerObject *> newObjects;
	NetworkId controllerObjectId;

	std::list< ServerEventAreaInfo >::iterator objIter = eventList->begin();
	for (; objIter != eventList->end(); ++objIter)
	{
		// Object Creation

		//BEGIN OBJECT CREATION CODE FROM ABOVE!
		BuildoutRow const &buildoutRow = *(*objIter).buildOut;

		NetworkId const objectId(static_cast<NetworkId::NetworkIdType>(buildoutRow.m_id));

		int64 const containerId = buildoutRow.m_containerId;

		if (containerId == 0)
		{
			//-- check to see if this is the controller object
			//-- if it is, it gets created regardless of its position relative to the node
			if (!controllerObjectId.isValid())
			{
				if (!buildoutRow.m_objvars.empty())
				{
					if (buildoutRow.m_objvars.find("isControllerObject|") != std::string::npos)
					{
						controllerObjectId = objectId;
					}
				}
			}
		}

		ServerObject * const containingObject = containerId ? safe_cast<ServerObject *>(NetworkIdManager::getObjectById(NetworkId(static_cast<NetworkId::NetworkIdType>(containerId)))) : 0;
		FATAL(containerId && !containingObject, ("could not find containing object obj=%d container=%d", (int)objectId.getValue(), containerId));

		//-- object has already been created
		//-- this must be the controller object or one of its contents.
		if (NetworkIdManager::getObjectById(objectId))
			continue;

		ServerObject * const newObject = safe_cast<ServerObject *>(buildoutRow.m_serverTemplate->createObject());
		FATAL(!newObject, ("could not create buildout object"));

		LOGC(ConfigServerGame::getLogEventObjectCreation(), "EventBuildout", ("Loading Object for Event [%s].\nTemplate Name[%s]\nNetwork ID[%s]\nCell[%d]\nPosition[%-4.3f %-4.3f %-4.3f]\n\n",
			eventName.c_str(), buildoutRow.m_serverTemplate->getName(), objectId.getValueString().c_str(), buildoutRow.m_cellIndex,
			buildoutRow.m_transform_p.getPosition_p().x, buildoutRow.m_transform_p.getPosition_p().y, buildoutRow.m_transform_p.getPosition_p().z));

		newObject->setNetworkId(objectId);
		newObject->setIncludeInBuildout(true);

		CellObject * const newCellObject = newObject->asCellObject();

		if (newCellObject)
		{
			newCellObject->setCell(buildoutRow.m_cellIndex);
		}

		if (!newObject->getController())
		{
			newObject->createDefaultController();
		}

		newObject->setAuthority();

		if (newObject->asTangibleObject() != nullptr)
			newObject->asTangibleObject()->initializeVisibility();

		//
		// if we're NOT building editing we want to set some of the objects
		// to be client cached.  If we ARE building editing we do not want
		// to client-cache any objects otherwise we will not be able to
		// move them around or delete them, etc.
		//

		const bool isEditing = ConfigServerGame::getBuildoutAreaEditingEnabled();

		if (isEditing == false)
		{
			//
			// cells and their parents should NOT BE CLIENT CACHED
			//
			if (newObject->asCellObject())
			{
				// by the time we get here the cache version on the POB has
				// already been set to non-zero, so we need to reset it back
				// to zero
				containingObject->setCacheVersion(0);
			}
			else
			{
				//
				// ok so we're here because this object is not a cell.
				// what needs to be done now
				// is make sure that objects that are player visible
				// are client cached.
				//
				// the reason for this is that there are server only objects,
				// like SPAWNERS and WAYPOINTS. These objects are NOT
				// in the client buildout files, so the MUST NOT be client cached.
				// If you make them client cached, you will not be able to see
				// them in god mode!!!
				//
				const int vfCount = buildoutRow.m_serverTemplate->getVisibleFlagsCount();

				for (int index = 0; index < vfCount; ++index)
				{
					const ServerObjectTemplate::VisibleFlags vf = buildoutRow.m_serverTemplate->getVisibleFlags(index);

					if (vf == ServerObjectTemplate::VF_player)
					{
						// Removing this code temporarily since we don't have a proper way to do client side, cached event build out objects.
						// All event objects are objects that are streamed down from the server. Until we have a way for the client to know
						// what events are going on so it can load its' own build out datatables; this value needs to stay at its' default value (0).

						//newObject->setCacheVersion(s_buildoutCacheVersion);
						break;
					}
				}
			}
		}

		if (buildoutRow.m_transform_p != Transform::identity)
		{
			newObject->setTransform_o2p(buildoutRow.m_transform_p);
		}

		newObject->getContainedByProperty()->setContainedBy(NetworkId(static_cast<NetworkId::NetworkIdType>(containerId)));

		DynamicVariableList objVars;

		buildoutRow.m_serverTemplate->getObjvars(objVars);

		for (DynamicVariableList::MapType::const_iterator i = objVars.begin(); i != objVars.end(); ++i)
		{
			newObject->setObjVarItem(i->first, i->second);
		}

		if (!buildoutRow.m_objvars.empty())
		{
			newObject->setPackedObjVars(buildoutRow.m_objvars);
		}

		int const count = buildoutRow.m_serverTemplate->getScriptsCount();

		for (int j = 0; j < count; ++j)
		{
			newObject->getScriptObject()->attachScript(buildoutRow.m_serverTemplate->getScripts(j), false);
		}

		if (!buildoutRow.m_scripts.empty())
		{
			GameScriptObject * const gso = newObject->getScriptObject();

			if (nullptr != gso)
			{
				std::string const &scripts = buildoutRow.m_scripts;
				unsigned int pos = 0;
				while (pos < scripts.size())
				{
					unsigned int const oldPos = pos;
					while (scripts[pos] && scripts[pos] != ':')
						++pos;

					std::string const & scriptName = scripts.substr(oldPos, pos - oldPos);
					if (!gso->hasScript(scriptName))
						IGNORE_RETURN(gso->attachScript(scriptName, false));

					if (scripts[pos] == ':')
						++pos;
				}
			}
		}

		(*objIter).loadedObject = newObject;

		newObjects.push_back(newObject);
	}

	// run through newly instantiated objects and initialize them
	{
		for (std::vector<ServerObject *>::reverse_iterator it = newObjects.rbegin(); it != newObjects.rend(); ++it)
		{
			(*it)->serverObjectEndBaselines(false);
		}
	}

	// run through newly instantiated objects and notify the controller object
	if (controllerObjectId.isValid())
	{
		ServerObject * const controllerObject = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(controllerObjectId));
		if (nullptr != controllerObject)
		{
			GameScriptObject * const script = controllerObject->getScriptObject();

			for (std::vector<ServerObject *>::reverse_iterator it = newObjects.rbegin(); it != newObjects.rend(); ++it)
			{
				ServerObject * const obj = *it;
				if (obj == controllerObject)
					continue;

				int registerWithController = 0;
				if (obj->getObjVars().getItem("registerWithController", registerWithController))
				{
					if (registerWithController)
					{
						if (nullptr != script)
						{
							ScriptParams p;
							p.addParam(obj->getNetworkId());
							IGNORE_RETURN(script->trigAllScripts(Scripting::TRIG_BUILDOUT_OBJECT_REGISTER_WITH_CONTROLLER, p));
						}
					}
				}
			}
		}
	}

	// initialize any theaters
	{
		for (std::vector<ServerObject *>::reverse_iterator it = newObjects.rbegin(); it != newObjects.rend(); ++it)
		{
			ServerObject *obj = *it;

			if (obj  && obj->isAuthoritative())
			{
				GameScriptObject *scripts = obj->getScriptObject();

				if (scripts) //&& scripts->hasScript( "poi.template.scene.base.base_theater" ) )
				{
					scripts->handleMessage("startTheaterFromBuildout", std::vector<int8>());
				}
			}
		}
	}
}

// ----------------------------------------------------------------------

void ServerBuildoutManager::onEventStopped(std::string const & eventName)
{
	if (s_eventObjects.empty())
		return;

	ServerEventMap::iterator iter = s_eventObjects.find(eventName);

	if (iter == s_eventObjects.end())
		return; // We stopped an event with no objects? Warning?

	std::list< ServerEventAreaInfo >* eventList = &(*iter).second;

	std::list< ServerEventAreaInfo >::iterator objIter = eventList->begin();
	for (; objIter != eventList->end(); ++objIter)
	{
		// Object clean up.
		if ((*objIter).loadedObject)
		{
			ServerObject* object = (*objIter).loadedObject;

			LOGC(ConfigServerGame::getLogEventObjectDestruction(), "EventBuildout", ("Unloading Object for Event [%s].\nTemplate Name[%s]\nNetwork ID[%s]\nPosition[%-4.3f %-4.3f %-4.3f]\n\n",
				eventName.c_str(), object->getTemplateName(), object->getNetworkId().getValueString().c_str(),
				object->getTransform_o2p().getPosition_p().x, object->getTransform_o2p().getPosition_p().y, object->getTransform_o2p().getPosition_p().z));

			object->unload();

			(*objIter).loadedObject = nullptr;
		}
	}
}
// ======================================================================