//===================================================================
//
// SamplerProceduralTerrainAppearanceTemplate.h
// asommers
//
// copyright 2000, verant interactive
//
//===================================================================

#ifndef INCLUDED_SamplerProceduralTerrainAppearanceTemplate_h
#define INCLUDED_SamplerProceduralTerrainAppearanceTemplate_h

//===================================================================

#include "sharedTerrain/ProceduralTerrainAppearanceTemplate.h"

class Iff;

//===================================================================

class SamplerProceduralTerrainAppearanceTemplate : public ProceduralTerrainAppearanceTemplate
{
public:

	static AppearanceTemplate* create (const char* filename, Iff* iff);

public:

	SamplerProceduralTerrainAppearanceTemplate (const char* filename, Iff* iff);
	virtual ~SamplerProceduralTerrainAppearanceTemplate ();

	void setMapsToSample(unsigned maps);

	virtual Appearance* createAppearance () const;

	int                     getNumberOfSampledFlora() const { return m_numberOfSampledFlora; }
	void                    setFloraTileSample(int key, const FloraGroup::Info &i_data, float height);
	bool                    getFloraHeightSample(int key, float &o_height) const;

	void                    beginStaticCollidableFloraSampling();
	bool                    loadStaticCollidableFloraFile(const char *i_filename);
	bool                    writeStaticCollidableFloraFile(const char *i_filename, int tileBounds[4]) const;
	bool                    writeOldStaticCollidableFloraFile(const char *i_filename) const;
	bool                    writeOldStaticCollidableFloraHeightFile(const char *i_filename) const;
	void                    commitStaticCollidableFloraSamples(int *tileBounds); // commit current sample data to base appearance template

	const FloraGroup::Info *getStaticCollidableFloraSamples() const { return m_staticCollidableFloraSamples; }

	const PackedIntegerMap    *getStaticCollidableFloraMap() const { return m_staticCollidableFloraMap; }
	const PackedFixedPointMap *getStaticCollidableFloraHeightMap() const { return m_staticCollidableFloraHeightMap; }

private:

	SamplerProceduralTerrainAppearanceTemplate ();
	SamplerProceduralTerrainAppearanceTemplate (const SamplerProceduralTerrainAppearanceTemplate&);
	SamplerProceduralTerrainAppearanceTemplate& operator= (const SamplerProceduralTerrainAppearanceTemplate&);

	// ----------------------------------
	float                   m_staticCollidableFloraHeightMin;
	float                   m_staticCollidableFloraHeightMax;
	float                  *m_staticCollidableFloraHeightSamples;
	FloraGroup::Info       *m_staticCollidableFloraSamples;
	int                     m_numberOfSampledFlora;
	// ----------------------------------
}; 

//===================================================================

#endif
 
