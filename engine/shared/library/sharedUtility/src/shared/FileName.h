// ======================================================================
//
// FileName.h
// asommers 5-26-99
//
// copyright 1999, bootprint entertainment
//
// ======================================================================

#ifndef INCLUDED_FileName_H
#define INCLUDED_FileName_H

// ======================================================================

class FileName
{
public:

	enum Path
	{
		P_none,

		P_animationMap,
		P_camera,
		P_inputmap,
		P_appearance,
		P_animation,
		P_music,
		P_object,
		P_sample,
		P_scene,
		P_script,
		P_shader,
		P_shaderEffect,
		P_terrain,
		P_texture,

		P_COUNT
	};

private:

	struct Table
	{
		const char* path;
		const char* ext;
	};

private:

	static Table pathTable [P_COUNT];

private:

	char* fullName;

private:

	FileName ();
	FileName (const FileName&);
	FileName& operator= (const FileName&);
	
public:

	explicit FileName (const char* const newName);
	FileName (const char* path, const char* filename, const char* ext=0);
	FileName (Path path, const char* filename, const char* ext=0);
	~FileName ();

	operator const char* () const; //lint !e1930 //-- conversion operator found
	const char* getString () const;

	void set (const char* const newName);
	void stripPathAndExt ();
	void stripPath ();
    void stripSpecificPathAndExt (Path path);
        
	static void stripPathAndExt (char* nameBuffer, int nameBufferLength);
	static void stripPath (char* nameBuffer, int nameBufferLength);
    static void stripSpecificPathAndExt (Path path, char* nameBuffer, int nameBufferLength);
}; 

//-------------------------------------------------------------------

inline FileName::operator const char* () const
{
	return fullName;
}

//-------------------------------------------------------------------

inline const char* FileName::getString () const
{
	return fullName;
}

// ======================================================================

#endif
