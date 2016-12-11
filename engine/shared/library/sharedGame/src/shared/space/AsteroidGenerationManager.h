//======================================================================
//
// AsteroidGenerationManager.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_AsteroidGenerationManager_H
#define INCLUDED_AsteroidGenerationManager_H

//======================================================================

#include "sharedMath/SphereTree.h"
#include "sharedMath/Vector.h"
#include <vector>

//======================================================================

class SpatialSubdivisionHandle;
class Sphere;
class WaveForm3D;

//======================================================================

class AsteroidGenerationManager
{
public:
	struct AsteroidData
	{
		Vector position;
		Vector orientation;
		uint32 templateCrc;
		float scale;
		Vector rotationVector;

		void clear();
	};

	struct RadialAsteroidData
	{
		std::string appearance;
		float likelihood;
		float scaleMin;
		float scaleMax;
		float spawnRange;
		float deleteRange;
		std::string hitEffectName;
	};

	struct AsteroidFieldData
	{
		enum FieldType {FT_cube, FT_spline};
		
		FieldType fieldType;
		Vector centerPosition;
		float radius;
		std::vector<Vector> splineControlPoints;
		int count;		
		uint32 seed;
		float flattenDepth;
		Vector faceTowards;
		float scaleMin;
		float scaleMax;
		float rotationMin;
		float rotationMax;
		std::vector<std::pair<uint32, float> > templateCrcsAndLikelihood;
		std::vector<RadialAsteroidData> radialData;
		int radialCount;
		float radialLikelihoodSum;
		float maxViewableDistance;
		float viewAllDistance;
		std::string soundEffect;
	};

	typedef float (*GetExtentRadiusFunction)(uint32 crc);

public:
	static void install();
	static bool generateField(AsteroidFieldData const & fieldData, std::vector<AsteroidData> & result);
	static bool generateField(Vector const & centerPosition, float radius, int count, uint32 seed, float scaleMin, float scaleMax, float rotationMin, float rotationMax, std::vector<std::pair<uint32, float> > const & templateCrcsAndLikelihood, std::vector<AsteroidData> & asteroidDatas);
	static void loadStaticFieldDataForScene(std::string const & sceneName, bool server);
	static void clearStaticFieldData();
	static void clearInstantiatedData();
	static bool hasDataForScene(std::string const & sceneName);
	static std::vector<AsteroidFieldData> const & getDataForScene(std::string const & sceneName);
	static void registerGetExtentRadiusFunction(GetExtentRadiusFunction);

private:
	class AsteroidDataSphereExtentAccessor: public BaseSphereTreeAccessor<Sphere *, AsteroidDataSphereExtentAccessor>
	{
	public:
		static Sphere const getExtent(Sphere const * const sphere);
		static char const *getDebugName(Sphere * sphere);
	};

	static void remove();
	static AsteroidData generateAsteroidDataCube(Vector const & centerPosition, float radius, float scaleMin, float scaleMax, float rotationMin, float rotationMax, std::vector<std::pair<uint32, float> > templateCrcsAndLikelihoodWeighted);
	static AsteroidData generateAsteroidDataSpline(WaveForm3D & waveform, float radius, float flattenDepth, Vector const & faceTowards, float scaleMin, float scaleMax, float rotationMin, float rotationMax, std::vector<std::pair<uint32, float> > templateCrcsAndLikelihoodWeighted);

private:
	typedef std::string SceneNameKey;
	static std::map<SceneNameKey, std::vector<AsteroidFieldData>> ms_staticFieldData;
	static SphereTree<Sphere*, AsteroidDataSphereExtentAccessor> ms_collisionSphereTree;
	static std::vector<SpatialSubdivisionHandle*> ms_objectsInSphereTree;
	static GetExtentRadiusFunction ms_getRadiusFunction;

	//disabled
	AsteroidGenerationManager();
	~AsteroidGenerationManager();
	AsteroidGenerationManager(AsteroidGenerationManager const &);
	AsteroidGenerationManager &operator =(AsteroidGenerationManager const &);
};

//======================================================================

#endif
