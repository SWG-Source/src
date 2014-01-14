// ======================================================================
//
// PortallizedSphereTree.h
//
// Copyright 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_PortallizedSphereTree_H
#define INCLUDED_PortallizedSphereTree_H

// ======================================================================
//
// PortallizedSphereTree is a spatial organization class which keeps
// objects grouped in the space of the pob containing them, such that
// the structure only needs to be updated for objects that move
// relative to their parent.
//
// For each pob with a containing object added to the PortallizedSphereTree,
// there is a SphereTree of the objects in that pob, kept in the local
// space of the pob.  There is also an equivalent separate tree of objects
// in the world cell.  We then keep a SphereTree of the pobs themselves,
// which references the tree for the pob, whose spheres are the containing
// spheres for the SphereTree for the individual pob, in world space.
//
// A PortallizedSphereTree<ObjectType, Accessor> should be set up such
// that Accessor is a derived class of PortallizedSphereTreeAccessor
// <ObjectType, Accessor>.  It should additionally provide, at a minimum,
//   static Sphere const getExtent(ObjectType object);
//   static Object const *getCurrentPob(ObjectType object);
// These do not need to be exact signature matches, only assignment-compatible
// functions.  getExtent should return the extent sphere for object, in
// parent space.  getCurrentPob should return the containing pob whose space
// object is in, or 0 for world cell objects.
//
// A PortallizedSphereTree should be updated when an object should be
// added to it, or an object already in it should be removed or moved,
// via the onObjectAdded, onObjectRemoved, and onObjectMoved members.
//
// Searching the tree should be done via the various findInRange members.
//
// ======================================================================

#include "sharedFoundation/Watcher.h"
#include "sharedMath/Capsule.h"
#include "sharedMath/SphereTree.h"

// ======================================================================

class Object;

// ======================================================================

template <class ObjectType, class Accessor>
class PortallizedSphereTreeNodeHandle: public SphereTreeNode<ObjectType, Accessor>::NodeHandle
{
public:

	PortallizedSphereTreeNodeHandle() :
		SphereTreeNode<ObjectType, Accessor>::NodeHandle(),
		m_storedPob(0)
	{
	}

	Object const *getStoredPob() const
	{
		return m_storedPob;
	}

	void setStoredPob(Object const *pob)
	{
		m_storedPob = pob;
	}

private:
	Object const *m_storedPob;
};

// ======================================================================

template <class ObjectType, class Accessor>
class PortallizedSphereTree
{
public:
	PortallizedSphereTree();
	~PortallizedSphereTree();

	void onObjectAdded(ObjectType object);
	void onObjectRemoved(ObjectType object);
	void onObjectMoved(ObjectType object);

	// search functions which check multiple pobs
	void findInRange(Vector const &center_w, float radius, std::vector<ObjectType> &results) const;
	void findInRange(Vector const &center_w, float radius, SpatialSubdivisionFilter<ObjectType> const &filter, std::vector<ObjectType> &results) const;
	void findInRange(Capsule const &queryCapsule_w, std::vector<ObjectType> &results) const;
	void findInRange(Capsule const &queryCapsule_w, SpatialSubdivisionFilter<ObjectType> const &filter, std::vector<ObjectType> &results) const;
	// search functions which check a single pob
	void findInRange(Object const *pob, Vector const &center_p, float radius, std::vector<ObjectType> &results) const;
	void findInRange(Object const *pob, Vector const &center_p, float radius, SpatialSubdivisionFilter<ObjectType> const &filter, std::vector<ObjectType> &results) const;

	void dumpSphereTree(std::vector<std::pair<ObjectType, Sphere> > &results) const;

private:
	PortallizedSphereTree(PortallizedSphereTree const &);
	PortallizedSphereTree &operator=(PortallizedSphereTree const &);

	class PobContentsInfo;

	class PobTreeAccessor: public BaseSphereTreeAccessor<PobContentsInfo *, PobTreeAccessor>
	{
	public:
		static Sphere const getExtent(PobContentsInfo const *pct)
		{
			Sphere const &localSphere = pct->contents.getRealSphere();
			Object const * const pob = pct->getPob();
			DEBUG_FATAL(!pob, ("Tried to get the extent for a pob which no longer exists.  This should have been caught earlier."));
			if (pob)
			{
				Transform const &transform = pob->getTransform_o2p();
				return Sphere(transform.rotateTranslate_l2p(localSphere.getCenter()), localSphere.getRadius());
			}
			return localSphere;
		}
	};

	typedef SphereTree<ObjectType, Accessor> PobContentsTree;
	typedef PortallizedSphereTreeNodeHandle<ObjectType, Accessor> PobContentsTreeNodeHandle;
	typedef typename SphereTreeNode<PobContentsInfo *, PobTreeAccessor>::NodeHandle PobTreeNodeHandle;
	typedef SphereTree<PobContentsInfo *, PobTreeAccessor> PobTree;
	typedef std::map<Object const *, PobContentsInfo *> PobContentsInfoMap;

	class PobContentsInfo
	{
	public:
		PobContentsInfo(Object const *newPob) :
			nodeHandle(0),
			pobWatcher(newPob),
			contents()
		{
		}

		Object const *getPob() const
		{
			return pobWatcher.getPointer();
		}

		bool isPobEmpty() const
		{
			return !pobWatcher.getPointer() || contents.empty();
		}

		PobTreeNodeHandle *nodeHandle;
		ConstWatcher<Object> pobWatcher;
		PobContentsTree contents;

	private:
		PobContentsInfo(PobContentsInfo const &);
		PobContentsInfo &operator=(PobContentsInfo const &);
	};

	void onObjectAddedInternal(ObjectType object, Object const *pob);
	void onObjectRemovedInternal(ObjectType object, PobContentsTreeNodeHandle *objectNodeHandle, Object const *oldPob);

	PobContentsInfoMap m_pobContentsInfoMap;
	PobTree m_pobTree;
	PobContentsTree m_worldCellContentsTree;
};

// ----------------------------------------------------------------------

template <class ObjectType, class Accessor>
PortallizedSphereTree<ObjectType, Accessor>::PortallizedSphereTree() :
	m_pobContentsInfoMap(),
	m_pobTree()
{
}

// ----------------------------------------------------------------------

template <class ObjectType, class Accessor>
PortallizedSphereTree<ObjectType, Accessor>::~PortallizedSphereTree()
{
	for (typename PobContentsInfoMap::const_iterator i = m_pobContentsInfoMap.begin(); i != m_pobContentsInfoMap.end(); ++i)
		delete (*i).second;
	m_pobContentsInfoMap.clear();
}

// ----------------------------------------------------------------------

template <class ObjectType, class Accessor>
void PortallizedSphereTree<ObjectType, Accessor>::onObjectAdded(ObjectType object)
{
	onObjectAddedInternal(object, Accessor::getCurrentPob(object));
}

// ----------------------------------------------------------------------

template <class ObjectType, class Accessor>
void PortallizedSphereTree<ObjectType, Accessor>::onObjectAddedInternal(ObjectType object, Object const *pob)
{
	if (!pob)
	{
		// object in the world cell
		PobContentsTreeNodeHandle * const objectNodeHandle = static_cast<PobContentsTreeNodeHandle *>(m_worldCellContentsTree.addObject(object));
		Accessor::setNodeHandle(object, objectNodeHandle);
		objectNodeHandle->setStoredPob(0);
	}
	else
	{
		// object not in the world cell
		typename PobContentsInfoMap::iterator i = m_pobContentsInfoMap.lower_bound(pob);
		if (i == m_pobContentsInfoMap.end() || (*i).first != pob)
		{
			// This pob did not have a PobContentsInfo in the map, so create one,
			// add it to the map and pob tree, and set the iterator to the new
			// entry in the map
			PobContentsInfo * const newPobContentsInfo = new PobContentsInfo(pob);
			i = m_pobContentsInfoMap.insert(i, std::make_pair(pob, newPobContentsInfo));
			newPobContentsInfo->nodeHandle = static_cast<PobTreeNodeHandle *>(m_pobTree.addObject(newPobContentsInfo));
		}

		PobContentsInfo * const pobContentsInfo = (*i).second;

		// Add the object to the contents tree for this pob
		PobContentsTreeNodeHandle * const objectNodeHandle = static_cast<PobContentsTreeNodeHandle *>(pobContentsInfo->contents.addObject(object));
		Accessor::setNodeHandle(object, objectNodeHandle);
		objectNodeHandle->setStoredPob(pob);

		// Update the pob tree node since its extents may have changed
		pobContentsInfo->nodeHandle->move();
	}
}

// ----------------------------------------------------------------------

template <class ObjectType, class Accessor>
void PortallizedSphereTree<ObjectType, Accessor>::onObjectRemoved(ObjectType object)
{
	PobContentsTreeNodeHandle * const objectNodeHandle = Accessor::getNodeHandle(object);
	if (objectNodeHandle)
		onObjectRemovedInternal(object, objectNodeHandle, objectNodeHandle->getStoredPob());
}

// ----------------------------------------------------------------------

template <class ObjectType, class Accessor>
void PortallizedSphereTree<ObjectType, Accessor>::onObjectRemovedInternal(ObjectType object, PobContentsTreeNodeHandle *objectNodeHandle, Object const * oldPob)
{
	if (!oldPob)
	{
		// object was in the world cell
		objectNodeHandle->removeObject();
		Accessor::setNodeHandle(object, 0);
	}
	else
	{
		// object was not in the world cell, but may potentially have been in a pob which has been deleted
		typename PobContentsInfoMap::iterator i = m_pobContentsInfoMap.find(oldPob);
		if (i == m_pobContentsInfoMap.end())
			Accessor::setNodeHandle(object, 0);
		else
		{
			// remove object from the sphere tree for the pob
			objectNodeHandle->removeObject();
			Accessor::setNodeHandle(object, 0);

			if ((*i).second->isPobEmpty())
			{
				// if it was the last object in the pob, remove the pob from the pob tree
				(*i).second->nodeHandle->removeObject();
				delete (*i).second;
				m_pobContentsInfoMap.erase(i);
			}
			else
			{
				// there are still objects in the pob, so update the pob's sphere
				(*i).second->nodeHandle->move();
			}
		}
	}
}

// ----------------------------------------------------------------------

template <class ObjectType, class Accessor>
void PortallizedSphereTree<ObjectType, Accessor>::onObjectMoved(ObjectType object)
{
	PobContentsTreeNodeHandle * const objectNodeHandle = Accessor::getNodeHandle(object);
	if (objectNodeHandle)
	{
		Object const * const startPob = objectNodeHandle->getStoredPob();
		Object const * const endPob = Accessor::getCurrentPob(object);

		if (startPob == endPob)
		{
			// staying in the same parent space, so notify the object node that it moved
			objectNodeHandle->move();
			// if not in the world pob, moving a contained object may change the extents in the pob tree
			if (startPob)
			{
				typename PobContentsInfoMap::iterator i = m_pobContentsInfoMap.find(startPob);
				FATAL(i == m_pobContentsInfoMap.end(), ("Tried to move an object within a pob not in the tree"));
				(*i).second->nodeHandle->move();
			}
		}
		else
		{
			// changing parent spaces
			onObjectRemovedInternal(object, objectNodeHandle, startPob);
			onObjectAddedInternal(object, endPob);
		}
	}
}

// ----------------------------------------------------------------------

template <class ObjectType, class Accessor>
void PortallizedSphereTree<ObjectType, Accessor>::findInRange(Vector const &center_w, float radius, std::vector<ObjectType> &results) const
{
	m_worldCellContentsTree.findInRange(center_w, radius, results);

	static std::vector<PobContentsInfo *> pobResults;
	m_pobTree.findInRange(center_w, radius, pobResults);
	for (typename std::vector<PobContentsInfo *>::const_iterator i = pobResults.begin(); i != pobResults.end(); ++i)
	{
		Object const * const pob = (*i)->getPob();
		if (pob)
			(*i)->contents.findInRange(pob->getTransform_o2p().rotateTranslate_p2l(center_w), radius, results);
	}
	pobResults.clear();
}

// ----------------------------------------------------------------------

template <class ObjectType, class Accessor>
void PortallizedSphereTree<ObjectType, Accessor>::findInRange(Vector const &center_w, float radius, SpatialSubdivisionFilter<ObjectType> const &filter, std::vector<ObjectType> &results) const
{
	m_worldCellContentsTree.findInRange(center_w, radius, filter, results);

	static std::vector<PobContentsInfo *> pobResults;
	m_pobTree.findInRange(center_w, radius, pobResults);
	for (typename std::vector<PobContentsInfo *>::const_iterator i = pobResults.begin(); i != pobResults.end(); ++i)
	{
		Object const * const pob = (*i)->getPob();
		if (pob)
			(*i)->contents.findInRange(pob->getTransform_o2p().rotateTranslate_p2l(center_w), radius, filter, results);
	}
	pobResults.clear();
}

// ----------------------------------------------------------------------

template <class ObjectType, class Accessor>
void PortallizedSphereTree<ObjectType, Accessor>::findInRange(Capsule const &queryCapsule_w, std::vector<ObjectType> &results) const
{
	m_worldCellContentsTree.findInRange(queryCapsule_w, results);

	static std::vector<PobContentsInfo *> pobResults;
	m_pobTree.findInRange(queryCapsule_w, pobResults);
	if (!pobResults.empty())
	{
		Vector const &querySphereCenter_w = queryCapsule_w.getCenter();
		float const querySphereRadius = queryCapsule_w.getTotalRadius();
		for (typename std::vector<PobContentsInfo *>::const_iterator i = pobResults.begin(); i != pobResults.end(); ++i)
		{
			Object const * const pob = (*i)->getPob();
			if (pob)
				(*i)->contents.findInRange(pob->getTransform_o2p().rotateTranslate_p2l(querySphereCenter_w), querySphereRadius, results);
		}
	}
	pobResults.clear();
}

// ----------------------------------------------------------------------

template <class ObjectType, class Accessor>
void PortallizedSphereTree<ObjectType, Accessor>::findInRange(Capsule const &queryCapsule_w, SpatialSubdivisionFilter<ObjectType> const &filter, std::vector<ObjectType> &results) const
{
	m_worldCellContentsTree.findInRange(queryCapsule_w, filter, results);

	static std::vector<PobContentsInfo *> pobResults;
	m_pobTree.findInRange(queryCapsule_w, pobResults);
	if (!pobResults.empty())
	{
		Vector const &querySphereCenter_w = queryCapsule_w.getCenter();
		float const querySphereRadius = queryCapsule_w.getTotalRadius();
		for (typename std::vector<PobContentsInfo *>::const_iterator i = pobResults.begin(); i != pobResults.end(); ++i)
		{
			Object const * const pob = (*i)->getPob();
			if (pob)
				(*i)->contents.findInRange(pob->getTransform_o2p().rotateTranslate_p2l(querySphereCenter_w), querySphereRadius, filter, results);
		}
	}
	pobResults.clear();
}

// ----------------------------------------------------------------------

template <class ObjectType, class Accessor>
void PortallizedSphereTree<ObjectType, Accessor>::findInRange(Object const *pob, Vector const &center_p, float radius, std::vector<ObjectType> &results) const
{
	if (!pob)
		m_worldCellContentsTree.findInRange(center_p, radius, results);
	else
	{
		typename PobContentsInfoMap::const_iterator i = m_pobContentsInfoMap.find(pob);
		if (i != m_pobContentsInfoMap.end())
			(*i).second->contents.findInRange(center_p, radius, results);
	}
}

// ----------------------------------------------------------------------

template <class ObjectType, class Accessor>
void PortallizedSphereTree<ObjectType, Accessor>::findInRange(Object const *pob, Vector const &center_p, float radius, SpatialSubdivisionFilter<ObjectType> const &filter, std::vector<ObjectType> &results) const
{
	if (!pob)
		m_worldCellContentsTree.findInRange(center_p, radius, filter, results);
	else
	{
		typename PobContentsInfoMap::const_iterator i = m_pobContentsInfoMap.find(pob);
		if (i != m_pobContentsInfoMap.end())
			(*i).second->contents.findInRange(center_p, radius, filter, results);
	}
}

// ----------------------------------------------------------------------

template <class ObjectType, class Accessor>
void PortallizedSphereTree<ObjectType, Accessor>::dumpSphereTree(std::vector<std::pair<ObjectType, Sphere> > &results) const
{
	m_worldCellContentsTree.dumpSphereTree(results);

	for (typename PobContentsInfoMap::const_iterator i = m_pobContentsInfoMap.begin(); i != m_pobContentsInfoMap.end(); ++i)
	{
		Object const * const pob = (*i).second->getPob();
		if (pob)
		{
			std::vector<std::pair<ObjectType, Sphere> > tempResults;
			(*i).second->contents.dumpSphereTree(tempResults);
			for (typename std::vector<std::pair<ObjectType, Sphere> >::iterator j = tempResults.begin(); j != tempResults.end(); ++j)
			{
				(*j).second.setCenter(pob->getTransform_o2p().rotateTranslate_l2p((*j).second.getCenter()));
				results.push_back(*j);
			}
		}
	}
}

// ======================================================================

template <class ObjectType, class Accessor>
class PortallizedSphereTreeAccessor
{
public:
	// derived classes need to implement the following - types need only be assignement-compatible, not exact:
	//   static Sphere const getExtent(ObjectType object);
	//   static Object const *getCurrentPob(ObjectType object);

	static PortallizedSphereTreeNodeHandle<ObjectType, Accessor> *getNodeHandle(ObjectType object)
	{
		return static_cast<PortallizedSphereTreeNodeHandle<ObjectType, Accessor> *>(object->getSpatialSubdivisionHandle());
	}

	static void setNodeHandle(ObjectType object, SpatialSubdivisionHandle *nodeHandle)
	{
		object->setSpatialSubdivisionHandle(nodeHandle);
	}

	static typename SphereTreeNode<ObjectType, Accessor>::NodeHandle *allocateNodeHandle()
	{
		return new PortallizedSphereTreeNodeHandle<ObjectType, Accessor>();
	}

};

// ======================================================================

#endif // INCLUDED_PortallizedSphereTree_H

