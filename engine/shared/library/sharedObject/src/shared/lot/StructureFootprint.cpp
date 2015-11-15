//===================================================================
//
// StructureFootprint.cpp
// asommers
//
// copyright 2002, sony online entertainment
//
//===================================================================

#include "sharedObject/FirstSharedObject.h"
#include "sharedObject/StructureFootprint.h"

#include "sharedFile/Iff.h"

#include <string>

//===================================================================

namespace StructureFootprintLoaderNamespace
{
	const Tag TAG_FOOT = TAG (F,O,O,T);
	const Tag TAG_PRNT = TAG (P,R,N,T);
}

using namespace StructureFootprintLoaderNamespace;

//===================================================================
// PUBLIC StructureFootprint
//===================================================================

StructureFootprint::StructureFootprint () :
	m_width (1),
	m_height (1),
	m_boxTestRect(),
	m_pivotX (0),
	m_pivotZ (0),
	m_data (new LotType [1]),
	m_hardReservationTolerance (1.f),
	m_structureReservationTolerance (1.f),
	m_numberOfReservations (1),
	m_numberOfHardReservations (0),
	m_numberOfStructureReservations (1)
{
	//-- the default reservation takes up just lot itself
	m_data [0] = LT_structure;
}

//-------------------------------------------------------------------
	
StructureFootprint::~StructureFootprint ()
{
	delete [] m_data;
	m_data = 0;
}

//-------------------------------------------------------------------

void StructureFootprint::load (const char* const filename)
{
	delete [] m_data;
	m_data = 0;

	Iff iff;
	if (iff.open (filename, true))
		load (iff);
	else
		DEBUG_WARNING (true, ("StructureFootprint::load - %s not found", filename));
}

//-------------------------------------------------------------------

int StructureFootprint::getWidth () const
{
	return m_width;
}

//-------------------------------------------------------------------

int StructureFootprint::getHeight () const
{
	return m_height;
}

//-------------------------------------------------------------------

Rectangle2d const & StructureFootprint::getBoxTestRect () const
{
	return m_boxTestRect;
}

//-------------------------------------------------------------------

int StructureFootprint::getPivotX () const
{
	return m_pivotX;
}

//-------------------------------------------------------------------

int StructureFootprint::getPivotZ () const
{
	return m_pivotZ;
}

//-------------------------------------------------------------------

LotType StructureFootprint::getLotType (int x, int z) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE (0, x, m_width);
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE (0, z, m_height);

	return m_data [m_width * z + x];
}

//-------------------------------------------------------------------

float StructureFootprint::getHardReservationTolerance () const
{
	return m_hardReservationTolerance;
}

//-------------------------------------------------------------------

float StructureFootprint::getStructureReservationTolerance () const
{
	return m_structureReservationTolerance;
}

//-------------------------------------------------------------------

int StructureFootprint::getNumberOfReservations () const
{
	return m_numberOfReservations;
}

//-------------------------------------------------------------------

int StructureFootprint::getNumberOfHardReservations () const
{
	return m_numberOfHardReservations;
}

//-------------------------------------------------------------------

int StructureFootprint::getNumberOfStructureReservations () const
{
	return m_numberOfStructureReservations;
}

//-------------------------------------------------------------------

void StructureFootprint::getBorder (int x, int z, bool& top, bool& bottom, bool& left, bool& right) const
{
	top    = false;
	bottom = false;
	left   = false;
	right  = false;

	const bool center           = getLotType (x, z) == LT_structure;
	const bool outOfRangeTop    = z < 0 + 1;
	const bool outOfRangeBottom = z >= m_height - 1;
	const bool outOfRangeLeft   = x < 0 + 1;
	const bool outOfRangeRight  = x >= m_width - 1;

	if (center)
	{
		top    = outOfRangeTop    || (!outOfRangeTop    && getLotType (x,     z - 1) != LT_structure);
		bottom = outOfRangeBottom || (!outOfRangeBottom && getLotType (x,     z + 1) != LT_structure);
		left   = outOfRangeLeft   || (!outOfRangeLeft   && getLotType (x - 1, z    ) != LT_structure);
		right  = outOfRangeRight  || (!outOfRangeRight  && getLotType (x + 1, z    ) != LT_structure);
	}
}

//===================================================================
// PRIVATE StructureFootprint
//===================================================================

void StructureFootprint::load (Iff& iff)
{
	iff.enterForm (TAG_FOOT);

	switch (iff.getCurrentName ())
	{
	case TAG_0000:
		load_0000 (iff);
		break;

	default:
		{
			char tagBuffer [5];
			ConvertTagToString (iff.getCurrentName (), tagBuffer);

			char buffer [128];
			iff.formatLocation (buffer, sizeof (buffer));
			DEBUG_FATAL (true, ("StructureFootprint::load - unknown version %s/%s", buffer, tagBuffer));
		}
		break;
	}

	iff.exitForm (TAG_FOOT);
}

//-------------------------------------------------------------------

void StructureFootprint::load_0000 (Iff& iff)
{
	iff.enterForm (TAG_0000);

		//--
		iff.enterChunk (TAG_INFO);

			m_width  = iff.read_int32 ();
			DEBUG_FATAL (m_width == 0, ("width == 0"));
			m_height = iff.read_int32 ();
			DEBUG_FATAL (m_height == 0, ("height == 0"));

			m_pivotX = iff.read_int32 ();
			m_pivotZ = iff.read_int32 ();
			m_hardReservationTolerance = iff.read_float ();
			m_structureReservationTolerance = iff.read_float ();

		iff.exitChunk (TAG_INFO);

		m_boxTestRect.x0 = m_boxTestRect.y0 = 0.0f;
		m_boxTestRect.x1 = static_cast<float>(m_width);
		m_boxTestRect.y1 = static_cast<float>(m_height);

		if (iff.enterChunk(TAG_DATA, true))
		{
			m_boxTestRect.x0 = static_cast<float>(iff.read_int32());
			m_boxTestRect.y0 = static_cast<float>(iff.read_int32());
			m_boxTestRect.x1 = static_cast<float>(iff.read_int32());
			m_boxTestRect.y1 = static_cast<float>(iff.read_int32());
			 
			iff.exitChunk(TAG_DATA);
		}

		//--
		iff.enterChunk (TAG_PRNT);

			std::string footprint;
			
			int i;
			for (i = 0; i < m_height; ++i)
			{
				DEBUG_FATAL (iff.getChunkLengthLeft () == 0, ("StructureFootprintLoader::load_0000 [%s]: height mismatch", iff.getFileName ()));

				char* buffer = iff.read_string ();
				DEBUG_FATAL (istrlen (buffer) != m_width, ("StructureFootprintLoader::load_0000 [%s]: width mismatch", iff.getFileName ()));

				const std::string tempFootprint = footprint + buffer;
				footprint = tempFootprint;

				delete [] buffer;
			}

		iff.exitChunk (TAG_PRNT);

		//-- allocate footprint
		IS_NULL (m_data);
		m_data = new LotType [static_cast<size_t> (m_width * m_height)];

		//-- convert string to footprint
		const char* const buffer = footprint.c_str ();

		m_numberOfReservations = 0;
		m_numberOfHardReservations = 0;
		m_numberOfStructureReservations = 0;

		const int n = m_width * m_height;
		for (i = 0; i < n; ++i)
		{
			switch (buffer [i])
			{
			case 'F':
			case 'f':
				m_data [i] = LT_structure;
				++m_numberOfReservations;
				++m_numberOfStructureReservations;
				break;

			case 'H':
			case 'h':
				m_data [i] = LT_hard;
				++m_numberOfReservations;
				++m_numberOfHardReservations;
				break;

			case '.':
				m_data [i] = LT_nothing;
				break;

			default:
				m_data [i] = LT_hard;
				++m_numberOfReservations;
				++m_numberOfHardReservations;
				DEBUG_WARNING (true, ("StructureFootprintLoader::load [%s] - unknown lot type %c", iff.getFileName (), buffer [i]));
				break;
			}
		}

	iff.exitForm (TAG_0000);
}

//===================================================================
