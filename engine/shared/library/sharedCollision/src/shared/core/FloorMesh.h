// ======================================================================
//
// FloorMesh.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_FloorMesh_H
#define INCLUDED_FloorMesh_H

// ======================================================================

#include "sharedCollision/CollisionMesh.h"
#include "sharedFoundation/Tag.h"
#include "sharedFoundation/DataResource.h"
#include "sharedCollision/CollisionEnums.h"

class FloorTri;
class ConnectedTri;
class Iff;
class FloorMesh;
class FloorLocator;
class Circle;
class Ribbon3d;
class DebugShapeRenderer;
class BaseClass;
class Appearance;

#define FLOOR_LOG(A)       DEBUG_REPORT_LOG_PRINT(ConfigSharedCollision::getReportMessages(),(A))
#define FLOOR_WARNING(A)   DEBUG_WARNING(ConfigSharedCollision::getReportWarnings(),(A))

template<class T>
class DataResourceList;
typedef DataResourceList<FloorMesh>	 FloorMeshList;
typedef std::vector<FloorTri>     FloorTriVec;
typedef std::pair<int,int>           IntPair;

class FloorEdgeId
{
public:

	FloorEdgeId () : triId(-1), edgeId(-1) {}

	FloorEdgeId ( int triangle, int edge ) : triId(triangle), edgeId(edge) {}

	int           triId;
	int           edgeId;
};

typedef std::vector<FloorEdgeId> FloorEdgeIdVec;

typedef std::vector<FloorLocator> FloorLocatorVec;

// ======================================================================

class FloorMesh : public CollisionMesh, public DataResource
{
public:

	explicit FloorMesh(const std::string & filename);
	FloorMesh( VectorVector const & vertices, IntVector const & indices );

	virtual ~FloorMesh();

	// ----------

	static void                 install         ( void );
	static void                 remove          ( void );

	static FloorMesh *          create          ( const std::string & filename );

	virtual void                calcBounds      ( void ) const;

	// ----------
	// Basic interface inherited from CollisionMesh - accessors/modifiers

	// -----

	virtual int                 getVertexCount  ( void ) const;
	virtual Vector const &      getVertex       ( int whichVertex ) const;
	virtual void                setVertex       ( int whichVertex, Vector const & newPoint );

	virtual int                 getTriCount     ( void ) const;
	virtual IndexedTri const &  getIndexedTri   ( int whichTri ) const;
	virtual void                setIndexedTri   ( int whichTri, IndexedTri const & newTri );

	virtual Triangle3d          getTriangle     ( int whichTri ) const;

	virtual void                deleteVertex    ( int whichVertex );
	virtual void                deleteVertices  ( IntVector const & vertIndices );

	virtual void                deleteTri       ( int whichTri );
	virtual void                deleteTris      ( IntVector const & triIndices );

	// ----------

	void clear();

	// ----------

	void                        clearMarks      ( int clearMarkValue );

	void                        setObjectFloor  ( bool objectFloor );

	void                        setAppearance   ( Appearance const * appearance );

	// ----------

	void                        addTriangle     ( Triangle3d const & newTri );
	void                        addTriangle     ( int iA, int iB, int iC );

	void                        build           ( VectorVector const & verts, IntVector const & indices );

	// ----------

	FloorTri &                  getFloorTri     ( int whichTri );
	FloorTri const &            getFloorTri     ( int whichTri ) const;

	FloorEdgeIdVec const &      getEdgeList     ( FloorEdgeType edgeType ) const;

	// ----------

	BaseClass *                 getPathGraph    ( void );
	BaseClass const *           getPathGraph    ( void ) const;
	void                        attachPathGraph ( BaseClass * newGraph );

	// ----------

	void    addCrossableEdges       ( IntVector const & edgePairs );
	void    addRampEdges            ( IntVector const & edgePairs );
	void    addFallthroughTris      ( IntVector const & triIDs );
	bool    flagPortalEdges         ( VectorVector const & portalVerts, int portalId );
	void    clearPortalEdges        ( void );

	// ----------

	void    write           ( Iff & iff );
	void    read            ( Iff & iff );
	void    read_0000       ( Iff & iff );
	void    read_0001       ( Iff & iff );
	void    read_0002       ( Iff & iff );
	void    read_0003       ( Iff & iff );
	void    read_0004       ( Iff & iff );
	void    read_0005       ( Iff & iff );
	void    read_0006       ( Iff & iff );

	// ----------

	void    compile         ( void );       //! Do this once all data has been added to the floor mesh

	// ----------
	// Interface inherited from CollisionSurface

	virtual Transform const &   getTransform_o2p    ( void ) const;

	// ----------
	// Floor-specific methods

	bool    findFloorTri        ( FloorLocator const & testLoc,
	                              float heightTolerance, 
	                              bool bAllowJump, 
	                              FloorLocator & outLoc ) const;
	
	bool    findFloorTri        ( FloorLocator const & testLoc,
	                              bool bAllowJump, 
	                              FloorLocator & out ) const;
	
	PathWalkResult pathWalkPoint( FloorLocator const & startLoc, Vector const & delta, 
	                              int & outHitTriId, int & outHitEdge, float & outHitTime ) const;
	
	PathWalkResult pathWalkCircle(FloorLocator const & startLoc, Vector const & delta,
								  int ignoreTriId, int ignoreEdge,
	                              FloorLocator & result ) const;
	
	PathWalkResult pathWalkCircleGetIds(FloorLocator const & startLoc, Vector const & delta,
								  int ignoreTriId, int ignoreEdge,
	                              float & outHitTime, int & outHitTriId, int & outHitEdge, int & outCenterTriId ) const;
		
	void    makeHitResult       ( Vector const & begin, 
	                              Vector const & delta, 
	                              int hitTriId, 
	                              int hitEdge, 
	                              float hitTime,
	                              FloorLocator & result ) const;

	void    makeHitResult2      ( Vector const & begin, 
	                              Vector const & delta, 
	                              int centerTriId,
	                              int hitTriID, 
	                              int hitEdge, 
	                              float hitTime,
	                              FloorLocator & result ) const;
	
	void    makeExitResult      ( Vector const & begin, 
	                              Vector const & delta, 
	                              int hitTriID, 
	                              int hitEdge, 
	                              float hitTime,
	                              FloorLocator & result ) const;
	
	void    makeHitPortalResult ( Vector const & begin, 
	                              Vector const & delta, 
	                              int hitTriID, 
	                              int hitEdge, 
	                              float hitTime,
	                              FloorLocator & result ) const;
	
	bool    calcHitTime         ( Segment3d const & seg,
	                              int hitTri,
	                              int hitEdge,
	                              float & outHitTime ) const;
	
	bool    attach              ( FloorLocator & loc ) const;

	void    makeSuccessResult   ( FloorLocator const & loc, FloorLocator & result ) const;
	
	void    makeFailureResult   ( FloorLocator & result ) const;
	
	bool    intersectBoundary   ( FloorLocator const & startLoc, Vector const & delta, bool useRadius, FloorLocatorVec & results ) const;

	bool    intersectBoundary   ( FloorEdgeIdVec * edgeList, FloorLocator const & startLoc, Vector const & delta, bool useRadius, FloorLocatorVec & results ) const;

	bool    intersectEdge       ( FloorLocator const & startLoc, Vector const & delta, FloorEdgeId const & id, bool useRadius, bool testFront, FloorLocator & outLoc ) const;

	bool    findEntrance        ( FloorLocator const & loc, Vector const & delta, bool useRadius, FloorLocator & outLoc ) const;

	PathWalkResult findStartingTri ( FloorLocator const & startLoc, Vector const & delta, bool useRadius, int & outTriId ) const;

	bool    findClosestLocation ( FloorLocator const & testLoc, FloorLocator & outLoc ) const;
	
	bool    dropTest            ( FloorLocator const & testLoc, FloorLocator & outLoc ) const;
	bool    dropTest            ( FloorLocator const & testLoc, float hopHeight, FloorLocator & outLoc ) const;
	bool    dropTest            ( FloorLocator const & testLoc, int triId, FloorLocator & outLoc ) const;
	
	void    drawDebugShapes     ( DebugShapeRenderer * renderer, bool drawExtent ) const;
	
	bool    validate            ( FloorLocator const & loc ) const;
	
	// Determine if the locator can fit on the floor without hitting a non-crossable edge
	
	bool    testClear           ( FloorLocator const & testLoc ) const;
	bool    testAboveCrossables ( FloorLocator const & testLoc ) const;
	bool    testAboveCrossables ( FloorLocator const & testLoc, Vector const & delta ) const;
	bool    getClosestCollidableEdge ( FloorLocator const & testLoc, int & outTriId, int & outEdgeId, float & outDist ) const;
	
	bool    testConnectable     ( FloorLocator const & A, FloorLocator const & B ) const;
	
	void    loadFromIff         ( Iff & file );

	bool    getGoodLocation     ( float radius, Vector & outLoc ) const;

	bool    canEnterEdge        ( FloorLocator const & enterLoc ) const;
	bool    canExitEdge         ( FloorLocator const & exitLoc, Vector const & delta, bool useRadius) const;

	bool    canEnterEdge        ( FloorLocator const & startLoc, Vector const & delta, FloorEdgeId const & id, bool useRadius ) const;
	bool    canExitEdge         ( FloorLocator const & startLoc, Vector const & delta, FloorEdgeId const & id, bool useRadius ) const;

	bool    getDistanceUncrossable2d  ( Vector const & position_l, float maxDistance, float & outDistance, FloorEdgeId & outEdgeId ) const;

	int     getPartCount        ( void ) const;

protected:

	// ----------
	// Interface inherited from DataResource

	virtual void    load        ( Iff &file );
	virtual Tag     getId       ( void ) const;
	virtual void    release     ( void ) const;

	// ----------

	void    sweep               ( void );       //! Do a mark-sweep to remove any unused vertices
	void    link                ( void );       //! Build connectivity info for the floor tris
	void    flagCrossableEdges  ( void );       //! Mark all edges with neighbors as crossable
	void    assignIndices       ( void );       //! Reassign indices to all the tris in the mesh
	void    setPartTags         ( void );       //! Calculate part tags for each floor tri
	void    calcHeightFuncs     ( void );

	int     getTriMarkValue     ( void ) const;

	typedef std::pair<int,int> EdgeId;

	typedef std::vector<EdgeId> EdgeIdVec;

	void            findAdjacentBoundaryEdges   ( VectorVector const & polyVerts, EdgeIdVec & outIds ) const;

	static bool     matchSegmentToPoly          ( Vector const & a, Vector const & b, VectorVector const & polyVerts );

	void            buildBoundaryEdgeList       ( void );
	void            buildCrossableEdgeList      ( void );

#ifdef _DEBUG

	virtual void buildDebugData ( void );

	VectorVector * m_crossableLines;
	VectorVector * m_uncrossableLines;
	VectorVector * m_interiorLines;
	VectorVector * m_portalLines;
	VectorVector * m_rampLines;
	VectorVector * m_fallthroughTriLines;
	VectorVector * m_solidTriLines;
	VectorVector * m_pathLines;

#endif

private:

	// Disable copying

	FloorMesh(const FloorMesh &source);
	FloorMesh & operator =(const FloorMesh &source);

	// ----------
	// Data

	VectorVector *      m_vertices;
	FloorTriVec *       m_floorTris;

	FloorEdgeIdVec *    m_crossableEdges;
	FloorEdgeIdVec *    m_uncrossableEdges;
	FloorEdgeIdVec *    m_wallBaseEdges;
	FloorEdgeIdVec *    m_wallTopEdges;

	BaseClass *         m_pathGraph; // blind pointer to path graph data

	Appearance const *  m_appearance;

	mutable int         m_triMarkCounter;

	mutable bool        m_objectFloor;
};

// ----------------------------------------------------------------------

inline void FloorMesh::loadFromIff(Iff &file)
{
	load(file);
}

inline void FloorMesh::setObjectFloor ( bool objectFloor )
{
	m_objectFloor = objectFloor;
}

// ----------------------------------------------------------------------

#endif
