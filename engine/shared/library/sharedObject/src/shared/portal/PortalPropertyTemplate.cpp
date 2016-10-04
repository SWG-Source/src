// ======================================================================
//
// PortalPropertyTemplate.cpp
// Copyright 2002, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedObject/FirstSharedObject.h"
#include "sharedObject/PortalPropertyTemplate.h"

#include "sharedFile/Iff.h"
#include "sharedFoundation/DataResourceList.h"
#include "sharedMath/AxialBox.h"
#include "sharedMath/IndexedTriangleList.h"
#include "sharedMath/Plane.h"
#include "sharedCollision/BaseClass.h"
#include "sharedCollision/ConfigSharedCollision.h"
#include "sharedCollision/Extent.h"
#include "sharedCollision/ExtentList.h"
#include "sharedCollision/FloorMesh.h"
#include "sharedCollision/FloorManager.h" // for the path graph factory
#include "sharedObject/AppearanceTemplate.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/ConfigSharedObject.h"
#include "sharedObject/Object.h"
#include "sharedObject/Portal.h"
#include "sharedObject/PortalPropertyTemplateList.h"
#include "sharedPathfinding/SimplePathGraph.h"
#include "sharedPathfinding/DynamicPathGraph.h"
#include "sharedPathfinding/DynamicPathNode.h"
#include "sharedPathfinding/PathEdge.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"
#include "sharedUtility/FileName.h"

#include <map>
#include <stdio.h>
#include <vector>
#include <set>

typedef BaseClass * (*ExpandBuildingGraphHook)( PortalPropertyTemplate * portalTemplate, BaseClass * baseBuildingGraph );

ExpandBuildingGraphHook g_expandBuildingGraphHook = nullptr;

// ======================================================================

const Tag TAG_CRC  = TAG3(C,R,C);

const Tag TAG_CELL = TAG(C,E,L,L);
const Tag TAG_CELS = TAG(C,E,L,S);
const Tag TAG_LGHT = TAG(L,G,H,T);
const Tag TAG_PRTL = TAG(P,R,T,L);
const Tag TAG_PRTO = TAG(P,R,T,O);
const Tag TAG_PRTS = TAG(P,R,T,S);
const Tag TAG_PGRF = TAG(P,G,R,F);

// ======================================================================

namespace PortalPropertyTemplateNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	typedef std::map<PersistentCrcString, Transform>  NameTransformMap;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	void  remove();

	int   getRequiredColumnNumberFromNamedTable(char const *filename, DataTable const *table, char const *columnName);
	void  loadEjectionPointData(char const *filename);

	IndexedTriangleList * createPortalGeometry(PortalPropertyTemplate::VertexList const & VertexList);
	void  scalePortalVertices(IndexedTriangleList & indexedTriangleList);

#ifdef _DEBUG
	void  validateCoplanar(IndexedTriangleList const & indexedTriangleList, char const * const fileName);
#endif

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool              s_installed;
	NameTransformMap  s_ejectionTransformMap;
}

using namespace PortalPropertyTemplateNamespace;

// ======================================================================
// namespace PortalPropertyTemplateNamespace
// ======================================================================

void PortalPropertyTemplateNamespace::remove()
{
	FATAL(!s_installed, ("PortalPropertyTemplateNamespace not installed."));
	s_installed = false;

	s_ejectionTransformMap.clear();
}

// ----------------------------------------------------------------------

int PortalPropertyTemplateNamespace::getRequiredColumnNumberFromNamedTable(char const *filename, DataTable const *table, char const *columnName)
{
	NOT_NULL(columnName);

	int const columnNumber = table->findColumnNumber(columnName);
	FATAL(columnNumber < 0, ("failed to find column name [%s] in PortalPropertyTemplate ejection transform file [%s].", columnName, filename));

	return columnNumber;
}

// ----------------------------------------------------------------------

void PortalPropertyTemplateNamespace::loadEjectionPointData(char const *filename)
{
	NOT_NULL(filename);

	//-- Load the data table.
	DataTable *const table = DataTableManager::getTable(filename, true);
	FATAL(!table, ("PortalPropertyTemplate ejection point data file [%s] failed to open.", filename));

	//-- Find required data column numbers.
	int const pobFileNameColumnNumber    = getRequiredColumnNumberFromNamedTable(filename, table, "pob_file_name");
	int const ejectXColumnNumber         = getRequiredColumnNumberFromNamedTable(filename, table, "eject_x");
	int const ejectYColumnNumber         = getRequiredColumnNumberFromNamedTable(filename, table, "eject_y");
	int const ejectZColumnNumber         = getRequiredColumnNumberFromNamedTable(filename, table, "eject_z");
	int const ejectFacingYawColumnNumber = getRequiredColumnNumberFromNamedTable(filename, table, "eject_facing_yaw_degrees");

	//-- Loop through data, constructing entries as necessary.
	int const rowCount = table->getNumRows();
	for (int rowIndex = 0; rowIndex < rowCount; ++rowIndex)
	{
		//-- Get pob file name.
		std::string const &pobFileName = table->getStringValue(pobFileNameColumnNumber, rowIndex);
		FATAL(pobFileName.empty(), ("File [%s], row [%d]: POB filename is zero-length.", filename, rowIndex + 1));

		//-- Construct the transform for the ejection point.  Ejection point transforms are in the
		//   same parent space as the building.
		float const ejectX     = table->getFloatValue(ejectXColumnNumber, rowIndex);
		float const ejectY     = table->getFloatValue(ejectYColumnNumber, rowIndex);
		float const ejectZ     = table->getFloatValue(ejectZColumnNumber, rowIndex);
		float const yawDegrees = table->getFloatValue(ejectFacingYawColumnNumber, rowIndex);

		Transform  ejectionTransform;
		ejectionTransform.yaw_l(yawDegrees * PI_OVER_180);
		ejectionTransform.setPosition_p(Vector(ejectX, ejectY, ejectZ));

		//-- Add to map.
		std::pair<NameTransformMap::iterator, bool> const result = s_ejectionTransformMap.insert(NameTransformMap::value_type(PersistentCrcString(pobFileName.c_str(), true), ejectionTransform));
		FATAL(!result.second, ("Failed to insert pob filename [%s] into ejection point map, probably a duplicate name in datatable filename [%s].", pobFileName.c_str(), filename));
	}

	DataTableManager::close(filename);
	
}

// ----------------------------------------------------------------------

#ifdef _DEBUG
void PortalPropertyTemplateNamespace::validateCoplanar(IndexedTriangleList const & indexedTriangleList, char const * const fileName)
{
	std::vector<Vector> const & vertexList = indexedTriangleList.getVertices();
	uint const numberOfVertices = vertexList.size();
	DEBUG_FATAL(numberOfVertices < 3, ("Portal with fewer than 3 vertices")); 
	Plane p(vertexList[0], vertexList[1], vertexList[2]);
	for (uint i = 3; i < numberOfVertices; ++i)
	{
		float const distance = p.computeDistanceTo(vertexList[i]);
		WARNING(std::abs(distance) > 0.001f, ("[%s]: portal is not planar, vertex %d is %8.6f from the plane", fileName, static_cast<int>(i), distance));
	}
}
#endif

// ----------------------------------------------------------------------

IndexedTriangleList * PortalPropertyTemplateNamespace::createPortalGeometry(PortalPropertyTemplate::VertexList const & vertices)
{
	IndexedTriangleList * const indexedTriangleList = new IndexedTriangleList;
	indexedTriangleList->addTriangleFan(&vertices[0], static_cast<int>(vertices.size()));
	return indexedTriangleList;
}

// ----------------------------------------------------------------------

void PortalPropertyTemplateNamespace::scalePortalVertices(IndexedTriangleList & indexedTriangleList)
{
	std::vector<Vector> & vertexList = indexedTriangleList.getVertices();

	const uint numberOfVertices = vertexList.size();
	const float portalScale = ConfigSharedObject::getPortalScale();
	if (numberOfVertices > 0 && portalScale > 1.f)
	{
		uint j;

		// compute the center
		Vector center;
		for (j = 0; j < numberOfVertices; ++j)
			center += vertexList[j];

		center /= static_cast<float>(numberOfVertices);

		// scale portal vertices
		for (j = 0; j < numberOfVertices; ++j)
		{
			vertexList[j] -= center;
			vertexList[j] *= portalScale;
			vertexList[j] += center;
		}
	}
}

// ======================================================================

class PortalPropertyTemplateCellPortal::PreloadManager
{
public:

	explicit PreloadManager (const PortalPropertyTemplateCellPortal* portalPropertyTemplateCellPortal);
	~PreloadManager ();

private:
	
	PreloadManager ();
	PreloadManager (const PreloadManager&);
	PreloadManager& operator= (const PreloadManager&);

private:

	typedef std::vector<const AppearanceTemplate*> PreloadAppearanceTemplateList;
	PreloadAppearanceTemplateList m_preloadAppearanceTemplateList;
};

// ----------------------------------------------------------------------

PortalPropertyTemplateCellPortal::PreloadManager::PreloadManager (const PortalPropertyTemplateCellPortal* const portalPropertyTemplateCellPortal) :
	m_preloadAppearanceTemplateList ()
{
	if (portalPropertyTemplateCellPortal->m_doorStyle && portalPropertyTemplateCellPortal->m_doorStyle [0])
		Portal::preloadDoorStyle (portalPropertyTemplateCellPortal->m_doorStyle, m_preloadAppearanceTemplateList);
}

// ----------------------------------------------------------------------

PortalPropertyTemplateCellPortal::PreloadManager::~PreloadManager ()
{
	uint i;
	for (i = 0; i < m_preloadAppearanceTemplateList.size (); ++i)
		AppearanceTemplateList::release (m_preloadAppearanceTemplateList [i]);

	m_preloadAppearanceTemplateList.clear ();
}

// ======================================================================

PortalPropertyTemplateCellPortal::PortalPropertyTemplateCellPortal(const PortalPropertyTemplate &portalPropertyTemplate, int cellIndex, int cellPortalIndex, Iff &iff)
:
	m_disabled(false),
	m_passable(false),
	m_geometryWindingClockwise(true),
	m_portalGeometry(nullptr),
	m_doorStyle(nullptr),
	m_hasDoorHardpoint(false),
	m_doorHardpoint(),
	m_plane(),
	m_preloadManager (0)
{
	load(portalPropertyTemplate, cellIndex, cellPortalIndex, iff);
	computePlaneEquation();
}

// ----------------------------------------------------------------------

PortalPropertyTemplateCellPortal::~PortalPropertyTemplateCellPortal()
{
	m_portalGeometry = nullptr;
	delete [] m_doorStyle;

	if (m_preloadManager)
	{
		delete m_preloadManager;
		m_preloadManager = 0;
	}
}

// ----------------------------------------------------------------------

void PortalPropertyTemplateCellPortal::load(const PortalPropertyTemplate &portalPropertyTemplate, int cellIndex, int cellPortalIndex, Iff &iff)
{
	iff.enterForm(TAG_PRTL);

		switch (iff.getCurrentName())
		{
			case TAG_0001:
				load_0001(portalPropertyTemplate, cellIndex, cellPortalIndex, iff);
				break;

			case TAG_0002:
				load_0002(portalPropertyTemplate, cellIndex, cellPortalIndex, iff);
				break;

			case TAG_0003:
				load_0003(portalPropertyTemplate, cellIndex, cellPortalIndex, iff);
				break;

			case TAG_0004:
				load_0004(portalPropertyTemplate, cellIndex, cellPortalIndex, iff);
				break;

			case TAG_0005:
				load_0005(portalPropertyTemplate, cellIndex, cellPortalIndex, iff);
				break;

			default:
				{
					char buffer[256];
					iff.formatLocation(buffer, sizeof(buffer));
					DEBUG_FATAL(true, ("Unknown portal version at %s", buffer));
				}
		}

	iff.exitForm(TAG_PRTL);
}

// ----------------------------------------------------------------------

void PortalPropertyTemplateCellPortal::load_0001(const PortalPropertyTemplate &portalPropertyTemplate, int cellIndex, int cellPortalIndex, Iff &iff)
{
	// read the portal data and reverse flag
	iff.enterChunk(TAG_0001);

		m_disabled = false;
		m_passable = true;
		int const geometryIndex = iff.read_int32();
		m_geometryWindingClockwise = iff.read_bool8();
		m_portalGeometry = portalPropertyTemplate.getPortalGeometry(geometryIndex, cellIndex, cellPortalIndex, m_geometryWindingClockwise);
		IGNORE_RETURN(iff.read_int32()); // target cell index

	iff.exitChunk(TAG_0001);
}

// ----------------------------------------------------------------------

void PortalPropertyTemplateCellPortal::load_0002(const PortalPropertyTemplate &portalPropertyTemplate, int cellIndex, int cellPortalIndex, Iff &iff)
{
	iff.enterChunk(TAG_0002);

		m_disabled = false;
		m_passable = iff.read_bool8();
		int const geometryIndex = iff.read_int32();
		m_geometryWindingClockwise = iff.read_bool8();
		m_portalGeometry = portalPropertyTemplate.getPortalGeometry(geometryIndex, cellIndex, cellPortalIndex, m_geometryWindingClockwise);
		IGNORE_RETURN(iff.read_int32()); // target cell index

	iff.exitChunk(TAG_0002);
}

// ----------------------------------------------------------------------

void PortalPropertyTemplateCellPortal::load_0003(const PortalPropertyTemplate &portalPropertyTemplate, int cellIndex, int cellPortalIndex, Iff &iff)
{
	iff.enterChunk(TAG_0003);

		m_disabled = false;
		m_passable = iff.read_bool8();
		int const geometryIndex = iff.read_int32();
		m_geometryWindingClockwise = iff.read_bool8();
		m_portalGeometry = portalPropertyTemplate.getPortalGeometry(geometryIndex, cellIndex, cellPortalIndex, m_geometryWindingClockwise);
		IGNORE_RETURN(iff.read_int32()); // target cell index
		m_doorStyle = iff.read_string();

	iff.exitChunk(TAG_0003);
}

// ----------------------------------------------------------------------

void PortalPropertyTemplateCellPortal::load_0004(const PortalPropertyTemplate &portalPropertyTemplate, int cellIndex, int cellPortalIndex, Iff &iff)
{
	iff.enterChunk(TAG_0004);

		m_passable = iff.read_bool8();
		int const geometryIndex = iff.read_int32();
		m_geometryWindingClockwise = iff.read_bool8();
		m_portalGeometry = portalPropertyTemplate.getPortalGeometry(geometryIndex, cellIndex, cellPortalIndex, m_geometryWindingClockwise);
		IGNORE_RETURN(iff.read_int32()); // target cell index
		m_doorStyle = iff.read_string();
		m_hasDoorHardpoint = iff.read_bool8();
		m_doorHardpoint = iff.read_floatTransform();

	iff.exitChunk(TAG_0004);
}

// ----------------------------------------------------------------------

void PortalPropertyTemplateCellPortal::load_0005(const PortalPropertyTemplate &portalPropertyTemplate, int cellIndex, int cellPortalIndex, Iff &iff)
{
	iff.enterChunk(TAG_0005);

		m_disabled = iff.read_bool8();
		m_passable = iff.read_bool8();
		int const geometryIndex = iff.read_int32();
		m_geometryWindingClockwise = iff.read_bool8();
		m_portalGeometry = portalPropertyTemplate.getPortalGeometry(geometryIndex, cellIndex, cellPortalIndex, m_geometryWindingClockwise);
		IGNORE_RETURN(iff.read_int32()); // target cell index
		m_doorStyle = iff.read_string();
		m_hasDoorHardpoint = iff.read_bool8();
		m_doorHardpoint = iff.read_floatTransform();

	iff.exitChunk(TAG_0005);
}

// ----------------------------------------------------------------------

void PortalPropertyTemplateCellPortal::computePlaneEquation()
{
	NOT_NULL(m_portalGeometry);
	std::vector<Vector> const & vertices = m_portalGeometry->getVertices();
	std::vector<int> const & indices = m_portalGeometry->getIndices();
	Vector const & v0 = vertices[indices[0]];
	Vector const & v1 = vertices[m_geometryWindingClockwise ? indices[1] : indices[2]];
	Vector const & v2 = vertices[m_geometryWindingClockwise ? indices[2] : indices[1]];
	m_plane.set(v0, v1, v2);
}

// ----------------------------------------------------------------------

Transform const PortalPropertyTemplateCellPortal::getDoorTransform(bool useDoorHardpoint) const
{
	if (useDoorHardpoint && m_hasDoorHardpoint)
		return m_doorHardpoint;

	// If there's no door hardpoint, the door orientation is aligned with the portal's plane
	Transform doorTransform;
	Vector portalNormal = m_plane.getNormal();
	Vector up(0.0f,1.0f,0.0f);
	Vector right = up.cross(portalNormal);
	doorTransform.setLocalFrameIJK_p(right, up, portalNormal);

	// The door position is the center of the bottom of the portal's bounding box
	if (m_portalGeometry)
	{
		std::vector<Vector> const & vertices = m_portalGeometry->getVertices();
		
		AxialBox tempBox;
		std::vector<Vector>::const_iterator iEnd = vertices.end();
		for (std::vector<Vector>::const_iterator i = vertices.begin(); i != iEnd; ++i)
			tempBox.add(*i);

		doorTransform.setPosition_p(tempBox.getBase());
	}

	return doorTransform;
}

// ----------------------------------------------------------------------

void PortalPropertyTemplateCellPortal::flipPassable()
{
	m_passable = !m_passable;
}

// ----------------------------------------------------------------------

void PortalPropertyTemplateCellPortal::preloadAssets() const
{
	if (!m_preloadManager)
		m_preloadManager = new PreloadManager (this);
}

// ----------------------------------------------------------------------

void PortalPropertyTemplateCellPortal::garbageCollect() const
{
	if (m_preloadManager)
	{
		delete m_preloadManager;
		m_preloadManager = 0;
	}
}

// ======================================================================

class PortalPropertyTemplateCell::PreloadManager
{
public:

	explicit PreloadManager (const PortalPropertyTemplateCell* portalPropertyTemplateCell);
	~PreloadManager ();

private:
	
	PreloadManager ();
	PreloadManager (const PreloadManager&);
	PreloadManager& operator= (const PreloadManager&);

private:

	const AppearanceTemplate* m_preloadAppearanceTemplate;
};

// ----------------------------------------------------------------------

PortalPropertyTemplateCell::PreloadManager::PreloadManager (const PortalPropertyTemplateCell* const portalPropertyTemplateCell) :
	m_preloadAppearanceTemplate (0)
{
	if (portalPropertyTemplateCell->m_appearanceName)
	{
		m_preloadAppearanceTemplate = AppearanceTemplateList::fetch (portalPropertyTemplateCell->m_appearanceName);
		m_preloadAppearanceTemplate->preloadAssets ();
	}


	IGNORE_RETURN( portalPropertyTemplateCell->getFloorMesh() );
}

// ----------------------------------------------------------------------

PortalPropertyTemplateCell::PreloadManager::~PreloadManager ()
{
	if (m_preloadAppearanceTemplate)
		AppearanceTemplateList::release (m_preloadAppearanceTemplate);
}

// ======================================================================

PortalPropertyTemplateCell::PortalPropertyTemplateCell(const PortalPropertyTemplate &portalPropertyTemplate, int index, Iff &iff)
:
	m_name(nullptr),
	m_appearanceName(nullptr),
	m_floorName(nullptr),
	m_floorMesh(nullptr),
	m_canSeeParentCell(false),
	m_lightList(nullptr),
	m_portalList(new PortalPropertyTemplateCellPortalList),
	m_collisionExtent(nullptr),
	m_preloadManager (0)
{
	load(portalPropertyTemplate, index, iff);
}

// ----------------------------------------------------------------------

PortalPropertyTemplateCell::~PortalPropertyTemplateCell()
{
	delete [] m_name;
	delete [] m_appearanceName;
	delete [] m_floorName;
	delete m_lightList;

	{
		while (!m_portalList->empty())
		{
			delete m_portalList->back();
			m_portalList->pop_back();
		}
		delete m_portalList;
	}

	delete m_collisionExtent;
	m_collisionExtent = nullptr;

	if (m_preloadManager)
	{
		delete m_preloadManager;
		m_preloadManager = 0;
	}

	if (m_floorMesh)
	{
		m_floorMesh->releaseReference ();
		m_floorMesh = nullptr;
	}
}

// ----------------------------------------------------------------------

void PortalPropertyTemplateCell::load(const PortalPropertyTemplate &portalPropertyTemplate, int index, Iff &iff)
{
	iff.enterForm(TAG_CELL);

		switch (iff.getCurrentName())
		{
			case TAG_0001:
				load_0001(portalPropertyTemplate, index, iff);
				break;

			case TAG_0002:
				load_0002(portalPropertyTemplate, index, iff);
				break;

			case TAG_0003:
				load_0003(portalPropertyTemplate, index, iff);
				break;

			case TAG_0004:
				load_0004(portalPropertyTemplate, index, iff);
				break;

			case TAG_0005:
				load_0005(portalPropertyTemplate, index, iff);
				break;

			default:
				{
					char buffer[256];
					iff.formatLocation(buffer, sizeof(buffer));
					DEBUG_FATAL(true, ("Unknown portal object version at %s", buffer));
				}
		}

	iff.exitForm(TAG_CELL);
}

// ----------------------------------------------------------------------

void PortalPropertyTemplateCell::load_0001(const PortalPropertyTemplate &portalPropertyTemplate, int index, Iff &iff)
{
	iff.enterForm(TAG_0001);

		iff.enterChunk(TAG_DATA);
			const int numberOfPortals = iff.read_int32();
			m_canSeeParentCell = iff.read_bool8();
			m_appearanceName = iff.read_string();
		iff.exitChunk(TAG_DATA);

		m_portalList->reserve(static_cast<uint>(numberOfPortals));
		for (int i = 0; i < numberOfPortals; ++i)
			m_portalList->push_back(new PortalPropertyTemplateCellPortal(portalPropertyTemplate, index, i, iff));

	iff.exitForm(TAG_0001);

	char buffer[16];
	sprintf(buffer, "old_%d", index);
	m_name = DuplicateString(buffer);
}

// ----------------------------------------------------------------------

void PortalPropertyTemplateCell::load_0002(const PortalPropertyTemplate &portalPropertyTemplate, int index, Iff &iff)
{
	iff.enterForm(TAG_0002);

		iff.enterChunk(TAG_DATA);
			const int numberOfPortals = iff.read_int32();
			m_canSeeParentCell = iff.read_bool8();
			m_appearanceName = iff.read_string();
			bool hasFloor = iff.read_bool8();
			if(hasFloor)
			{
				m_floorName = iff.read_string();
			}
		iff.exitChunk(TAG_DATA);

		m_portalList->reserve(static_cast<uint>(numberOfPortals));
		for (int i = 0; i < numberOfPortals; ++i)
			m_portalList->push_back(new PortalPropertyTemplateCellPortal(portalPropertyTemplate, index, i, iff));

	iff.exitForm(TAG_0002);

	char buffer[16];
	sprintf(buffer, "old_%d", index);
	m_name = DuplicateString(buffer);
}

// ----------------------------------------------------------------------

void PortalPropertyTemplateCell::load_0003(const PortalPropertyTemplate &portalPropertyTemplate, int index, Iff &iff)
{
	iff.enterForm(TAG_0003);

		iff.enterChunk(TAG_DATA);
			const int numberOfPortals = iff.read_int32();
			m_canSeeParentCell = iff.read_bool8();
			m_appearanceName = iff.read_string();
			bool hasFloor = iff.read_bool8();
			if(hasFloor)
			{
				m_floorName = iff.read_string();
			}
		iff.exitChunk(TAG_DATA);

		m_portalList->reserve(static_cast<uint>(numberOfPortals));
		for (int i = 0; i < numberOfPortals; ++i)
			m_portalList->push_back(new PortalPropertyTemplateCellPortal(portalPropertyTemplate, index, i, iff));

		iff.enterChunk(TAG_LGHT);
			const int numberOfLights = iff.read_int32();

			if (numberOfLights)
			{
				m_lightList = new LightList;
				m_lightList->reserve(static_cast<LightList::size_type>(numberOfLights));

				for (int j = 0; j < numberOfLights; ++j)
				{
					Light light;

					light.type = static_cast<Light::Type>(iff.read_int8());
					light.diffuseColor = iff.read_floatVectorArgb();
					light.specularColor = iff.read_floatVectorArgb();
					light.transform = iff.read_floatTransform();
					light.constantAttenuation = iff.read_float();
					light.linearAttenuation = iff.read_float();
					light.quadraticAttenuation = iff.read_float();

					// hack to deal with bad data coming out of maya exporter
					light.transform.yaw_l(PI);

					m_lightList->push_back(light);
				}
			}
		iff.exitChunk(TAG_LGHT);

	iff.exitForm(TAG_0003);

	char buffer[16];
	sprintf(buffer, "old_%d", index);
	m_name = DuplicateString(buffer);
}

// ----------------------------------------------------------------------

void PortalPropertyTemplateCell::load_0004(const PortalPropertyTemplate &portalPropertyTemplate, int index, Iff &iff)
{
	iff.enterForm(TAG_0004);

		iff.enterChunk(TAG_DATA);
			const int numberOfPortals = iff.read_int32();
			m_canSeeParentCell = iff.read_bool8();
			m_name = iff.read_string();
			m_appearanceName = iff.read_string();
			bool hasFloor = iff.read_bool8();

			if(hasFloor)
				m_floorName = iff.read_string();

		iff.exitChunk(TAG_DATA);

		m_portalList->reserve(static_cast<uint>(numberOfPortals));
		for (int i = 0; i < numberOfPortals; ++i)
			m_portalList->push_back(new PortalPropertyTemplateCellPortal(portalPropertyTemplate, index, i, iff));

		iff.enterChunk(TAG_LGHT);
			const int numberOfLights = iff.read_int32();

			if (numberOfLights)
			{
				IS_NULL(m_lightList);
				m_lightList = new LightList;
				m_lightList->reserve(static_cast<LightList::size_type>(numberOfLights));

				for (int j = 0; j < numberOfLights; ++j)
				{
					Light light;

					light.type = static_cast<Light::Type>(iff.read_int8());
					light.diffuseColor = iff.read_floatVectorArgb();
					light.specularColor = iff.read_floatVectorArgb();
					light.transform = iff.read_floatTransform();
					light.constantAttenuation = iff.read_float();
					light.linearAttenuation = iff.read_float();
					light.quadraticAttenuation = iff.read_float();

					// hack to deal with bad data coming out of maya exporter
					light.transform.yaw_l(PI);

					m_lightList->push_back(light);
				}
			}
		iff.exitChunk(TAG_LGHT);

	iff.exitForm(TAG_0004);
}

// ----------------------------------------------------------------------
//@todo - This is just skipping over the collision extent form for now

void PortalPropertyTemplateCell::load_0005(const PortalPropertyTemplate &portalPropertyTemplate, int index, Iff &iff)
{
	iff.enterForm(TAG_0005);

		iff.enterChunk(TAG_DATA);
			const int numberOfPortals = iff.read_int32();
			m_canSeeParentCell = iff.read_bool8();
			m_name = iff.read_string();
			m_appearanceName = iff.read_string();
			bool hasFloor = iff.read_bool8();

			if(hasFloor)
				m_floorName = iff.read_string();

		iff.exitChunk(TAG_DATA);

		m_collisionExtent = ExtentList::create(iff);

		m_portalList->reserve(static_cast<uint>(numberOfPortals));
		for (int i = 0; i < numberOfPortals; ++i)
			m_portalList->push_back(new PortalPropertyTemplateCellPortal(portalPropertyTemplate, index, i, iff));

		iff.enterChunk(TAG_LGHT);
			const int numberOfLights = iff.read_int32();

			if (numberOfLights)
			{
				IS_NULL(m_lightList);
				m_lightList = new LightList;
				m_lightList->reserve(static_cast<LightList::size_type>(numberOfLights));

				for (int j = 0; j < numberOfLights; ++j)
				{
					Light light;

					light.type = static_cast<Light::Type>(iff.read_int8());
					light.diffuseColor = iff.read_floatVectorArgb();
					light.specularColor = iff.read_floatVectorArgb();
					light.transform = iff.read_floatTransform();
					light.constantAttenuation = iff.read_float();
					light.linearAttenuation = iff.read_float();
					light.quadraticAttenuation = iff.read_float();

					// hack to deal with bad data coming out of maya exporter
					light.transform.yaw_l(PI);

					m_lightList->push_back(light);
				}
			}
		iff.exitChunk(TAG_LGHT);

	iff.exitForm(TAG_0005);
}

// ----------------------------------------------------------------------

const char *PortalPropertyTemplateCell::getName() const
{
	return m_name;
}

// ----------------------------------------------------------------------

const char *PortalPropertyTemplateCell::getAppearanceName() const
{
	return m_appearanceName;
}

// ----------------------------------------------------------------------

bool PortalPropertyTemplateCell::canSeeParentCell() const
{
	return m_canSeeParentCell;
}

// ----------------------------------------------------------------------

PortalPropertyTemplateCell::PortalList *PortalPropertyTemplateCell::createPortalList(CellProperty *parentCell, Object *relativeToObject) const
{
	PortalList *portalList = new PortalList;
	portalList->reserve(m_portalList->size());

	// create all the portals for the cell
	PortalPropertyTemplateCellPortalList::const_iterator iEnd = m_portalList->end();
	for (PortalPropertyTemplateCellPortalList::const_iterator i = m_portalList->begin(); i != iEnd; ++i)
		portalList->push_back(new Portal(**i, parentCell, relativeToObject));

	return portalList;
}

// ----------------------------------------------------------------------

const char *PortalPropertyTemplateCell::getFloorName() const
{
	return m_floorName;
}

// ----------------------------------------------------------------------

FloorMesh const * PortalPropertyTemplateCell::getFloorMesh() const
{
	if(m_floorMesh == nullptr)
	{
		if(m_floorName)
		{
			PortalPropertyTemplateCell * cell = const_cast<PortalPropertyTemplateCell*>(this);

			cell->m_floorMesh = FloorMeshList::fetch(m_floorName);
		}
		else
		{
			//-- cell template r0 always has a nullptr floor name for POBs without accessible exteriors (POB ships & POB-only dungeons)
			//-- don't warn for that cell template
			DEBUG_WARNING(ConfigSharedCollision::getReportWarnings() && strcmp(m_name, "r0"),
				("PortalPropertyTemplateCell::getFloorMesh() - Cell template %s on [%s] has no floor name", m_name, m_appearanceName));
		}
	}

	return m_floorMesh;
}


// ----------------------------------------------------------------------

PortalPropertyTemplateCell::LightList const *PortalPropertyTemplateCell::getLightList() const
{
	return m_lightList;
}

// ----------------------------------------------------------------------

const BaseExtent *PortalPropertyTemplateCell::getCollisionExtent() const
{
	return m_collisionExtent;
}

// ----------------------------------------------------------------------

void PortalPropertyTemplateCell::preloadAssets() const
{
	if (!m_preloadManager)
		m_preloadManager = new PreloadManager (this);

	int const numberOfPortals = m_portalList->size();
	for (int i = 0; i < numberOfPortals; ++i)
		(*m_portalList)[i]->preloadAssets();
}

// ----------------------------------------------------------------------

void PortalPropertyTemplateCell::garbageCollect() const
{
	if (m_preloadManager)
	{
		delete m_preloadManager;
		m_preloadManager = 0;
	}

	int const numberOfPortals = m_portalList->size();
	for (int i = 0; i < numberOfPortals; ++i)
		(*m_portalList)[i]->garbageCollect();
}

// ======================================================================

PortalPropertyTemplate::PortalOwners::Owner::Owner()
:
	cell(-1),
	portal(-1)
{
}

// ======================================================================

void PortalPropertyTemplate::install(char const *const pobEjectionTransformFilename)
{
	FATAL(s_installed, ("PortalPropertyTemplate already installed."));

	//-- Load the pob-specific ejection point information if specified.
	//   Only the server needs to load this data for SWG at the time of this writing.
	if (pobEjectionTransformFilename)
		loadEjectionPointData(pobEjectionTransformFilename);

	s_installed = true;
	ExitChain::add(PortalPropertyTemplateNamespace::remove, "PortalPropertyTemplate");
}

// ----------------------------------------------------------------------

bool PortalPropertyTemplate::extractPortalLayoutCrc (const char *fileName, uint32 &portalLayoutCrc)
{
	Iff iff;
	if (iff.open(fileName, true))
	{
		iff.enterForm(TAG_PRTO);

		switch (iff.getCurrentName())
		{
			case TAG_0000:
				return false;

			case TAG_0001:
				iff.enterForm(TAG_0001);
				if (iff.seekChunk(TAG_CRC))
				{
					iff.enterChunk(TAG_CRC);
						portalLayoutCrc = iff.read_uint32();
					iff.exitChunk(TAG_CRC);
					return true;
				}
				return false;

			case TAG_0002:
				iff.enterForm(TAG_0002);
				if (iff.seekChunk(TAG_CRC))
				{
					iff.enterChunk(TAG_CRC);
						portalLayoutCrc = iff.read_uint32();
					iff.exitChunk(TAG_CRC);
					return true;
				}
				return false;

			case TAG_0003:
				iff.enterForm(TAG_0003);
				if (iff.seekChunk(TAG_CRC))
				{
					iff.enterChunk(TAG_CRC);
						portalLayoutCrc = iff.read_uint32();
					iff.exitChunk(TAG_CRC);
					return true;
				}
				return false;

			case TAG_0004:
				iff.enterForm(TAG_0004);
				if (iff.seekChunk(TAG_CRC))
				{
					iff.enterChunk(TAG_CRC);
						portalLayoutCrc = iff.read_uint32();
					iff.exitChunk(TAG_CRC);
					return true;
				}
				return false;

			default:
				return false;
		}
	}

	return false;
}

// ======================================================================

PortalPropertyTemplate::PortalPropertyTemplate(const CrcString &name)
:
	m_referenceCount(0),
	m_name(name),
	m_shortName(),
	m_portalGeometryList(new PortalGeometryList),
	m_portalOwnersList(new PortalOwnersList),
	m_cellList(new CellList),
	m_cellNameList(new CellNameList),
	m_crc(0),
	m_pathGraph(nullptr),
	m_radarPortalGeometry(0)
{
	FileName shortName(name.getString());
	shortName.stripPathAndExt();
	m_shortName.set(shortName, true);

	Iff iff(m_name.getString());
	load(iff);
	buildRadarPortalGeometry();
}

// ----------------------------------------------------------------------

PortalPropertyTemplate::~PortalPropertyTemplate()
{
	PortalPropertyTemplateList::erase(*this);

	{
		while (!m_portalGeometryList->empty())
		{
			delete m_portalGeometryList->back();
			m_portalGeometryList->pop_back();
		}
		delete m_portalGeometryList;
	}

	{
		while (!m_cellList->empty())
		{
			delete m_cellList->back();
			m_cellList->pop_back();
		}
		delete m_cellList;
	}

	delete m_cellNameList;
	delete m_portalOwnersList;

	delete m_pathGraph;
	m_pathGraph = nullptr;

	delete m_radarPortalGeometry;
	m_radarPortalGeometry = 0;
}

// ----------------------------------------------------------------------

void PortalPropertyTemplate::release() const
{
	if (--m_referenceCount <= 0)
	{
		DEBUG_FATAL(m_referenceCount < 0, ("reference count went negative"));
		delete const_cast<PortalPropertyTemplate*>(this);
	}
}

// ----------------------------------------------------------------------

void PortalPropertyTemplate::preloadAssets() const
{
	int const numberOfCells = getNumberOfCells();
	for (int i = 0; i < numberOfCells; ++i)
		getCell(i).preloadAssets();
}

// ----------------------------------------------------------------------

void PortalPropertyTemplate::garbageCollect() const
{
	int const numberOfCells = getNumberOfCells();
	for (int i = 0; i < numberOfCells; ++i)
		getCell(i).garbageCollect();
}

// ----------------------------------------------------------------------

IndexedTriangleList const * PortalPropertyTemplate::getPortalGeometry(int portalIndex) const
{
	DEBUG_FATAL(portalIndex < 0 || portalIndex >= static_cast<int>(m_portalGeometryList->size()), ("index out of range %d/%d", portalIndex, static_cast<int>(m_portalGeometryList->size())));
	return (*m_portalGeometryList)[static_cast<PortalGeometryList::size_type>(portalIndex)];
}

// ----------------------------------------------------------------------

IndexedTriangleList const * PortalPropertyTemplate::getPortalGeometry(int portalIndex, int cell, int cellPortalIndex, bool clockwise) const
{
	DEBUG_FATAL(portalIndex < 0 || portalIndex >= static_cast<int>(m_portalGeometryList->size()), ("index out of range %d/%d", portalIndex, static_cast<int>(m_portalGeometryList->size())));
	PortalOwners::Owner &owner = (*m_portalOwnersList)[static_cast<PortalOwnersList::size_type>(portalIndex)].owners[clockwise ? 1 : 0];
	owner.cell = cell;
	owner.portal = cellPortalIndex;
	return (*m_portalGeometryList)[static_cast<PortalGeometryList::size_type>(portalIndex)];
}

// ----------------------------------------------------------------------

void PortalPropertyTemplate::load(Iff &iff)
{
	iff.enterForm(TAG_PRTO);

		switch (iff.getCurrentName())
		{
			case TAG_0000:
				load_0000(iff);
				break;

			case TAG_0001:
				load_0001(iff);
				break;

			case TAG_0002:
				load_0002(iff);
				break;

			case TAG_0003:
				load_0003(iff);
				break;

			case TAG_0004:
				load_0004(iff);
				break;

			// **************************************************************************************************************
			// NOTE! If you add new load functions, you MUST also ensure that PortalPropertyTemplate::extractPortalLayoutCrc() works!
			// **************************************************************************************************************

			default:
				{
					char buffer[256];
					iff.formatLocation(buffer, sizeof(buffer));
					DEBUG_FATAL(true, ("Unknown portal object version at %s", buffer));
				}
		}

	iff.exitForm(TAG_PRTO);
}

// ----------------------------------------------------------------------

void PortalPropertyTemplate::load_0000(Iff &iff)
{
	iff.enterForm(TAG_0000);

		iff.enterChunk(TAG_DATA);
			const int numberOfPortals = iff.read_int32();
			const int numberOfCells = iff.read_int32();
		iff.exitChunk(TAG_DATA);

		// load all the portal geometry
		iff.enterForm(TAG_PRTS);
			m_portalGeometryList->reserve(static_cast<uint>(numberOfPortals));
			m_portalOwnersList->resize(static_cast<uint>(numberOfPortals));
			for (int i = 0; i < numberOfPortals; ++i)
			{
				iff.enterChunk(TAG_PRTL);

					VertexList vertexList;

					const int numberOfVertices = iff.read_int32();
					vertexList.reserve(static_cast<uint>(numberOfVertices));
					for (int j = 0; j < numberOfVertices; ++j)
						vertexList.push_back(iff.read_floatVector());

					IndexedTriangleList * const portalGeometry = createPortalGeometry(vertexList);
					m_portalGeometryList->push_back(portalGeometry);

#ifdef _DEBUG
					validateCoplanar(*portalGeometry, iff.getFileName());
#endif

					scalePortalVertices(*portalGeometry);

				iff.exitChunk(TAG_PRTL);
			}
		iff.exitForm(TAG_PRTS);

		// load all the cells
		iff.enterForm(TAG_CELS);
			m_cellList->reserve(static_cast<uint>(numberOfCells));
			for (int j = 0; j < numberOfCells; ++j)
			{
				Cell *cell = new Cell(*this, j, iff);
				m_cellList->push_back(cell);

				if (j > 0)
				{
					const char *cellName = cell->getName();
					if (cellName)
						m_cellNameList->push_back(cellName);
				}
			}
		iff.exitForm(TAG_CELS);

		DEBUG_WARNING(true, ("PortalProperty::load_0000 - %s does not have a portal property crc and should be re-exported", iff.getFileName ()));

#ifdef _DEBUG
		{
			// verify all the portals have two matches
			PortalOwnersList::const_iterator const iEnd = m_portalOwnersList->end();
			for (PortalOwnersList::const_iterator k = m_portalOwnersList->begin(); k != iEnd; ++k)
			{
				PortalOwners const &owners = *k;
				DEBUG_FATAL(owners.owners[0].cell < 0 || owners.owners[0].portal < 0, ("Could not find counter-clockwise owner for portal"));
				DEBUG_FATAL(owners.owners[1].cell < 0 || owners.owners[1].portal < 0, ("Could not find clockwise owner for portal"));
			}
		}
#endif

	iff.exitForm(TAG_0000);
}

// ----------------------------------------------------------------------

void PortalPropertyTemplate::load_0001(Iff &iff)
{
	iff.enterForm(TAG_0001);

		iff.enterChunk(TAG_DATA);
			const int numberOfPortals = iff.read_int32();
			const int numberOfCells = iff.read_int32();
		iff.exitChunk(TAG_DATA);

		// load all the portal geometry
		iff.enterForm(TAG_PRTS);
			m_portalGeometryList->reserve(static_cast<uint>(numberOfPortals));
			m_portalOwnersList->resize(static_cast<uint>(numberOfPortals));
			for (int i = 0; i < numberOfPortals; ++i)
			{
				iff.enterChunk(TAG_PRTL);

					VertexList vertexList;

					const int numberOfVertices = iff.read_int32();
					vertexList.reserve(static_cast<uint>(numberOfVertices));
					for (int j = 0; j < numberOfVertices; ++j)
						vertexList.push_back(iff.read_floatVector());

					IndexedTriangleList * const portalGeometry = createPortalGeometry(vertexList);
					m_portalGeometryList->push_back(portalGeometry);

#ifdef _DEBUG
					validateCoplanar(*portalGeometry, iff.getFileName());
#endif

					scalePortalVertices(*portalGeometry);

				iff.exitChunk(TAG_PRTL);
			}
		iff.exitForm(TAG_PRTS);

		// load all the cells
		iff.enterForm(TAG_CELS);
			m_cellList->reserve(static_cast<uint>(numberOfCells));
			for (int j = 0; j < numberOfCells; ++j)
			{
				Cell *cell = new Cell(*this, j, iff);
				m_cellList->push_back(cell);

				if (j > 0)
				{
					const char *cellName = cell->getName();
					if (cellName)
						m_cellNameList->push_back(cellName);
				}
			}
		iff.exitForm(TAG_CELS);

		iff.enterChunk(TAG_CRC);
			m_crc = iff.read_int32();
		iff.exitChunk(TAG_CRC);

#ifdef _DEBUG
		{
			// verify all the portals have two matches
			PortalOwnersList::const_iterator const iEnd = m_portalOwnersList->end();
			for (PortalOwnersList::const_iterator k = m_portalOwnersList->begin(); k != iEnd; ++k)
			{
				PortalOwners const &owners = *k;
				DEBUG_FATAL(owners.owners[0].cell < 0 || owners.owners[0].portal < 0, ("Could not find counter-clockwise owner for portal"));
				DEBUG_FATAL(owners.owners[1].cell < 0 || owners.owners[1].portal < 0, ("Could not find clockwise owner for portal"));
			}
		}
#endif

	iff.exitForm(TAG_0001);
}

// ----------------------------------------------------------------------

void PortalPropertyTemplate::load_0002(Iff &iff)
{
	iff.enterForm(TAG_0002);

		iff.enterChunk(TAG_DATA);
			const int numberOfPortals = iff.read_int32();
			const int numberOfCells = iff.read_int32();
		iff.exitChunk(TAG_DATA);

		// load all the portal geometry
		iff.enterForm(TAG_PRTS);
			m_portalGeometryList->reserve(static_cast<uint>(numberOfPortals));
			m_portalOwnersList->resize(static_cast<uint>(numberOfPortals));
			for (int i = 0; i < numberOfPortals; ++i)
			{
				iff.enterChunk(TAG_PRTL);

					VertexList vertexList;

					const int numberOfVertices = iff.read_int32();
					vertexList.reserve(static_cast<uint>(numberOfVertices));
					for (int j = 0; j < numberOfVertices; ++j)
						vertexList.push_back(iff.read_floatVector());

					IndexedTriangleList * const portalGeometry = createPortalGeometry(vertexList);
					m_portalGeometryList->push_back(portalGeometry);

#ifdef _DEBUG
					validateCoplanar(*portalGeometry, iff.getFileName());
#endif

					scalePortalVertices(*portalGeometry);

				iff.exitChunk(TAG_PRTL);
			}
		iff.exitForm(TAG_PRTS);

		// load all the cells
		iff.enterForm(TAG_CELS);
			m_cellList->reserve(static_cast<uint>(numberOfCells));
			for (int j = 0; j < numberOfCells; ++j)
			{
				Cell *cell = new Cell(*this, j, iff);
				m_cellList->push_back(cell);

				if (j > 0)
				{
					const char *cellName = cell->getName();
					if (cellName)
						m_cellNameList->push_back(cellName);
				}
			}
		iff.exitForm(TAG_CELS);

		if(!iff.atEndOfForm() && iff.getCurrentName() == TAG_PGRF)
		{
			ObjectFactory pathGraphFactory = FloorManager::getPathGraphFactory();

			if(pathGraphFactory)
			{
				BaseClass * pathGraph = pathGraphFactory(iff);

				attachBuildingPathGraph(pathGraph);
			}
			else
			{
				// We have a path graph to load but no factory to load it 
				// with, so skip the form

				iff.enterForm();
				iff.exitForm(true);
			}
		}
	
		iff.enterChunk(TAG_CRC);
			m_crc = iff.read_int32();
		iff.exitChunk(TAG_CRC);

#ifdef _DEBUG
		{
			// verify all the portals have two matches
			PortalOwnersList::const_iterator const iEnd = m_portalOwnersList->end();
			for (PortalOwnersList::const_iterator k = m_portalOwnersList->begin(); k != iEnd; ++k)
			{
				PortalOwners const &owners = *k;
				DEBUG_FATAL(owners.owners[0].cell < 0 || owners.owners[0].portal < 0, ("Could not find counter-clockwise owner for portal"));
				DEBUG_FATAL(owners.owners[1].cell < 0 || owners.owners[1].portal < 0, ("Could not find clockwise owner for portal"));
			}
		}
#endif

	iff.exitForm(TAG_0002);

	// ----------
	// Version 2 pobs have their passable flags flipped 

	{
		for(uint i = 0; i < m_cellList->size(); i++)
		{
			PortalPropertyTemplateCell * cell = m_cellList->at(i);

			for(uint j = 0; j < cell->m_portalList->size(); j++)
			{
				PortalPropertyTemplateCellPortal * portal = cell->m_portalList->at(j);

				portal->flipPassable();
			}
		}
	}
}

// ----------------------------------------------------------------------

void PortalPropertyTemplate::load_0003(Iff &iff)
{
	iff.enterForm(TAG_0003);

		iff.enterChunk(TAG_DATA);
			const int numberOfPortals = iff.read_int32();
			const int numberOfCells = iff.read_int32();
		iff.exitChunk(TAG_DATA);

		// load all the portal geometry
		iff.enterForm(TAG_PRTS);
			m_portalGeometryList->reserve(static_cast<uint>(numberOfPortals));
			m_portalOwnersList->resize(static_cast<uint>(numberOfPortals));
			for (int i = 0; i < numberOfPortals; ++i)
			{
				iff.enterChunk(TAG_PRTL);

					VertexList vertexList;

					const int numberOfVertices = iff.read_int32();
					vertexList.reserve(static_cast<uint>(numberOfVertices));
					for (int j = 0; j < numberOfVertices; ++j)
						vertexList.push_back(iff.read_floatVector());

					IndexedTriangleList * const portalGeometry = createPortalGeometry(vertexList);
					m_portalGeometryList->push_back(portalGeometry);

#ifdef _DEBUG
					validateCoplanar(*portalGeometry, iff.getFileName());
#endif

					scalePortalVertices(*portalGeometry);

				iff.exitChunk(TAG_PRTL);
			}
		iff.exitForm(TAG_PRTS);

		// load all the cells
		iff.enterForm(TAG_CELS);
			m_cellList->reserve(static_cast<uint>(numberOfCells));
			for (int j = 0; j < numberOfCells; ++j)
			{
				Cell *cell = new Cell(*this, j, iff);
				m_cellList->push_back(cell);

				if (j > 0)
				{
					const char *cellName = cell->getName();
					if (cellName)
						m_cellNameList->push_back(cellName);
				}
			}
		iff.exitForm(TAG_CELS);

		if(!iff.atEndOfForm() && iff.getCurrentName() == TAG_PGRF)
		{
			ObjectFactory pathGraphFactory = FloorManager::getPathGraphFactory();

			if(pathGraphFactory)
			{
				BaseClass * pathGraph = pathGraphFactory(iff);

				attachBuildingPathGraph(pathGraph);
			}
			else
			{
				// We have a path graph to load but no factory to load it 
				// with, so skip the form

				iff.enterForm();
				iff.exitForm(true);
			}
		}
	
		iff.enterChunk(TAG_CRC);
			m_crc = iff.read_int32();
		iff.exitChunk(TAG_CRC);

#ifdef _DEBUG
		{
			// verify all the portals have two matches
			PortalOwnersList::const_iterator const iEnd = m_portalOwnersList->end();
			for (PortalOwnersList::const_iterator k = m_portalOwnersList->begin(); k != iEnd; ++k)
			{
				PortalOwners const &owners = *k;
				DEBUG_FATAL(owners.owners[0].cell < 0 || owners.owners[0].portal < 0, ("Could not find counter-clockwise owner for portal"));
				DEBUG_FATAL(owners.owners[1].cell < 0 || owners.owners[1].portal < 0, ("Could not find clockwise owner for portal"));
			}
		}
#endif

	iff.exitForm(TAG_0003);
}

// ----------------------------------------------------------------------

void PortalPropertyTemplate::load_0004(Iff &iff)
{
	iff.enterForm(TAG_0004);

		iff.enterChunk(TAG_DATA);
			const int numberOfPortals = iff.read_int32();
			const int numberOfCells = iff.read_int32();
		iff.exitChunk(TAG_DATA);

		// load all the portal geometry
		iff.enterForm(TAG_PRTS);
			m_portalGeometryList->reserve(static_cast<uint>(numberOfPortals));
			m_portalOwnersList->resize(static_cast<uint>(numberOfPortals));
			for (int i = 0; i < numberOfPortals; ++i)
			{
				IndexedTriangleList * const portalGeometry = new IndexedTriangleList(iff);
				m_portalGeometryList->push_back(portalGeometry);

				scalePortalVertices(*portalGeometry);
			}
		iff.exitForm(TAG_PRTS);

		// load all the cells
		iff.enterForm(TAG_CELS);
			m_cellList->reserve(static_cast<uint>(numberOfCells));
			for (int j = 0; j < numberOfCells; ++j)
			{
				Cell *cell = new Cell(*this, j, iff);
				m_cellList->push_back(cell);

				if (j > 0)
				{
					const char *cellName = cell->getName();
					if (cellName)
						m_cellNameList->push_back(cellName);
				}
			}
		iff.exitForm(TAG_CELS);

		if(!iff.atEndOfForm() && iff.getCurrentName() == TAG_PGRF)
		{
			ObjectFactory pathGraphFactory = FloorManager::getPathGraphFactory();

			if(pathGraphFactory)
			{
				BaseClass * pathGraph = pathGraphFactory(iff);

				attachBuildingPathGraph(pathGraph);
			}
			else
			{
				// We have a path graph to load but no factory to load it 
				// with, so skip the form

				iff.enterForm();
				iff.exitForm(true);
			}
		}
	
		iff.enterChunk(TAG_CRC);
			m_crc = iff.read_int32();
		iff.exitChunk(TAG_CRC);

#ifdef _DEBUG
		{
			// verify all the portals have two matches
			PortalOwnersList::const_iterator const iEnd = m_portalOwnersList->end();
			for (PortalOwnersList::const_iterator k = m_portalOwnersList->begin(); k != iEnd; ++k)
			{
				PortalOwners const &owners = *k;
				DEBUG_FATAL(owners.owners[0].cell < 0 || owners.owners[0].portal < 0, ("Could not find counter-clockwise owner for portal"));
				DEBUG_FATAL(owners.owners[1].cell < 0 || owners.owners[1].portal < 0, ("Could not find clockwise owner for portal"));
			}
		}
#endif

	iff.exitForm(TAG_0004);
}

// ----------------------------------------------------------------------

int PortalPropertyTemplate::getCrc() const
{
	return m_crc;
}

// ----------------------------------------------------------------------

int PortalPropertyTemplate::getNumberOfCells() const
{
	return static_cast<int>(m_cellList->size());
}

// ----------------------------------------------------------------------

const PortalPropertyTemplate::CellNameList &PortalPropertyTemplate::getCellNames() const
{
	NOT_NULL(m_cellNameList);
	return *m_cellNameList;
}

// ----------------------------------------------------------------------

const PortalPropertyTemplateCell &PortalPropertyTemplate::getCell(int cellIndex) const
{
	DEBUG_FATAL(cellIndex < 0 || cellIndex >= getNumberOfCells(), ("cell index out of range %d/%d", cellIndex, getNumberOfCells()));
	NOT_NULL((*m_cellList)[static_cast<CellList::size_type>(cellIndex)]);
	return *(*m_cellList)[static_cast<CellList::size_type>(cellIndex)];
}

// ----------------------------------------------------------------------

BaseClass const *PortalPropertyTemplate::getBuildingPathGraph() const
{
	return m_pathGraph;
}

// ----------

void PortalPropertyTemplate::attachBuildingPathGraph(BaseClass *newPathGraph)
{
	if(g_expandBuildingGraphHook)
	{
		delete m_pathGraph;
		m_pathGraph = g_expandBuildingGraphHook(this,newPathGraph);

		delete newPathGraph;
	}
	else
	{
		if(m_pathGraph != newPathGraph)
		{
			delete m_pathGraph;
			m_pathGraph = newPathGraph;
		}
	}
}

// ----------------------------------------------------------------------

const char *PortalPropertyTemplate::getExteriorAppearanceName() const
{
	return (*m_cellList)[0]->getAppearanceName();
}

// ----------------------------------------------------------------------

const char *PortalPropertyTemplate::getExteriorFloorName() const
{
	return (*m_cellList)[0]->getFloorName();
}

// ----------------------------------------------------------------------

PortalPropertyTemplate::PortalList *PortalPropertyTemplate::createExteriorPortalList(CellProperty *parentCell, Object *relativeToObject) const
{
	return (*m_cellList)[0]->createPortalList(parentCell, relativeToObject);
}

// ----------------------------------------------------------------------

Transform const PortalPropertyTemplate::getEjectionLocationTransform() const
{
	NameTransformMap::iterator const findIt = s_ejectionTransformMap.find(m_name);
	if (findIt != s_ejectionTransformMap.end())
	{
		//-- Return the data-defined ejection point for this POB.
		return findIt->second;
	}
	else
	{
		//-- Return the transform for the first door to the outside world, scooted forward a few meters.
		Transform result = (*m_cellList)[0]->m_portalList->operator[](0)->getDoorTransform(false);
		result.move_l(Vector(0.0f, 0.0f, 5.0f));
		return result;
	}
}

// ----------------------------------------------------------------------

const PortalPropertyTemplate::VertexList *PortalPropertyTemplate::getRadarPortalGeometry() const
{
	return m_radarPortalGeometry;
}

// ----------------------------------------------------------------------

void PortalPropertyTemplate::buildRadarPortalGeometry()
{
	uint i;
	for (i = 0; i < m_portalOwnersList->size(); ++i)
	{
		const PortalPropertyTemplate::PortalOwners& portalOwners = (*m_portalOwnersList)[i];
		if (portalOwners.owners[0].cell == 0 || portalOwners.owners[1].cell == 0) 
		{
			IndexedTriangleList const * const portalGeometry = NON_NULL(getPortalGeometry(static_cast<int>(i)));
			std::vector<Vector> const & vertices = portalGeometry->getVertices();
			std::vector<int> const & indices = portalGeometry->getIndices();
			uint const numberOfFaces = indices.size() / 3;

			if (!m_radarPortalGeometry)
				m_radarPortalGeometry = new std::vector<Vector>;

			for (uint faceIndex = 0, index = 0; faceIndex < numberOfFaces; ++faceIndex)
			{
				Vector const & v0 = vertices[indices[index++]];
				Vector const & v1 = vertices[indices[index++]];
				Vector const & v2 = vertices[indices[index++]];

				m_radarPortalGeometry->push_back(v0);
				m_radarPortalGeometry->push_back(v1);
				m_radarPortalGeometry->push_back(v1);
				m_radarPortalGeometry->push_back(v2);
				m_radarPortalGeometry->push_back(v2);
				m_radarPortalGeometry->push_back(v0);
			}
		}
	}
}

// ----------------------------------------------------------------------

const CrcString &PortalPropertyTemplate::getShortName() const
{
	return m_shortName;
}

// ----------------------------------------------------------------------

void PortalPropertyTemplate::setExpandBuildingGraphHook ( ExpandBuildingGraphHook hook )
{
	g_expandBuildingGraphHook = hook;
}

// ======================================================================
