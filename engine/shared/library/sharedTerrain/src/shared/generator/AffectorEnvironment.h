//===================================================================
//
// AffectorEnvironment.h
// asommers 9-30-2000
//
// copyright 2000, verant interactive
//

//===================================================================

#ifndef INCLUDED_AffectorEnvironment_H
#define INCLUDED_AffectorEnvironment_H

//===================================================================

#include "sharedTerrain/TerrainGenerator.h"

//===================================================================

class AffectorEnvironment : public TerrainGenerator::Affector
{
private:

	//-- not accessible
	mutable int                     m_cachedFamilyId;
	mutable EnvironmentGroup::Info  m_cachedEgi;
	mutable float                   m_cachedFeatherClamp;

	//-- accessible
	int                             m_familyId;
	bool                            m_useFeatherClampOverride;
	float                           m_featherClampOverride;

private:

	void                            load_0000 (Iff& iff);

private:

	AffectorEnvironment (const AffectorEnvironment& rhs);
	AffectorEnvironment& operator= (const AffectorEnvironment& rhs);

public:

	AffectorEnvironment ();
	virtual ~AffectorEnvironment ();

	virtual void              prepare ();
	virtual void              affect (float worldX, float worldZ, int x, int z, float amount, const TerrainGenerator::GeneratorChunkData& generatorChunkData) const;
	virtual void              load (Iff& iff);
	virtual void              save (Iff& iff) const;
	virtual unsigned          getAffectedMaps() const;

	int          getFamilyId () const;
	void         setFamilyId (int newFamilyId);

	bool         getUseFeatherClampOverride () const;
	void         setUseFeatherClampOverride (bool useFeatherClampOverride);
	float        getFeatherClampOverride () const;
	void         setFeatherClampOverride (float featherClampOverride);
};

//-------------------------------------------------------------------

inline int AffectorEnvironment::getFamilyId () const
{
	return m_familyId;
}

//-------------------------------------------------------------------

inline bool AffectorEnvironment::getUseFeatherClampOverride () const
{
	return m_useFeatherClampOverride;
}

//-------------------------------------------------------------------

inline float AffectorEnvironment::getFeatherClampOverride () const
{
	return m_featherClampOverride;
}

//===================================================================

#endif
