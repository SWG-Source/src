//===================================================================
//
// Feather.h
// asommers 2001-01-28
//
// copyright 2001, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_Feather_H
#define INCLUDED_Feather_H

//===================================================================

class Feather
{
public:

	explicit Feather (TerrainGeneratorFeatherFunction newFeatherFunction);
	~Feather ();

	float feather (float start, float end, float t) const;

private:

	float linear (float start, float end, float t) const;
	float easeIn (float start, float end, float t) const;
	float easeOut (float start, float end, float t) const;
	float easeInOut (float start, float end, float t) const;

private:

	Feather ();
	Feather (const Feather&);
	Feather& operator= (const Feather&);

private:

	const TerrainGeneratorFeatherFunction featherFunction;
};

//===================================================================

inline Feather::Feather (TerrainGeneratorFeatherFunction newFeatherFunction) :
	featherFunction (newFeatherFunction)
{
}

//-------------------------------------------------------------------
	
inline Feather::~Feather ()
{
}

//-------------------------------------------------------------------

inline float Feather::linear (const float start, const float end, const float t) const
{
	return linearInterpolate (start, end, t);
}

//-------------------------------------------------------------------

inline float Feather::easeIn (const float start, const float end, const float t) const
{
	return linearInterpolate (start, end, sqr (t));
}

//-------------------------------------------------------------------

inline float Feather::easeOut (const float start, const float end, const float t) const
{
	return linearInterpolate (start, end, sqrt (t));
}

//-------------------------------------------------------------------

inline float Feather::easeInOut (const float start, const float end, const float t) const
{
	const float a = ((3.0f - (2.0f * t)) * t * t);

	return linearInterpolate (start, end, a);
}

//-------------------------------------------------------------------

inline float Feather::feather (const float start, const float end, const float t) const
{
	switch (featherFunction)
	{
	case TGFF_linear:
		return linear (start, end, t);

	case TGFF_easeIn:
		return easeIn (start, end, t);

	case TGFF_easeOut:
		return easeOut (start, end, t);

	case TGFF_easeInOut:
		return easeInOut (start, end, t);

	case TGFF_COUNT:
	default:
		DEBUG_FATAL (true, ("invalid feather function"));
		return 0.0f;  //lint !e527  //unreachable
	}
}

//===================================================================

#endif
