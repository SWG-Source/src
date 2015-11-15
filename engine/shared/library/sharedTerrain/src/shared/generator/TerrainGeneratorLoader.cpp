//
// TerrainGeneratorLoader.cpp
// asommers 9-11-2000
//
// copyright 2000, verant interactive
//

//-------------------------------------------------------------------

#include "sharedTerrain/FirstSharedTerrain.h"
#include "sharedTerrain/TerrainGeneratorLoader.h"

#include "sharedFile/Iff.h"
#include "sharedTerrain/Affector.h"
#include "sharedTerrain/AffectorColor.h"
#include "sharedTerrain/AffectorEnvironment.h"
#include "sharedTerrain/AffectorFloraDynamic.h"
#include "sharedTerrain/AffectorFloraStatic.h"
#include "sharedTerrain/AffectorRibbon.h"
#include "sharedTerrain/AffectorHeight.h"
#include "sharedTerrain/AffectorPassable.h"
#include "sharedTerrain/AffectorRiver.h"
#include "sharedTerrain/AffectorRoad.h"
#include "sharedTerrain/AffectorShader.h"
#include "sharedTerrain/Boundary.h"
#include "sharedTerrain/Filter.h"

//-------------------------------------------------------------------

void TerrainGeneratorLoader::loadLayerItem (Iff& iff, TerrainGenerator* terrainGenerator, TerrainGenerator::Layer* layer)
{
	TerrainGenerator::Boundary* boundary = createBoundary (iff);

	if (boundary)
		layer->addBoundary (boundary);
	else
	{
		TerrainGenerator::Filter* filter = createFilter (iff, terrainGenerator);

		if (filter)
			layer->addFilter (filter);
		else
		{
			TerrainGenerator::Affector* affector = createAffector (iff, terrainGenerator);

			if (affector)
				layer->addAffector (affector);
			else
			{
				DEBUG_FATAL (iff.getCurrentName () != TAG_LAYR, ("not at layer in terrain file"));

				TerrainGenerator::Layer* sublayer = new TerrainGenerator::Layer ();
				sublayer->load (iff, terrainGenerator);
				layer->addLayer (sublayer);
			}  //lint !e429  //-- sublayer has not been freed or returned
		}
	}
}

//-------------------------------------------------------------------

TerrainGenerator::Boundary* TerrainGeneratorLoader::createBoundary (Iff& iff)
{
	TerrainGenerator::Boundary* boundary = 0;

	switch (iff.getCurrentName ())
	{
	case TAG_BALL:
		{
			//-- this boundary is no longer used
			iff.enterForm (TAG_BALL);
			iff.exitForm (TAG_BALL, true);
		}
		break;

	case TAG_BCIR:
		{
			iff.enterForm (TAG_BCIR);

				boundary = new BoundaryCircle ();
				boundary->load (iff);

			iff.exitForm (TAG_BCIR);
		}
		break;

	case TAG_BREC:
		{
			iff.enterForm (TAG_BREC);

				boundary = new BoundaryRectangle ();
				boundary->load (iff);

			iff.exitForm (TAG_BREC);
		}
		break;

	case TAG_BPOL:
		{
			iff.enterForm (TAG_BPOL);

				boundary = new BoundaryPolygon ();
				boundary->load (iff);

			iff.exitForm (TAG_BPOL);
		}
		break;

	case TAG_BSPL:
		{
			//-- this boundary is no longer used
			iff.enterForm (TAG_BSPL);
			iff.exitForm (TAG_BSPL, true);
		}
		break;

	case TAG_BPLN:
		{
			iff.enterForm (TAG_BPLN);

				boundary = new BoundaryPolyline ();
				boundary->load (iff);

			iff.exitForm (TAG_BPLN);
		}
		break;

	default:
		break;
	}

	return boundary;
}

//-------------------------------------------------------------------

TerrainGenerator::Filter* TerrainGeneratorLoader::createFilter (Iff& iff, TerrainGenerator* terrainGenerator)
{
	TerrainGenerator::Filter* filter = 0;

	switch (iff.getCurrentName ())
	{
	case TAG_FHGT:
		{
			iff.enterForm (TAG_FHGT);

				filter = new FilterHeight ();
				filter->load (iff);

			iff.exitForm (TAG_FHGT);
		}
		break;

	case TAG_FFRA:
		{
			iff.enterForm (TAG_FFRA);

				FilterFractal* filterFractal = new FilterFractal ();
				filterFractal->load (iff, terrainGenerator->getFractalGroup ());
				filter = filterFractal;

			iff.exitForm (TAG_FFRA);
		}
		break;

	case TAG_FBIT:
		{
			iff.enterForm (TAG_FBIT);

				FilterBitmap* filterBitmap = new FilterBitmap ();
				filterBitmap->load (iff /*, terrainGenerator->getBitmapGroup ()*/);
				filter = filterBitmap;

			iff.exitForm (TAG_FBIT);
		}
		break;

	case TAG_FSLP:
		{
			iff.enterForm (TAG_FSLP);

				filter = new FilterSlope ();
				filter->load (iff);

			iff.exitForm (TAG_FSLP);
		}
		break;

	case TAG_FDIR:
		{
			iff.enterForm (TAG_FDIR);

				filter = new FilterDirection ();
				filter->load (iff);

			iff.exitForm (TAG_FDIR);
		}
		break;

	case TAG_FSHD:
		{
			iff.enterForm (TAG_FSHD);

				filter = new FilterShader ();
				filter->load (iff);

			iff.exitForm (TAG_FSHD);
		}
		break;

	default:
		break;
	}

	return filter;
}

//-------------------------------------------------------------------

TerrainGenerator::Affector* TerrainGeneratorLoader::createAffector (Iff& iff, TerrainGenerator* terrainGenerator)
{
	TerrainGenerator::Affector* affector = 0;

	switch (iff.getCurrentName ())
	{
	case TAG_AHSM:
		{
			//-- AffectorHeightSmoother no longer exists
			iff.enterForm (TAG_AHSM);
			iff.exitForm (TAG_AHSM, true);
		}
		break;

	case TAG_AHBM:
		{
			//-- AffectorHeightBitmap no longer exists
			iff.enterForm (TAG_AHBM);
			iff.exitForm (TAG_AHBM, true);
		}
		break;

	case TAG_ACBM:
		{
			//-- AffectorColorBitmap no longer exists
			iff.enterForm (TAG_ACBM);
			iff.exitForm (TAG_ACBM, true);
		}
		break;

	case TAG_ASBM:
		{
			//-- AffectorShaderBitmap no longer exists
			iff.enterForm (TAG_ASBM);
			iff.exitForm (TAG_ASBM, true);
		}
		break;

	case TAG_AFBM:
		{
			//-- AffectorFloraBitmap no longer exists
			iff.enterForm (TAG_AFBM);
			iff.exitForm (TAG_AFBM, true);
		}
		break;

	case TAG_AENV:
		{
			iff.enterForm (TAG_AENV);

				affector = new AffectorEnvironment ();
				affector->load (iff);

			iff.exitForm (TAG_AENV);
		}
		break;

	case TAG_AHTR:
		{
			iff.enterForm (TAG_AHTR);

				affector = new AffectorHeightTerrace ();
				affector->load (iff);

			iff.exitForm (TAG_AHTR);
		}
		break;

	case TAG_AHCN:
		{
			iff.enterForm (TAG_AHCN);

				affector = new AffectorHeightConstant ();
				affector->load (iff);

			iff.exitForm (TAG_AHCN);
		}
		break;

	case TAG_AHFR:
		{
			iff.enterForm (TAG_AHFR);

				AffectorHeightFractal* affectorHeightFractal = new AffectorHeightFractal ();
				affectorHeightFractal->load (iff, terrainGenerator->getFractalGroup ());
				affector = affectorHeightFractal;

			iff.exitForm (TAG_AHFR);
		}
		break;

	case TAG_ACCN:
		{
			iff.enterForm (TAG_ACCN);

				affector = new AffectorColorConstant ();
				affector->load (iff);

			iff.exitForm (TAG_ACCN);
		}
		break;

	case TAG_ACRH:
		{
			iff.enterForm (TAG_ACRH);

				affector = new AffectorColorRampHeight ();
				affector->load (iff);

			iff.exitForm (TAG_ACRH);
		}
		break;

	case TAG_ACRF:
		{
			iff.enterForm (TAG_ACRF);

				AffectorColorRampFractal* affectorColorRampFractal = new AffectorColorRampFractal ();
				affectorColorRampFractal->load (iff, terrainGenerator->getFractalGroup ());
				affector = affectorColorRampFractal;

			iff.exitForm (TAG_ACRF);
		}
		break;

	case TAG_ASCN:
		{
			iff.enterForm (TAG_ASCN);

				affector = new AffectorShaderConstant ();
				affector->load (iff);

			iff.exitForm (TAG_ASCN);
		}
		break;

	case TAG_ASRP:
		{
			iff.enterForm (TAG_ASRP);

				affector = new AffectorShaderReplace ();
				affector->load (iff);

			iff.exitForm (TAG_ASRP);
		}
		break;

	case TAG_AFCN:
		{
			iff.enterForm (TAG_AFCN);

				affector = new AffectorFloraStaticCollidableConstant ();
				affector->load (iff);

			iff.exitForm (TAG_AFCN);
		}
		break;

	case TAG_AFSC:
		{
			iff.enterForm (TAG_AFSC);

				affector = new AffectorFloraStaticCollidableConstant ();
				affector->load (iff);

			iff.exitForm (TAG_AFSC);
		}
		break;

	case TAG_AFSN:
		{
			iff.enterForm (TAG_AFSN);

				affector = new AffectorFloraStaticNonCollidableConstant ();
				affector->load (iff);

			iff.exitForm (TAG_AFSN);
		}
		break;

	case TAG_ARCN:
		{
			iff.enterForm (TAG_ARCN);

				affector = new AffectorFloraDynamicNearConstant ();
				affector->load (iff);

			iff.exitForm (TAG_ARCN);
		}
		break;

	case TAG_AFDN:
		{
			iff.enterForm (TAG_AFDN);

				affector = new AffectorFloraDynamicNearConstant ();
				affector->load (iff);

			iff.exitForm (TAG_AFDN);
		}
		break;

	case TAG_AFDF:
		{
			iff.enterForm (TAG_AFDF);

				affector = new AffectorFloraDynamicFarConstant ();
				affector->load (iff);

			iff.exitForm (TAG_AFDF);
		}
		break;

	case TAG_ARIB:
		{
			iff.enterForm (TAG_ARIB);

				affector = new AffectorRibbon ();
				affector->load (iff);

			iff.exitForm (TAG_ARIB);
		}
		break;

	case TAG_AEXC:
		{
			iff.enterForm (TAG_AEXC);

				affector = new AffectorExclude ();
				affector->load (iff);

			iff.exitForm (TAG_AEXC);
		}
		break;

	case TAG_APAS:
		{
			iff.enterForm (TAG_APAS);

				affector = new AffectorPassable();
				affector->load (iff);

			iff.exitForm (TAG_APAS);
		}
		break;

	case TAG_AROA:
		{
			iff.enterForm (TAG_AROA);

				affector = new AffectorRoad ();
				affector->load (iff);

			iff.exitForm (TAG_AROA);
		}
		break;

	case TAG_ARIV:
		{
			iff.enterForm (TAG_ARIV);

				affector = new AffectorRiver ();
				affector->load (iff);

			iff.exitForm (TAG_ARIV);
		}
		break;

	default:
		break;
	}

	return affector;
}

//-------------------------------------------------------------------

