//
// MultiFractalReaderWriter.h
// asommers
//
// copyright 2001, sony online entertainment
//

//-------------------------------------------------------------------

#ifndef INCLUDED_MultiFractalReaderWriter_H
#define INCLUDED_MultiFractalReaderWriter_H

//-------------------------------------------------------------------

class Iff;
class MultiFractal;

//-------------------------------------------------------------------

class MultiFractalReaderWriter
{
public:

	static void save (Iff& iff, const MultiFractal& multiFractal);
	static void load (Iff& iff, MultiFractal& multiFractal);

private:

	static void load_0000 (Iff& iff, MultiFractal& multiFractal);
	static void load_0001 (Iff& iff, MultiFractal& multiFractal);
};

//-------------------------------------------------------------------

#endif
