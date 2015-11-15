// ======================================================================
//
// FloorManager.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_FloorManager_H
#define INCLUDED_FloorManager_H

// ======================================================================

class Appearance;
class BaseClass;
class DebugShapeRenderer;
class Floor;
class Iff;
class Object;

typedef BaseClass * (*ObjectFactory)  ( Iff & iff );
typedef void        (*ObjectWriter)   ( BaseClass const * object, Iff & iff );
typedef void        (*ObjectRenderer) ( BaseClass const * object, DebugShapeRenderer * renderer );

// ----------------------------------------------------------------------

class FloorManager
{
public:

	// ----------
	
	static void           setPathGraphFactory  ( ObjectFactory factory );
	static ObjectFactory  getPathGraphFactory  ( void );
	
	static void           setPathGraphWriter   ( ObjectWriter writer );
	static ObjectWriter   getPathGraphWriter   ( void );

	static void           setPathGraphRenderer ( ObjectRenderer renderer );
	static ObjectRenderer getPathGraphRenderer ( void );
	
	// ----------
	
	//! Load the floor mesh with the given filename and create a new floor
	// that uses the mesh
	
	static Floor *  createFloor         ( const char * floorMeshFilename, Object const * owner, Appearance const * appearance, bool objectFloor );
};

// ======================================================================

#endif // #ifndef INCLUDED_FloorManager_H

