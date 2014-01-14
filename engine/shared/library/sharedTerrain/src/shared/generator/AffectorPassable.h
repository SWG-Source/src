//======================================================================
//
// AffectorPassable.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_AffectorPassable_H
#define INCLUDED_AffectorPassable_H

//===================================================================

#include "sharedTerrain/TerrainGenerator.h"

//===================================================================

class AffectorPassable : public TerrainGenerator::Affector
{
private:

	void load_0000 (Iff& iff);

private:

	AffectorPassable (const AffectorPassable& rhs);
	AffectorPassable& operator= (const AffectorPassable& rhs);

public:

	AffectorPassable ();
	virtual ~AffectorPassable ();

	virtual void              affect (float worldX, float worldZ, int x, int z, float amount, const TerrainGenerator::GeneratorChunkData& generatorChunkData) const;
	virtual void              load (Iff& iff);
	virtual void              save (Iff& iff) const;
	virtual unsigned          getAffectedMaps() const;

	bool isPassable() const;
	float getFeatherThreshold() const;

	void setPassable(bool passable);
	void setFeatherThreshold(float featherThreshold);

private:

	bool m_passable;
	float m_featherThreshold;

};

//===================================================================

#endif
