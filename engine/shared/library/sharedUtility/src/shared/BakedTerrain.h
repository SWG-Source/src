//===================================================================
//
// BakedTerrain.h
// asommers
//
// copyright 2002, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_BakedTerrain_H
#define INCLUDED_BakedTerrain_H

//===================================================================

class Iff;
class Rectangle2d;
class Vector2d;

//===================================================================

class BakedTerrain
{
	friend class TerrainEditorDoc;

public:

	BakedTerrain ();
	~BakedTerrain ();

	void  load (Iff& iff);
	void  save (Iff& iff) const;

	float getMapWidthInMeters () const;
	float getChunkWidthInMeters () const;
	bool  getWater (int chunkX, int chunkY) const;
	bool  getSlope (int chunkX, int chunkY) const;
	bool  getWater (const Vector2d& position) const;
	bool  getSlope (const Vector2d& position) const;
	bool  getWater (const Rectangle2d& rectangle) const;
	bool  getSlope (const Rectangle2d& rectangle) const;

	//-- tool use only
	void  setMap (float mapWidthInMeters, float chunkWidthInMeters);
	void  setWater (int chunkX, int chunkY, bool water);
	void  setSlope (int chunkX, int chunkY, bool slope);

private:

	void destroy ();

	int  calculateChunkX (const Vector2d& position) const;
	int  calculateChunkY (const Vector2d& position) const;
	void calculateMapXy (const Vector2d& position, int& x, int& y) const;
	bool getBool (const uint8* map, int mapX, int mapY) const;
	bool getMap (const uint8* map, const Rectangle2d& rectangle) const;
	void setBool (uint8* map, int mapX, int mapY, bool value);

	void load_0000 (Iff& iff);
	void load_0001 (Iff& iff);

private:

	BakedTerrain (const BakedTerrain&);
	BakedTerrain& operator= (const BakedTerrain&);

private:

	float          m_mapWidthInMeters;
	float          m_chunkWidthInMeters;
	int            m_width;
	int            m_height;
	uint8*         m_waterMap;
	uint8*         m_slopeMap;
};

//===================================================================

#endif

