//======================================================================
//
// NebulaManager.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/NebulaManager.h"

#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedGame/Nebula.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedMath/SphereTree.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"
#include "sharedFile/TreeFile.h"

#include <algorithm>
#include <map>

//======================================================================

namespace NebulaManagerNamespace
{
	typedef NebulaManager::NebulaVector NebulaVector;
	typedef std::map<int, Nebula const *> NebulaMap;
	typedef std::map<std::string, NebulaVector> NebulasByScene;

	//-- nebula vector OWNS the nebulas
	NebulasByScene s_nebulasByScene;
	NebulaMap s_nebulaMap;
	std::string s_currentSceneId;

	class NebulaSphereAccessor: public BaseSphereTreeAccessor<Nebula const *, NebulaSphereAccessor>
	{
	public:

		static Sphere const getExtent(Nebula const *nebula)
		{
			return NON_NULL(nebula)->getSphere();
		}
		
		//-------------------------------------------------------------------
		
		static char const *getDebugName(Nebula const *)
		{
			return 0;
		}

	};

	typedef SphereTree<Nebula const *, NebulaSphereAccessor> NebulaSphereTree;
	NebulaSphereTree * s_collisionSphereTree = nullptr;

	enum DatatableColumns
	{
		DC_name,
		DC_x,
		DC_y,
		DC_z,
		DC_radius,
		DC_density,
		DC_facingPercent,
		DC_facingA,
		DC_facingR,
		DC_facingG,
		DC_facingB,
		DC_facingRampA,
		DC_facingRampR,
		DC_facingRampG,
		DC_facingRampB,
		DC_orientedA,
		DC_orientedR,
		DC_orientedG,
		DC_orientedB,
		DC_orientedRampA,
		DC_orientedRampR,
		DC_orientedRampG,
		DC_orientedRampB,
		DC_ambientSound,
		DC_ambientSoundVolume,
		DC_cameraJitterAngle,
		DC_effectReactor,
		DC_effectEngine,
		DC_effectShields,
		DC_lightningFrequency,
		DC_lightningDurationMax,
		DC_lightningDamageMin,
		DC_lightningDamageMax,
		DC_lightningAppearance,
		DC_lightningA,
		DC_lightningR,
		DC_lightningG,
		DC_lightningB,
		DC_lightningRampA,
		DC_lightningRampR,
		DC_lightningRampG,
		DC_lightningRampB,
		DC_lightningSound,
		DC_lightningSoundLoop,
		DC_lightningHitEffectClient,
		DC_lightningHitEffectServer,
		DC_environmentalDamageFrequency,
		DC_environmentalDamage,
		DC_environmentalDamageEffect,
		DC_shaderIndex
	};

	//----------------------------------------------------------------------

	NebulaManager::ImplementationClearFunction s_clearFunction = nullptr;
}

using namespace NebulaManagerNamespace;

//----------------------------------------------------------------------

void NebulaManager::install()
{
	InstallTimer const installTimer("NebulaManager::install");

	ExitChain::add(NebulaManager::remove, "NebulaManager::remove");
}

//----------------------------------------------------------------------

void NebulaManager::remove()
{
	clear();

	NebulasByScene::iterator ii = s_nebulasByScene.begin();
	NebulasByScene::iterator iiEnd = s_nebulasByScene.end();
	for (; ii != iiEnd; ++ii)
	{
		NebulaVector & nebulaVector = ii->second;

		std::for_each(nebulaVector.begin(), nebulaVector.end(), PointerDeleter());
		nebulaVector.clear();
	}

	s_nebulasByScene.clear();
}

//----------------------------------------------------------------------

void NebulaManager::clear()
{
	//-- Remove nebulas from the collision sphere tree and delete the sphere tree
	{
		for (NebulaMap::iterator iter = s_nebulaMap.begin(); iter != s_nebulaMap.end(); ++iter)
		{
			Nebula const * const nebula = iter->second;
			s_collisionSphereTree->removeObject(nebula->getSpatialSubdivisionHandle());
		}
	}

	s_nebulaMap.clear();

	if (s_collisionSphereTree != nullptr)
	{
		delete s_collisionSphereTree;
		s_collisionSphereTree = nullptr;
	}

	//-- Remove nebulas for the current scene from the scene map
	{
		NebulasByScene::iterator iter = s_nebulasByScene.find(s_currentSceneId);
		if (iter != s_nebulasByScene.end())
		{
			NebulaVector & nebulaVector = iter->second;

			std::for_each(nebulaVector.begin(), nebulaVector.end(), PointerDeleter());
			nebulaVector.clear();
		}
	}

	s_currentSceneId.clear();

	if (s_clearFunction != nullptr)
		s_clearFunction();
}

//----------------------------------------------------------------------

void NebulaManager::loadSceneData(std::string const & sceneId)
{
	if (sceneId.empty())
		return;

	// already loaded this data
	if (hasSceneData(sceneId))
		return;

	std::string const & filename = "datatables/space/nebula/" + sceneId + ".iff";
	
	//technically this means that this will be called twice for existing nebula tables
	// but it's worth it to kill that fucking annoying warning
	// not a space scene...or a missing space scene
	if (!TreeFile::exists(filename.c_str()))
	{
		return;
	}

	DataTable * const dt = DataTableManager::getTable(filename, true);
	
	float styleWeightings[16] = {0};

	const int numRows = dt->getNumRows();
	
	//lint -e641 //enums to int

	for (int row = 0; row < numRows; ++row)
	{
		Nebula * const nebula = new Nebula(row + 1, dt->getStringValue(DC_name, row));
		Vector const pos(dt->getFloatValue(DC_x, row), dt->getFloatValue(DC_y, row), dt->getFloatValue(DC_z, row));
		nebula->setExtent(pos, dt->getFloatValue(DC_radius, row));
		nebula->setDensity(dt->getFloatValue(DC_density, row));
		VectorArgb const colorFacing (dt->getFloatValue(DC_facingA, row), dt->getFloatValue(DC_facingR, row), dt->getFloatValue(DC_facingG, row), dt->getFloatValue(DC_facingB, row));
		VectorArgb const colorRampFacing (dt->getFloatValue(DC_facingRampA, row), dt->getFloatValue(DC_facingRampR, row), dt->getFloatValue(DC_facingRampG, row), dt->getFloatValue(DC_facingRampB, row));
		VectorArgb const colorOriented (dt->getFloatValue(DC_orientedA, row), dt->getFloatValue(DC_orientedR, row), dt->getFloatValue(DC_orientedG, row), dt->getFloatValue(DC_orientedB, row));
		VectorArgb const colorRampOriented (dt->getFloatValue(DC_orientedRampA, row), dt->getFloatValue(DC_orientedRampR, row), dt->getFloatValue(DC_orientedRampG, row), dt->getFloatValue(DC_orientedRampB, row));
		nebula->setColor(colorFacing, colorRampFacing, colorOriented, colorRampOriented);
		
		nebula->setAmbientSound(dt->getStringValue(DC_ambientSound, row), dt->getFloatValue(DC_ambientSoundVolume, row));
		nebula->setCameraJitterAngle(convertDegreesToRadians(dt->getFloatValue(DC_cameraJitterAngle, row)));
		nebula->setShipEffects(dt->getFloatValue(DC_effectReactor, row), dt->getFloatValue(DC_effectEngine, row), dt->getFloatValue(DC_effectShields, row));

		VectorArgb const colorLightning (dt->getFloatValue(DC_lightningA, row), dt->getFloatValue(DC_lightningR, row), dt->getFloatValue(DC_lightningG, row), dt->getFloatValue(DC_lightningB, row));
		VectorArgb const colorRampLightning (dt->getFloatValue(DC_lightningRampA, row), dt->getFloatValue(DC_lightningRampR, row), dt->getFloatValue(DC_lightningRampG, row), dt->getFloatValue(DC_lightningRampB, row));

		nebula->setLightningEffects(dt->getFloatValue(DC_lightningFrequency, row),
			dt->getFloatValue(DC_lightningDurationMax, row),
			dt->getStringValue(DC_lightningAppearance, row), 
			colorLightning, colorRampLightning,
			dt->getStringValue(DC_lightningSound, row), dt->getStringValue(DC_lightningSoundLoop, row),
			dt->getStringValue(DC_lightningHitEffectClient, row), dt->getStringValue(DC_lightningHitEffectServer, row));
		nebula->setLightningDamage(dt->getFloatValue(DC_lightningDamageMin, row), dt->getFloatValue(DC_lightningDamageMax, row));
		nebula->setEnvironmentalDamage(dt->getFloatValue(DC_environmentalDamageFrequency, row), dt->getFloatValue(DC_environmentalDamage, row), dt->getStringValue(DC_environmentalDamageEffect, row));

		int const shaderIndex = dt->getIntValue(DC_shaderIndex, row);
		float const facingPercent = dt->getFloatValue(DC_facingPercent, row);

		nebula->setShaderInfo(shaderIndex, styleWeightings, facingPercent);

		s_nebulasByScene[sceneId].push_back (nebula);
	} //lint !e429 //nebula owned

	DataTableManager::close(filename);
}

//----------------------------------------------------------------------

bool NebulaManager::hasSceneData(std::string const & sceneId)
{
	NebulasByScene::const_iterator ii = s_nebulasByScene.find(sceneId);

	if (ii != s_nebulasByScene.end())
	{
		NebulaVector const & currentNebulaVector = ii->second;
		return !currentNebulaVector.empty();
	}

	return false;
}

//----------------------------------------------------------------------

void NebulaManager::loadScene(std::string const & sceneId)
{
	clear();
	
	s_collisionSphereTree = new NebulaSphereTree;

	loadSceneData(sceneId);

	NebulasByScene::iterator ii = s_nebulasByScene.find(sceneId);
	if (ii != s_nebulasByScene.end())
	{
		s_currentSceneId = sceneId;

		NebulaVector & currentNebulaVector = ii->second;

		NebulaVector::iterator jj = currentNebulaVector.begin();
		NebulaVector::iterator jjEnd = currentNebulaVector.end();

		for (; jj != jjEnd; ++jj)
		{
			Nebula const * const nebula = *jj;
			addNebula(*nebula);
		}
	}
}

//----------------------------------------------------------------------

void NebulaManager::addNebula(Nebula const & nebula)
{
	IGNORE_RETURN(s_nebulaMap.insert(std::make_pair(nebula.getId(), &nebula)));
	nebula.setSpatialSubdivisionHandle(s_collisionSphereTree->addObject(&nebula));
}

//----------------------------------------------------------------------

void NebulaManager::getNebulasAtPosition(Vector const & pos, NebulaVector & nebulaVector)
{
	NON_NULL(s_collisionSphereTree)->findAtPoint(pos, nebulaVector);
}

//----------------------------------------------------------------------

void NebulaManager::getNebulasInSphere(Vector const & pos, float const radius, NebulaVector & nebulaVector)
{
	NON_NULL(s_collisionSphereTree)->findInRange(pos, radius, nebulaVector);
}

//----------------------------------------------------------------------

Nebula const * NebulaManager::getClosestNebula(Vector const & pos, float const maxDistance, float & outMinDistance, float & outMaxDistance)
{
	Nebula const * nebula = nullptr;
	if (NON_NULL(s_collisionSphereTree)->findClosest(pos, maxDistance, nebula, outMinDistance, outMaxDistance))
		return nebula;

	return nullptr;
}

//----------------------------------------------------------------------

Nebula const * NebulaManager::getNebulaById(int const id)
{
	NebulaMap::const_iterator const it = s_nebulaMap.find(id);
	if (it != s_nebulaMap.end())
		return (*it).second;

	return nullptr;
}

//----------------------------------------------------------------------

NebulaVector const & NebulaManager::getNebulaVector(std::string const & sceneId)
{
	return s_nebulasByScene[sceneId];
}

//----------------------------------------------------------------------

NebulaVector const & NebulaManager::getNebulaVector()
{
	// this will create an empty nebula vector that
	// will be returned if this is the ground game
	return s_nebulasByScene[s_currentSceneId];
}

//----------------------------------------------------------------------

void NebulaManager::setClearFunction(ImplementationClearFunction func)
{
	s_clearFunction = func;
}

//======================================================================
