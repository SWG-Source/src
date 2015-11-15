//===================================================================
//
// AffectorExclude.h
// asommers 9-30-2000
//
// copyright 2000, verant interactive
//

//===================================================================

#ifndef INCLUDED_AffectorExclude_H
#define INCLUDED_AffectorExclude_H

//===================================================================

#include "sharedTerrain/TerrainGenerator.h"

//===================================================================

class AffectorExclude : public TerrainGenerator::Affector
{
private:

	void load_0000 (Iff& iff);

private:

	AffectorExclude (const AffectorExclude& rhs);
	AffectorExclude& operator= (const AffectorExclude& rhs);

public:

	AffectorExclude ();
	virtual ~AffectorExclude ();

	virtual void              affect (float worldX, float worldZ, int x, int z, float amount, const TerrainGenerator::GeneratorChunkData& generatorChunkData) const;
	virtual void              load (Iff& iff);
	virtual void              save (Iff& iff) const;
	virtual unsigned          getAffectedMaps() const;
};

//===================================================================

#endif
