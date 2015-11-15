//
// MultiFractalReaderWriter.cpp
// asommers
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#include "sharedFractal/FirstSharedFractal.h"
#include "sharedFractal/MultiFractalReaderWriter.h"

#include "sharedFile/Iff.h"
#include "sharedFractal/MultiFractal.h"

//-------------------------------------------------------------------

void MultiFractalReaderWriter::save (Iff& iff, const MultiFractal& multiFractal)
{
	iff.insertForm (TAG (M,F,R,C));

		iff.insertForm (TAG_0001);

			iff.insertChunk (TAG_DATA);

				iff.insertChunkData (multiFractal.getSeed ());
				iff.insertChunkData (multiFractal.getUseBias () ? static_cast<int32> (1) : static_cast<int32> (0));
				iff.insertChunkData (multiFractal.getBias ());
				iff.insertChunkData (multiFractal.getUseGain () ? static_cast<int32> (1) : static_cast<int32> (0));
				iff.insertChunkData (multiFractal.getGain ());
				iff.insertChunkData (multiFractal.getNumberOfOctaves ());
				iff.insertChunkData (multiFractal.getFrequency ());
				iff.insertChunkData (multiFractal.getAmplitude ());
				iff.insertChunkData (multiFractal.getScaleX ());
				iff.insertChunkData (multiFractal.getScaleY ());
				iff.insertChunkData (multiFractal.getOffsetX ());
				iff.insertChunkData (multiFractal.getOffsetY ());
				iff.insertChunkData (static_cast<int32> (multiFractal.getCombinationRule ()));

			iff.exitChunk ();

		iff.exitForm ();

	iff.exitForm ();
}

//-------------------------------------------------------------------

void MultiFractalReaderWriter::load (Iff& iff, MultiFractal& multiFractal)
{
	iff.enterForm (TAG (M,F,R,C));

	switch (iff.getCurrentName ())
	{
	case TAG_0000:
		load_0000 (iff, multiFractal);
		break;

	case TAG_0001:
		load_0001 (iff, multiFractal);
		break;

	default:
		{
			char tagBuffer [5];
			ConvertTagToString (iff.getCurrentName (), tagBuffer);

			char buffer [128];
			iff.formatLocation (buffer, sizeof (buffer));
			DEBUG_FATAL (true, ("unknown affector type %s/%s", buffer, tagBuffer));
		}
		break;
	}

	iff.exitForm ();
}

//-------------------------------------------------------------------

void MultiFractalReaderWriter::load_0000 (Iff& iff, MultiFractal& multiFractal)
{
	iff.enterForm (TAG_0000);

		iff.enterChunk (TAG_DATA);

			multiFractal.setSeed (iff.read_uint32 ());

			const bool useBias = iff.read_int32 () != 0;
			const real bias = iff.read_float ();
			multiFractal.setBias (useBias, bias);

			const bool useGain = iff.read_int32 () != 0;
			const real gain = iff.read_float ();
			multiFractal.setGain (useGain, gain);

			multiFractal.setNumberOfOctaves (iff.read_int32 ());
			multiFractal.setFrequency (iff.read_float ());
			multiFractal.setAmplitude (iff.read_float ());

			const real scaleX = iff.read_float ();
			const real scaleY = iff.read_float ();
			multiFractal.setScale (scaleX, scaleY);

			multiFractal.setCombinationRule (static_cast<MultiFractal::CombinationRule> (iff.read_int32 ()));

		iff.exitChunk ();

	iff.exitForm ();
}

//-------------------------------------------------------------------

void MultiFractalReaderWriter::load_0001 (Iff& iff, MultiFractal& multiFractal)
{
	iff.enterForm (TAG_0001);

		iff.enterChunk (TAG_DATA);

			multiFractal.setSeed (iff.read_uint32 ());

			const bool useBias = iff.read_int32 () != 0;
			const real bias = iff.read_float ();
			multiFractal.setBias (useBias, bias);

			const bool useGain = iff.read_int32 () != 0;
			const real gain = iff.read_float ();
			multiFractal.setGain (useGain, gain);

			multiFractal.setNumberOfOctaves (iff.read_int32 ());
			multiFractal.setFrequency (iff.read_float ());
			multiFractal.setAmplitude (iff.read_float ());

			const real scaleX = iff.read_float ();
			const real scaleY = iff.read_float ();
			multiFractal.setScale (scaleX, scaleY);

			const real offsetX = iff.read_float ();
			const real offsetY = iff.read_float ();
			multiFractal.setOffset (offsetX, offsetY);

			multiFractal.setCombinationRule (static_cast<MultiFractal::CombinationRule> (iff.read_int32 ()));

		iff.exitChunk ();

	iff.exitForm ();
}

//-------------------------------------------------------------------

