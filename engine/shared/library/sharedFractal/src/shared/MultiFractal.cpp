//
// MultiFractal.cpp
// asommers
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#include "sharedFractal/FirstSharedFractal.h"
#include "sharedFractal/MultiFractal.h"

#include <cmath>

//-------------------------------------------------------------------

//@todo codereorg
namespace
{
	inline bool FloatsEqual (const float a, const float b)
	{
		return WithinEpsilonInclusive (a, b, 0.00001f);
	}
}

//-------------------------------------------------------------------
//
// MultiFractal::NoiseGenerator
//
#define PERLIN_floor(t,it) (it - ((t) < 0 && (t) != it))
#define PERLIN_lerp(t,a,b) (a + t * (b - a))
#define PERLIN_scurve(t)  ((3.0f - (2.0f * t)) * t * t)
#define PERLIN_setup(i,b0,b1,r0,r1)     \
	t =  i + N;                         \
	it = static_cast<int> (t);          \
	ft = PERLIN_floor (t, it);          \
	b0 = ft & BM;                       \
	b1 = (b0 + 1) & BM;                 \
	r0 = t - ft;                        \
	r1 = r0 - 1.f;
#define PERLIN_dot2(rx,ry) (rx * q[0] + ry * q[1])

//-------------------------------------------------------------------

inline void PERLIN_normalize2 (float v[2])
{
	const float s = static_cast<float> (sqrt (sqr (v[0]) + sqr (v[1])));

	v[0] /= s;
	v[1] /= s;

	DEBUG_FATAL (v [0] < -1 || v [0] > 1, (""));
	DEBUG_FATAL (v [1] < -1 || v [1] > 1, (""));
}

//-------------------------------------------------------------------

MultiFractal::NoiseGenerator::NoiseGenerator (uint32 seed) :
	m_random ()
{
	init (seed);
}

//-------------------------------------------------------------------

MultiFractal::NoiseGenerator::~NoiseGenerator (void)
{
}

//-------------------------------------------------------------------

void MultiFractal::NoiseGenerator::init (uint32 seed)
{
	m_random.setSeed (seed);

	int i, j, k;

	for (i = 0; i < B; i++)
	{
		m_p [i] = i;

		m_g1[i] = static_cast<float>((m_random.random () % (B + B)) - B) / B;

		for (j = 0; j < 2; j++)
			m_g2[i][j] = static_cast<float>((m_random.random () % (B + B)) - B) / B;

		PERLIN_normalize2 (m_g2[i]);
	}

	while (--i)
	{
		k = m_p[i];
		m_p [i] = m_p [j = m_random.random () % B];
		m_p [j] = k;
	}

	for (i = 0; i < B + 2; i++)
	{
		m_p  [B + i] = m_p [i];
		m_g1 [B + i] = m_g1[i];
		for (j = 0 ; j < 2 ; j++)
			m_g2 [B + i][j] = m_g2 [i][j];
	}
}

//-------------------------------------------------------------------

float MultiFractal::NoiseGenerator::getValue (float x) const
{
	int  it, ft, bx0, bx1;
	float t, rx0, rx1;

	PERLIN_setup (x, bx0, bx1, rx0, rx1);  //lint !e514  //-- unusual use of a boolean

	const float sx = PERLIN_scurve(rx0);
	const float u  = rx0 * m_g1 [m_p [bx0]];
	const float v  = rx1 * m_g1 [m_p [bx1]];

	const float result = PERLIN_lerp (sx, u, v);
	DEBUG_FATAL (result < -1.0f || result > 1.0f, ("result < -1.0f || result > 1.0f"));
	return result;
}

//-------------------------------------------------------------------

float MultiFractal::NoiseGenerator::getValue (float x, float y) const
{
	int  it, ft, bx0, bx1, by0, by1;
	float t, rx0, rx1, ry0, ry1;

	PERLIN_setup (x, bx0, bx1, rx0, rx1);  //lint !e514  //-- unusual use of a boolean
	PERLIN_setup (y, by0, by1, ry0, ry1);  //lint !e514  //-- unusual use of a boolean

	const float sx = PERLIN_scurve (rx0);
	const float sy = PERLIN_scurve (ry0);

	const int b00 = m_p [m_p [bx0] + by0];
	const int b01 = m_p [m_p [bx0] + by1];
	const int b10 = m_p [m_p [bx1] + by0];
	const int b11 = m_p [m_p [bx1] + by1];

	const float *q;
	float u, v;

	q = m_g2 [b00];
	u = PERLIN_dot2 (rx0, ry0);
	q = m_g2 [b10];
	v = PERLIN_dot2 (rx1, ry0);
	const float a = PERLIN_lerp (sx, u, v);

	q = m_g2 [b01];
	u = PERLIN_dot2 (rx0, ry1);
	q = m_g2 [b11];
	v = PERLIN_dot2 (rx1, ry1);
	const float b = PERLIN_lerp (sx, u, v);

	const float result = PERLIN_lerp (sy, a, b);
	DEBUG_FATAL (result < -1.0f || result > 1.0f, ("result < -1.0f || result > 1.0f"));

	return result;
}

//-------------------------------------------------------------------
//
// MultiFractal
//
const float MultiFractal::ms_defaultScaleX          = 0.01f;
const float MultiFractal::ms_defaultScaleY          = 0.01f;
const float MultiFractal::ms_defaultOffsetX         = 0.0f;
const float MultiFractal::ms_defaultOffsetY         = 0.0f;
const int  MultiFractal::ms_defaultNumberOfOctaves = 2;
const float MultiFractal::ms_defaultFrequency       = 4.0f;
const float MultiFractal::ms_defaultAmplitude       = 0.5f;
const float MultiFractal::ms_defaultBias            = 0.5f;
const float MultiFractal::ms_defaultGain            = 0.7f;

#ifdef _DEBUG
int        MultiFractal::ms_numberOfMultiFractalGetValueCalls;
int        MultiFractal::ms_numberOfMultiFractalGetValueCacheHits;
#endif

//-------------------------------------------------------------------

MultiFractal::MultiFractal (void) :
	m_seed (0),
	m_scaleX (ms_defaultScaleX),
	m_scaleY (ms_defaultScaleY),
	m_offsetX (ms_defaultOffsetX),
	m_offsetY (ms_defaultOffsetY),
	m_numberOfOctaves (ms_defaultNumberOfOctaves),
	m_frequency (ms_defaultFrequency),
	m_amplitude (ms_defaultAmplitude),
	m_ooTotalAmplitude (1),
	m_useBias (false),
	m_bias (ms_defaultBias),
	m_useGain (false),
	m_gain (ms_defaultGain),
	m_useSin (false),
	m_combinationRule (CR_add),
	m_combinationFunction_1 (0),
	m_combinationFunction_2 (0),
	m_noiseGenerator (),
	m_cacheX (0),
	m_cacheY (0),
	m_cache (0)
{
	initTotalAmplitude ();

	setCombinationRule (CR_add);

	m_noiseGenerator.init (m_seed);
}

//-------------------------------------------------------------------

MultiFractal::~MultiFractal (void)
{
	if (m_cache)
	{
		delete [] m_cache;
		m_cache = 0;
	}
}

//-------------------------------------------------------------------

MultiFractal::MultiFractal (const MultiFractal& rhs) :
	m_seed (0),
	m_scaleX (ms_defaultScaleX),
	m_scaleY (ms_defaultScaleY),
	m_offsetX (ms_defaultOffsetX),
	m_offsetY (ms_defaultOffsetY),
	m_numberOfOctaves (ms_defaultNumberOfOctaves),
	m_frequency (ms_defaultFrequency),
	m_amplitude (ms_defaultAmplitude),
	m_ooTotalAmplitude (1),
	m_useBias (false),
	m_bias (ms_defaultBias),
	m_useGain (false),
	m_gain (ms_defaultGain),
	m_useSin (false),
	m_combinationRule (CR_add),
	m_combinationFunction_1 (0),
	m_combinationFunction_2 (0),
	m_noiseGenerator (),
	m_cacheX (0),
	m_cacheY (0),
	m_cache (0)
{
	copy (rhs);
}

//-------------------------------------------------------------------

MultiFractal& MultiFractal::operator= (const MultiFractal& rhs)
{
	if (this != &rhs)
		copy (rhs);

	return *this;
}

//-------------------------------------------------------------------

void MultiFractal::copy (const MultiFractal& rhs)
{
	//-- perform shallow copy
	m_seed                  = rhs.m_seed;
	m_scaleX                = rhs.m_scaleX;
	m_scaleY                = rhs.m_scaleY;
	m_offsetX               = rhs.m_offsetX;
	m_offsetY               = rhs.m_offsetY;
	m_numberOfOctaves       = rhs.m_numberOfOctaves;
	m_frequency             = rhs.m_frequency;
	m_amplitude             = rhs.m_amplitude;
	m_ooTotalAmplitude      = rhs.m_ooTotalAmplitude;
	m_useBias               = rhs.m_useBias;
	m_bias                  = rhs.m_bias;
	m_useGain               = rhs.m_useGain;
	m_gain                  = rhs.m_gain;
	m_useSin                = rhs.m_useSin;
	m_combinationRule       = rhs.m_combinationRule;
	m_combinationFunction_1 = rhs.m_combinationFunction_1;
	m_combinationFunction_2 = rhs.m_combinationFunction_2;
	m_noiseGenerator        = rhs.m_noiseGenerator;

	//-- reset cache parameters
	m_cache  = 0;
	m_cacheX = 0;
	m_cacheY = 0;

	//-- allocate cache (cache will be empty)
	allocateCache (rhs.m_cacheX, rhs.m_cacheY);
}

//-------------------------------------------------------------------

void MultiFractal::allocateCache (int x, int y)
{
	if (x > m_cacheX || y > m_cacheY)
	{
		if (m_cache)
		{
			delete [] m_cache;
			m_cache = 0;
		}

		if (x != 0 && y != 0)
		{
			m_cacheX = x;
			m_cacheY = y;
			m_cache  = new CachedNode [static_cast<uint> (x * y)];

			resetCache ();
		}
	}
}

//-------------------------------------------------------------------

void MultiFractal::resetCache ()
{
	if (m_cache)
		memset (m_cache, 0, static_cast<uint> (isizeof (CachedNode) * m_cacheX * m_cacheY));
}

//-------------------------------------------------------------------

void MultiFractal::setSeed (uint32 seed)
{
	if (m_seed != seed)
	{
		m_seed = seed;

		m_noiseGenerator.init (seed);

		resetCache ();
	}
}

//-------------------------------------------------------------------

void MultiFractal::setScale (float scaleX, float scaleY)
{
	m_scaleX = scaleX;
	m_scaleY = scaleY;

	resetCache ();
}

//-------------------------------------------------------------------

void MultiFractal::setOffset (float offsetX, float offsetY)
{
	m_offsetX = offsetX;
	m_offsetY = offsetY;

	resetCache ();
}

//-------------------------------------------------------------------

void MultiFractal::setNumberOfOctaves (int numberOfOctaves)
{
	m_numberOfOctaves = numberOfOctaves;

	resetCache ();
}

//-------------------------------------------------------------------

void MultiFractal::setFrequency (float frequency)
{
	m_frequency = frequency;

	resetCache ();
}

//-------------------------------------------------------------------

void MultiFractal::setAmplitude (float amplitude)
{
	m_amplitude = amplitude;

	initTotalAmplitude ();

	resetCache ();
}

//-------------------------------------------------------------------

void MultiFractal::setBias (bool useBias, float bias)
{
	m_useBias = useBias;
	m_bias    = bias;

	resetCache ();
}

//-------------------------------------------------------------------

void MultiFractal::setGain (bool useGain, float gain)
{
	m_useGain = useGain;
	m_gain    = gain;

	resetCache ();
}

//-------------------------------------------------------------------

void MultiFractal::setUseSin (bool useSin)
{
	m_useSin = useSin;

	resetCache ();
}

//-------------------------------------------------------------------

void MultiFractal::setCombinationRule (CombinationRule combinationRule)
{
	m_combinationRule = combinationRule;

	switch (m_combinationRule)
	{
	case CR_add:
	case CR_multiply:
		m_combinationFunction_1 = getValueAdd_1;
		m_combinationFunction_2 = getValueAdd_2;
		break;

	case CR_crest:
		m_combinationFunction_1 = getValueCrest_1;
		m_combinationFunction_2 = getValueCrest_2;
		break;

	case CR_turbulence:
		m_combinationFunction_1 = getValueTurbulence_1;
		m_combinationFunction_2 = getValueTurbulence_2;
		break;

	case CR_crestClamp:
		m_combinationFunction_1 = getValueCrestClamp_1;
		m_combinationFunction_2 = getValueCrestClamp_2;
		break;

	case CR_turbulenceClamp:
		m_combinationFunction_1 = getValueTurbulenceClamp_1;
		m_combinationFunction_2 = getValueTurbulenceClamp_2;
		break;

	case CR_COUNT:
	default:
		DEBUG_FATAL (true, ("invalid combination rule"));
		break;
	}

	resetCache ();
}

//-------------------------------------------------------------------

void MultiFractal::reset (void)
{
	m_seed            = 0;
	m_scaleX          = ms_defaultScaleX;
	m_scaleY          = ms_defaultScaleY;
	m_offsetX         = ms_defaultOffsetX;
	m_offsetY         = ms_defaultOffsetY;
	m_numberOfOctaves = ms_defaultNumberOfOctaves;
	m_frequency       = ms_defaultFrequency;
	m_amplitude       = ms_defaultAmplitude;
	m_useBias         = false;
	m_bias            = ms_defaultBias;
	m_useGain         = false;
	m_gain            = ms_defaultGain;
	m_useSin          = false;
 	setCombinationRule (CR_add);

	initTotalAmplitude ();

	resetCache ();
}

//-------------------------------------------------------------------

void MultiFractal::initTotalAmplitude (void)
{
	m_ooTotalAmplitude = 0.0f;

	float amplitude = 1.0f;

	int i;
	for (i = 0; i < m_numberOfOctaves; ++i, amplitude *= m_amplitude)
		m_ooTotalAmplitude += amplitude;

	m_ooTotalAmplitude = RECIP (m_ooTotalAmplitude);
}

//-------------------------------------------------------------------

//bias (b, 0)  =0
//bias (b, 0.5)=b
//bias (b, 1)  =1
inline float NG_bias (float a, float b)
{
	return powf (a, logf (b) / logf (.5f));
}

//-------------------------------------------------------------------

const float NG_log_0_5f = logf (0.5f);

//gain (g, t) = if (t < 0.5) then bias (1 - g, 2 * t) else 1 - bias (1 - g,2 - 2 * 2t) / 2
inline float NG_gain (float a, float b)
{
	if (a < .001)
		return 0.;
	else if (a > .999)
		return 1.;

	float p = logf (1.f - b) / NG_log_0_5f;

	if (a < 0.5f)
		return powf (2.f * a, p) * 0.5f;
	else
		return 1.f - powf (2.f * (1.f - a), p) * 0.5f;
}

//-------------------------------------------------------------------

float MultiFractal::getValueAdd_1 (const float x, const MultiFractal& multiFractal)
{
	float frequency = 1.0f;
	float amplitude = 1.0f;
	float sum       = 0.0f;

	int i;
	for (i = 0; i < multiFractal.m_numberOfOctaves; ++i, frequency *= multiFractal.m_frequency, amplitude *= multiFractal.m_amplitude)
		sum += amplitude * multiFractal.m_noiseGenerator.getValue (x * frequency);

	if (multiFractal.m_useSin)
		sum = sinf (x + sum);

	return ((sum * multiFractal.m_ooTotalAmplitude) + 1.0f) * 0.5f;
}

//-------------------------------------------------------------------

float MultiFractal::getValueCrest_1 (const float x, const MultiFractal& multiFractal)
{
	float frequency = 1.0f;
	float amplitude = 1.0f;
	float sum       = 0.0f;

	int i;
	for (i = 0; i < multiFractal.m_numberOfOctaves; ++i, frequency *= multiFractal.m_frequency, amplitude *= multiFractal.m_amplitude)
		sum += amplitude * static_cast<float> (1.0f - fabsf (multiFractal.m_noiseGenerator.getValue (x * frequency)));

	if (multiFractal.m_useSin)
		sum = sinf (x + sum);

	return sum * multiFractal.m_ooTotalAmplitude;
}

//-------------------------------------------------------------------

float MultiFractal::getValueTurbulence_1 (const float x, const MultiFractal& multiFractal)
{
	float frequency = 1.0f;
	float amplitude = 1.0f;
	float sum       = 0.0f;

	int i;
	for (i = 0; i < multiFractal.m_numberOfOctaves; ++i, frequency *= multiFractal.m_frequency, amplitude *= multiFractal.m_amplitude)
		sum += amplitude * static_cast<float> (fabsf (multiFractal.m_noiseGenerator.getValue (x * frequency)));

	if (multiFractal.m_useSin)
		sum = sinf (x + sum);

	return sum * multiFractal.m_ooTotalAmplitude;
}

//-------------------------------------------------------------------

float MultiFractal::getValueCrestClamp_1 (const float x, const MultiFractal& multiFractal)
{
	float frequency = 1.0f;
	float amplitude = 1.0f;
	float sum       = 0.0f;

	int i;
	for (i = 0; i < multiFractal.m_numberOfOctaves; ++i, frequency *= multiFractal.m_frequency, amplitude *= multiFractal.m_amplitude)
		sum += amplitude * static_cast<float> (1.0f - clamp (0.f, multiFractal.m_noiseGenerator.getValue (x * frequency), 1.f));

	if (multiFractal.m_useSin)
		sum = sinf (x + sum);

	return sum * multiFractal.m_ooTotalAmplitude;
}

//-------------------------------------------------------------------

float MultiFractal::getValueTurbulenceClamp_1 (const float x, const MultiFractal& multiFractal)
{
	float frequency = 1.0f;
	float amplitude = 1.0f;
	float sum       = 0.0f;

	int i;
	for (i = 0; i < multiFractal.m_numberOfOctaves; ++i, frequency *= multiFractal.m_frequency, amplitude *= multiFractal.m_amplitude)
		sum += amplitude * static_cast<float> (clamp (0.f, multiFractal.m_noiseGenerator.getValue (x * frequency), 1.f));

	if (multiFractal.m_useSin)
		sum = sinf (x + sum);

	return sum * multiFractal.m_ooTotalAmplitude;
}

//-------------------------------------------------------------------

float MultiFractal::getValue (float x) const
{
	DEBUG_FATAL (m_numberOfOctaves == 0, ("m_numberOfOctaves == 0"));
	NOT_NULL (m_combinationFunction_1);

	float result = 0.0f;

	x *= m_scaleX;
	result = m_combinationFunction_1 (x + m_offsetX, *this);

	DEBUG_FATAL (result < 0.0f || result > 1.0f, ("result < 0.0f || result > 1.0f"));

	if (m_useBias)
		result = NG_bias (result, m_bias);

	if (m_useGain)
		result = NG_gain (result, m_gain);

	return result;
}

//-------------------------------------------------------------------

float MultiFractal::getValueAdd_2 (const float x, const float y, const MultiFractal& multiFractal)
{
	float frequency = 1.0f;
	float amplitude = 1.0f;
	float sum       = 0.0f;

	int i;
	for (i = 0; i < multiFractal.m_numberOfOctaves; ++i, frequency *= multiFractal.m_frequency, amplitude *= multiFractal.m_amplitude)
		sum += amplitude * multiFractal.m_noiseGenerator.getValue (x * frequency, y * frequency);

	if (multiFractal.m_useSin)
		sum = sinf (x + sum);

	return ((sum * multiFractal.m_ooTotalAmplitude) + 1.0f) * 0.5f;
}

//-------------------------------------------------------------------

float MultiFractal::getValueCrest_2 (const float x, const float y, const MultiFractal& multiFractal)
{
	float frequency = 1.0f;
	float amplitude = 1.0f;
	float sum       = 0.0f;

	int i;
	for (i = 0; i < multiFractal.m_numberOfOctaves; ++i, frequency *= multiFractal.m_frequency, amplitude *= multiFractal.m_amplitude)
		sum += amplitude * static_cast<float> (1.0f - fabsf (multiFractal.m_noiseGenerator.getValue (x * frequency, y * frequency)));

	if (multiFractal.m_useSin)
		sum = sinf (x + sum);

	return sum * multiFractal.m_ooTotalAmplitude;
}

//-------------------------------------------------------------------

float MultiFractal::getValueTurbulence_2 (const float x, const float y, const MultiFractal& multiFractal)
{
	float frequency = 1.0f;
	float amplitude = 1.0f;
	float sum       = 0.0f;

	int i;
	for (i = 0; i < multiFractal.m_numberOfOctaves; ++i, frequency *= multiFractal.m_frequency, amplitude *= multiFractal.m_amplitude)
		sum += amplitude * static_cast<float> (fabsf (multiFractal.m_noiseGenerator.getValue (x * frequency, y * frequency)));

	if (multiFractal.m_useSin)
		sum = sinf (x + sum);

	return sum * multiFractal.m_ooTotalAmplitude;
}

//-------------------------------------------------------------------

float MultiFractal::getValueCrestClamp_2 (const float x, const float y, const MultiFractal& multiFractal)
{
	float frequency = 1.0f;
	float amplitude = 1.0f;
	float sum       = 0.0f;

	int i;
	for (i = 0; i < multiFractal.m_numberOfOctaves; ++i, frequency *= multiFractal.m_frequency, amplitude *= multiFractal.m_amplitude)
		sum += amplitude * static_cast<float> (1.0f - clamp (0.f, multiFractal.m_noiseGenerator.getValue (x * frequency, y * frequency), 1.f));

	if (multiFractal.m_useSin)
		sum = sinf (x + sum);

	return sum * multiFractal.m_ooTotalAmplitude;
}

//-------------------------------------------------------------------

float MultiFractal::getValueTurbulenceClamp_2 (const float x, const float y, const MultiFractal& multiFractal)
{
	float frequency = 1.0f;
	float amplitude = 1.0f;
	float sum       = 0.0f;

	int i;
	for (i = 0; i < multiFractal.m_numberOfOctaves; ++i, frequency *= multiFractal.m_frequency, amplitude *= multiFractal.m_amplitude)
		sum += amplitude * static_cast<float> (clamp (0.f, multiFractal.m_noiseGenerator.getValue (x * frequency, y * frequency), 1.f));

	if (multiFractal.m_useSin)
		sum = sinf (x + sum);

	return sum * multiFractal.m_ooTotalAmplitude;
}

//-------------------------------------------------------------------

float MultiFractal::getValue2 (float x, float y) const
{
	DEBUG_FATAL (m_numberOfOctaves == 0, ("m_numberOfOctaves == 0"));
	NOT_NULL (m_combinationFunction_2);

	float result = 0.0f;

	x *= m_scaleX;
	y *= m_scaleY;
	result = m_combinationFunction_2 (x + m_offsetX, y + m_offsetY, *this);

	DEBUG_FATAL (result < 0.0f || result > 1.0f, ("result < 0.0f || result > 1.0f"));

	if (m_useBias)
		result = NG_bias (result, m_bias);

	if (m_useGain)
		result = NG_gain (result, m_gain);

	return result;
}

//-------------------------------------------------------------------

float MultiFractal::getValueCache2 (float x, float y, int cx, int cy) const
{
	NOT_NULL (m_cache);

#ifdef _DEBUG
	++ms_numberOfMultiFractalGetValueCalls;
#endif

	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE (0, cx, m_cacheX);
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE (0, cy, m_cacheY);

	CachedNode& cachedNode = m_cache [m_cacheX * cy + cx];
	if (cachedNode.cached && FloatsEqual (cachedNode.x, x) && FloatsEqual (cachedNode.y, y))
	{
#ifdef _DEBUG
		++ms_numberOfMultiFractalGetValueCacheHits;
#endif

		return cachedNode.value;
	}

	cachedNode.cached = true;
	cachedNode.x      = x;
	cachedNode.y      = y;

	DEBUG_FATAL (m_numberOfOctaves == 0, ("m_numberOfOctaves == 0"));
	NOT_NULL (m_combinationFunction_2);

	float result = 0.0f;

	x *= m_scaleX;
	y *= m_scaleY;
	result = m_combinationFunction_2 (x + m_offsetX, y + m_offsetY, *this);

	DEBUG_FATAL (result < 0.0f || result > 1.0f, ("result < 0.0f || result > 1.0f"));

	if (m_useBias)
		result = NG_bias (result, m_bias);

	if (m_useGain)
		result = NG_gain (result, m_gain);

	cachedNode.value = result;

	return result;
}

//-------------------------------------------------------------------

float MultiFractal::getValue (float x, float y) const
{
	DEBUG_FATAL (m_numberOfOctaves == 0, ("m_numberOfOctaves == 0"));
	NOT_NULL (m_combinationFunction_2);

	float frequency = 1.0f;
	float amplitude = 1.0f;
	float result    = 0.0f;

	x *= m_scaleX;
	y *= m_scaleY;

	switch (m_combinationRule)
	{
	case CR_add:
	case CR_multiply:
		{
			float sum = 0.0f;

			int i;
			for (i = 0; i < m_numberOfOctaves; ++i, frequency *= m_frequency, amplitude *= m_amplitude)
				sum += amplitude * m_noiseGenerator.getValue (x * frequency + m_offsetX * frequency, y * frequency + m_offsetY * frequency);

			if (m_useSin)
				sum = sinf (x + sum);

			result = ((sum * m_ooTotalAmplitude) + 1.0f) * 0.5f;
		}
		break;

	case CR_crest:
		{
			float sum = 0.0f;

			int i;
			for (i = 0; i < m_numberOfOctaves; ++i, frequency *= m_frequency, amplitude *= m_amplitude)
				sum += amplitude * static_cast<float> (1.0f - fabsf (m_noiseGenerator.getValue (x * frequency + m_offsetX * frequency, y * frequency + m_offsetY * frequency)));

			if (m_useSin)
				sum = sinf (x + sum);

			result = sum * m_ooTotalAmplitude;
		}
		break;

	case CR_turbulence:
		{
			float sum = 0.0f;

			int i;
			for (i = 0; i < m_numberOfOctaves; ++i, frequency *= m_frequency, amplitude *= m_amplitude)
				sum += amplitude * static_cast<float> (fabsf (m_noiseGenerator.getValue (x * frequency + m_offsetX * frequency, y * frequency + m_offsetY * frequency)));

			if (m_useSin)
				sum = sinf (x + sum);

			result = sum * m_ooTotalAmplitude;
		}
		break;

	case CR_crestClamp:
		{
			float sum = 0.0f;

			int i;
			for (i = 0; i < m_numberOfOctaves; ++i, frequency *= m_frequency, amplitude *= m_amplitude)
				sum += amplitude * static_cast<float> (1.0f - clamp (0.f, m_noiseGenerator.getValue (x * frequency + m_offsetX * frequency, y * frequency + m_offsetY * frequency), 1.f));

			if (m_useSin)
				sum = sinf (x + sum);

			result = sum * m_ooTotalAmplitude;
		}
		break;

	case CR_turbulenceClamp:
		{
			float sum = 0.0f;

			int i;
			for (i = 0; i < m_numberOfOctaves; ++i, frequency *= m_frequency, amplitude *= m_amplitude)
				sum += amplitude * static_cast<float> (clamp (0.f, m_noiseGenerator.getValue (x * frequency + m_offsetX * frequency, y * frequency + m_offsetY * frequency), 1.f));

			if (m_useSin)
				sum = sinf (x + sum);

			result = sum * m_ooTotalAmplitude;
		}
		break;

	case CR_COUNT:
	default:
		DEBUG_FATAL (true, ("invalid combination rule"));
		break;
	}

	if (m_useBias)
		result = NG_bias (result, m_bias);

	if (m_useGain)
		result = NG_gain (result, m_gain);

	return result;
}

//-------------------------------------------------------------------

float MultiFractal::getValueCache (float x, float y, int cx, int cy) const
{
	NOT_NULL (m_cache);

#ifdef _DEBUG
	++ms_numberOfMultiFractalGetValueCalls;
#endif

	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE (0, cx, m_cacheX);
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE (0, cy, m_cacheY);

	CachedNode& cachedNode = m_cache [m_cacheX * cy + cx];
	if (cachedNode.cached && FloatsEqual (cachedNode.x, x) && FloatsEqual (cachedNode.y, y))
	{
#ifdef _DEBUG
		++ms_numberOfMultiFractalGetValueCacheHits;
#endif

		return cachedNode.value;
	}

	cachedNode.cached = true;
	cachedNode.x      = x;
	cachedNode.y      = y;

	DEBUG_FATAL (m_numberOfOctaves == 0, ("m_numberOfOctaves == 0"));
	NOT_NULL (m_combinationFunction_2);

	float frequency = 1.0f;
	float amplitude = 1.0f;
	float result    = 0.0f;

	x *= m_scaleX;
	y *= m_scaleY;

	switch (m_combinationRule)
	{
	case CR_add:
	case CR_multiply:
		{
			float sum = 0.0f;

			int i;
			for (i = 0; i < m_numberOfOctaves; ++i, frequency *= m_frequency, amplitude *= m_amplitude)
				sum += amplitude * m_noiseGenerator.getValue (x * frequency + m_offsetX * frequency, y * frequency + m_offsetY * frequency);

			if (m_useSin)
				sum = sinf (x + sum);

			result = ((sum * m_ooTotalAmplitude) + 1.0f) * 0.5f;
		}
		break;

	case CR_crest:
		{
			float sum = 0.0f;

			int i;
			for (i = 0; i < m_numberOfOctaves; ++i, frequency *= m_frequency, amplitude *= m_amplitude)
				sum += amplitude * static_cast<float> (1.0f - fabsf (m_noiseGenerator.getValue (x * frequency + m_offsetX * frequency, y * frequency + m_offsetY * frequency)));

			if (m_useSin)
				sum = sinf (x + sum);

			result = sum * m_ooTotalAmplitude;
		}
		break;

	case CR_turbulence:
		{
			float sum = 0.0f;

			int i;
			for (i = 0; i < m_numberOfOctaves; ++i, frequency *= m_frequency, amplitude *= m_amplitude)
				sum += amplitude * static_cast<float> (fabsf (m_noiseGenerator.getValue (x * frequency + m_offsetX * frequency, y * frequency + m_offsetY * frequency)));

			if (m_useSin)
				sum = sinf (x + sum);

			result = sum * m_ooTotalAmplitude;
		}
		break;

	case CR_crestClamp:
		{
			float sum = 0.0f;

			int i;
			for (i = 0; i < m_numberOfOctaves; ++i, frequency *= m_frequency, amplitude *= m_amplitude)
				sum += amplitude * static_cast<float> (1.0f - clamp (0.f, m_noiseGenerator.getValue (x * frequency + m_offsetX * frequency, y * frequency + m_offsetY * frequency), 1.f));

			if (m_useSin)
				sum = sinf (x + sum);

			result = sum * m_ooTotalAmplitude;
		}
		break;

	case CR_turbulenceClamp:
		{
			float sum = 0.0f;

			int i;
			for (i = 0; i < m_numberOfOctaves; ++i, frequency *= m_frequency, amplitude *= m_amplitude)
				sum += amplitude * static_cast<float> (clamp (0.f, m_noiseGenerator.getValue (x * frequency + m_offsetX * frequency, y * frequency + m_offsetY * frequency), 1.f));

			if (m_useSin)
				sum = sinf (x + sum);

			result = sum * m_ooTotalAmplitude;
		}
		break;

	case CR_COUNT:
	default:
		DEBUG_FATAL (true, ("invalid combination rule"));
		break;
	}

	if (m_useBias)
		result = NG_bias (result, m_bias);

	if (m_useGain)
		result = NG_gain (result, m_gain);

	cachedNode.value = result;

	return result;
}

//-------------------------------------------------------------------

bool MultiFractal::operator== (const MultiFractal& rhs) const
{
	const bool seedEqual = m_seed == rhs.m_seed;
	if (!seedEqual)
		return false;

	const bool scaleEqual = FloatsEqual (m_scaleX, rhs.m_scaleX) && FloatsEqual (m_scaleY, rhs.m_scaleY);
	if (!scaleEqual)
		return false;

	const bool offsetEqual = FloatsEqual (m_offsetX, rhs.m_offsetX) && FloatsEqual (m_offsetY, rhs.m_offsetY);
	if (!offsetEqual)
		return false;

	const bool octavesEqual = m_numberOfOctaves == rhs.m_numberOfOctaves;
	if (!octavesEqual)
		return false;

	const bool frequencyEqual = m_numberOfOctaves == 1 ? true : FloatsEqual (m_frequency, rhs.m_frequency);
	if (!frequencyEqual)
		return false;

	const bool amplitudeEqual = m_numberOfOctaves == 1 ? true : FloatsEqual (m_amplitude, rhs.m_amplitude);
	if (!amplitudeEqual)
		return false;

	const bool biasEqual = (m_useBias == rhs.m_useBias) ? (m_useBias ? FloatsEqual (m_bias, rhs.m_bias) : true) : false;
	if (!biasEqual)
		return false;

	const bool gainEqual = (m_useGain == rhs.m_useGain) ? (m_useGain ? FloatsEqual (m_gain, rhs.m_gain) : true) : false;
	if (!gainEqual)
		return false;

	const bool combinationRuleEqual = m_combinationRule == rhs.m_combinationRule;
	if (!combinationRuleEqual)
		return false;

	const bool sinEqual = m_useSin == rhs.m_useSin;
	if (!sinEqual)
		return false;

	return true;
}

//-------------------------------------------------------------------

#ifdef _DEBUG

void MultiFractal::debugDump ()
{
	DEBUG_REPORT_LOG_PRINT (true, ("MultiFractal::getValue calls      = %i\n", ms_numberOfMultiFractalGetValueCalls));
	DEBUG_REPORT_LOG_PRINT (true, ("MultiFractal::getValue cache hits = %i\n", ms_numberOfMultiFractalGetValueCacheHits));
}

#endif

//-------------------------------------------------------------------
