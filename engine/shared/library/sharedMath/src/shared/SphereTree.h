// SphereTree.h
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_SphereTree_H
#define	_INCLUDED_SphereTree_H

//-----------------------------------------------------------------------

#include "sharedMath/Sphere.h"
#include "sharedMath/Capsule.h"
#include "sharedMath/SpatialSubdivision.h"
#include "sharedMath/SphereTreeNode.h"

//-----------------------------------------------------------------------
/**
	@brief a templatized implementation of a sphere tree. 

	A SphereTree organizes data hierarchically in sphere nodes.

	Sphere nodes have a containment relationship, starting from the
	more general, larger sphere nodes, to smaller, more granular
	nodes which contain objects. Any node may contain objects as well
	as child nodes.

	The tree balances itself as it discovers usable world space. 
	Its depth is defined by the potential worldspace covered by the tree
	and the extent data of the objects contained within the tree.

	The default behavior of the tree is to provide general coverage of
	world space with about 32 nodes. Child nodes follow this heuristic
	as items are inserted into the tree. Item extents (by default) 
	want to be ten times smaller than a container node's maximum extent.
	If this ratio is exceeded, the object continues to fall through the
	tree until a suitable leaf node is located.

	These heuristics may be tweaked, but generally show good behavior, 
	favoring move, range, and reintegration to insertion performance.

	In simulations with 1 million objects in a 16 cubic kilometer world
	space, the tree provides performance of over 1,000 move operations
	per millisecond.

	How it works ---
	
	INSERTING 

	When an object is first added to the SphereTree, it is passed to 
	the root node, which encompasses all world space. If the object
	falls outside of the root node, the root node is expanded, and
	new max sizes for child nodes are calculated to keep the tree
	generally balanced for future operations. If the object extent
	exceeds the ratio of object-to-leaf-max size, then the maximum
	node size is again recalculated for general balancing. The first
	few insertions into the tree will trigger these recalculations,
	but have no effect on existing nodes.

	Once the balancing calculations are done (if they were needed at
	all in the first place), the root node finds a candidate child
	node to accept the new object. If no candidate exists, a new
	child sphere is created, and the object is passed to the child.

	The child will repeat the previous step, finding candidates
	(new nodes don't check, they don't have children), and passing
	the object further down the tree until a suitably sized child 
	node contains the object. 

	The default behavior is to find child nodes that are a
	factor of 4 times smaller with each step towards the solution
	leaf node. The solution leaf must be (by default) 10 times larger
	than objects it contains.

	An object with a 1 meter sphere in a 16 kilometer world follows
	this path: [ root -> 4k child -> 1k child -> 250 meter child ->
		62 meter child ] or a depth of 4.

	MOVING

	When an object moves, the destination sphere is checked against
	the containing node. If the real size of the containing node's
	sphere can contain the object, the operation is complete.

	If the real node size cannot contain the object, then a check
	is made to see if the max size can contain the object. If it
	can, the node is resized and the operation completes.

	If the max sphere size cannot contain the object, then the
	object is passed to the parent. If the object can be contained within
	the parent, then a candidate target node is identified and the
	object is then placed in it.
	
	In most cases, the object stays within the real sphere size. The
	next most common case is the object being contained in the container
	node's max sphere. Both of these are very fast operations and
	incur little cost on the system. Re-integration is slightly more
	expensive but also not as common. 

	The worst case is that all objects warp around and must be
	fully re-integrated from the root node. This is as expensive
	as insertion plus the time required to traverse up the tree
	and check containment candidates en route to the root node.

	The worst case simulation with 1 million objects clocked move
	operations at < 0.05 milliseconds (all 1 million objects moving to 
	origin from random locations)

	FINDING IN RANGE

	When the SphereTree receives a find request, it queries the root
	node for all child spheres which intersect the sphere described
	by the range query (location, radius). A recursive query is 
	made into child nodes which satisfy the intersection query,
	culling more candidates as the find operations solves for all objects
	intersecting the range sphere. 

	Find operations are faster for smaller range spheres, slower for
	larger spheres, averaging thousands of results per millisecond. Most
	of the time is spent filling a result vector.

	OTHER NOTES

	The sphere tree defines a node handle that derives from 
	SpatialSubdivisionHandle. Since the superclass API defines
	operations in terms of a SpatialSubdivisionHandle, the sphere
	tree can use this handle to go directly to a leaf node for
	move operations. The application using the SphereTree merely
	needs to perform all operations (except addObject) in terms
	of the handle.

	Tests were performed on a 700MHz P-III system with 256MB RAM running
	Windows 2000. The dev environment is MSVC.

	SphereTree's are succesfully in use on Linux, gcc-2.95-3 on a RedHat 6.2,
	7.0 and 7.1 distributions.


	DEPENDENCIES

	Sphere that defines the following:
	\code
	void  setCenter(real x, real y, real z);
	void  setCenter(const Vector &center);
	void  setRadius(real radius);

	const Vector &getCenter() const;
	const real    getRadius() const;

	bool contains(const Vector &point) const;
	bool contains(const Sphere &other) const;
	bool intersectsSphere(const Sphere &other) const;
	bool intersectsLine(const Vector &startPoint, const Vector &endPoint) const;
	bool intersectsLineSegment(const Vector &startPoint, const Vector &endPoint) const;
	bool intersectsRay(const Vector & startPoint, const Vector & normalizedDirection) const;
	\endcode

	and Vector that defines
	\code
	real                 magnitudeSquared(void) const;
	real                 magnitude(void) const;
	real                 magnitudeBetween(const Vector &vector) const;
	real                 magnitudeBetweenSquared(const Vector &vector) const;
	const Vector         findClosestPointOnLine(const Vector &line0, const Vector &line1) const;
	const Vector         findClosestPointOnLineSegment(const Vector & startPoint, const Vector & endPoint) const;
	real                 dot(const Vector &vector) const;
	\endcode

	These dependencies can be satisfied with the BootPrint/Sony Online Entertainment 
	sharedMath library.

	@see SpatialSubdivision
	@see SpatialSubdivisionHandle

	@author Justin Randall
*/
template<class ObjectType, class ExtentAccessor>
class SphereTree : public SpatialSubdivision<ObjectType, Sphere, ExtentAccessor>
{
public:
	typedef SphereTreeNode<ObjectType, ExtentAccessor> NodeType;

	                                    SphereTree         ();
	                                    ~SphereTree        ();
	virtual SpatialSubdivisionHandle *  addObject          (ObjectType object);
	virtual const bool                  canSee             (SpatialSubdivisionHandle * target, const Vector & start, const float distance, const float fov=0.0f) const;
	void dumpSphereTreeObjects(std::vector<ObjectType> & results) const;
	void                                dumpSphereTree     (std::vector<std::pair<ObjectType, Sphere> > & results) const;
	void                                dumpSphereTreeNodes(std::vector<std::pair<ObjectType, Sphere> > & results) const;
	void                                dumpSphereTreeObjs (std::vector<std::pair<ObjectType, Sphere> > & results) const;
	void                                dumpEdgeList       (std::vector<Vector> & results) const;
	virtual void                        findInRange        (const Vector & origin, const float distance, std::vector<ObjectType> & results) const;
	virtual void                        findInRange        (const Vector & origin, const float distance, std::vector<ObjectType> & results, int & testCounter) const;
	virtual void                        findInRange        (const Vector & origin, const float distance, const SpatialSubdivisionFilter<ObjectType> &filter, std::vector<ObjectType> & results) const;
	virtual void                        findInRange        (const Vector & origin, const float distance, const SpatialSubdivisionFilter<ObjectType> &filter, std::vector<ObjectType> & results, int & testCounter) const;
	virtual void                        findOnRay          (const Vector & begin, const Vector & dir, std::vector<ObjectType> & results) const;
	virtual void                        findOnSegment      (const Vector & begin, const Vector & end, std::vector<ObjectType> & results) const;
	virtual void findOnSegment(Vector const & begin, Vector const & end, SpatialSubdivisionFilter<ObjectType> const & filter, std::vector<ObjectType> & results) const;
	virtual void                        findAtPoint        (const Vector & point, std::vector<ObjectType> & results) const;
	virtual void                        findInRange        (const Capsule & range, std::vector<ObjectType> & results) const;
	virtual void                        findInRange        (const Capsule & range, const SpatialSubdivisionFilter<ObjectType> &filter, std::vector<ObjectType> & results) const;
	virtual bool                        findClosest        (const Vector & begin, const float maxDistance, ObjectType & outClosest, float & outMinDistance, float & outMaxDistance) const;
	virtual bool                        findClosest        (const Vector & begin, const float maxDistance, ObjectType & outClosest, float & outMinDistance, float & outMaxDistance, int & testCounter) const;
	virtual bool                        findClosest2d      (const Vector & begin, const float maxDistance, ObjectType & outClosest, float & outMinDistance, float & outMaxDistance) const;
	virtual bool                        findClosest2d      (const Vector & begin, const float maxDistance, ObjectType & outClosest, float & outMinDistance, float & outMaxDistance, int & testCounter) const;
	virtual void                        move               (SpatialSubdivisionHandle * object);
	virtual void                        removeObject       (SpatialSubdivisionHandle * object);
	virtual void                        validate           () const;
	virtual int                         getNodeCount       () const;
	bool                                empty              () const;
	virtual int                         getObjectCount     () const;

	void                                apply              (typename SphereTreeNode<ObjectType,ExtentAccessor>::NodeFunctor N);

	bool isWithin (Vector const & position) const;
	Sphere const &                      getRealSphere      () const;

private:
	SphereTree &  operator =  (const SphereTree & rhs);
	              SphereTree  (const SphereTree & source);
private:
	NodeType           root;
};

//-----------------------------------------------------------------------
/**
	@brief construct a sphere tree

	Initializes the root node with no parent.

	ExtentAccessor is defined in terms of a struct with a 
	getExtent(ObjectType) member function that returns a Sphere.
*/
template<class ObjectType, class ExtentAccessor>
inline SphereTree<ObjectType, ExtentAccessor>::SphereTree() :
root()
{
}

//-----------------------------------------------------------------------
/**
	@brief destroy the sphere tree

	Doesn't do anything. Included here for completeness.
*/
template<class ObjectType, class ExtentAccessor>
inline SphereTree<ObjectType, ExtentAccessor>::~SphereTree()
{
}

//-----------------------------------------------------------------------
/**
	@brief add an object to the sphere tree

	Calls addObject on the root node.

	@see SphereTreeNode
	@see SpatialSubdivision::addObject(ObjectType object)

	@author Justin Randall
*/
template<class ObjectType, class ExtentAccessor>
inline SpatialSubdivisionHandle * SphereTree<ObjectType, ExtentAccessor>::addObject(ObjectType object)
{
	return root.addObject(object);
}

//-----------------------------------------------------------------------
/**
	@brief not implemented

	@todo implement!
*/
template<class ObjectType, class ExtentAccessor>
inline const bool SphereTree<ObjectType, ExtentAccessor>::canSee(SpatialSubdivisionHandle * target, const Vector & start, const float distance, const float fov) const
{
	UNREF(target);
	UNREF(start);
	UNREF(distance);
	UNREF(fov);

	//@todo implement
	return true;
}

//-----------------------------------------------------------------------

template<class ObjectType, class ExtentAccessor>
inline void SphereTree<ObjectType, ExtentAccessor>::dumpSphereTreeObjects(std::vector<ObjectType> & results) const
{
	root.dumpSphereTreeObjects(results);
}

template<class ObjectType, class ExtentAccessor>
inline void SphereTree<ObjectType, ExtentAccessor>::dumpSphereTree(std::vector<std::pair<ObjectType, Sphere> > & results) const
{
	root.dumpSphereTree(results);
}

template<class ObjectType, class ExtentAccessor>
inline void SphereTree<ObjectType, ExtentAccessor>::dumpSphereTreeNodes(std::vector<std::pair<ObjectType, Sphere> > & results) const
{
	root.dumpSphereTreeNodes(results);
}

template<class ObjectType, class ExtentAccessor>
inline void SphereTree<ObjectType, ExtentAccessor>::dumpSphereTreeObjs(std::vector<std::pair<ObjectType, Sphere> > & results) const
{
	root.dumpSphereTreeObjs(results);
}

template<class ObjectType, class ExtentAccessor>
inline void SphereTree<ObjectType, ExtentAccessor>::dumpEdgeList(std::vector<Vector>& results) const
{
	root.dumpEdgeList(results);
}

//-----------------------------------------------------------------------
/**
	@brief find all objects contained in a sphere defined by the range params

	Finds all objects intersecting the sphere described by the origin and 
	distance parameters. The results are placed in the results vector.

	@param origin     A point describing the center of the range query
	@param distance   The radius of the query. The origin and distance
	                  are used to create a Sphere at 'origin' with radius
					  'distance'
	@param results    A user supplied vector of ObjectType that will receive
	                  the solution to the range query.

	@see SpatialSubdivision::findInRange()
	@see SphereTreeNode::findInRange()

	@author Justin Randall
*/
template<class ObjectType, class ExtentAccessor>
inline void SphereTree<ObjectType, ExtentAccessor>::findInRange(const Vector & origin, const float distance, std::vector<ObjectType> & results) const
{
	const Sphere range(origin, distance);
	root.findInRange(range, results);
}

template<class ObjectType, class ExtentAccessor>
inline void SphereTree<ObjectType, ExtentAccessor>::findInRange(const Vector & origin, const float distance, std::vector<ObjectType> & results, int & testCounter) const
{
	const Sphere range(origin, distance);
	root.findInRange(range, results, testCounter);
}

//-----------------------------------------------------------------------
/**
	@brief find all objects contained in a sphere defined by the range params 
  which meet a filter criteria

	Finds all objects intersecting the sphere described by the origin and 
	distance parameters. The results are placed in the results vector.

	@param origin     A point describing the center of the range query
	@param distance   The radius of the query. The origin and distance
	                  are used to create a Sphere at 'origin' with radius
					  'distance'
  @param filter     A user supplied filter functor.  
	                  filter.operator(const ObjectType & object) should return 
                    true if the object is to be returned.
	@param results    A user supplied vector of ObjectType that will receive
	                  the solution to the range query.

	@see SpatialSubdivision::findInRange()
	@see SphereTreeNode::findInRange()

	@author Acy Stapp
*/

template<class ObjectType, class ExtentAccessor>
inline void SphereTree<ObjectType, ExtentAccessor>::findInRange(const Vector & origin, const float distance, const SpatialSubdivisionFilter<ObjectType> &filter, std::vector<ObjectType> & results) const
{
	const Sphere range(origin, distance);
	root.findInRange(range, filter, results);
}

template<class ObjectType, class ExtentAccessor>
inline void SphereTree<ObjectType, ExtentAccessor>::findInRange(const Vector & origin, const float distance, const SpatialSubdivisionFilter<ObjectType> &filter, std::vector<ObjectType> & results, int & testCounter) const
{
	const Sphere range(origin, distance);
	root.findInRange(range, filter, results, testCounter);
}

//-----------------------------------------------------------------------
/**
	@brief find all objects that hit the given ray

	Finds all objects intersecting the ray described by the start point 
    and direction parameters. The results are placed in the results vector.

	@param begin      A point describing the origin of the ray query
	@param dir        A vector describing the direction of the ray
	@param results    A user supplied vector of ObjectType that will receive the results of the query.

	@see SpatialSubdivision::findOnRay()
	@see SphereTreeNode::findOnRay()

	@author Austin Appleby
*/
template<class ObjectType, class ExtentAccessor>
inline void SphereTree<ObjectType, ExtentAccessor>::findOnRay(const Vector & begin, const Vector & dir, std::vector<ObjectType> & results) const
{
	Vector normDir = dir;
	IGNORE_RETURN( normDir.normalize() );

	root.findOnRay(begin, normDir, results);
}

//-----------------------------------------------------------------------
/**
	@brief find all objects that hit the given segment

	Finds all objects intersecting the segment described by the start point 
    and end point parameters. The results are placed in the results vector.

	@param begin      The start point of the test segment
	@param dir        The end point of the test segment
	@param results    A user supplied vector of ObjectType that will receive the results of the query.

	@see SpatialSubdivision::findOnSegment()
	@see SphereTreeNode::findOnSegment()

	@author Austin Appleby
*/
template<class ObjectType, class ExtentAccessor>
inline void SphereTree<ObjectType, ExtentAccessor>::findOnSegment(const Vector & begin, const Vector & end, std::vector<ObjectType> & results) const
{
	root.findOnSegment(begin, end, results);
}

//-----------------------------------------------------------------------

template<class ObjectType, class ExtentAccessor>
inline void SphereTree<ObjectType, ExtentAccessor>::findOnSegment(Vector const & begin, Vector const & end, SpatialSubdivisionFilter<ObjectType> const & filter, std::vector<ObjectType> & results) const
{
	root.findOnSegment(begin, end, filter, results);
}

//-----------------------------------------------------------------------
/**
	@brief find all objects that hit the given segment

	Finds all objects overlapping the given point.
	The results are placed in the results vector.

	@param begin      The start point of the test segment
	@param dir        The end point of the test segment
	@param results    A user supplied vector of ObjectType that will receive the results of the query.

	@see SpatialSubdivision::findOnSegment()
	@see SphereTreeNode::findOnSegment()

	@author Austin Appleby
*/
template<class ObjectType, class ExtentAccessor>
inline void SphereTree<ObjectType, ExtentAccessor>::findAtPoint(const Vector & point, std::vector<ObjectType> & results) const
{
	root.findAtPoint(point,results);
}

//-----------------------------------------------------------------------

template<class ObjectType, class ExtentAccessor>
inline void SphereTree<ObjectType, ExtentAccessor>::findInRange(const Capsule & range, std::vector<ObjectType> & results) const
{
	root.findInRange(range, results);
}

//-----------------------------------------------------------------------

template<class ObjectType, class ExtentAccessor>
inline void SphereTree<ObjectType, ExtentAccessor>::findInRange(const Capsule & range, const SpatialSubdivisionFilter<ObjectType> &filter, std::vector<ObjectType> & results) const
{
	root.findInRange(range, filter, results);
}

//-----------------------------------------------------------------------
/**
	@brief Find the closest node in the sphere tree to the given point

	Finds the closest object to the given point that is within maxDistance
	of it. Returns true if such an object is found.

	@param begin       The start point
	@param maxDistance The maximum distance from the start point we want to search
	@param outClosest  The closest item found (if one was found)
	@param outDistance The distance to the closest item found

	@see SphereTreeNode::findClosest()

	@author Austin Appleby
*/
template<class ObjectType, class ExtentAccessor>
inline bool SphereTree<ObjectType, ExtentAccessor>::findClosest(const Vector & begin, const float maxDistance, ObjectType & outClosest, float & outMinDistance, float & outMaxDistance ) const
{
	outMinDistance = maxDistance;

	return root.findClosest(begin, maxDistance, outClosest, outMinDistance, outMaxDistance);
}

template<class ObjectType, class ExtentAccessor>
inline bool SphereTree<ObjectType, ExtentAccessor>::findClosest(const Vector & begin, const float maxDistance, ObjectType & outClosest, float & outMinDistance, float & outMaxDistance, int & testCounter ) const
{
	outMinDistance = maxDistance;

	return root.findClosest(begin, maxDistance, outClosest, outMinDistance, outMaxDistance, testCounter);
}

//-----------------------------------------------------------------------
/**
	@brief Find the closest node in the sphere tree to the given point

	Finds the closest (in X-Z) object to the given point that is within maxDistance
	of it. Returns true if such an object is found.

	@param begin       The start point
	@param maxDistance The maximum distance (in X-Z) from the start point we want to search
	@param outClosest  The closest item found (if one was found)
	@param outDistance The distance to the closest item found

	@see SphereTreeNode::findClosest()

	@author Austin Appleby
*/
template<class ObjectType, class ExtentAccessor>
inline bool SphereTree<ObjectType, ExtentAccessor>::findClosest2d(const Vector & begin, const float maxDistance, ObjectType & outClosest, float & outMinDistance, float & outMaxDistance ) const
{
	outMinDistance = maxDistance;

	return root.findClosest2d(begin, maxDistance, outClosest, outMinDistance, outMaxDistance);
}

template<class ObjectType, class ExtentAccessor>
inline bool SphereTree<ObjectType, ExtentAccessor>::findClosest2d(const Vector & begin, const float maxDistance, ObjectType & outClosest, float & outMinDistance, float & outMaxDistance, int & testCounter ) const
{
	outMinDistance = maxDistance;

	return root.findClosest2d(begin, maxDistance, outClosest, outMinDistance, outMaxDistance, testCounter);
}

//-----------------------------------------------------------------------
/**
	@brief relocate an object in the sphere tree

	The sphere tree will find the SphereTreeNode that currently contains
	this object. That node is specified in the SpatialSubdivisionHandle
	(which is actually a SphereTreeNode::NodeHandle). If the object
	remains in the node, the operation completes almost immediately. 
	If the object leaves it's node, it is re-intergrated in terms of the
	parent nodes until a new node is located or created, and he
	object handle is updated to reflect the new container node for
	the object.

	@param object  A SphereTreeNode::handle describing the object and the
	               SphereTreeNode that contains the object
	@param start   Where the object started moving from
	@param end     Where the object ends up after moving

	@see SpatialSubdivision::move()
	@see SphereTreeNode::NodeHandle::move()

	@author Justin Randall
*/
template<class ObjectType, class ExtentAccessor>
inline void SphereTree<ObjectType, ExtentAccessor>::move(SpatialSubdivisionHandle *object)
{
	if (object)
		static_cast<typename SphereTreeNode<ObjectType, ExtentAccessor>::NodeHandle *>(object)->move();
}

//-----------------------------------------------------------------------
/**
	@brief remove an object from the sphere tree

	Advises the containing node that it no longer contains this object.
	If the node is empty, then it unlinks itself from the parent node. If
	the parent node becomes empty, it unlinks from it's parent. This recurses
	until all empty nodes are pruned from the tree.

	@param object   A SphereTreeNode::NodeHandle pointer describing the 
	                object and the SphereTreeNode that contains the object.

	@see SpatialSubdivision::removeObject
	@see SphereTreeNode::NodeHandle::removeObject

	@author Justin Randall
*/
template<class ObjectType, class ExtentAccessor>
inline void SphereTree<ObjectType, ExtentAccessor>::removeObject(SpatialSubdivisionHandle * object)
{
	if (object)
		static_cast<typename SphereTreeNode<ObjectType, ExtentAccessor>::NodeHandle *>(object)->removeObject();
}

// ----------------------------------------------------------------------

template<class ObjectType, class ExtentAccessor>
inline void SphereTree<ObjectType, ExtentAccessor>::validate() const
{
	root.validate();
}

// ----------------------------------------------------------------------

template<class ObjectType, class ExtentAccessor>
inline int SphereTree<ObjectType, ExtentAccessor>::getNodeCount() const
{
	return root.getNodeCount();
}

// ----------------------------------------------------------------------

template<class ObjectType, class ExtentAccessor>
inline bool SphereTree<ObjectType, ExtentAccessor>::empty() const
{
	return root.empty();
}

// ----------------------------------------------------------------------

template<class ObjectType, class ExtentAccessor>
inline int SphereTree<ObjectType, ExtentAccessor>::getObjectCount() const
{
	return root.getObjectCount();
}

// ----------------------------------------------------------------------

template<class ObjectType, class ExtentAccessor>
inline void SphereTree<ObjectType, ExtentAccessor>::apply(typename SphereTreeNode<ObjectType, ExtentAccessor>::NodeFunctor N)
{
	return root.apply(N);
}

//-----------------------------------------------------------------------

template<class ObjectType, class ExtentAccessor>
inline bool SphereTree<ObjectType, ExtentAccessor>::isWithin (Vector const & position) const
{
	return root.isWithin (position);
}

// ----------------------------------------------------------------------

template<class ObjectType, class ExtentAccessor>
Sphere const &SphereTree<ObjectType, ExtentAccessor>::getRealSphere() const
{
	return root.getRealSphere();
}

// ======================================================================

#endif	// _INCLUDED_SphereTree_H

