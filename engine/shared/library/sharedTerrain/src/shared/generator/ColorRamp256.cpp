//===================================================================
//
// ColorRamp256.cpp
// asommers
//
// copyright 2001, sony online entertainment
//
//===================================================================

#include "sharedTerrain/FirstSharedTerrain.h"
#include "sharedTerrain/ColorRamp256.h"

#include "sharedFile/Iff.h"

//===================================================================

namespace
{
	const Tag TAG_RAMP = TAG (R,A,M,P);
}

//===================================================================

ColorRamp256::ColorRamp256 ()
	//lint -esym (1926, ColorRamp256::m_ramp)  // not in initializer list
	//lint -esym (1926, ColorRamp256::m_node)  // not in initializer list
{
	int i;
	for (i = 0; i < 256; ++i)
	{
		m_ramp [i] = PackedRgb::solidBlack;
		m_node [i] = false;
	}

	m_node [0]   = true;
	m_node [255] = true;
}

//-------------------------------------------------------------------

ColorRamp256::~ColorRamp256 ()
{
}

//-------------------------------------------------------------------

void ColorRamp256::setColor (int index, const PackedRgb& color)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE (0, index, 256);
	m_ramp [index] = color;
}

//-------------------------------------------------------------------

void ColorRamp256::setNode (int index, bool node)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE (0, index, 256);
	m_node [index] = node;
}

//-------------------------------------------------------------------

void ColorRamp256::setRed (int index, uint8 r)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE (0, index, 256);
	m_ramp [index].r = r;
}

//-------------------------------------------------------------------

void ColorRamp256::setGreen (int index, uint8 g)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE (0, index, 256);
	m_ramp [index].g = g;
}

//-------------------------------------------------------------------

void ColorRamp256::setBlue (int index, uint8 b)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE (0, index, 256);
	m_ramp [index].b = b;
}

//-------------------------------------------------------------------

void ColorRamp256::load (Iff& iff)
{
	iff.enterForm (TAG_RAMP);
		iff.enterChunk (TAG_0000);

			int i;
			for (i = 0; i < 256; ++i)
			{
				m_ramp [i].r = iff.read_uint8 ();
				m_ramp [i].g = iff.read_uint8 ();
				m_ramp [i].b = iff.read_uint8 ();
				m_node [i]   = iff.read_uint8 () != 0;
			}

		iff.exitChunk ();
	iff.exitForm ();
}

//-------------------------------------------------------------------

void ColorRamp256::save (Iff& iff) const
{
	iff.insertForm (TAG_RAMP);
		iff.insertChunk (TAG_0000);

			int i;
			for (i = 0; i < 256; ++i)
			{
				iff.insertChunkData (m_ramp [i].r);
				iff.insertChunkData (m_ramp [i].g);
				iff.insertChunkData (m_ramp [i].b);
				iff.insertChunkData (m_node [i] ? static_cast<char> (1) : static_cast<char> (0));
			}

		iff.exitChunk ();
	iff.exitForm ();
}

//===================================================================

