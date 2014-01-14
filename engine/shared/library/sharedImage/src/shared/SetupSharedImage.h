// ======================================================================
//
// SetupSharedImage.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#ifndef INCLUDED_SetupSharedImage_H
#define INCLUDED_SetupSharedImage_H

// ======================================================================

class TargaFormat;

// ======================================================================

class SetupSharedImage
{
public:

	struct Data
	{
		bool m_supportTarga;
	};

public:

	static void setupDefaultData(Data &data);
	static void install(const Data &data);

private:

	static void remove();

private:

	static bool         ms_installed;
	static TargaFormat *ms_targaFormat;
};

// ======================================================================

#endif
