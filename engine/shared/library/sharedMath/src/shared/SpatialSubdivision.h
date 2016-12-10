// SpatialSubdivision.h
// copyright 2001 Sony Online Entertainment
// Author: Justin Randall

#ifndef	_INCLUDED_SpatialSubdivision_H
#define	_INCLUDED_SpatialSubdivision_H

//-----------------------------------------------------------------------

class Vector;

//-----------------------------------------------------------------------
/**
	Helper handle to expedite operations in a particular spatial 
	subdivision implementation (e.g. moving an object within a Sphere or
	overlapped quadrant, traversing up from a leaf in a tree structure, 
	etc..)
*/
class SpatialSubdivisionHandle
{
public:
	SpatialSubdivisionHandle();
	virtual ~SpatialSubdivisionHandle() = 0;
	SpatialSubdivisionHandle(const SpatialSubdivisionHandle & source);
	SpatialSubdivisionHandle & operator=(const SpatialSubdivisionHandle & source);
};

//-----------------------------------------------------------------------

/**
	@brief a templatized base class for a spatially organized container.

	The SpatialSubdivision template defines an interface for all 
	spatially organized containers. Specific implementations may
	be partially templatized (specifying the extent type, for example)
	while leaving other implementation details, such as ObjectType
	and ExtentAccessor routines configurable per instantiation.

	For example, a multiplayer game application may have ClientObject 
	types on the game client, which organizes data in a Quadtree, using
	axis aligned bounding boxes for extent information. A game server may 
	have a ServerObject that defines its extent as a Sphere and 
	organizes objects in a SphereTree. Additionally, a game message
	routing server may have ClientConnection objects that use
	Sphere extents. A single SphereTree implementation derived
	from the SpatialSubdivision template could be declared as:

	\code
	template<class ObjectType, class ExtentAccessor>
	class SphereTree : public SpatialSubdivision<ObjectType, Sphere, ExtentAccessor>
	\endcode

	or the client Quadtree as
	\code
	class QuadTree : public SpatialSubdivision<ClientObject, AxisAlignedBoundingBox, ClientObject>
	\endcode
	
	There is little penalty for generalization, since the implementation
	may be written very specifically for a particular application, yet
	maintain a common template interface that does not rely on objects or 
	extents.

	Because the SpatialSubdivision class is intended to be an interface
	definition, it doesn't DO anything and is pure virtual.

	@author Justin Randall
*/
template <class ObjectType>
class SpatialSubdivisionFilter
{
public:
	virtual ~SpatialSubdivisionFilter() {}
	virtual bool operator()(const ObjectType &) const=0;
};

template<class ObjectType, class ExtentType, class ExtentAccessor>
class SpatialSubdivision 
{
public:
                                      SpatialSubdivision   ();
	virtual                             ~SpatialSubdivision  () = 0;
	virtual SpatialSubdivisionHandle *  addObject            (ObjectType object) = 0;
	virtual const bool                  canSee               (SpatialSubdivisionHandle * target, const Vector & start, const float distance, const float fov=0.0f) const = 0; //lint !e1735 // virtual function has default parameter
	virtual void                        findInRange          (const Vector & origin, const float distance, typename std::vector<ObjectType> & results) const = 0;
	virtual void                        findInRange          (const Vector & origin, const float distance, const SpatialSubdivisionFilter<ObjectType> &filter, typename std::vector<ObjectType> & results) const = 0;
	virtual void                        findOnRay            (const Vector & begin, const Vector & dir, typename std::vector<ObjectType> & results) const = 0;
	virtual void                        findOnSegment        (const Vector & begin, const Vector & end, typename std::vector<ObjectType> & results) const = 0;
	virtual void                        move                 (SpatialSubdivisionHandle * object) = 0;
	virtual void                        removeObject         (SpatialSubdivisionHandle * object) = 0;

private:
	SpatialSubdivision & operator = (const SpatialSubdivision & rhs);
	SpatialSubdivision(const SpatialSubdivision & source);
};

//-----------------------------------------------------------------------
/**
	@brief construct for a SpatialSubdivision template instance. 

	This constructor doesn't do anything. It is present for completeness.

	@author Justin Randall
*/
template<class ObjectType, class ExtentType, class ExtentAccessor>
inline SpatialSubdivision<ObjectType, ExtentType, ExtentAccessor>::SpatialSubdivision()
{
}

//-----------------------------------------------------------------------
/**
	@brief destroy a SpatialSubdivision template instance

	This destructor doesn't do anything. It is present for completeness.

	@author Justin Randall
*/
template<class ObjectType, class ExtentType, class ExtentAccessor>
inline SpatialSubdivision<ObjectType, ExtentType, ExtentAccessor>::~SpatialSubdivision()
{
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_SpatialSubdivision_H
