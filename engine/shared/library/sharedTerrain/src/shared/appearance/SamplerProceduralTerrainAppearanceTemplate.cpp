//===================================================================
//
// SamplerProceduralTerrainAppearanceTemplate.cpp
// asommers
//
// copyright 2000, verant interactive
//
//===================================================================

#include "sharedTerrain/FirstSharedTerrain.h"
#include "sharedTerrain/SamplerProceduralTerrainAppearanceTemplate.h"
#include "sharedTerrain/SamplerProceduralTerrainAppearance.h"

#include "sharedFoundation/ExitChain.h"
#include "sharedUtility/PackedIntegerMap.h"
#include "sharedUtility/PackedFixedPointMap.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedTerrain/ConfigSharedTerrain.h"
#include "fileInterface/StdioFile.h"
#include "sharedFile/TreeFile.h"

#include <malloc.h>
#if defined(PLATFORM_LINUX)
#include <alloca.h>
#endif

//===================================================================

struct StaticFloraSampleFileHeader
{
	char  terrainName[64];
	float floraTileWidthInMeters;
	int   numberOfFloraSampled;
};

struct StaticFloraSampleHeightPlaneHeader
{
	int   version;
	float minHeight;
	float maxHeight;
	float scale;
	int   nibblesPerEntry;
};

struct CollidableFloraSampleFileHeader
{
	uint32 type;
	int   version;
	char  terrainName[64];
	float floraTileWidthInMeters;
	int   minFloraTileX;
	int   minFloraTileZ;
	int   maxFloraTileX;
	int   maxFloraTileZ;
	int   numberOfFloraSampled;
	float minHeight;
	float maxHeight;
};

//===================================================================

AppearanceTemplate* SamplerProceduralTerrainAppearanceTemplate::create (const char* filename, Iff* iff)
{
	return new SamplerProceduralTerrainAppearanceTemplate (filename, iff);
}

//===================================================================

SamplerProceduralTerrainAppearanceTemplate::SamplerProceduralTerrainAppearanceTemplate (const char* filename, Iff* iff) :
	ProceduralTerrainAppearanceTemplate (filename, iff, false, 0, 1, true),
	m_staticCollidableFloraHeightMin(0),
	m_staticCollidableFloraHeightMax(0),
	m_staticCollidableFloraHeightSamples(0),
	m_staticCollidableFloraSamples(0),
	m_numberOfSampledFlora(0)
{
	if (m_legacyMap)
	{
		_setSamplingParameters(true, 3, 3);
	}
}

//-------------------------------------------------------------------

SamplerProceduralTerrainAppearanceTemplate::~SamplerProceduralTerrainAppearanceTemplate ()
{
	delete [] m_staticCollidableFloraHeightSamples;
	m_staticCollidableFloraHeightSamples=0;

	delete [] m_staticCollidableFloraSamples;
	m_staticCollidableFloraSamples=0;
}

//-------------------------------------------------------------------

Appearance* SamplerProceduralTerrainAppearanceTemplate::createAppearance () const
{
	return new SamplerProceduralTerrainAppearance (const_cast<SamplerProceduralTerrainAppearanceTemplate *>(this));
}

//-------------------------------------------------------------------

void SamplerProceduralTerrainAppearanceTemplate::setMapsToSample(unsigned maps)
{
	m_terrainGenerator->setMapsToSample(maps);
}

//-------------------------------------------------------------------

bool SamplerProceduralTerrainAppearanceTemplate::getFloraHeightSample(int key, float &o_height) const
{
	if (!m_staticCollidableFloraHeightSamples)
	{
		o_height=0;
		return false;
	}
	else
	{
		o_height = m_staticCollidableFloraHeightSamples[key];
		return true;
	}
}

//-------------------------------------------------------------------

void SamplerProceduralTerrainAppearanceTemplate::beginStaticCollidableFloraSampling()
{
	m_staticCollidableFloraHeightMin      =  FLT_MAX;
	m_staticCollidableFloraHeightMax      = -FLT_MAX;

	const int numElements = sqr(getMapWidthInFlora());

	float *staticCollidableFloraHeightSamples = new float[numElements];
	memset(staticCollidableFloraHeightSamples, 0, numElements*sizeof(*staticCollidableFloraHeightSamples));
	m_staticCollidableFloraHeightSamples=staticCollidableFloraHeightSamples;

	FloraGroup::Info *staticCollidableFloraSamples = new FloraGroup::Info[numElements];
	memset(staticCollidableFloraSamples, 0, numElements*sizeof(*staticCollidableFloraSamples));
	m_staticCollidableFloraSamples = staticCollidableFloraSamples;

	m_numberOfSampledFlora=0;
}

//-------------------------------------------------------------------

bool SamplerProceduralTerrainAppearanceTemplate::loadStaticCollidableFloraFile(const char *i_filename)
{
	if (!m_staticCollidableFloraHeightSamples)
	{
		beginStaticCollidableFloraSampling();
	}

	// -------------------------------------------------------

	AbstractFile *mapFile = TreeFile::open(i_filename, AbstractFile::PriorityData, true);
	if (!mapFile)
	{
		REPORT_LOG_PRINT(true, ("Could not open sample file %s\n", i_filename));
		return false;
	}

	// -------------------------------------------------------

	CollidableFloraSampleFileHeader fileHeader;
	mapFile->read(&fileHeader, sizeof(fileHeader));

	if (fileHeader.type!=TGM_floraStaticCollidable)
	{
		return false;
	}

	if (fileHeader.version!=1)
	{
		return false;
	}

	if (fileHeader.floraTileWidthInMeters!=getFloraTileWidthInMeters())
	{
		return false;
	}

	// -------------------------------------------------------

	int xWidth = fileHeader.maxFloraTileX - fileHeader.minFloraTileX;
	int z;
	for (z=fileHeader.minFloraTileZ;z<fileHeader.maxFloraTileZ;z++)
	{
		FloraGroup::Info *row = m_staticCollidableFloraSamples + z*getMapWidthInFlora();
		mapFile->read(row+fileHeader.minFloraTileX, xWidth*sizeof(*row));
	}

	for (z=fileHeader.minFloraTileZ;z<fileHeader.maxFloraTileZ;z++)
	{
		float *row = m_staticCollidableFloraHeightSamples + z*getMapWidthInFlora();
		mapFile->read(row+fileHeader.minFloraTileX, xWidth*sizeof(*row));
	}

	// -------------------------------------------------------


	delete mapFile;

	return true;
}

//-------------------------------------------------------------------

void SamplerProceduralTerrainAppearanceTemplate::setFloraTileSample(int key, const FloraGroup::Info &i_data, float height)
{
	FloraGroup::Info &info = m_staticCollidableFloraSamples[key];
	if (!info.getFamilyId())
	{
		info=i_data;
		if (i_data.getFamilyId())
		{
			if (height>m_staticCollidableFloraHeightMax)
			{
				m_staticCollidableFloraHeightMax=height;
			}
			if (height<m_staticCollidableFloraHeightMin)
			{
				m_staticCollidableFloraHeightMin=height;
			}

			m_staticCollidableFloraHeightSamples[key]=height;

			m_numberOfSampledFlora++;
		}
	}
}

//-------------------------------------------------------------------

static bool _extractPlanetName(char *o_buffer, const char *i_filename)
{
	o_buffer[0]=0;
	if (!i_filename || !*i_filename || i_filename[0]=='.')
	{
		return false;
	}

	const char *iter;

	iter=strrchr(i_filename, 0);
	while (iter>i_filename)
	{
		if (  iter[-1]=='/' 
			|| iter[-1]=='\\'
			|| iter[-1]==':'
			)
		{
			break;
		}
		iter--;
	}

	// iter should now point to beginning of planet name
	char *oiter=o_buffer;
	while (*iter && *iter!='.')
	{
		*oiter++=*iter++;
	}
	*oiter++=0;

	return true;
}

//-------------------------------------------------------------------

bool SamplerProceduralTerrainAppearanceTemplate::writeStaticCollidableFloraFile(const char *i_filename, int tileBounds[4]) const
{
	if (!m_samplingMode)
	{
		return false;
	}
	AbstractFile *of = StdioFileFactory().createFile(i_filename, "wb");
	if (!of)
	{
		return false;
	}

	char planetname[256];
	strcpy(planetname, "terrain\\");
	if (!_extractPlanetName(strrchr(planetname, 0), i_filename))
	{
		return false;
	}
	strcat(planetname, ".trn");

	// --------------------------------------------

	CollidableFloraSampleFileHeader fileHeader;
	Zero(fileHeader);

	fileHeader.type=TGM_floraStaticCollidable;
	fileHeader.version=1;
	strcpy(fileHeader.terrainName, planetname);
	fileHeader.floraTileWidthInMeters = getFloraTileWidthInMeters();
	fileHeader.minFloraTileX = tileBounds[0];
	fileHeader.minFloraTileZ = tileBounds[1];
	fileHeader.maxFloraTileX = tileBounds[2];
	fileHeader.maxFloraTileZ = tileBounds[3];
	fileHeader.numberOfFloraSampled=0;
	fileHeader.minHeight =  FLT_MAX;
	fileHeader.maxHeight = -FLT_MAX;

	// --------------------------------------------

	int x, z;
	for (z=tileBounds[1];z<tileBounds[3];z++)
	{
		const float *heightRow = m_staticCollidableFloraHeightSamples + z*getMapWidthInFlora();
		for (x=tileBounds[0];x<tileBounds[2];x++)
		{
			if (heightRow[x]<fileHeader.minHeight)
			{
				fileHeader.minHeight=heightRow[x];
			}
			if (heightRow[x]>fileHeader.maxHeight)
			{
				fileHeader.maxHeight=heightRow[x];
			}
		}

		const FloraGroup::Info *infoRow = m_staticCollidableFloraSamples + z*getMapWidthInFlora();
		for (x=tileBounds[0];x<tileBounds[2];x++)
		{
			FloraGroup::Info fgi = infoRow[x];
			if (fgi.getFamilyId())
			{
				fileHeader.numberOfFloraSampled++;
			}
		}
	}

	// --------------------------------------------

	of->write(sizeof(fileHeader), &fileHeader);

	int xWidth = tileBounds[2] - tileBounds[0];
	for (z=tileBounds[1];z<tileBounds[3];z++)
	{
		const FloraGroup::Info *row = m_staticCollidableFloraSamples + z*getMapWidthInFlora();
		of->write(xWidth*sizeof(*row), row+tileBounds[0]);
	}

	for (z=tileBounds[1];z<tileBounds[3];z++)
	{
		const float *row = m_staticCollidableFloraHeightSamples + z*getMapWidthInFlora();
		of->write(xWidth*sizeof(*row), row+tileBounds[0]);
	}

	of->close();

	delete of;

	// --------------------------------------------------------


	return true;
}

//-------------------------------------------------------------------

bool SamplerProceduralTerrainAppearanceTemplate::writeOldStaticCollidableFloraFile(const char *i_filename) const
{
	if (!m_samplingMode)
	{
		return false;
	}
	AbstractFile *of = StdioFileFactory().createFile(i_filename, "wb");
	if (!of)
	{
		return false;
	}

	char planetname[256];
	strcpy(planetname, "terrain\\");
	if (!_extractPlanetName(strrchr(planetname, 0), i_filename))
	{
		return false;
	}
	strcat(planetname, ".trn");

	StaticFloraSampleFileHeader fileHeader;
	Zero(fileHeader);

	strcpy(fileHeader.terrainName, planetname);
	fileHeader.floraTileWidthInMeters = getFloraTileWidthInMeters();
	fileHeader.numberOfFloraSampled=m_numberOfSampledFlora;

	of->write(sizeof(fileHeader), &fileHeader);
	of->write(sqr(getMapWidthInFlora())*sizeof(*m_staticCollidableFloraSamples), m_staticCollidableFloraSamples);
	of->close();
	delete of;

	// --------------------------------------------------------

	char buffer[1024];
	strcpy(buffer, i_filename);
	strcat(buffer, "2");
	writeOldStaticCollidableFloraHeightFile(buffer);

	// --------------------------------------------------------

	return true;
}

//-------------------------------------------------------------------

bool SamplerProceduralTerrainAppearanceTemplate::writeOldStaticCollidableFloraHeightFile(const char *i_filename) const
{
	if (!m_samplingMode)
	{
		return false;
	}
	AbstractFile *of = StdioFileFactory().createFile(i_filename, "wb");
	if (!of)
	{
		return false;
	}

	char planetname[256];
	strcpy(planetname, "terrain\\");
	if (!_extractPlanetName(strrchr(planetname, 0), i_filename))
	{
		return false;
	}
	strcat(planetname, ".trn");

	StaticFloraSampleHeightPlaneHeader fileHeader;
	Zero(fileHeader);

	//strcpy(fileHeader.terrainName, planetname);

	fileHeader.version         = 1;
	fileHeader.minHeight       = m_staticCollidableFloraHeightMin;
	fileHeader.maxHeight       = m_staticCollidableFloraHeightMax;
	fileHeader.scale           = 1.0f / 50.0f;
	fileHeader.nibblesPerEntry = 8;

	of->write(sizeof(fileHeader), &fileHeader);
	size_t heightSize = sqr(getMapWidthInFlora());
	of->write(heightSize*sizeof(*m_staticCollidableFloraHeightSamples), m_staticCollidableFloraHeightSamples);
	of->close();
	delete of;

	return true;
}

//-------------------------------------------------------------------

void SamplerProceduralTerrainAppearanceTemplate::commitStaticCollidableFloraSamples(int *tileBounds)
{
	const int mapWidthInFlora = getMapWidthInFlora();

	if (tileBounds)
	{
		int x, z;

		// initialize everything outside the bounds with the original data.

		const int xstart = (tileBounds) ? clamp(0, tileBounds[0], mapWidthInFlora) : 0;
		const int zstart = (tileBounds) ? clamp(0, tileBounds[1], mapWidthInFlora) : 0;
		const int xstop  = (tileBounds) ? clamp(0, tileBounds[2], mapWidthInFlora) : mapWidthInFlora;
		const int zstop  = (tileBounds) ? clamp(0, tileBounds[3], mapWidthInFlora) : mapWidthInFlora;

		for (z=0;z<mapWidthInFlora;z++)
		{
			if (z>=zstart && z<zstop)
			{
				continue;
			}

			for (x=0;x<xstart;x++)
			{
				int familyId = getStaticCollidableFloraFamily(x, z);
				float height = getStaticCollidableFloraHeight(x, z);
				const int index = z*mapWidthInFlora + x;
				m_staticCollidableFloraSamples[index].setFamilyId(familyId);
				m_staticCollidableFloraHeightSamples[index]=height;
			}
			for (x=xstop;x<mapWidthInFlora;x++)
			{
				int familyId = getStaticCollidableFloraFamily(x, z);
				float height = getStaticCollidableFloraHeight(x, z);
				const int index = z*mapWidthInFlora + x;
				m_staticCollidableFloraSamples[index].setFamilyId(familyId);
				m_staticCollidableFloraHeightSamples[index]=height;
			}
		}
	}

	// --------------------------------------------------------

	_setStaticCollidableFloraInfo(m_staticCollidableFloraSamples, m_staticCollidableFloraHeightSamples);
}

//-------------------------------------------------------------------

//===================================================================
