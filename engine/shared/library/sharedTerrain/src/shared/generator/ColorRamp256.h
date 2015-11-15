//===================================================================
//
// ColorRamp256.h
// asommers
//
// copyright 2001, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_ColorRamp_H
#define INCLUDED_ColorRamp_H

//===================================================================

#include "sharedMath/PackedRgb.h"

class Iff;

//===================================================================

class ColorRamp256
{
public:

	ColorRamp256 ();
	~ColorRamp256 ();

	void             setColor (int index, const PackedRgb& color);
	void             setRed (int index, uint8 r);
	void             setGreen (int index, uint8 g);
	void             setBlue (int index, uint8 b);
	void             setNode (int index, bool node);

	const PackedRgb& getColor (int index) const;
	uint8            getRed (int index) const;
	uint8            getGreen (int index) const;
	uint8            getBlue (int index) const;
	bool             getNode (int index) const;

	void             load (Iff& iff);
	void             save (Iff& iff) const;

private:

	PackedRgb        m_ramp [256];
	bool             m_node [256];
};

//-------------------------------------------------------------------

inline const PackedRgb& ColorRamp256::getColor (int index) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE (0, index, 256);
	return m_ramp [index];
}

//-------------------------------------------------------------------

inline bool ColorRamp256::getNode (int index) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE (0, index, 256);
	return m_node [index];
}

//-------------------------------------------------------------------

inline uint8 ColorRamp256::getRed (int index) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE (0, index, 256);
	return m_ramp [index].r;
}

//-------------------------------------------------------------------

inline uint8 ColorRamp256::getGreen (int index) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE (0, index, 256);
	return m_ramp [index].g;
}

//-------------------------------------------------------------------

inline uint8 ColorRamp256::getBlue (int index) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE (0, index, 256);
	return m_ramp [index].b;
}

//===================================================================

#endif
