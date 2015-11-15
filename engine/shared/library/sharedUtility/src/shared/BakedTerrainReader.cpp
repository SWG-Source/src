// ======================================================================
//
// BakedTerrainReader.cpp
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#include "sharedUtility/FirstSharedUtility.h"
#include "sharedUtility/BakedTerrainReader.h"

#include "sharedFile/Iff.h"
#include "sharedUtility/BakedTerrain.h"

// ======================================================================
// BakedTerrainReaderNamespace
// ======================================================================

namespace BakedTerrainReaderNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	Tag const TAG_MPTA = TAG (M,P,T,A);
	Tag const TAG_PTAT = TAG (P,T,A,T);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	BakedTerrain * load (Iff & iff)
	{
		if (iff.getCurrentName () != TAG_MPTA && iff.getCurrentName () != TAG_PTAT)
			return 0;

		iff.enterForm ();

			iff.enterForm ();

				iff.enterChunk ();
				iff.exitChunk (true);

				iff.enterForm ();
				iff.exitForm (true);

				BakedTerrain * const bakedTerrain = new BakedTerrain ();
				bakedTerrain->load (iff);

			iff.exitForm (true);

		iff.exitForm (true);

		DEBUG_REPORT_LOG (true, ("BakedTerrainReader: loaded terrain for %s\n", iff.getFileName ()));
		return bakedTerrain;
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
}

using namespace BakedTerrainReaderNamespace;

// ======================================================================
// STATIC BakedTerrainReader
// ======================================================================

BakedTerrain * BakedTerrainReader::create (char const * const fileName)
{
	Iff iff;
	if (iff.open (fileName, true))
		return load (iff);

	return 0;
}

// ======================================================================
