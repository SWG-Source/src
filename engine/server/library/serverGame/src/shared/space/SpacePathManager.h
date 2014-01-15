// ======================================================================
// 
// SpacePathManager.h
// Copyright Sony Online Entertainment, Inc.
//
// ======================================================================

#ifndef INCLUDED_SpacePathManager_H
#define INCLUDED_SpacePathManager_H

class SpacePath;

// ----------------------------------------------------------------------
class SpacePathManager
{
public:

	static void install();
	static SpacePath * fetch(SpacePath * const path, void const * const object, float const objectRadius);
	static void release(SpacePath * const path, void const * const object);
	static void alter(float const deltaTime);
	static void removeFromRefineQueue(SpacePath * path);


private:

	static void remove();

	// Disable

	SpacePathManager();
	~SpacePathManager();
	SpacePathManager(SpacePathManager const &);
	SpacePathManager & operator =(SpacePathManager const &);
};

// ======================================================================

#endif // INCLUDED_SpacePathManager_H
