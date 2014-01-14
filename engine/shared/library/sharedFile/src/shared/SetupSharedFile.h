// ======================================================================
//
// SetupSharedFile.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SetupSharedFile_H
#define INCLUDED_SetupSharedFile_H

// ======================================================================

class SetupSharedFile
{
public:

	static void install(bool useFileStreamer, uint32 skuBits=0);

private:
	SetupSharedFile();
	SetupSharedFile(const SetupSharedFile &);
	SetupSharedFile &operator =(const SetupSharedFile &);
};

// ======================================================================

#endif
