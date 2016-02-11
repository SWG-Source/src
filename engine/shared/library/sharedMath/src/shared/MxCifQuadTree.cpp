// ======================================================================
//
// MxCifQuadTree.cpp
//
// Copyright 2002, Sony Online Entertainment
//
// ======================================================================

#include "sharedMath/FirstSharedMath.h"
#include "sharedMath/MxCifQuadTree.h"
#include "sharedMath/MxCifQuadTreeBounds.h"

#include <algorithm>


//==============================================================================

/**
 * Class constructor.
 *
 * @param minX			min x coordinate of our area
 * @param minY			min y coordinate of our area
 * @param maxX			max x coordinate of our area
 * @param maxY			max y coordinate of our area
 * @param maxDepth		max depth of this tree
 */
MxCifQuadTree::MxCifQuadTree(float minX, float minY, float maxX, float maxY, int maxDepth) :
	m_minX(minX),
	m_minY(minY),
	m_maxX(maxX),
	m_maxY(maxY),
	m_centerX((m_maxX - m_minX) / 2.0f + m_minX),
	m_centerY((m_maxY - m_minY) / 2.0f + m_minY),
	m_maxDepth(maxDepth),
	m_urTree(nullptr),
	m_ulTree(nullptr),
	m_llTree(nullptr),
	m_lrTree(nullptr),
	m_xAxisTree(minX, maxX, maxDepth),
	m_yAxisTree(minY, maxY, maxDepth)
{
}	// MxCifQuadTree::MxCifQuadTree

//------------------------------------------------------------------------------

/**
 * Class destructor.
 */
MxCifQuadTree::~MxCifQuadTree()
{
	delete m_urTree;
	m_urTree = nullptr;
	delete m_ulTree;
	m_ulTree = nullptr;
	delete m_llTree;
	m_llTree = nullptr;
	delete m_lrTree;
	m_lrTree = nullptr;
}	// MxCifQuadTree::~MxCifQuadTree

//------------------------------------------------------------------------------

/**
 * Splits this quad into four sub-quads.
 *
 * @return true if we split, false if we have reaced the max depth
 */
bool MxCifQuadTree::split(void)
{
	if (m_maxDepth <= 1)
		return false;

	int newDepth = m_maxDepth - 1;

	m_urTree = new MxCifQuadTree(m_centerX, m_centerY,    m_maxX,    m_maxY, newDepth);
	m_ulTree = new MxCifQuadTree(   m_minX, m_centerY, m_centerX,    m_maxY, newDepth);
	m_llTree = new MxCifQuadTree(   m_minX,    m_minY, m_centerX, m_centerY, newDepth);
	m_lrTree = new MxCifQuadTree(m_centerX,    m_minY,    m_maxX, m_centerY, newDepth);
	return true;
}	// MxCifQuadTree::split

//------------------------------------------------------------------------------

/**
 * Adds an object to the tree.
 *
 * @param object		the object to add
 *
 * @return true if the object was added, false if not
 */
bool MxCifQuadTree::addObject(const MxCifQuadTreeBounds & object)
{
	// see if the object fits entirely within us
	if (object.getMaxX() <= m_maxX &&
		object.getMaxY() <= m_maxY &&
		object.getMinX() >= m_minX &&
		object.getMinY() >= m_minY)
	{
		// try putting the object in a child node
		if (m_maxDepth > 1)
		{
			if (m_urTree == nullptr)
			{
				if (!split())
					return false;
			}
			if (m_urTree->addObject(object) ||
				m_ulTree->addObject(object) ||
				m_llTree->addObject(object) ||
				m_lrTree->addObject(object))
			{
				return true;
			}
			// put the object into one of the axis trees
			if (object.getMaxX() < m_centerX ||
				object.getMinX() > m_centerX)
			{
				// add the object to our x-axis tree
				m_xAxisTree.addObject(object);
			}
			else
			{
				// add the object to our y-axis tree
				m_yAxisTree.addObject(object);
			}
			return true;
		}
		else
		{
			// can't subdivide further, add the object to our list
			if (object.getMaxX() - object.getMinX() < object.getMaxY() - object.getMinY())
				m_xAxisTree.addObject(object);
			else
				m_yAxisTree.addObject(object);
			return true;
		}
	}
	else
		return false;
}	// MxCifQuadTree::addObject

//------------------------------------------------------------------------------

/**
 * Removes an object from the tree.
 *
 * @param object		the object to remove
 *
 * @return true if the object was removed, false if not
 */
bool MxCifQuadTree::removeObject(const MxCifQuadTreeBounds & object)
{
	// see if the object fits entirely within us
	if (object.getMaxX() <= m_maxX &&
		object.getMaxY() <= m_maxY &&
		object.getMinX() >= m_minX &&
		object.getMinY() >= m_minY)
	{
		if (m_maxDepth > 1)
		{
			if (m_urTree != nullptr)
			{
				// check if the object is in a sub-node 
				if (m_urTree->removeObject(object) ||
					m_ulTree->removeObject(object) ||
					m_llTree->removeObject(object) ||
					m_lrTree->removeObject(object))
				{
					return true;
				}
			}
			// remove the object from one of the axis trees
			if (object.getMaxX() < m_centerX ||
				object.getMinX() > m_centerX)
			{
				return m_xAxisTree.removeObject(object);
			}
			else
			{
				return m_yAxisTree.removeObject(object);
			}
		}
		else
		{
			// can't subdivide further, remove the object from an axis tree
			if (object.getMaxX() - object.getMinX() < object.getMaxY() - object.getMinY())
				return m_xAxisTree.removeObject(object);
			else
				return m_yAxisTree.removeObject(object);
		}
	}
	else
		return false;
}	// MxCifQuadTree::removeObject

//------------------------------------------------------------------------------

/**
 * Finds all the objects that contain a given point.
 *
 * @param x			x coordinate of the point
 * @param y			y coordinate of the point
 * @param objects	vector that will be filled in with the objects that contain the point
 */
void MxCifQuadTree::getObjectsAt(float x, float y, 
	std::vector<const MxCifQuadTreeBounds *> & objects) const
{
	// find if we contain the point
	if (x <= m_maxX &&
		x >= m_minX &&
		y <= m_maxY &&
		y >= m_minY)
	{
		// if we have sub-trees, pass the point to the tree that contains it
		if (m_urTree != nullptr)
		{
			if (x >= m_centerX && y >= m_centerY)
				m_urTree->getObjectsAt(x, y, objects);
			else if (x <= m_centerX && y >= m_centerY)
				m_ulTree->getObjectsAt(x, y, objects);
			else if (x <= m_centerX && y <= m_centerY)
				m_llTree->getObjectsAt(x, y, objects);
			else
				m_lrTree->getObjectsAt(x, y, objects);
		}
		// test the objects in our axis trees
		m_xAxisTree.getObjectsAt(x, y, objects);
		m_yAxisTree.getObjectsAt(x, y, objects);
	}
}	// MxCifQuadTree::getObjectsAt

//------------------------------------------------------------------------------

/**
 * Returns all the objects in the tree.
 *
 * @param objects	vector that will be filled in with the objects
 */
void MxCifQuadTree::getAllObjects(std::vector<const MxCifQuadTreeBounds *> & objects) const
{
	// if we have sub-trees, pass the point to the tree that contains it
	if (m_urTree != nullptr)
	{
		m_urTree->getAllObjects(objects);
		m_ulTree->getAllObjects(objects);
		m_llTree->getAllObjects(objects);
		m_lrTree->getAllObjects(objects);
	}
	m_xAxisTree.getAllObjects(objects);
	m_yAxisTree.getAllObjects(objects);
}	// MxCifQuadTree::getAllObjects


//==============================================================================

/**
 * Class constructor.
 *
 * @param min			min range value
 * @param min			max range value
 * @param maxDepth		max depth of this tree
 */
MxCifQuadTree::MxCifBinTree::MxCifBinTree(float min, float max, int maxDepth) :
	m_min(min),
	m_max(max),
	m_center((max - min) / 2.0f + min),
	m_maxDepth(maxDepth),
	m_left(nullptr),
	m_right(nullptr),
	m_objects()
{
}	// MxCifBinTree::MxCifBinTree

/**
 * Class destructor.
 */
MxCifQuadTree::MxCifBinTree::~MxCifBinTree()
{
	delete m_left;
	m_left = nullptr;
	delete m_right;
	m_right = nullptr;
	m_objects.clear();
}	// MxCifBinTree::~MxCifBinTree

/**
 * Splits this tree into two sub-trees.
 *
 * @return true if we split, false if we have reaced the max depth
 */
bool MxCifQuadTree::MxCifBinTree::split(void)
{
	if (m_maxDepth <= 1)
		return false;

	int newDepth = m_maxDepth - 1;

	m_left  = createChild(   m_min, m_center, newDepth);
	m_right = createChild(m_center,    m_max, newDepth);
	return true;
}	// MxCifBinTree::split

/**
 * Adds an object to the tree.
 *
 * @param object		the object to add
 *
 * @return true if the object was added, false if not
 */
bool MxCifQuadTree::MxCifBinTree::addObject(const MxCifQuadTreeBounds & object)
{
	// see if the object fits entirely within us
	if (isObjectInRange(object))
	{
		// try putting the object in a child node
		if (m_maxDepth > 1)
		{
			if (m_left == nullptr)
			{
				if (!split())
					return false;
			}
			if (m_left->addObject(object) ||
				m_right->addObject(object))
			{
				return true;
			}
		}
		// add the object to us
		m_objects.push_back(&object);
		return true;
	}
	else
		return false;
}	// MxCifBinTree::addObject

/**
 * Removes an object from the tree.
 *
 * @param object		the object to remove
 *
 * @return true if the object was removed, false if not
 */
bool MxCifQuadTree::MxCifBinTree::removeObject(const MxCifQuadTreeBounds & object)
{
	// see if the object fits entirely within us
	if (isObjectInRange(object))
	{
		if (m_maxDepth > 1)
		{
			if (m_left != nullptr)
			{
				// check if the object is in a sub-node 
				if (m_left->removeObject(object) ||
					m_right->removeObject(object))
				{
					return true;
				}
			}
		}
		// remove the object from us
		std::vector<const MxCifQuadTreeBounds *>::iterator result = std::find(
			m_objects.begin(), m_objects.end(), &object);
		if (result != m_objects.end())
		{
			m_objects.erase(result);
			return true;
		}
	}
	return false;
}	// MxCifBinTree::removeObject

/**
 * Returns all the objects in the tree.
 *
 * @param objects	vector that will be filled in with the objects that contain the point
 */
void MxCifQuadTree::MxCifBinTree::getAllObjects(
	std::vector<const MxCifQuadTreeBounds *> & objects) const
{
	// if we have sub-trees, pass the point to the tree that contains it
	if (m_left != nullptr)
	{
		m_right->getAllObjects(objects);
		m_left->getAllObjects(objects);
	}

	for (std::vector<const MxCifQuadTreeBounds *>::const_iterator iter = m_objects.begin();
		iter != m_objects.end(); ++iter)
	{
		objects.push_back(*iter);
	}
}	// MxCifBinTree::getAllObjects


//==============================================================================

/**
 * Class destructor.
 */
MxCifQuadTree::MxCifXBinTree::~MxCifXBinTree()
{
}	// MxCifXBinTree::~MxCifXBinTree

/**
 * Checks if an object's x-axis range is contained within our range.
 *
 * @param object		the object to check
 *
 * @return true if we caontain the object, false if not
 */
bool MxCifQuadTree::MxCifXBinTree::isObjectInRange(const MxCifQuadTreeBounds & object) const
{
	// see if the object fits entirely within us
	if (object.getMaxX() <= m_max &&
		object.getMinX() >= m_min)
	{
		return true;
	}
	return false;
}	// MxCifXBinTree::isObjectInRange

/**
 * Finds all the objects that contain a given point.
 *
 * @param x			x coordinate of the point
 * @param y			y coordinate of the point
 * @param objects	vector that will be filled in with the objects that contain the point
 */
void MxCifQuadTree::MxCifXBinTree::getObjectsAt(float x, float y, 
	std::vector<const MxCifQuadTreeBounds *> & objects) const
{
	// find if we contain the point
	if (x <= m_max &&
		x >= m_min)
	{
		// if we have sub-trees, pass the point to the tree that contains it
		if (m_left != nullptr)
		{
			if (x >= m_center)
				m_right->getObjectsAt(x, y, objects);
			else
				m_left->getObjectsAt(x, y, objects);
		}

		// check against each object in our list
		for (std::vector<const MxCifQuadTreeBounds *>::const_iterator iter = m_objects.begin();
			iter != m_objects.end(); ++iter)
		{
			const MxCifQuadTreeBounds * object = *iter;
			if (object->isPointIn(x, y))
			{
				objects.push_back(object);
			}
		}
	}
}	// MxCifXBinTree::getObjectsAt

//==============================================================================

/**
 * Class destructor.
 */
MxCifQuadTree::MxCifYBinTree::~MxCifYBinTree()
{
}	// MxCifYBinTree::~MxCifYBinTree

/**
 * Checks if an object's y-axis range is contained within our range.
 *
 * @param object		the object to check
 *
 * @return true if we caontain the object, false if not
 */
bool MxCifQuadTree::MxCifYBinTree::isObjectInRange(const MxCifQuadTreeBounds & object) const
{
	// see if the object fits entirely within us
	if (object.getMaxY() <= m_max &&
		object.getMinY() >= m_min)
	{
		return true;
	}
	return false;
}	// MxCifYBinTree::isObjectInRange

/**
 * Finds all the objects that contain a given point.
 *
 * @param x			x coordinate of the point
 * @param y			y coordinate of the point
 * @param objects	vector that will be filled in with the objects that contain the point
 */
void MxCifQuadTree::MxCifYBinTree::getObjectsAt(float x, float y, 
	std::vector<const MxCifQuadTreeBounds *> & objects) const
{
	// find if we contain the point
	if (y <= m_max &&
		y >= m_min)
	{
		// if we have sub-trees, pass the point to the tree that contains it
		if (m_left != nullptr)
		{
			if (y >= m_center)
				m_right->getObjectsAt(x, y, objects);
			else
				m_left->getObjectsAt(x, y, objects);
		}

		// check against each object in our list
		for (std::vector<const MxCifQuadTreeBounds *>::const_iterator iter = m_objects.begin();
			iter != m_objects.end(); ++iter)
		{
			const MxCifQuadTreeBounds * object = *iter;
			if (object->isPointIn(x, y))
			{
				objects.push_back(object);
			}
		}
	}
}	// MxCifYBinTree::getObjectsAt

//==============================================================================
