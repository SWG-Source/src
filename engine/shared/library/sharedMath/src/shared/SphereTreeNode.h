// SphereTreeNode.h
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved.
// Author: Justin Randall

#ifndef	_INCLUDED_SphereTreeNode_H
#define	_INCLUDED_SphereTreeNode_H

//-----------------------------------------------------------------------

#include "sharedMath/Sphere.h"
#include "sharedMath/Capsule.h"
#include "sharedMath/SpatialSubdivision.h"

#include <algorithm>
#include <vector>

static const float SphereTreeEpsilon = 0.01f;
static const float childScaleFactor = 0.25f;
static const float leafScaleFactor = 10.0f;

//-----------------------------------------------------------------------
/**
	@brief support class that tracks a vector of pointers

	This is used internally by the SphereTreeNode and
	SphereTreeNode::Handle classes. A static vector of free
	SphereTreeNode and SphereTreeNode::Handle objects is maintained to
	limit allocations as the tree reorganizes itself.

	Because the SphereTree is not a singleton class, it is possible
	to share free nodes between instances. The order of destruction
	would otherwise be undefinable. Forcing a static instance
	of a VectorPointerPool within a function of a node or handle
	will gaurantee proper destruction during program termnination.
*/
template<class ValueType>
class VectorPointerPool
{
public:
	VectorPointerPool();
	~VectorPointerPool();
	std::vector<ValueType *> *v;
};

//-----------------------------------------------------------------------

template<class ValueType>
VectorPointerPool<ValueType>::VectorPointerPool()
{
	// we were having a client crash bug caused by one of these objects getting constructed twice.
	// basically, only one static instance for the type existed, but the complier had two instances
	// of the flag that indicated whether or not the object had been constructed.  by doing this,
	// we can avoid any work on the second construction, and know to only really destruct the object
	// once.
	if (!v)
		v = new std::vector<ValueType *>;
}

//-----------------------------------------------------------------------

template<class ValueType>
VectorPointerPool<ValueType>::~VectorPointerPool()
{
	if (v)
	{
		typename std::vector<ValueType *>::iterator i;
		for(i = v->begin(); i != v->end(); ++i)
		{
			ValueType * vt = (*i);
			delete vt;
		}

		delete v;
		v = nullptr;
	}
}

//-----------------------------------------------------------------------
/**
	@brief Workhorse of the SphereTree

	Most of the functionality of the SphereTree is implemened in the
	SphereTreeNode. A SphereTreeNode represents a sphere in 3-space which
	can contain smaller child spheres or objects whose bounding spheres
	are completely contained in the node sphere.

*/
template<class ObjectType, class ExtentAccessor>
class SphereTreeNode
{
public:
	                            SphereTreeNode   ();
	                            ~SphereTreeNode  ();

	SpatialSubdivisionHandle *  addObject        (ObjectType object);
	void dumpSphereTreeObjects(std::vector<ObjectType> & results) const;
	void                        dumpSphereTree   (std::vector<std::pair<ObjectType, Sphere> > & results) const;
	void                        dumpSphereTreeNodes(std::vector<std::pair<ObjectType, Sphere> > & results) const;
	void                        dumpSphereTreeObjs (std::vector<std::pair<ObjectType, Sphere> > & results) const;
	void                        dumpEdgeList     (std::vector<Vector> & results) const;
	const bool                  canContain       (const Sphere & sphere) const;
	const bool                  canContain       (const Vector & point) const;
	void                        findInRange      (const Sphere & range, std::vector<ObjectType> & results) const;
	void                        findInRange      (const Sphere & range, std::vector<ObjectType> & results, int & testCounter ) const;
	void                        findInRange      (const Sphere & range, const SpatialSubdivisionFilter<ObjectType> &filter, std::vector<ObjectType> & results) const;
	void                        findInRange      (const Sphere & range, const SpatialSubdivisionFilter<ObjectType> &filter, std::vector<ObjectType> & results, int & testCounter) const;
	void                        findOnRay        (const Vector & begin, const Vector & dir, std::vector<ObjectType> & results) const;
	void                        findOnSegment    (const Vector & begin, const Vector & end, std::vector<ObjectType> & results) const;
	void findOnSegment(Vector const & begin, Vector const & end, SpatialSubdivisionFilter<ObjectType> const & filter, std::vector<ObjectType> & results) const;
	void                        findAtPoint      (const Vector & point, std::vector<ObjectType> & results) const;
	void                        findInRange      (const Capsule & capsule, std::vector<ObjectType> & results) const;
	void                        findInRange      (const Capsule & capsule, SpatialSubdivisionFilter<ObjectType> const & filter, std::vector<ObjectType> & results) const;

	bool                        findClosest      (const Vector & begin, float maxDistance, ObjectType & outClosest, float & outMinDistance, float & outMaxDistance ) const;
	bool                        findClosest      (const Vector & begin, float maxDistance, ObjectType & outClosest, float & outMinDistance, float & outMaxDistance, int & testCounter) const;
	bool                        findClosest2d    (const Vector & begin, float maxDistance, ObjectType & outClosest, float & outMinDistance, float & outMaxDistance ) const;
	bool                        findClosest2d    (const Vector & begin, float maxDistance, ObjectType & outClosest, float & outMinDistance, float & outMaxDistance, int & testCounter) const;

	void                        validate         () const;
	int                         getNodeCount     () const;
	bool                        empty            () const;
	int                         getObjectCount   () const;

	typedef void (*NodeFunctor)(SphereTreeNode * node);

	void                        apply            (NodeFunctor N);

	bool isWithin (Vector const & position) const;
	Sphere const &              getRealSphere    () const;

private:

	void                                    reinitialize();

	SphereTreeNode &                        operator =         (const SphereTreeNode & rhs);
	                                        SphereTreeNode     (const SphereTreeNode & source);

	void                                    getContents        (std::vector<ObjectType> & results);
	void                                    getContents        (const SpatialSubdivisionFilter<ObjectType> &filter, std::vector<ObjectType> & results);
	static std::vector<SphereTreeNode *> &  getNodeFreeList    ();
	static SphereTreeNode *                 getNode            ();
	static void                             releaseNode        (SphereTreeNode * node);
	void                                    resizeRealSphere   (const float newRadius);
	void                                    unlinkNode         (SphereTreeNode * node);

	SphereTreeNode *                        getParent          ();
	SphereTreeNode const *                  getParent          () const;
	void                                    setParent          (SphereTreeNode * newParent);

	Sphere                                  getSphere          (ObjectType object) const;

	bool                                    isValidSphere      (Sphere const & sphere) const;

	void                                    internalResizeRealSphere (float newRadius);

private:
	std::vector<SphereTreeNode *>  children;
	std::vector<ObjectType>        contents;
	Sphere                         maxSphere;
	SphereTreeNode *               parent;
	Sphere                         realSphere;

public:
	//-----------------------------------------------------------------------
	// NodeHandle Inner Class
	//
	/** @brief a node handle inner class, returned by addObject and fed
		to the SphereTreeNode via the SpatialDatabase interfaces. It's used
		to quickly identify a sphere tree node to expedite operations on
		objects in the tree.
	*/
	class NodeHandle : public SpatialSubdivisionHandle
	{
	public:
		NodeHandle   (){};
		~NodeHandle  (){};

		SphereTreeNode *                    getNode            () const { return node; };
		ObjectType                          getObject          () const { return object; };
		void                                setNode            (SphereTreeNode * newNode) { node = newNode; };
		void                                setObject          (ObjectType newObject) {object = newObject; };

		// accomodate MSVC's inability to inline template inner classes outside the
		// outer template class
		static std::vector<NodeHandle *> & getHandleFreeList()
		{
			static VectorPointerPool<NodeHandle> freeList;
			return *(freeList.v);
		};

		static NodeHandle * getNodeHandle()
		{
			NodeHandle * result = 0;
			if(! getHandleFreeList().empty())
			{
				result = getHandleFreeList().back();
				getHandleFreeList().pop_back();
			}
			else
			{
				result = ExtentAccessor::allocateNodeHandle();
			}
			return result;
		};

		void move()
		{
			if (node)
				node->move(this);
			else
			{
				WARNING_STRICT_FATAL(true, ("SphereTree::move was invoked for an object, but the real sphere tree node it refers to is nullptr."));
			}
		};

		static void releaseHandle(NodeHandle * oldHandle)
		{
			getHandleFreeList().push_back(oldHandle);
		};

		void removeObject()
		{
			if(node)
			{
				node->removeObject(this);
				releaseHandle(this);
			}
		};

	private:
		SphereTreeNode *  node;
		ObjectType        object;
	};
	// End NodeHandle Inner Class
	//-----------------------------------------------------------------------

public:
	void move            (NodeHandle * handle);
	void relocateObject  (NodeHandle * handle, const Sphere & sphere);
	void removeObject    (NodeHandle * object);
};

//-----------------------------------------------------------------------

template<class ObjectType, class ExtentAccessor>
inline SphereTreeNode<ObjectType, ExtentAccessor>::SphereTreeNode() :
children(),
contents(),
maxSphere(Sphere(Vector::zero, SphereTreeEpsilon)),
parent(/*newParent*/ 0),
realSphere()
{
}

//-----------------------------------------------------------------------

template<class ObjectType, class ExtentAccessor>
inline void SphereTreeNode<ObjectType, ExtentAccessor>::reinitialize()
{
	children.clear();
	contents.clear();
	maxSphere = Sphere(Vector::zero, SphereTreeEpsilon);
	parent = 0;
	realSphere = Sphere::zero;
}

//-----------------------------------------------------------------------

template<class ObjectType, class ExtentAccessor>
inline SphereTreeNode<ObjectType, ExtentAccessor>::~SphereTreeNode()
{
	// delete child nodes
	typename std::vector<SphereTreeNode *>::iterator i;
	for(i = children.begin(); i != children.end(); ++i)
	{
		SphereTreeNode * c = (*i);
		delete c;
	}
	children.clear();

	parent = 0;
	contents.clear();
}

//-----------------------------------------------------------------------

template<class ObjectType, class ExtentAccessor>
inline SpatialSubdivisionHandle * SphereTreeNode<ObjectType, ExtentAccessor>::addObject(ObjectType object)
{
	const Sphere & sphere = getSphere(object);

	if(!isValidSphere(sphere)) 
	{
		WARNING_STRICT_FATAL(true, ("SphereTreeNode::addObject - sphere for the object being added is invalid"));
		return nullptr;
	}

	SphereTreeNode * candidateNode = 0;
	NodeHandle * result = 0;

	if(sphere.getRadius() > 0.0f)
	{
		// quick check to keep supersphere distribution over
		// used world space to about 16 spheres (there may be
		// more with overlap, maybe a lot less depending on
		// object distribution)
		if(!parent)
		{
			if(sphere.getRadius() * leafScaleFactor > maxSphere.getRadius())
			{
				maxSphere.setRadius(sphere.getRadius() * leafScaleFactor);
			}

			// dirty hack.. floating point error becomes intolerable
			// at distances over 16000 units and we can fail our
			// heuristics. Ensure that no matter what, we can drop
			// this node in the tree.
			while(!maxSphere.contains(sphere))
			{
				maxSphere.setRadius(maxSphere.getRadius() * 2);
			}
		}

		// if the object's sphere extent (plus one unit) is greater than the maximum size of
		// this node sphere times the child scale factor, then the object doesn't "fit"
		// in any children. This means it belongs in THIS node.
		if((sphere.getRadius() + SphereTreeEpsilon) * leafScaleFactor > maxSphere.getRadius() * childScaleFactor)
		{
			// this object lives in the contents list
			contents.push_back(object);
			result = NodeHandle::getNodeHandle();
			result->setObject(object);
			result->setNode(this);

			// set real sphere size
			const float newSize = (sphere.getCenter() - realSphere.getCenter()).magnitude() + (sphere.getRadius() + SphereTreeEpsilon);
			if(newSize > realSphere.getRadius())
			{
				resizeRealSphere(newSize);
			}
		}
		else
		{
			// can recurse to child nodes, find the best child node
			typename std::vector<SphereTreeNode *>::iterator i;
			for(i = children.begin(); i != children.end(); ++i)
			{
				SphereTreeNode * n = (*i);

				// for faster insert times, simply select the
				// first available node. If trees become too
				// unbalanced and cause performance problems,
				// change this code to select the least-loaded
				// candidate node.
				if(n->canContain(sphere))
				{
					candidateNode = n;
					break;
				}
			}

			// no candidate node was identified when looking at the
			// child nodes. A new one must be created to hold this
			// object
			if(! candidateNode)
			{
				// select the node from the free list (or allocate
				// if the free list is empty)
				candidateNode = getNode();

				candidateNode->setParent(this);

				// size the child node's maximum radius relative to
				// this node's maximum radius times the childScaleFactor fraction
				candidateNode->maxSphere = Sphere(sphere.getCenter(), maxSphere.getRadius() * childScaleFactor);

				// the node only needs to be large enough to hold it's contents.
				// keeping it smaller reduces hits when querying the tree
				candidateNode->realSphere = Sphere(sphere.getCenter(), sphere.getRadius());

				children.push_back(candidateNode);
			}

			// by this point, if there were no memory allocation problems,
			// there is a valid child node to drop the object into.
			result = static_cast<NodeHandle *>(candidateNode->addObject(object));
		}
	}
	return result;
}

//-----------------------------------------------------------------------

template<class ObjectType, class ExtentAccessor>
inline const bool SphereTreeNode<ObjectType, ExtentAccessor>::canContain(const Sphere & sphere) const
{
	return maxSphere.contains(sphere);
}

//-----------------------------------------------------------------------

template<class ObjectType, class ExtentAccessor>
inline const bool SphereTreeNode<ObjectType, ExtentAccessor>::canContain(const Vector & point) const
{
	return maxSphere.contains(point);
}

//-----------------------------------------------------------------------

template<class ObjectType, class ExtentAccessor>
inline void SphereTreeNode<ObjectType, ExtentAccessor>::dumpSphereTreeObjects(std::vector<ObjectType> & results) const
{
	typename std::vector<ObjectType>::const_iterator c;
	for (c = contents.begin(); c != contents.end(); ++c)
		results.push_back(*c);

	// recurse to children
	typename std::vector<SphereTreeNode *>::const_iterator s;
	for (s = children.begin(); s != children.end(); ++s)
		(*s)->dumpSphereTreeObjects(results);
}

template<class ObjectType, class ExtentAccessor>
inline void SphereTreeNode<ObjectType, ExtentAccessor>::dumpSphereTree(std::vector<std::pair<ObjectType, Sphere> > & results) const
{
	// current node extents
	results.push_back(std::pair<ObjectType, Sphere>(0, realSphere));
	results.push_back(std::pair<ObjectType, Sphere>(0, maxSphere));

	// add content nodes
	typename std::vector<ObjectType>::const_iterator c;
	for(c = contents.begin(); c != contents.end(); ++ c)
	{
		const Sphere & sphere = getSphere(*c);
		results.push_back(std::pair<ObjectType, Sphere>((*c),sphere));
	}

	// recurse to children
	// recurse into qualfiying children
	typename std::vector<SphereTreeNode *>::const_iterator s;
	for(s = children.begin(); s != children.end(); ++s)
	{
		(*s)->dumpSphereTree(results);
	}
}

template<class ObjectType, class ExtentAccessor>
inline void SphereTreeNode<ObjectType, ExtentAccessor>::dumpSphereTreeNodes(std::vector<std::pair<ObjectType, Sphere> > & results) const
{
	// current node extents
	results.push_back(std::pair<ObjectType, Sphere>(0, realSphere));

	// recurse to children
	// recurse into qualfiying children
	typename std::vector<SphereTreeNode *>::const_iterator s;
	for(s = children.begin(); s != children.end(); ++s)
	{
		(*s)->dumpSphereTreeNodes(results);
	}
}

template<class ObjectType, class ExtentAccessor>
inline void SphereTreeNode<ObjectType, ExtentAccessor>::dumpSphereTreeObjs(std::vector<std::pair<ObjectType, Sphere> > & results) const
{
	// add content nodes
	typename std::vector<ObjectType>::const_iterator c;
	for(c = contents.begin(); c != contents.end(); ++ c)
	{
		const Sphere & sphere = getSphere(*c);
		results.push_back(std::pair<ObjectType, Sphere>((*c),sphere));
	}

	// recurse to children
	// recurse into qualfiying children
	typename std::vector<SphereTreeNode *>::const_iterator s;
	for(s = children.begin(); s != children.end(); ++s)
	{
		(*s)->dumpSphereTreeObjs(results);
	}
}

// ----------------------------------------------------------------------

template<class ObjectType, class ExtentAccessor>
inline void SphereTreeNode<ObjectType, ExtentAccessor>::dumpEdgeList(std::vector<Vector> & results) const
{
	typename std::vector<ObjectType>::const_iterator c;
	for(c = contents.begin(); c != contents.end(); ++ c)
	{
		const Sphere & sphere = getSphere(*c);

		results.push_back(sphere.getCenter());
		results.push_back(realSphere.getCenter());
	}

	/*
	if(parent)
	{
		results.push_back(realSphere.getCenter());
		results.push_back(parent->realSphere.getCenter());
	}
	*/

	// recurse to children
	typename std::vector<SphereTreeNode *>::const_iterator s;
	for(s = children.begin(); s != children.end(); ++s)
	{
		(*s)->dumpEdgeList(results);
	}
}

//-----------------------------------------------------------------------

template<class ObjectType, class ExtentAccessor>
inline void SphereTreeNode<ObjectType, ExtentAccessor>::findInRange(const Sphere & range, std::vector<ObjectType> & results) const
{
	// check contents
	typename std::vector<ObjectType>::const_iterator c;
	for(c = contents.begin(); c != contents.end(); ++ c)
	{
		const Sphere & sphere = getSphere(*c);
		if(range.intersectsSphere(sphere))
		{
			results.push_back((*c));
		}
	}

	// recurse into qualfiying children
	typename std::vector<SphereTreeNode *>::const_iterator s;
	for(s = children.begin(); s != children.end(); ++s)
	{
		if(range.contains((*s)->realSphere))
		{
			// wholly contained, therefore ALL children
			// and ALL contents are satisfied by this
			// range query
			(*s)->getContents(results);
		}
		else if(range.intersectsSphere((*s)->realSphere))
		{
			// there is a partial intersection with
			// a child node, which means some of it's
			// children or contents might be out of range.
			// each node/item in the child must be checked.
			(*s)->findInRange(range, results);
		}
	}
}

//-----------------------------------------------------------------------

template<class ObjectType, class ExtentAccessor>
inline void SphereTreeNode<ObjectType, ExtentAccessor>::findInRange(const Sphere & range, std::vector<ObjectType> & results, int & testCounter ) const
{
	// check contents
	typename std::vector<ObjectType>::const_iterator c;
	for(c = contents.begin(); c != contents.end(); ++ c)
	{
		const Sphere & sphere = getSphere(*c);

		testCounter++;

		if(range.intersectsSphere(sphere))
		{
			results.push_back((*c));
		}
	}

	// recurse into qualfiying children
	typename std::vector<SphereTreeNode *>::const_iterator s;
	for(s = children.begin(); s != children.end(); ++s)
	{
		SphereTreeNode * child = (*s);

		const Sphere & sphere = child->realSphere;

		testCounter++;

		if(range.contains(sphere))
		{
			// wholly contained, therefore ALL children
			// and ALL contents are satisfied by this
			// range query
			child->getContents(results);
		}
		else if(range.intersectsSphere(sphere))
		{
			testCounter++;

			// there is a partial intersection with
			// a child node, which means some of it's
			// children or contents might be out of range.
			// each node/item in the child must be checked.
			child->findInRange(range, results, testCounter);
		}
	}
}

//-----------------------------------------------------------------------

template<class ObjectType, class ExtentAccessor>
inline void SphereTreeNode<ObjectType, ExtentAccessor>::findInRange(const Sphere & range, const SpatialSubdivisionFilter<ObjectType> &filter, std::vector<ObjectType> & results) const
{
	// check contents
	typename std::vector<ObjectType>::const_iterator c;
	for(c = contents.begin(); c != contents.end(); ++ c)
	{
		const Sphere & sphere = getSphere(*c);
		if(range.intersectsSphere(sphere) && filter(*c))
		{
			results.push_back((*c));
		}
	}

	// recurse into qualfiying children
	typename std::vector<SphereTreeNode *>::const_iterator s;
	for(s = children.begin(); s != children.end(); ++s)
	{
		if(range.contains((*s)->realSphere))
		{
			// wholly contained, therefore ALL children
			// and ALL contents are satisfied by this
			// range query
			(*s)->getContents(filter, results);
		}
		else if(range.intersectsSphere((*s)->realSphere))
		{
			// there is a partial intersection with
			// a child node, which means some of it's
			// children or contents might be out of range.
			// each node/item in the child must be checked.
			(*s)->findInRange(range, filter, results);
		}
	}
}

// ----------------------------------------------------------------------

template<class ObjectType, class ExtentAccessor>
inline void SphereTreeNode<ObjectType, ExtentAccessor>::findInRange(const Sphere & range, const SpatialSubdivisionFilter<ObjectType> &filter, std::vector<ObjectType> & results, int & testCounter) const
{
	// check contents
	typename std::vector<ObjectType>::const_iterator c;
	for(c = contents.begin(); c != contents.end(); ++ c)
	{
		const Sphere & sphere = getSphere(*c);

		testCounter++;

		if(range.intersectsSphere(sphere) && filter(*c))
		{
			results.push_back((*c));
		}
	}

	// recurse into qualfiying children
	typename std::vector<SphereTreeNode *>::const_iterator s;
	for(s = children.begin(); s != children.end(); ++s)
	{
		testCounter++;

		if(range.contains((*s)->realSphere))
		{
			// wholly contained, therefore ALL children
			// and ALL contents are satisfied by this
			// range query
			(*s)->getContents(filter, results);
		}
		else if(range.intersectsSphere((*s)->realSphere))
		{
			testCounter++;

			// there is a partial intersection with
			// a child node, which means some of it's
			// children or contents might be out of range.
			// each node/item in the child must be checked.
			(*s)->findInRange(range, filter, results);
		}
	}
}

//-----------------------------------------------------------------------

template<class ObjectType, class ExtentAccessor>
inline void SphereTreeNode<ObjectType, ExtentAccessor>::findOnRay( const Vector & begin, const Vector & dir, std::vector<ObjectType> & results) const
{
	// check contents
	typename std::vector<ObjectType>::const_iterator c;
	for(c = contents.begin(); c != contents.end(); ++ c)
	{
		const Sphere & sphere = getSphere(*c);

		if(sphere.intersectsRay(begin,dir))
		{
			results.push_back((*c));
		}
	}

	// recurse into qualfiying children
	typename std::vector<SphereTreeNode *>::const_iterator s;
	for(s = children.begin(); s != children.end(); ++s)
	{
        const Sphere & sphere = (*s)->realSphere;

		if(sphere.intersectsRay(begin,dir))
		{
			(*s)->findOnRay(begin,dir,results);
		}
	}
}

//-----------------------------------------------------------------------

template<class ObjectType, class ExtentAccessor>
inline void SphereTreeNode<ObjectType, ExtentAccessor>::findOnSegment( const Vector & begin, const Vector & end, std::vector<ObjectType> & results) const
{
	// check contents
	typename std::vector<ObjectType>::const_iterator c;
	for(c = contents.begin(); c != contents.end(); ++ c)
	{
		const Sphere & sphere = getSphere(*c);

		if(sphere.intersectsLineSegment(begin,end))
		{
			results.push_back((*c));
		}
	}

	// recurse into qualfiying children
	typename std::vector<SphereTreeNode *>::const_iterator s;
	for(s = children.begin(); s != children.end(); ++s)
	{
        const Sphere & sphere = (*s)->realSphere;

		if(sphere.intersectsLineSegment(begin,end))
		{
			(*s)->findOnSegment(begin,end,results);
		}
	}
}

//-----------------------------------------------------------------------

template<class ObjectType, class ExtentAccessor>
inline void SphereTreeNode<ObjectType, ExtentAccessor>::findOnSegment(Vector const & begin, Vector const & end, SpatialSubdivisionFilter<ObjectType> const & filter, std::vector<ObjectType> & results) const
{
	// check contents
	{
		typename std::vector<ObjectType>::const_iterator c;
		for (c = contents.begin(); c != contents.end(); ++ c)
		{
			if (filter(*c))
			{
				Sphere const & sphere = getSphere(*c);
				if (sphere.intersectsLineSegment(begin, end))
					results.push_back((*c));
			}
		}
	}
	
	// recurse into qualfiying children
	{
		typename std::vector<SphereTreeNode *>::const_iterator s;
		for (s = children.begin(); s != children.end(); ++s)
		{
			Sphere const & sphere = (*s)->realSphere;
			if (sphere.intersectsLineSegment(begin, end))
				(*s)->findOnSegment(begin, end, results);
		}
	}
}

//-----------------------------------------------------------------------

template<class ObjectType, class ExtentAccessor>
inline void SphereTreeNode<ObjectType, ExtentAccessor>::findAtPoint( const Vector & point, std::vector<ObjectType> & results) const
{
	// check contents
	typename std::vector<ObjectType>::const_iterator c;
	for(c = contents.begin(); c != contents.end(); ++ c)
	{
		const Sphere & sphere = getSphere(*c);

		if(sphere.contains(point))
		{
			results.push_back((*c));
		}
	}

	// recurse into qualfiying children
	typename std::vector<SphereTreeNode *>::const_iterator s;
	for(s = children.begin(); s != children.end(); ++s)
	{
		const Sphere & sphere = (*s)->realSphere;

		if(sphere.contains(point))
		{
			(*s)->findAtPoint(point,results);
		}
	}
}

// ----------------------------------------------------------------------

template<class ObjectType, class ExtentAccessor>
inline void SphereTreeNode<ObjectType, ExtentAccessor>::findInRange(const Capsule & range, std::vector<ObjectType> & results) const
{
	// check contents
	typename std::vector<ObjectType>::const_iterator c;
	for(c = contents.begin(); c != contents.end(); ++ c)
	{
		const Sphere & sphere = getSphere(*c);
		if(range.intersectsSphere(sphere))
		{
			results.push_back((*c));
		}
	}

	// recurse into qualfiying children
	typename std::vector<SphereTreeNode *>::const_iterator s;
	for(s = children.begin(); s != children.end(); ++s)
	{
		if(range.contains((*s)->realSphere))
		{
			// wholly contained, therefore ALL children
			// and ALL contents are satisfied by this
			// range query
			(*s)->getContents(results);
		}
		else if(range.intersectsSphere((*s)->realSphere))
		{
			// there is a partial intersection with
			// a child node, which means some of it's
			// children or contents might be out of range.
			// each node/item in the child must be checked.
			(*s)->findInRange(range, results);
		}
	}
}

// ----------------------------------------------------------------------
template<class ObjectType, class ExtentAccessor>
inline void SphereTreeNode<ObjectType, ExtentAccessor>::findInRange(const Capsule & range, SpatialSubdivisionFilter<ObjectType> const & filter, std::vector<ObjectType> & results) const
{
	// check contents
	for (typename std::vector<ObjectType>::const_iterator c = contents.begin(); c != contents.end(); ++ c)
		if (filter(*c) && range.intersectsSphere(getSphere(*c)))
			results.push_back((*c));

	// recurse into qualfiying children
	for (typename std::vector<SphereTreeNode *>::const_iterator s = children.begin(); s != children.end(); ++s)
	{
		if (range.contains((*s)->realSphere))
		{
			// wholly contained, therefore ALL children
			// and ALL contents are satisfied by this
			// range query
			(*s)->getContents(filter, results);
		}
		else if (range.intersectsSphere((*s)->realSphere))
		{
			// there is a partial intersection with
			// a child node, which means some of it's
			// children or contents might be out of range.
			// each node/item in the child must be checked.
			(*s)->findInRange(range, filter, results);
		}
	}
}
// ----------------------------------------------------------------------

template<class ObjectType, class ExtentAccessor>
inline bool SphereTreeNode<ObjectType, ExtentAccessor>::findClosest( const Vector & begin, float maxDistance, ObjectType & outClosest, float & outMinDistance, float & outMaxDistance ) const
{
	int dummy = 0;

	return findClosest(begin,maxDistance,outClosest,outMinDistance,outMaxDistance,dummy);
}

// ----------

template<class ObjectType, class ExtentAccessor>
inline bool SphereTreeNode<ObjectType, ExtentAccessor>::findClosest( const Vector & begin, float maxDistance, ObjectType & outClosest, float & outMinDistance, float & outMaxDistance, int & testCounter ) const
{
	testCounter++;

	float minDist = realSphere.getCenter().magnitudeBetween(begin) - realSphere.getRadius();

	if(minDist >= maxDistance) return false;

	if(minDist >= outMinDistance) return false;

	// ----------

	bool found = false;

	typename std::vector<ObjectType>::const_iterator c;
	for(c = contents.begin(); c != contents.end(); ++ c)
	{
		const Sphere & sphere = getSphere(*c);

		testCounter++;

		float dist = sphere.getCenter().magnitudeBetween(begin);

		float minDist = dist - sphere.getRadius();
		float maxDist = dist + sphere.getRadius();

		if(minDist >= maxDistance)
		{
			continue;
		}

		if(minDist < outMinDistance)
		{
			found = true;

			outClosest = *c;
			outMinDistance = minDist;
			outMaxDistance = maxDist;
		}
	}

	// recurse into qualfiying children
	typename std::vector<SphereTreeNode *>::const_iterator s;
	for(s = children.begin(); s != children.end(); ++s)
	{
		found |= (*s)->findClosest(begin,maxDistance,outClosest,outMinDistance,outMaxDistance,testCounter);
	}

	return found;
}

// ----------------------------------------------------------------------

template<class ObjectType, class ExtentAccessor>
inline bool SphereTreeNode<ObjectType, ExtentAccessor>::findClosest2d( const Vector & begin, float maxDistance, ObjectType & outClosest, float & outMinDistance, float & outMaxDistance ) const
{
	int dummy = 0;

	return findClosest2d(begin,maxDistance,outClosest,outMinDistance,outMaxDistance,dummy);
}

// ----------

template<class ObjectType, class ExtentAccessor>
inline bool SphereTreeNode<ObjectType, ExtentAccessor>::findClosest2d( const Vector & begin, float maxDistance, ObjectType & outClosest, float & outMinDistance, float & outMaxDistance, int & testCounter ) const
{
	testCounter++;

	Vector delta = realSphere.getCenter() - begin;

	delta.y = 0.0f;

	float minDist = delta.magnitude() - realSphere.getRadius();

	if(minDist >= maxDistance) return false;

	if(minDist >= outMinDistance) return false;

	// ----------

	bool found = false;

	typename std::vector<ObjectType>::const_iterator c;
	for(c = contents.begin(); c != contents.end(); ++ c)
	{
		const Sphere & sphere = getSphere(*c);

		testCounter++;

		Vector delta = sphere.getCenter() - begin;

		delta.y = 0.0f;

		float dist = delta.magnitude();

		float minDist = dist - sphere.getRadius();
		float maxDist = dist + sphere.getRadius();

		if(minDist >= maxDistance)
		{
			continue;
		}

		if(minDist < outMinDistance)
		{
			found = true;

			outClosest = *c;
			outMinDistance = minDist;
			outMaxDistance = maxDist;
		}
	}

	// recurse into qualfiying children
	typename std::vector<SphereTreeNode *>::const_iterator s;
	for(s = children.begin(); s != children.end(); ++s)
	{
		found |= (*s)->findClosest2d(begin,maxDistance,outClosest,outMinDistance,outMaxDistance,testCounter);
	}

	return found;
}


//-----------------------------------------------------------------------

template<class ObjectType, class ExtentAccessor>
inline void SphereTreeNode<ObjectType, ExtentAccessor>::getContents(std::vector<ObjectType> & results)
{
	// copy the items from the contents list to the result
	// vector
	typename std::vector<ObjectType>::const_iterator c;
	for(c = contents.begin(); c != contents.end(); ++ c)
	{
		results.push_back((*c));
	}

	// get contents of all children as well
	typename std::vector<SphereTreeNode *>::const_iterator s;
	for(s = children.begin(); s != children.end(); ++s)
	{
		(*s)->getContents(results);
	}
}

//-----------------------------------------------------------------------

template<class ObjectType, class ExtentAccessor>
inline void SphereTreeNode<ObjectType, ExtentAccessor>::getContents(const SpatialSubdivisionFilter<ObjectType> &filter, std::vector<ObjectType> & results)
{
	// copy the items from the contents list to the result
	// vector
	typename std::vector<ObjectType>::const_iterator c;
	for(c = contents.begin(); c != contents.end(); ++ c)
	{
		if (filter(*c))
		{
			results.push_back((*c));
		}
	}

	// get contents of all children as well
	typename std::vector<SphereTreeNode *>::const_iterator s;
	for(s = children.begin(); s != children.end(); ++s)
	{
		(*s)->getContents(filter, results);
	}
}

//-----------------------------------------------------------------------

template<class ObjectType, class ExtentAccessor>
std::vector<SphereTreeNode<ObjectType, ExtentAccessor> *> & SphereTreeNode<ObjectType, ExtentAccessor>::getNodeFreeList()
{
	// a global sphere tree node free list. Provides constant time
	// allocation of nodes if there are any in the list
	static VectorPointerPool<SphereTreeNode> freeList;
	return *(freeList.v);
}

//-----------------------------------------------------------------------

template<class ObjectType, class ExtentAccessor>
inline SphereTreeNode<ObjectType, ExtentAccessor> * SphereTreeNode<ObjectType, ExtentAccessor>::getNode()
{
	SphereTreeNode * result = 0;

	// get a sphere tree node from the free list if it is not empty
	// otherwise allocate a new node (which will be put in the free list
	// when it is released)

	if(!getNodeFreeList().empty())
	{
		result = getNodeFreeList().back();
		result->reinitialize();
		getNodeFreeList().pop_back();
	}
	else
	{
		result = new SphereTreeNode;
	}

	return result;
}

// ----------------------------------------------------------------------

template<class ObjectType, class ExtentAccessor>
inline void SphereTreeNode<ObjectType, ExtentAccessor>::validate() const
{
	if(!isValidSphere(maxSphere))
	{
		DEBUG_FATAL(true,("SphereTreeNode::validate - Node's max sphere is invalid"));
	}

	if(!isValidSphere(realSphere))
	{
		DEBUG_FATAL(true,("SphereTreeNode::validate - Node's real sphere is invalid"));
	}

	if(!maxSphere.contains(realSphere))
	{
		DEBUG_FATAL(true,("SphereTreeNode::validate - Node's real sphere is larger than its max sphere\n"));
	}

	typename std::vector<ObjectType>::const_iterator c;
	for(c = contents.begin(); c != contents.end(); ++ c)
	{
		const Sphere & sphere = getSphere(*c);

		if(!isValidSphere(sphere))
		{
			DEBUG_FATAL(true,("SphereTreeNode::validate - Node has a content sphere that's invalid"));
		}

		if(!realSphere.contains(sphere))
		{
			DEBUG_FATAL(true,("SphereTreeNode::validate - Node doesn't contain all of its contents"));
		}
	}

	// recurse into qualfiying children
	typename std::vector<SphereTreeNode *>::const_iterator s;
	for(s = children.begin(); s != children.end(); ++s)
	{
		const Sphere & childReal = (*s)->realSphere;

		if(!isValidSphere(childReal))
		{
			DEBUG_FATAL(true,("SphereTreeNode::validate - Node has a child sphere that's invalid"));
		}

		if(!realSphere.contains(childReal))
		{
			DEBUG_FATAL(true,("SphereTreeNode::validate - Node's real sphere doesn't contain its child's real sphere"));
		}

		(*s)->validate();
	}
}

// ----------------------------------------------------------------------

template<class ObjectType, class ExtentAccessor>
inline int SphereTreeNode<ObjectType, ExtentAccessor>::getNodeCount() const
{
	int accum = 1;

	// recurse into qualfiying children
	typename std::vector<SphereTreeNode *>::const_iterator s;
	for(s = children.begin(); s != children.end(); ++s)
	{
		accum += (*s)->getNodeCount();
	}

	return accum;
}

// ----------------------------------------------------------------------

template<class ObjectType, class ExtentAccessor>
bool SphereTreeNode<ObjectType, ExtentAccessor>::empty() const
{
	if (!contents.empty())
		return false;
	for (typename std::vector<SphereTreeNode *>::const_iterator i = children.begin(); i != children.end(); ++i)
		if (!(*i)->empty())
			return false;
	return true;
}

// ----------------------------------------------------------------------

template<class ObjectType, class ExtentAccessor>
inline int SphereTreeNode<ObjectType, ExtentAccessor>::getObjectCount() const
{
	int accum = contents.size();

	// recurse into qualfiying children
	typename std::vector<SphereTreeNode *>::const_iterator s;
	for(s = children.begin(); s != children.end(); ++s)
	{
		accum += (*s)->getObjectCount();
	}

	return accum;
}

// ----------------------------------------------------------------------

template<class ObjectType, class ExtentAccessor>
inline void SphereTreeNode<ObjectType, ExtentAccessor>::apply( typename SphereTreeNode<ObjectType, ExtentAccessor>::NodeFunctor N)
{
	N(this);

	// recurse into qualfiying children
	typename std::vector<SphereTreeNode *>::const_iterator s;
	for(s = children.begin(); s != children.end(); ++s)
	{
		(*s)->apply(N);
	}
}

//-----------------------------------------------------------------------
/** @todo potential collision/interaction
*/
template<class ObjectType, class ExtentAccessor>
inline void SphereTreeNode<ObjectType, ExtentAccessor>::move(NodeHandle * handle)
{
	const Sphere & sphere = getSphere(handle->getObject());

	if(!isValidSphere(sphere)) 
	{
		WARNING_STRICT_FATAL(true, ("SphereTreeNode::move - sphere for the object being moved is invalid."));

		return;
	}

	// if this node cannot contain the update sphere,
	// perform sizeing or reintegration
	if(!realSphere.contains(sphere))
	{
		if(!maxSphere.contains(sphere))
		{
			// remove the object from this node
			typename std::vector<ObjectType>::iterator i;
			ObjectType o = handle->getObject();
			i = std::find(contents.begin(), contents.end(), o);
			if(i != contents.end())
			{
				*i = contents.back();
				contents.pop_back();
			}

			// reintergrate object in tree
			handle->setNode(0);

			if(parent)
			  parent->relocateObject(handle, sphere);
			else
			  relocateObject(handle, sphere);

			if(contents.empty() && children.empty())
			{
				// this node disappears
				if(parent)
				{
					parent->unlinkNode(this);
				}
			}


		}
		else
		{
			// resize the real sphere
			const float newSize = (sphere.getCenter() - realSphere.getCenter()).magnitude() + (sphere.getRadius() + SphereTreeEpsilon);
			if(newSize > realSphere.getRadius())
			{
				resizeRealSphere(newSize);
			}
		}
	}
}

//-----------------------------------------------------------------------

template<class ObjectType, class ExtentAccessor>
inline void SphereTreeNode<ObjectType, ExtentAccessor>::relocateObject(NodeHandle * handle, const Sphere & sphere)
{
	if(!maxSphere.contains(sphere))
	{
		// this node can not contain the sphere, pass it to the parent
		// node
		if(parent)
		{
			parent->relocateObject(handle, sphere);
		}
		else
		{
			// this is the root node, resize the root node because it
			// ALWAYS can contain an object.
			if(sphere.getRadius() * leafScaleFactor > maxSphere.getRadius())
			{
				maxSphere.setRadius(sphere.getRadius() * leafScaleFactor);
			}

			// dirty hack.. floating point error becomes intolerable
			// at distances over 16000 units and we can fail our
			// heuristics. Ensure that no matter what, we can drop
			// this node in the tree.
			while(!maxSphere.contains(sphere))
			{
				maxSphere.setRadius(maxSphere.getRadius() * 2);
			}

			// it should not fail the relocation test now.
			relocateObject(handle, sphere);
		}
	}
	else
	{
		// find a child node
		if((sphere.getRadius() + SphereTreeEpsilon) * leafScaleFactor > maxSphere.getRadius() * childScaleFactor)
		{
			// this object lives in the contents list
			contents.push_back(handle->getObject());
			handle->setNode(this);

			// set real sphere size
			const float newSize = (sphere.getCenter() - realSphere.getCenter()).magnitude() + sphere.getRadius() + SphereTreeEpsilon;
			if(newSize > realSphere.getRadius())
			{
				resizeRealSphere(newSize);
			}
		}
		else
		{
			// can recurse to child nodes
			SphereTreeNode * candidateNode = 0;
			typename std::vector<SphereTreeNode *>::iterator i;
			for(i = children.begin(); i != children.end(); ++i)
			{
				SphereTreeNode * n = (*i);
				if(n->canContain(sphere))
				{
					candidateNode = n;
					break;
				}
			}

			if(! candidateNode)
			{
				candidateNode = getNode();
				candidateNode->setParent(this);
				candidateNode->maxSphere = Sphere(sphere.getCenter(), maxSphere.getRadius() * childScaleFactor);
				candidateNode->realSphere = Sphere(sphere.getCenter(), sphere.getRadius());
				children.push_back(candidateNode);
			}
			candidateNode->relocateObject(handle, sphere);
		}
	}
}

//-----------------------------------------------------------------------

template<class ObjectType, class ExtentAccessor>
inline void SphereTreeNode<ObjectType, ExtentAccessor>::releaseNode(SphereTreeNode<ObjectType, ExtentAccessor> * node)
{
	// delete child nodes
	typename std::vector<SphereTreeNode *>::iterator i;
	for(i = node->children.begin(); i != node->children.end(); ++i)
	{
		SphereTreeNode * c = (*i);
		releaseNode(c);
	}
	node->children.clear();

	node->parent = 0;
	node->contents.clear();

	getNodeFreeList().push_back(node);
}

//-----------------------------------------------------------------------

template<class ObjectType, class ExtentAccessor>
inline void SphereTreeNode<ObjectType, ExtentAccessor>::removeObject(NodeHandle * handle)
{
	// find in contents
	// remove the object from this node
	typename std::vector<ObjectType>::iterator i;
	ObjectType o = handle->getObject();
	i = std::find(contents.begin(), contents.end(), o);
	if(i != contents.end())
	{
		*i = contents.back();
		contents.pop_back();
	}

	if(contents.empty() && children.empty())
	{
		// this node disappears
		if(parent)
		{
			parent->unlinkNode(this);
		}
	}
	handle->setNode(0);

}

//-----------------------------------------------------------------------

template<class ObjectType, class ExtentAccessor>
inline void SphereTreeNode<ObjectType, ExtentAccessor>::internalResizeRealSphere(const float newRadius)
{
	// if a realSphere resizes, that may affect the parent node's real
	// size as well. Ensure that the parent/child relationship is
	// maintained when resizing
	realSphere.setRadius(newRadius);
	if(parent)
	{
		if(! parent->realSphere.contains(realSphere))
		{
			// the parent cannot contain the new realSphere unless its
			// real sphere is resized
			const float upRadius = realSphere.getCenter().magnitudeBetween(parent->realSphere.getCenter()) + newRadius;

			float actualRadius = std::max(parent->realSphere.getRadius(), upRadius) + 0.1f;

			parent->internalResizeRealSphere(actualRadius);
		}
	}
	else
	{
		maxSphere = Sphere( realSphere.getCenter(), realSphere.getRadius() + 0.1f );
	}
}

//-----------------------------------------------------------------------

template<class ObjectType, class ExtentAccessor>
inline void SphereTreeNode<ObjectType, ExtentAccessor>::resizeRealSphere(const float newRadius)
{
	internalResizeRealSphere (newRadius);

#if _DEBUG
	if(parent && !parent->realSphere.contains(realSphere))
	{
		Vector A = parent->realSphere.getCenter();
		float AR = parent->realSphere.getRadius();

		Vector B = realSphere.getCenter();
		float BR = realSphere.getRadius();

		float D = A.magnitudeBetween(B);
		float O = std::abs(AR - (D + BR));

		// Fatal if the error is significant, otherwise just warn.

		if(O > 0.1f)
		{
			DEBUG_FATAL(true,("Sphere tree node isn't contained by its parent node.\n Parent (%f,%f,%f - %f), Child (%f,%f,%f - %f), distance %f, error %f\n",A.x,A.y,A.z,AR,B.x,B.y,B.z,BR,D,O));
		}
		else
		{
			DEBUG_WARNING(true,("Sphere tree node isn't contained by its parent node.\n Parent (%f,%f,%f - %f), Child (%f,%f,%f - %f), distance %f, error %f\n",A.x,A.y,A.z,AR,B.x,B.y,B.z,BR,D,O));
		}
	}
#endif
}

//-----------------------------------------------------------------------

template<class ObjectType, class ExtentAccessor>
inline void SphereTreeNode<ObjectType, ExtentAccessor>::unlinkNode(SphereTreeNode<ObjectType, ExtentAccessor> * node)
{
	// find the child node to be unlinked, then release it to the node
	// free list.
	typename std::vector<SphereTreeNode *>::iterator i;
	i = std::find(children.begin(), children.end(), node);
	if(i != children.end())
	{
		releaseNode(node);
		*i = children.back();
		children.pop_back();
	}

	// am I empty now?
	if(contents.empty())
	{
		if(children.empty())
		{
			if(parent)
			{
				// advise the parent that this node is going awawy. That
				// may cause the parent to be empty as well, so it in turn
				// should be unlinked
				parent->unlinkNode(this);
			}
		}
	}
}

// ----------------------------------------------------------------------

template<class ObjectType, class ExtentAccessor>
inline SphereTreeNode<ObjectType, ExtentAccessor> * SphereTreeNode<ObjectType, ExtentAccessor>::getParent()
{
	return parent;
}

template<class ObjectType, class ExtentAccessor>
inline SphereTreeNode<ObjectType, ExtentAccessor> const * SphereTreeNode<ObjectType, ExtentAccessor>::getParent() const
{
	return parent;
}

template<class ObjectType, class ExtentAccessor>
inline void SphereTreeNode<ObjectType, ExtentAccessor>::setParent(SphereTreeNode * newParent)
{
	// Make sure we're not making this node an ancestor of itself

#ifdef _DEBUG

	SphereTreeNode * cursor = newParent;

	while(cursor)
	{
		DEBUG_FATAL(cursor == this,("SphereTreeNode::setParent - Trying to make a node an ancestor of itself\n"));

		cursor = cursor->parent;
	}

#endif

	parent = newParent;
}

// ----------------------------------------------------------------------

template<class ObjectType, class ExtentAccessor>
inline Sphere SphereTreeNode<ObjectType, ExtentAccessor>::getSphere(ObjectType object) const
{
	Sphere temp = ExtentAccessor::getExtent(object);

	if(temp.getRadius() < 0.5f)
	{
		// HACK insure a minimum radius so that zero-sized or very small objects don't
		// cause the sphere tree to get too deep
		temp.setRadius(0.5f);
	}

	return temp;
}

// ----------------------------------------------------------------------

#if defined(PLATFORM_WIN32)

#include <float.h>

template<class ObjectType, class ExtentAccessor>
inline bool SphereTreeNode<ObjectType, ExtentAccessor>::isValidSphere(Sphere const & sphere) const
{
	Vector const & center = sphere.getCenter();
	float radius = sphere.getRadius();

	if(_isnan(center.x)) return false;
	if(_isnan(center.y)) return false;
	if(_isnan(center.z)) return false;
	if(_isnan(radius)) return false;

	if(!_finite(center.x)) return false;
	if(!_finite(center.y)) return false;
	if(!_finite(center.z)) return false;
	if(!_finite(radius)) return false;

	return true;
}

// ----------

#elif defined(PLATFORM_LINUX)

#include <math.h>

template<class ObjectType, class ExtentAccessor>
inline bool SphereTreeNode<ObjectType, ExtentAccessor>::isValidSphere(Sphere const & sphere) const
{
	Vector const & center = sphere.getCenter();
	float radius = sphere.getRadius();

	if(std::isnan(center.x)) return false;
	if(std::isnan(center.y)) return false;
	if(std::isnan(center.z)) return false;
	if(std::isnan(radius)) return false;

	if(!finite(center.x)) return false;
	if(!finite(center.y)) return false;
	if(!finite(center.z)) return false;
	if(!finite(radius)) return false;

	return true;
}

// ----------

#else

#error unsupported platform

#endif

//-----------------------------------------------------------------------

template<class ObjectType, class ExtentAccessor>
inline bool SphereTreeNode<ObjectType, ExtentAccessor>::isWithin (Vector const & position) const
{
	if (!canContain (position))
		return false;

	//-- search content nodes
	{
		typename std::vector<ObjectType>::const_iterator iter;
		for (iter = contents.begin (); iter != contents.end (); ++iter)
		{
			Sphere const & sphere = getSphere (*iter);
			if (sphere.contains (position))
				return true;
		}
	}

	//-- search children
	{
		typename std::vector<SphereTreeNode *>::const_iterator iter;
		for (iter = children.begin(); iter != children.end(); ++iter)
		{
			SphereTreeNode const * const node = *iter;
			if (node->isWithin (position))
				return true;
		}
	}

	return false;
}

//-----------------------------------------------------------------------

template<class ObjectType, class ExtentAccessor>
Sphere const &SphereTreeNode<ObjectType, ExtentAccessor>::getRealSphere() const
{
	return realSphere;
}

//-----------------------------------------------------------------------

template <class ObjectType, class ExtentAccessor>
class BaseSphereTreeAccessor
{
public:
	static typename SphereTreeNode<ObjectType, ExtentAccessor>::NodeHandle *allocateNodeHandle()
	{
		return new typename SphereTreeNode<ObjectType, ExtentAccessor>::NodeHandle;
	}
};

//-----------------------------------------------------------------------

#endif	// _INCLUDED_SphereTreeNode_H
