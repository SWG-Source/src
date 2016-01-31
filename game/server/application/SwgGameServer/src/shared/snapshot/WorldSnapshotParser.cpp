//===================================================================
//
// WorldSnapshotParser.cpp
// asommers
//
// copyright 2002, sony online entertainment
//
//===================================================================

#include "FirstSwgGameServer.h"
#include "SwgGameServer/WorldSnapshotParser.h"

#include "serverGame/ServerBuildingObjectTemplate.h"
#include "serverGame/ServerCellObjectTemplate.h"
#include "serverGame/ServerObjectTemplate.h"
#include "serverGame/ServerStaticObjectTemplate.h"
#include "serverGame/ServerTangibleObjectTemplate.h"
#include "serverGame/ServerWorld.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedGame/SharedObjectTemplate.h"
#include "sharedObject/ObjectTemplateList.h"
#include "sharedObject/PortalPropertyTemplate.h"
#include "sharedUtility/WorldSnapshotReaderWriter.h"
#include "sharedMath/Quaternion.h"
#include "sharedMath/Transform.h"
#include "sharedMath/Vector.h"

#include <algorithm>
#include <map>
#include <stdio.h>
#include <string>

//===================================================================

namespace
{
	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	const char* const ms_serverCellObjectTemplateName = "object/cell/cell.iff";

	typedef std::map<std::string, WorldSnapshotReaderWriter*> PlanetMap;
	PlanetMap ms_planetMap;

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	static void install ()
	{
		IGNORE_RETURN (ms_planetMap.insert (std::make_pair (std::string ("corellia"),  new WorldSnapshotReaderWriter)));
		IGNORE_RETURN (ms_planetMap.insert (std::make_pair (std::string ("dantooine"), new WorldSnapshotReaderWriter)));
		IGNORE_RETURN (ms_planetMap.insert (std::make_pair (std::string ("dathomir"),  new WorldSnapshotReaderWriter)));
		IGNORE_RETURN (ms_planetMap.insert (std::make_pair (std::string ("dungeon1"),  new WorldSnapshotReaderWriter)));
		IGNORE_RETURN (ms_planetMap.insert (std::make_pair (std::string ("endor"),     new WorldSnapshotReaderWriter)));
		IGNORE_RETURN (ms_planetMap.insert (std::make_pair (std::string ("lok"),       new WorldSnapshotReaderWriter)));
		IGNORE_RETURN (ms_planetMap.insert (std::make_pair (std::string ("naboo"),     new WorldSnapshotReaderWriter)));
		IGNORE_RETURN (ms_planetMap.insert (std::make_pair (std::string ("rori"),      new WorldSnapshotReaderWriter)));
		IGNORE_RETURN (ms_planetMap.insert (std::make_pair (std::string ("talus"),     new WorldSnapshotReaderWriter)));
		IGNORE_RETURN (ms_planetMap.insert (std::make_pair (std::string ("tatooine"),  new WorldSnapshotReaderWriter)));
		IGNORE_RETURN (ms_planetMap.insert (std::make_pair (std::string ("yavin4"),    new WorldSnapshotReaderWriter)));
	}

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	static void remove ()
	{
		std::for_each (ms_planetMap.begin (), ms_planetMap.end (), PointerDeleterPairSecond ());
		ms_planetMap.clear ();
	}

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	void advancePastWhitespace (const char*& buffer)
	{
		NOT_NULL (buffer);
		while (*buffer && (*buffer == ' ' || *buffer == '\t' || *buffer == '\r' || *buffer == '\n'))
			++buffer;
	}

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	void advancePastToken (const char*& buffer)
	{
		NOT_NULL (buffer);
		while (*buffer && !(*buffer == ' ' || *buffer == '\t' || *buffer == '\r' || *buffer == '\n'))
			++buffer;

		advancePastWhitespace (buffer);
	}

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
}

//===================================================================

void WorldSnapshotParser::createWorldSnapshots (const char* const filename)
{
	//-- make sure the file exists
	FILE* infile = fopen (filename, "rt");
	FATAL (!infile, ("%s does not exist", filename));

	//-- install
	install ();

	DEBUG_REPORT_LOG_PRINT (true, ("----- START: WorldSnapshotParser::createWorldSnapshots -----\n"));

	//-- setup parsing
	char       buffer [1024];
	char       planetName [32];
	int        networkIdInt;
	Vector     position;
	Quaternion orientation;
	int        containedByNetworkIdInt;
	char       serverObjectTemplateName [256];
	int        cellIndex;
	uint32     portalLayoutCrc;
	std::vector<const ObjectTemplate*> objectTemplateList;
	std::vector<int> validNetworkIdList;

/*
	select scene_id||chr(9)||
		   object_id||chr(9)||
		   x||chr(9)||
		   y||chr(9)||
		   z||chr(9)||
		   quaternion_w||chr(9)||
		   quaternion_x||chr(9)||
		   quaternion_y||chr(9)||
		   quaternion_z||chr(9)||
		   contained_by||chr(9)||
		   object_template_crc||chr(10)
		   cell_index where object_template == ms_serverCellObjectTemplateName
		   <portalProperty.crc>
*/

	//-- scan each line of the file
	//bool quit = false;
	while (fgets (buffer, 1024, infile) != 0)
	{
		const char* current = buffer;

		//-- skip past blank lines
		advancePastWhitespace (current);
		if (*current == 0)
			continue;

		//-- start parsing parameters
		sscanf (current, "%s", planetName);
		advancePastToken (current);

		sscanf (current, "%i", &networkIdInt);
		advancePastToken (current);

		sscanf (current, "%f", &position.x);
		advancePastToken (current);

		sscanf (current, "%f", &position.y);
		advancePastToken (current);

		sscanf (current, "%f", &position.z);
		advancePastToken (current);

		sscanf (current, "%f", &orientation.w);
		advancePastToken (current);

		sscanf (current, "%f", &orientation.x);
		advancePastToken (current);

		sscanf (current, "%f", &orientation.y);
		advancePastToken (current);

		sscanf (current, "%f", &orientation.z);
		advancePastToken (current);

		sscanf (current, "%i", &containedByNetworkIdInt);
		advancePastToken (current);

		char possibleServerObjectTemplateName [256];
		sscanf (current, "%s", possibleServerObjectTemplateName);
		advancePastToken (current);

		if (possibleServerObjectTemplateName [0] == 'o')
			strcpy (serverObjectTemplateName, possibleServerObjectTemplateName);
		else
			strcpy (serverObjectTemplateName, ObjectTemplateList::lookUp (static_cast<uint32> (atoi (possibleServerObjectTemplateName))).getString ());

		//-- check to see of the object is a cell, and if so, read a cell index (all cells have a cell index)
		cellIndex = 0;
		if (strcmp (serverObjectTemplateName, ms_serverCellObjectTemplateName) == 0)
		{
			sscanf (current, "%i", &cellIndex);
			advancePastToken (current);
		}

		//-- check to see if the object is at the origin
		if (containedByNetworkIdInt == 0 && position == Vector::zero)
		{
			DEBUG_REPORT_LOG_PRINT (true, ("skipping %i - object %s is at the origin of the world on planet %s\n", networkIdInt, serverObjectTemplateName, planetName));
			continue;
		}

		//-- check to see if the object is too close to the origin
		if (containedByNetworkIdInt == 0 && position.magnitude () < 10.f)
		{
			DEBUG_REPORT_LOG_PRINT (true, ("skipping %i - object %s is suspiciously close to the origin of the world on planet %s\n", networkIdInt, serverObjectTemplateName, planetName));
			continue;
		}

		//-- check to see of the line has a portal crc (may be there, may not)
		portalLayoutCrc = 0;
		sscanf (current, "%lu", &portalLayoutCrc);

		//-- find the planet
		const std::string planetNameString (planetName);
		PlanetMap::iterator iter = ms_planetMap.find (planetNameString);
		if (iter != ms_planetMap.end ())
		{
			WorldSnapshotReaderWriter* const worldSnapshotWriter = iter->second;
			NOT_NULL (worldSnapshotWriter);

			//-- verify that the object has a valid contained by object
			if (containedByNetworkIdInt != 0)
			{
				std::sort (validNetworkIdList.begin (), validNetworkIdList.end ());
				if (!std::binary_search (validNetworkIdList.begin (), validNetworkIdList.end (), containedByNetworkIdInt))
				{
					DEBUG_REPORT_LOG_PRINT (true, ("skipping %i - containedByNetworkId %i not found for %s\n", networkIdInt, containedByNetworkIdInt, serverObjectTemplateName));
					continue;
				}
			}

			//-- check to see that the server object template exists
			if (!TreeFile::exists (serverObjectTemplateName))
			{
				DEBUG_REPORT_LOG_PRINT (true, ("skipping %i - server object template %s does not exist\n", networkIdInt, serverObjectTemplateName));
				continue;
			}

			//-- check to see that the object template is of the correct type
			const ServerObjectTemplate* const serverObjectTemplate = dynamic_cast<const ServerObjectTemplate*> (ObjectTemplateList::fetch (serverObjectTemplateName));
			if (!serverObjectTemplate)
			{
				DEBUG_REPORT_LOG_PRINT (true, ("skipping %i - object template %s is not a valid ServerObjectTemplate\n", networkIdInt, serverObjectTemplateName));
				continue;
			}

			objectTemplateList.push_back (serverObjectTemplate);

			if (!(serverObjectTemplate->getId () == ServerBuildingObjectTemplate::ServerBuildingObjectTemplate_tag ||
				serverObjectTemplate->getId () == ServerCellObjectTemplate::ServerCellObjectTemplate_tag ||
				serverObjectTemplate->getId () == ServerTangibleObjectTemplate::ServerTangibleObjectTemplate_tag ||
				serverObjectTemplate->getId () == ServerStaticObjectTemplate::ServerStaticObjectTemplate_tag))
			{
				char tagString [5];
				ConvertTagToString (serverObjectTemplate->getId (), tagString);
				DEBUG_REPORT_LOG_PRINT (true, ("skipping %i - object template %s not building, cell, or static [%s]\n", networkIdInt, serverObjectTemplateName, tagString));
				continue;
			}

			//-- compute the transform
			Transform transform_p;
			orientation.getTransform (&transform_p);
			transform_p.setPosition_p (position);

			//-- extract any necessary variables from the object template
			const std::string& sharedObjectTemplateName = serverObjectTemplate->getSharedTemplate ();

			//-- check to see that the shared object template exists
			if (!TreeFile::exists (sharedObjectTemplateName.c_str ()))
			{
				DEBUG_REPORT_LOG_PRINT (true, ("skipping %i - server object template %s specifies shared object template %s which does not exist\n", networkIdInt, serverObjectTemplateName, sharedObjectTemplateName.c_str ()));
				continue;
			}

			const float radius = serverObjectTemplate->getUpdateRanges (ServerObjectTemplate::UR_far);

			//-- verify portalLayoutCrc
			{
				//-- make sure the shared template could be loaded
				const SharedObjectTemplate* const sharedObjectTemplate = safe_cast<const SharedObjectTemplate*> (ObjectTemplateList::fetch (sharedObjectTemplateName.c_str ()));
				if (!sharedObjectTemplate)
				{
					DEBUG_REPORT_LOG_PRINT (true, ("skipping %i - server object template %s specifies shared object template %s which could not be loaded\n", networkIdInt, serverObjectTemplateName, sharedObjectTemplateName.c_str ()));
					continue;
				}

				objectTemplateList.push_back (sharedObjectTemplate);

				//--
				const std::string& portalLayoutFilename = sharedObjectTemplate->getPortalLayoutFilename ();
				if (!portalLayoutFilename.empty ())
				{
					uint32 extractedPortalLayoutCrc;
					if (!PortalPropertyTemplate::extractPortalLayoutCrc (portalLayoutFilename.c_str (), extractedPortalLayoutCrc))
					{
						DEBUG_REPORT_LOG_PRINT (true, ("skipping %i - server object template %s specifies shared object template %s with portal layout file %s which could not extract portal layout crc\n", networkIdInt, serverObjectTemplateName, sharedObjectTemplateName.c_str (), portalLayoutFilename.c_str ()));
						continue;
					}

					if (portalLayoutCrc != extractedPortalLayoutCrc)
					{
						DEBUG_REPORT_LOG_PRINT (true, ("skipping %i - server object template %s specifies shared object template %s with portal layout file %s with mismatched crcs [%lu != %lu] <%s %1.2f, %1.2f>\n", networkIdInt, serverObjectTemplateName, sharedObjectTemplateName.c_str (), portalLayoutFilename.c_str (), portalLayoutCrc, extractedPortalLayoutCrc, planetName, position.x, position.z));
						continue;
					}
				}
			}

			//-- add the object to the list of valid objects
			validNetworkIdList.push_back (networkIdInt);

			//-- add to writer
			worldSnapshotWriter->addObject (
				networkIdInt,
				containedByNetworkIdInt,
				ConstCharCrcString(sharedObjectTemplateName.c_str()),
				cellIndex,
				transform_p,
				radius,
				portalLayoutCrc);

//			DEBUG_REPORT_LOG_PRINT (true, ("added %s - %s, %s\n", networkIdString, planetName, sharedObjectTemplateName.c_str ()));
		}
		else
		{
			DEBUG_REPORT_LOG_PRINT (true, ("skipping %i - unknown planet %s\n", networkIdInt, planetName));
		}
	}

	//-- release all object templates
	{
		uint i;
		for (i = 0; i < objectTemplateList.size (); ++i)
			objectTemplateList [i]->releaseReference ();

		objectTemplateList.clear ();
	}

	DEBUG_REPORT_LOG_PRINT (true, ("----- SUMMARY: WorldSnapshotParser::createWorldSnapshots -----\n"));

	//-- write the planet maps
	{
		PlanetMap::iterator iter = ms_planetMap.begin ();
		for (; iter != ms_planetMap.end (); ++iter)
		{
			const WorldSnapshotReaderWriter* const writer = iter->second;

			const std::string fileName = iter->first + ".ws";
			if (!writer->save (fileName.c_str ()))
				DEBUG_WARNING (true, ("could not save world snapshot file %s", fileName.c_str ()));

#ifdef _DEBUG
			const int numberOfNodes = writer->getNumberOfNodes ();
			const int totalNumberOfNodes = writer->getTotalNumberOfNodes ();
			DEBUG_REPORT_LOG_PRINT (true, ("%s has %i root objects and %i total objects\n", iter->first.c_str (), numberOfNodes, totalNumberOfNodes));
#endif
		}
	}

	//-- remove
	remove ();

	DEBUG_REPORT_LOG_PRINT (true, ("----- END: WorldSnapshotParser::createWorldSnapshots -----\n"));
}

//===================================================================

