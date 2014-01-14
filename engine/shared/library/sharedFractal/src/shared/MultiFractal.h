//
// MultiFractal.h
// asommers
//
// copyright 2001, sony online entertainment
//
//--
//
// MultiFractal will return values from 0 to 1

//-------------------------------------------------------------------

#ifndef INCLUDED_MultiFractal_H
#define INCLUDED_MultiFractal_H

//-------------------------------------------------------------------

#include "sharedRandom/RandomGenerator.h"

//-------------------------------------------------------------------

class MultiFractal
{
public:

	static const float ms_defaultScaleX;
	static const float ms_defaultScaleY;
	static const float ms_defaultOffsetX;
	static const float ms_defaultOffsetY;
	static const float ms_defaultBias;
	static const float ms_defaultGain;
	static const int  ms_defaultNumberOfOctaves;
	static const float ms_defaultFrequency;
	static const float ms_defaultAmplitude;

public:

#ifdef _DEBUG
	static void debugDump ();
#endif

public:

	enum CombinationRule
	{
		CR_add,
		CR_multiply,
		CR_crest,
		CR_turbulence,
		CR_crestClamp,
		CR_turbulenceClamp,

		CR_COUNT
	};

public:

	MultiFractal (void);
	~MultiFractal (void);
	MultiFractal (const MultiFractal& rhs);
	MultiFractal& operator= (const MultiFractal& rhs);

	bool operator== (const MultiFractal& rhs) const;

	void   allocateCache (int x, int y);

	//-- 
	float   getValue (float x) const;
	float   getValue (float x, float y) const;
	float   getValueCache (float x, float y, int cx, int cy) const;
	float   getValue2 (float x, float y) const;
	float   getValueCache2 (float x, float y, int cx, int cy) const;

	//-- parameters
	uint32 getSeed (void) const;
	void   setSeed (uint32 seed);

	float   getScaleX (void) const;
	float   getScaleY (void) const;
	void   setScale (float x=ms_defaultScaleX, float y=ms_defaultScaleY);

	float   getOffsetX (void) const;
	float   getOffsetY (void) const;
	void   setOffset (float x=ms_defaultOffsetX, float y=ms_defaultOffsetY);

	//-- number of iterations of the noise
	int    getNumberOfOctaves (void) const;
	void   setNumberOfOctaves (int numberOfOctaves=ms_defaultNumberOfOctaves);

	//-- multiplier for the period of each successive octave
	float   getFrequency (void) const;
	void   setFrequency (float frequency=ms_defaultFrequency);

	//-- multiplier for the amplitude of each successive octave
	float   getAmplitude (void) const;
	void   setAmplitude (float amplitude=ms_defaultAmplitude);

	//-- rule used to combine octaves
	CombinationRule getCombinationRule (void) const;
	void            setCombinationRule (CombinationRule combinationRule);

	//-- modifiers
	bool   getUseSin (void) const;
	void   setUseSin (bool newUseSin);

	bool   getUseBias (void) const;
	float   getBias (void) const;
	void   setBias (bool useBias, float bias);

	bool   getUseGain (void) const;
	float   getGain (void) const;
	void   setGain (bool useGain, float gain);

	//--
	void   reset (void);

private:

	static float getValueAdd_1 (float x, const MultiFractal& multiFractal);
	static float getValueCrest_1 (float x, const MultiFractal& multiFractal);
	static float getValueTurbulence_1 (float x, const MultiFractal& multiFractal);
	static float getValueCrestClamp_1 (float x, const MultiFractal& multiFractal);
	static float getValueTurbulenceClamp_1 (float x, const MultiFractal& multiFractal);

	static float getValueAdd_2 (float x, float y, const MultiFractal& multiFractal);
	static float getValueCrest_2 (float x, float y, const MultiFractal& multiFractal);
	static float getValueTurbulence_2 (float x, float y, const MultiFractal& multiFractal);
	static float getValueCrestClamp_2 (float x, float y, const MultiFractal& multiFractal);
	static float getValueTurbulenceClamp_2 (float x, float y, const MultiFractal& multiFractal);

private:

	void copy (const MultiFractal& rhs);

	void initTotalAmplitude (void);
	void resetCache ();

private:

	typedef float (*CombinationFunction_1) (float x, const MultiFractal& multiFractal);
	typedef float (*CombinationFunction_2) (float x, float y, const MultiFractal& multiFractal);

private:

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// The NoiseGenerator algorithm came from Ken Perlin's PERLIN.C. 
	//	it has been c++-ified. NoiseGenerator will return values from -1 to 1

	class NoiseGenerator
	{
	public:

		explicit NoiseGenerator (uint32 seed=0);
		~NoiseGenerator (void);

		void init (uint32 seed);

		float getValue (float x) const;
		float getValue (float x, float y) const;

	private:

		float realGetValue (float x) const;
		float realGetValue (float x, float y) const;

	private:

		enum
		{
			B  = 256,
			BM = 255,
			N  = 4096,
			NP = 12,
			NM = 4095
		};

	private:

		RandomGenerator m_random;

		int             m_p  [B + B + 2];
		float            m_g2 [B + B + 2][2];
		float            m_g1 [B + B + 2];
	};

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

private:

#ifdef _DEBUG
	static int            ms_numberOfMultiFractalGetValueCalls;
	static int            ms_numberOfMultiFractalGetValueCacheHits;
#endif

private:

	uint32                m_seed;
	float                  m_scaleX;
	float                  m_scaleY;
	float                  m_offsetX;
	float                  m_offsetY;
	int                   m_numberOfOctaves;
	float                  m_frequency;
	float                  m_amplitude;
	float                  m_ooTotalAmplitude;
	bool                  m_useBias;
	float                  m_bias;
	bool                  m_useGain;
	float                  m_gain;
	bool                  m_useSin;

	CombinationRule       m_combinationRule;
	CombinationFunction_1 m_combinationFunction_1;
	CombinationFunction_2 m_combinationFunction_2;

	NoiseGenerator        m_noiseGenerator;

	//-- used to cache generated values
	struct CachedNode
	{
		bool  cached;
		float x;
		float y;
		float value;
	};

	int                 m_cacheX;
	int                 m_cacheY;
	mutable CachedNode* m_cache;
};

//-------------------------------------------------------------------

inline uint32 MultiFractal::getSeed (void) const
{
	return m_seed;
}

//-------------------------------------------------------------------

inline float MultiFractal::getScaleX (void) const
{
	return m_scaleX;
}

//-------------------------------------------------------------------

inline float MultiFractal::getScaleY (void) const
{
	return m_scaleY;
}

//-------------------------------------------------------------------

inline float MultiFractal::getOffsetX (void) const
{
	return m_offsetX;
}

//-------------------------------------------------------------------

inline float MultiFractal::getOffsetY (void) const
{
	return m_offsetY;
}

//-------------------------------------------------------------------

inline int MultiFractal::getNumberOfOctaves (void) const
{
	return m_numberOfOctaves;
}

//-------------------------------------------------------------------

inline float MultiFractal::getFrequency (void) const
{
	return m_frequency;
}

//-------------------------------------------------------------------

inline float MultiFractal::getAmplitude (void) const
{
	return m_amplitude;
}

//-------------------------------------------------------------------

inline bool MultiFractal::getUseBias (void) const
{
	return m_useBias;
}

//-------------------------------------------------------------------

inline float MultiFractal::getBias (void) const
{
	return m_bias;
}

//-------------------------------------------------------------------

inline bool MultiFractal::getUseGain (void) const
{
	return m_useGain;
}

//-------------------------------------------------------------------

inline float MultiFractal::getGain (void) const
{
	return m_gain;
}

//-------------------------------------------------------------------

inline bool MultiFractal::getUseSin (void) const
{
	return m_useSin;
}

//-------------------------------------------------------------------

inline MultiFractal::CombinationRule MultiFractal::getCombinationRule (void) const
{
	return m_combinationRule;
}

//-------------------------------------------------------------------

#endif
