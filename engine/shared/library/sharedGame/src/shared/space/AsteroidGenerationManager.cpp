//======================================================================
//
// AsteroidGenerationManager.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/AsteroidGenerationManager.h"

#include "sharedDebug/InstallTimer.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedGame/ConfigSharedGame.h"
#include "sharedMath/SpatialSubdivision.h"
#include "sharedMath/WaveForm3D.h"
#include "sharedRandom/RandomGenerator.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"

#include <algorithm>
#include <set>
#include <map>

//======================================================================

std::map<AsteroidGenerationManager::SceneNameKey, std::vector<AsteroidGenerationManager::AsteroidFieldData> > AsteroidGenerationManager::ms_staticFieldData;
SphereTree<Sphere*, AsteroidGenerationManager::AsteroidDataSphereExtentAccessor> AsteroidGenerationManager::ms_collisionSphereTree;
std::vector<SpatialSubdivisionHandle*>   AsteroidGenerationManager::ms_objectsInSphereTree;
AsteroidGenerationManager::GetExtentRadiusFunction AsteroidGenerationManager::ms_getRadiusFunction;

//======================================================================

namespace AsteroidGenerationManagerNamespace
{
	bool s_installed = false;

	const std::string s_baseDir     ("datatables/space/asteroidfield/");
	const std::string s_filenameExt (".iff");

	//this will be reseeded before use
	RandomGenerator s_randomGenerator(0);
}

using namespace AsteroidGenerationManagerNamespace;

//======================================================================

Sphere const AsteroidGenerationManager::AsteroidDataSphereExtentAccessor::getExtent(Sphere const * const sphere)
{
	return *sphere;
}

//-------------------------------------------------------------------

char const *AsteroidGenerationManager::AsteroidDataSphereExtentAccessor::getDebugName(Sphere *)
{
	return 0;
}

//======================================================================

void AsteroidGenerationManager::AsteroidData::clear()
{
	position.makeZero();
	orientation.makeZero();
	scale = 0.0f;
	rotationVector.makeZero();
}

//======================================================================

void AsteroidGenerationManager::install()
{
	InstallTimer const installTimer("AsteroidGenerationManager::install");

	DEBUG_FATAL(s_installed, ("AsteroidGenerationManager already installed"));
	s_installed = true;

	ms_getRadiusFunction = nullptr;

	ExitChain::add(AsteroidGenerationManager::remove, "AsteroidGenerationManager::remove");
}

//----------------------------------------------------------------------

void AsteroidGenerationManager::remove()
{
	ms_getRadiusFunction = nullptr;

	clearStaticFieldData();
	clearInstantiatedData();

	s_installed = false;
}

//----------------------------------------------------------------------

void AsteroidGenerationManager::registerGetExtentRadiusFunction(AsteroidGenerationManager::GetExtentRadiusFunction getRadiusFunction)
{
	ms_getRadiusFunction = getRadiusFunction;
}

//----------------------------------------------------------------------

/** Load up the asteroid data for a given planet.  This does NOT create the actual field (client/server specific code must do this).
*/
void AsteroidGenerationManager::loadStaticFieldDataForScene(std::string const & sceneName, bool server)
{
	if(!s_installed)
	{
		DEBUG_FATAL (true, ("AsteroidGenerationManager not installed"));
		return; //lint !e527 unreachable (reachable in release)
	}

	if (!AsteroidGenerationManager::hasDataForScene(sceneName))
	{
		//-- skip non-space scenes
		if (strncmp(sceneName.c_str(), "space_", 6))
			return;

		std::string tableName = s_baseDir + sceneName + s_filenameExt;
		if(!TreeFile::exists(tableName.c_str()))
		{
			DEBUG_WARNING(true, ("Static asteroid field data not defined for %s", sceneName.c_str()));
			return;
		}

		std::set<uint32> seeds;

		DataTable* table = DataTableManager::getTable(tableName, true);
		if(table)
		{
			const int numRows = table->getNumRows ();

			for (int i = 0; i < numRows; ++i)
			{
				AsteroidFieldData newData;

				newData.templateCrcsAndLikelihood.clear();

				switch (table->getIntValue("Type", i))
				{
				case 1:
					newData.fieldType = AsteroidFieldData::FT_cube;
					break;
				case 2:
					newData.fieldType = AsteroidFieldData::FT_spline;
					break;
				default:
					DEBUG_FATAL(true, ("Row %i in asteroid field table for %s specifies invalid field type %i", i, sceneName.c_str(), table->getIntValue("Type", i)));					
				}
				newData.centerPosition.x = table->getFloatValue("CenterLocationX", i);
				newData.centerPosition.y = table->getFloatValue("CenterLocationY", i);
				newData.centerPosition.z = table->getFloatValue("CenterLocationZ", i);
				newData.radius = table->getFloatValue("Radius", i);
				newData.flattenDepth = table->getFloatValue("FlattenDepth", i);
				newData.count = table->getIntValue("NumAsteroids", i);
				newData.seed  = static_cast<unsigned int>(table->getIntValue("RandomSeed", i));
				newData.scaleMin = table->getFloatValue("ScaleMin", i);
				newData.scaleMax = table->getFloatValue("ScaleMax", i);
				newData.rotationMin = table->getFloatValue("RotationMin", i);
				newData.rotationMax = table->getFloatValue("RotationMax", i);
				std::string const & fieldStyleTableName = table->getStringValue("FieldStyleTable", i);
				std::string const & radialStyleTableName = table->getStringValue("RadialStyleTable", i);
				newData.radialCount = table->getIntValue("NumRadialAsteroids", i);
				newData.radialLikelihoodSum = 0;
				newData.maxViewableDistance = table->getFloatValue("MaxViewableDistance", i);
				newData.viewAllDistance = table->getFloatValue("ViewAllDistance", i);
				newData.soundEffect = table->getStringValue("SoundEffect", i);
				
				// read spline control points
				newData.splineControlPoints.clear();
				if (newData.fieldType == AsteroidFieldData::FT_spline)
				{
					std::string pointsAsString(table->getStringValue("SplineControlPoints", i));
					pointsAsString += ":";
					size_t pos = 0;
					bool done=false;
					while (!done)
					{
						size_t nextColon = pointsAsString.find(':',pos);
						if (nextColon != static_cast<size_t>(std::string::npos))
						{
							Vector onePoint;
							std::string const onePointAsString(pointsAsString,pos,nextColon-pos);
							sscanf(onePointAsString.c_str(),"%f,%f,%f", &onePoint.x, &onePoint.y, &onePoint.z);
							newData.splineControlPoints.push_back(onePoint);
							pos = nextColon+1;
						}
						else
							done = true;
					}

					DEBUG_FATAL(newData.splineControlPoints.size() < 2,("Row %i in asteroid file table for %s specified < 2 spline control points.  At least 2 are required.", i, sceneName.c_str()));

					std::string const facingAsString(table->getStringValue("FaceTowards",i));
					sscanf(facingAsString.c_str(),"%f,%f,%f", &newData.faceTowards.x, &newData.faceTowards.y, &newData.faceTowards.z);
				}
				
				if(seeds.find(newData.seed) != seeds.end())
				{
					DEBUG_FATAL(true, ("Each static asteroid field must have a seed unique to the scene, seed [%d] seed > once for %s", newData.seed, sceneName.c_str()));
					continue; //lint !e527 unreachable (reachable in release)
				}
				IGNORE_RETURN(seeds.insert(newData.seed));

				if(!TreeFile::exists(fieldStyleTableName.c_str()))
				{
					DEBUG_WARNING(true, ("Could not find asteroid field style table %s", fieldStyleTableName.c_str()));
					continue; //lint !e527 unreachable (reachable in release)
				}

				//now read the rest of the data from the field style table

				DataTable* fieldStyleTable = DataTableManager::getTable(fieldStyleTableName, true);
				if(fieldStyleTable)
				{
					const int fieldStyleNumRows = fieldStyleTable->getNumRows ();
					uint32 crc = Crc::crcNull;
					float likelihood = 0.0f;
					for (int j = 0; j < fieldStyleNumRows; ++j)
					{
						if(server)
							crc = Crc::calculate(fieldStyleTable->getStringValue("ServerTemplate", j));
						else
							crc = Crc::calculate(fieldStyleTable->getStringValue("SharedTemplate", j));
						likelihood = fieldStyleTable->getFloatValue("Likelihood", j);
						if(likelihood <= 0.0f)
						{
							continue;
						}
						newData.templateCrcsAndLikelihood.push_back(std::make_pair(crc, likelihood));
					}
				}

				//read the radial data from the radial style table

				if (!server)
				{
					DataTable* radialStyleTable = DataTableManager::getTable(radialStyleTableName, true);
					if(radialStyleTable)
					{
						const int radialStyleNumRows = radialStyleTable->getNumRows ();
						//uint32 crc = Crc::crcNull;
						//float likelihood = 0.0f;
						for (int j = 0; j < radialStyleNumRows; ++j)
						{
							RadialAsteroidData rad;
							rad.appearance  = radialStyleTable->getStringValue("Appearance", j);
							rad.likelihood  = radialStyleTable->getFloatValue ("Likelihood", j);
							rad.scaleMin    = radialStyleTable->getFloatValue ("ScaleMin", j);
							rad.scaleMax    = radialStyleTable->getFloatValue ("ScaleMax", j);
							rad.spawnRange  = radialStyleTable->getFloatValue ("SpawnRange", j);
							rad.deleteRange = radialStyleTable->getFloatValue ("DeleteRange", j);
							rad.hitEffectName = radialStyleTable->getStringValue ("HitEffect", j);

							newData.radialLikelihoodSum += rad.likelihood;

							newData.radialData.push_back(rad);
						}
					}
				}

				ms_staticFieldData[sceneName].push_back(newData);
			}
		}
	}
}

//----------------------------------------------------------------------

void AsteroidGenerationManager::clearStaticFieldData()
{
	typedef std::map<AsteroidGenerationManager::SceneNameKey, std::vector<AsteroidGenerationManager::AsteroidFieldData> > StaticFieldData;

	StaticFieldData::iterator ii = ms_staticFieldData.begin();
	StaticFieldData::iterator iiEnd = ms_staticFieldData.end();

	for (; ii != iiEnd; ++ii)
	{
		ii->second.clear();
	}
}

//----------------------------------------------------------------------

void AsteroidGenerationManager::clearInstantiatedData()
{
	if(!s_installed)
	{
		DEBUG_FATAL (true, ("AsteroidGenerationManager not installed"));
		return; //lint !e527 unreachable (reachable in release)
	}

	SpatialSubdivisionHandle* handle;

	std::vector<std::pair<Sphere *, Sphere> > spheres;
	ms_collisionSphereTree.dumpSphereTreeObjs (spheres);
	for(std::vector<std::pair<Sphere *, Sphere> >::iterator i = spheres.begin(); i != spheres.end(); ++i)
	{
		delete i->first;
	}

	for(std::vector<SpatialSubdivisionHandle*>::iterator i2 = ms_objectsInSphereTree.begin(); i2 != ms_objectsInSphereTree.end(); ++i2)
	{
		handle = *i2;
		if(handle)
		{
			ms_collisionSphereTree.removeObject(handle);
		}
	}
	ms_objectsInSphereTree.clear();
}

//----------------------------------------------------------------------

/** Get the static asteroid field data for the current planet
*/
std::vector<AsteroidGenerationManager::AsteroidFieldData> const & AsteroidGenerationManager::getDataForScene(std::string const & sceneName)
{
	return ms_staticFieldData[sceneName];
}

//----------------------------------------------------------------------

bool AsteroidGenerationManager::hasDataForScene(std::string const & sceneName)
{
	typedef std::map<AsteroidGenerationManager::SceneNameKey, std::vector<AsteroidGenerationManager::AsteroidFieldData> > StaticFieldData;

	StaticFieldData::const_iterator ii = ms_staticFieldData.find(sceneName);
	return ii != ms_staticFieldData.end();
}

//----------------------------------------------------------------------

/** Generate the DATA (NOT the objects) for a asteroid field given seed data.
    The same seed data will always generate the same asteroid field.
		The field will have randomly even distribution of position, orientation, rotation range, scale range, and template selection based on the ratios given in the fields style table
*/
bool AsteroidGenerationManager::generateField(AsteroidFieldData const & fieldData, std::vector<AsteroidData> & result)
{
	if(!s_installed)
	{
		DEBUG_FATAL (true, ("AsteroidGenerationManager not installed"));
		return false; //lint !e527 unreachable (reachable in release)
	}

	if(fieldData.rotationMin > fieldData.rotationMax)
	{
		DEBUG_FATAL(true, ("AsteroidGenerationManager::generateField rotationMin > rotationMax"));
		return false; //lint !e527 unreachable (reachable in release)
	}

	if(fieldData.scaleMin > fieldData.scaleMax)
	{
		DEBUG_FATAL(true, ("AsteroidGenerationManager::generateField scaleMin > scaleMax"));
		return false; //lint !e527 unreachable (reachable in release)
	}

	clearInstantiatedData();

	std::vector<std::pair<uint32, float> > templateCrcsAndLikelihoodWeighted;

	float sum = 0.0f;
	{
		for(std::vector<std::pair<uint32, float> >::const_iterator i = fieldData.templateCrcsAndLikelihood.begin(); i != fieldData.templateCrcsAndLikelihood.end(); ++i)
		{
			sum += i->second;
		}
	}
	if(sum <= 0.0f)
		sum = 1.0f;

	for(std::vector<std::pair<uint32, float> >::const_iterator it = fieldData.templateCrcsAndLikelihood.begin(); it != fieldData.templateCrcsAndLikelihood.end(); ++it)
	{
		templateCrcsAndLikelihoodWeighted.push_back(std::make_pair(it->first, it->second / sum));
	}

	s_randomGenerator.setSeed(fieldData.seed);

	WaveForm3D * splineWaveform = nullptr;
	if (fieldData.fieldType == AsteroidFieldData::FT_spline)
	{
		splineWaveform = new WaveForm3D;
		
		size_t numPoints = fieldData.splineControlPoints.size();
		size_t count = 0;
		for (std::vector<Vector>::const_iterator i = fieldData.splineControlPoints.begin(); i!=fieldData.splineControlPoints.end(); ++i)
		{
			splineWaveform->insert(static_cast<float>(count) / static_cast<float>(numPoints - 1), *i);
			++count;
		}
	}

	result.clear();

	AsteroidData newAsteroid;

	for(int i2 = 0; i2 < fieldData.count; ++i2)
	{
		bool valid = false;
		std::vector<Sphere*> collisionResult;
		Sphere * s = nullptr;
		while(!valid)
		{
			//create asteroid locations until we find one that doesn't penetrate any existing objects
			collisionResult.clear();
			switch (fieldData.fieldType)
			{
			case AsteroidFieldData::FT_cube:
				newAsteroid = generateAsteroidDataCube(fieldData.centerPosition, fieldData.radius, fieldData.scaleMin, fieldData.scaleMax, fieldData.rotationMin, fieldData.rotationMax, templateCrcsAndLikelihoodWeighted);
				break;

			case AsteroidFieldData::FT_spline:				
				newAsteroid = generateAsteroidDataSpline(*NON_NULL(splineWaveform), fieldData.radius, fieldData.flattenDepth, fieldData.faceTowards,fieldData.scaleMin, fieldData.scaleMax, fieldData.rotationMin, fieldData.rotationMax, templateCrcsAndLikelihoodWeighted);
				break;
			}

			s = new Sphere;
			s->setCenter(newAsteroid.position);
			s->setRadius(1.0f);

			if(ms_getRadiusFunction)
			{
				typedef std::map<uint32, float> RadiusMapType;
				static RadiusMapType radiusMap;
				RadiusMapType::const_iterator i=radiusMap.find(newAsteroid.templateCrc);
				if (i==radiusMap.end())
					i=radiusMap.insert(std::make_pair(newAsteroid.templateCrc,ms_getRadiusFunction(newAsteroid.templateCrc))).first;
				
				s->setRadius(i->second * newAsteroid.scale);
			}

			ms_collisionSphereTree.findInRange (newAsteroid.position, s->getRadius(), collisionResult);
			if(collisionResult.empty())
			{
				DEBUG_REPORT_LOG_PRINT(ConfigSharedGame::getSpamAsteroidGenerationData(), ("Added Asteroid at [%f, %f, %f], radius [%f].\n", newAsteroid.position.x, newAsteroid.position.y, newAsteroid.position.z, s->getRadius()));
				valid = true;
			}
			else
			{
				DEBUG_REPORT_LOG_PRINT(ConfigSharedGame::getSpamAsteroidGenerationData(), ("Asteroid creation collision at [%f, %f, %f], radius [%f], trying again...\n", newAsteroid.position.x, newAsteroid.position.y, newAsteroid.position.z, s->getRadius()));
				delete s;
				s = nullptr;
			}
		}

		NOT_NULL(s); //lint !e644 // s could be uninitialized (not true -- initialized to nullptr above, should be set to a value during the while loop)

		SpatialSubdivisionHandle* handle = ms_collisionSphereTree.addObject(s);
		if(handle)
			ms_objectsInSphereTree.push_back(handle);

		result.push_back(newAsteroid);
	} //lint !e429 // not leaking s because it is stored in the sphere tree

	delete splineWaveform;

	return true;
}

//----------------------------------------------------------------------

AsteroidGenerationManager::AsteroidData AsteroidGenerationManager::generateAsteroidDataCube(Vector const & centerPosition, float radius, float scaleMin, float scaleMax, float rotationMin, float rotationMax, std::vector<std::pair<uint32, float> > templateCrcsAndLikelihoodWeighted)
{
	float randomNum = 0.0f;
	float sum = 0.0f;
	AsteroidData newAsteroid;

	//position
	randomNum = s_randomGenerator.randomReal(-1.0f, 1.0f);
	newAsteroid.position.x = centerPosition.x + (radius * randomNum);
	randomNum = s_randomGenerator.randomReal(-1.0f, 1.0f);
	newAsteroid.position.y = centerPosition.y + (radius * randomNum);
	randomNum = s_randomGenerator.randomReal(-1.0f, 1.0f);
	newAsteroid.position.z = centerPosition.z + (radius * randomNum);

	//orientation
	newAsteroid.orientation.set(s_randomGenerator.randomReal(0.0f, PI_TIMES_2), s_randomGenerator.randomReal(0.0f, PI_TIMES_2), s_randomGenerator.randomReal(0.0f, PI_TIMES_2));

	//rotation speed
	newAsteroid.rotationVector.set(s_randomGenerator.randomReal(rotationMin, rotationMax), s_randomGenerator.randomReal(rotationMin, rotationMax), s_randomGenerator.randomReal(rotationMin, rotationMax));

	//scale
	newAsteroid.scale = s_randomGenerator.randomReal(scaleMin, scaleMax);

	//template selection
	randomNum = s_randomGenerator.randomReal();
	std::vector<std::pair<uint32, float> >::const_iterator it = templateCrcsAndLikelihoodWeighted.begin();
	sum = it->second;
	while(sum < randomNum && it != templateCrcsAndLikelihoodWeighted.end())
	{
		++it;
		sum += it->second;
	}

	newAsteroid.templateCrc = it->first;

	return newAsteroid;
}

//----------------------------------------------------------------------

AsteroidGenerationManager::AsteroidData AsteroidGenerationManager::generateAsteroidDataSpline(WaveForm3D & waveform, float radius, float flattenDepth, Vector const & faceTowards, float scaleMin, float scaleMax, float rotationMin, float rotationMax, std::vector<std::pair<uint32, float> > templateCrcsAndLikelihoodWeighted)
{
	float sum = 0.0f;
	float const t = s_randomGenerator.randomReal(0.0f,1.0f);
	float const tForPoint2 = (t<0.9f) ? t + 0.1f : t - 0.1f;
	Vector wavePoint;
	waveform.getValue(t,wavePoint,true);
	Vector wavePoint2; // another point along the spine, to figure out which direction the spline is going
	waveform.getValue(tForPoint2,wavePoint2,true);
	Vector alongSpline = wavePoint2 - wavePoint;

	AsteroidData newAsteroid;

	//position
	if (flattenDepth >= 0.0f)
	{
		// ribbon-shaped field.  Cross-section is a rectangle
		Vector planeX = wavePoint - faceTowards; // flat direction of the plane, which points towards the "faceTowards" point
		if (!planeX.normalize())
			FATAL(true,("Data bug:  Asteroid field specified facing towards point (%f,%f,%f), which is in (or very close to) the field itself.",faceTowards.x,faceTowards.y,faceTowards.z));
		Vector planeY = planeX.cross(alongSpline); // wide direction of the plane
		IGNORE_RETURN(planeY.normalize());
		planeX *= s_randomGenerator.randomReal(-flattenDepth, flattenDepth);
		planeY *= s_randomGenerator.randomReal(-radius, radius);

		newAsteroid.position = wavePoint + planeX + planeY;

	}
	else
	{
		// standard field.  Cross-section is a square
		Vector planeX = Vector::perpendicular(alongSpline);
		Vector planeY = planeX.cross(alongSpline);
		IGNORE_RETURN(planeX.normalize());
		IGNORE_RETURN(planeY.normalize());
		planeX *= s_randomGenerator.randomReal(-radius, radius);
		planeY *= s_randomGenerator.randomReal(-radius, radius);

		newAsteroid.position = wavePoint + planeX + planeY;
	}

	//orientation
	newAsteroid.orientation.set(s_randomGenerator.randomReal(0.0f, PI_TIMES_2), s_randomGenerator.randomReal(0.0f, PI_TIMES_2), s_randomGenerator.randomReal(0.0f, PI_TIMES_2));

	//rotation speed
	newAsteroid.rotationVector.set(s_randomGenerator.randomReal(rotationMin, rotationMax), s_randomGenerator.randomReal(rotationMin, rotationMax), s_randomGenerator.randomReal(rotationMin, rotationMax));

	//scale
	newAsteroid.scale = s_randomGenerator.randomReal(scaleMin, scaleMax);

	//template selection
	float const randomNum = s_randomGenerator.randomReal();
	std::vector<std::pair<uint32, float> >::const_iterator it = templateCrcsAndLikelihoodWeighted.begin();
	sum = it->second;
	while(sum < randomNum && it != templateCrcsAndLikelihoodWeighted.end())
	{
		++it;
		sum += it->second;
	}

	newAsteroid.templateCrc = it->first;

	return newAsteroid;
}

//======================================================================
