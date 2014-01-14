//===================================================================
//
// AffectorShader.h
// asommers 9-30-2000
//
// copyright 2000, verant interactive
//

//===================================================================

#ifndef INCLUDED_AffectorShader_H
#define INCLUDED_AffectorShader_H

//===================================================================

#include "sharedTerrain/TerrainGenerator.h"

//===================================================================

class AffectorShaderConstant : public TerrainGenerator::Affector
{
private:

	//-- not accessible
	mutable int                m_cachedFamilyId;
	mutable ShaderGroup::Info  m_cachedSgi;
	mutable float              m_cachedFeatherClamp;

	//-- accessible
	int                        m_familyId;
	bool                       m_useFeatherClampOverride;
	float                      m_featherClampOverride;

private:

	void                       load_0000 (Iff& iff);
	void                       load_0001 (Iff& iff);

private:

	AffectorShaderConstant (const AffectorShaderConstant& rhs);
	AffectorShaderConstant& operator= (const AffectorShaderConstant& rhs);

	void _legacyAffect (const float /*worldX*/, const float /*worldZ*/, const int x, const int z, const float amount, const TerrainGenerator::GeneratorChunkData& generatorChunkData) const;

public:

	AffectorShaderConstant ();
	virtual ~AffectorShaderConstant ();

	virtual void              prepare ();
	virtual void              affect (float worldX, float worldZ, int x, int z, float amount, const TerrainGenerator::GeneratorChunkData& generatorChunkData) const;
	virtual void              load (Iff& iff);
	virtual void              save (Iff& iff) const;
	virtual bool              affectsShader () const;
	virtual unsigned          getAffectedMaps() const;

	int          getFamilyId () const;
	void         setFamilyId (int newFamilyId);

	bool         getUseFeatherClampOverride () const;
	void         setUseFeatherClampOverride (bool useFeatherClampOverride);
	float        getFeatherClampOverride () const;
	void         setFeatherClampOverride (float featherClampOverride);
};

//-------------------------------------------------------------------

inline int AffectorShaderConstant::getFamilyId () const
{
	return m_familyId;
}

//-------------------------------------------------------------------

inline bool AffectorShaderConstant::getUseFeatherClampOverride () const
{
	return m_useFeatherClampOverride;
}

//-------------------------------------------------------------------

inline float AffectorShaderConstant::getFeatherClampOverride () const
{
	return m_featherClampOverride;
}

//===================================================================

class AffectorShaderReplace : public TerrainGenerator::Affector
{
private:

	//-- not accessible
	mutable int                m_cachedFamilyId;
	mutable ShaderGroup::Info  m_cachedSgi;
	mutable float              m_cachedFeatherClamp;

	//-- accessible
	int                        m_sourceFamilyId;
	int                        m_destinationFamilyId;
	bool                       m_useFeatherClampOverride;
	float                      m_featherClampOverride;

private:

	void                       load_0000 (Iff& iff);
	void                       load_0001 (Iff& iff);

private:

	AffectorShaderReplace (const AffectorShaderReplace& rhs);
	AffectorShaderReplace& operator= (const AffectorShaderReplace& rhs);

public:

	AffectorShaderReplace ();
	virtual ~AffectorShaderReplace ();

	virtual void              prepare ();
	virtual void              affect (float worldX, float worldZ, int x, int z, float amount, const TerrainGenerator::GeneratorChunkData& generatorChunkData) const;
	virtual void              load (Iff& iff);
	virtual void              save (Iff& iff) const;
	virtual bool              affectsShader () const;
	virtual unsigned          getAffectedMaps() const;
              
	int          getSourceFamilyId () const;
	void         setSourceFamilyId (int newSourceFamilyId);
	int          getDestinationFamilyId () const;
	void         setDestinationFamilyId (int newDestinationFamilyId);

	bool         getUseFeatherClampOverride () const;
	void         setUseFeatherClampOverride (bool useFeatherClampOverride);
	float        getFeatherClampOverride () const;
	void         setFeatherClampOverride (float featherClampOverride);
};

//-------------------------------------------------------------------

inline int AffectorShaderReplace::getSourceFamilyId () const
{
	return m_sourceFamilyId;
}

//-------------------------------------------------------------------

inline int AffectorShaderReplace::getDestinationFamilyId () const
{
	return m_destinationFamilyId;
}

//-------------------------------------------------------------------

inline bool AffectorShaderReplace::getUseFeatherClampOverride () const
{
	return m_useFeatherClampOverride;
}

//-------------------------------------------------------------------

inline float AffectorShaderReplace::getFeatherClampOverride () const
{
	return m_featherClampOverride;
}

//===================================================================

#endif
