// ======================================================================
//
// BoxTree.cpp
//
// ======================================================================

#include "sharedCollision/FirstSharedCollision.h"
#include "sharedCollision/BoxTree.h"

#include "sharedCollision/BaseClass.h"
#include "sharedCollision/Overlap3d.h"
#include "sharedCollision/Containment3d.h"
#include "sharedCollision/Distance3d.h"

#include "sharedFile/Iff.h"

#include "sharedFoundation/ExitChain.h"

#include "sharedMath/VectorArgb.h"
#include "sharedMath/DebugShapeRenderer.h"
#include "sharedMath/Line3d.h"

#include "sharedFoundation/MemoryBlockManagerMacros.h"
#include "sharedFoundation/MemoryBlockManager.h"

#include <vector>

const Tag TAG_BTRE = TAG(B,T,R,E);
const Tag TAG_NODS = TAG(N,O,D,S);

// ======================================================================

class BoxTreeNode
{
	MEMORY_BLOCK_MANAGER_INTERFACE_WITH_INSTALL;

public:

	BoxTreeNode();
	~BoxTreeNode();
	
	BoxTreeNode ( AxialBox const & box, int userId );
	BoxTreeNode ( BoxTreeNode * childA, BoxTreeNode * childB );
	
	// ----------
	
	void    drawDebugShapes ( DebugShapeRenderer * renderer, VectorArgb color, int level ) const;
	
	int     getNodeCount    ( void ) const;
	float   calcWeight      ( void ) const;
	float   calcBalance     ( void ) const;
	
	void    assignIndices   ( void );
	
	void    recursePackInto ( BoxTreeNode * & nodeCursor, BoxTreeNode * base ) const;
	void    packInto        ( BoxTreeNode * flatNode, BoxTreeNode * base ) const;
	
	void    assignIndices   ( int & counter );

	void    deleteChildren  ( void );

	void    read            ( Iff & iff, BoxTreeNode * base );
	void    write           ( Iff & iff, BoxTreeNode * base );

	bool    findClosest     ( Vector const & V, float maxDistance, float & outDistance, int & outId ) const;

	float   distanceTo      ( Vector const & V ) const;
	float   childDistanceTo ( Vector const & V ) const;

	// ----------
	// This is stripped down to be 40 bytes, I can't really get it much smaller
	// without majorly changing how the code works.

	AxialBox        m_box;      // The box. Can't live without this, though it could be made lower-precision.
	
	int             m_index;    // Used during packing to keep track of where in the packed array the node goes.
	                            // Not absolutely necessary, but without it packing would be a pain.
	
	int             m_userId;   // A user-supplied integer
	
    BoxTreeNode *   m_childA;
    BoxTreeNode *   m_childB;
};

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITH_INSTALL(BoxTreeNode, true, 0, 0, 0);

// ======================================================================

typedef std::vector<BoxTreeNode*> BoxTreeNodePVec;

typedef std::pair<int,int> IntPair;

IntPair	findClosestPair( BoxTreeNodePVec const & nodes )
{
	int nodeCount = nodes.size();

	float minValue = REAL_MAX;
	int minA = -1;
	int minB = -1;

	for(int i = 0; i < nodeCount - 1; i++)
	{
		for(int j = i+1; j < nodeCount; j++)
		{
			AxialBox tempBox( nodes[i]->m_box, nodes[j]->m_box );

			float tempValue = tempBox.getVolume();

			if(tempValue < minValue)
			{
				minValue = tempValue;
				minA = i;
				minB = j;
			}
		}
	}

	return IntPair(minA,minB);
}

// ----------
// Erase one or two elements from a vector in constant time,
// but this changes the order of the elements.

inline void erase( BoxTreeNodePVec & nodes, int whichNode )
{
	nodes[whichNode] = nodes.back();

	nodes.resize(nodes.size()-1);
}

// When erasing more than one element, we have to erase them in order
// from back to front.

inline void erase( BoxTreeNodePVec & nodes, int nodeA, int nodeB )
{
	if(nodeA == nodeB)
	{
		erase(nodes,nodeA);
	}
	else if(nodeA > nodeB)
	{
		erase(nodes,nodeA);
		erase(nodes,nodeB);
	}
	else
	{
		erase(nodes,nodeB);
		erase(nodes,nodeA);
	}
}

// ----------------------------------------------------------------------

BoxTreeNode::BoxTreeNode()
: m_box(),
  m_index(-1),
  m_userId(-1),
  m_childA(nullptr),
  m_childB(nullptr)
{
}

BoxTreeNode::BoxTreeNode ( AxialBox const & box, int userId )
: m_box(box),
  m_index(-1),
  m_userId(userId),
  m_childA(nullptr),
  m_childB(nullptr)
{
}

BoxTreeNode::BoxTreeNode ( BoxTreeNode * childA, BoxTreeNode * childB )
: m_box(),
  m_index(-1),
  m_userId(-1),
  m_childA(childA),
  m_childB(childB)
{
	if(m_childA) m_box.add(m_childA->m_box);
	if(m_childB) m_box.add(m_childB->m_box);
}

BoxTreeNode::~BoxTreeNode()
{
}

// ----------------------------------------------------------------------

void BoxTreeNode::drawDebugShapes ( DebugShapeRenderer * renderer, VectorArgb color, int level ) const
{
	UNREF(renderer);
	UNREF(color);
	UNREF(level);

#ifdef _DEBUG

	if(m_childA == nullptr) return;
	if(m_childB == nullptr) return;

	if(renderer == nullptr) return;

	VectorArgb newColor( color.a, color.r * 0.9f, color.g * 0.9f, color.b * 0.9f );

	if(m_childA) m_childA->drawDebugShapes(renderer,newColor,level+1);
	if(m_childB) m_childB->drawDebugShapes(renderer,newColor,level+1);

	renderer->setColor(color);
	renderer->drawBox(m_box);

#endif
}

// ----------------------------------------------------------------------

int BoxTreeNode::getNodeCount ( void ) const
{
	int accum = 1;

	if(m_childA) accum += m_childA->getNodeCount();
	if(m_childB) accum += m_childB->getNodeCount();	

	return accum;
}

// ----------

float BoxTreeNode::calcWeight ( void ) const
{
	if((m_childA == nullptr) && (m_childB == nullptr)) return 1.0f;

	float accum = 0.0f;

	if(m_childA) accum += m_childA->calcWeight();
	if(m_childB) accum += m_childB->calcWeight();

	return accum;
}

// ----------

float BoxTreeNode::calcBalance ( void ) const
{
	if((m_childA == nullptr) && (m_childB == nullptr)) return 1.0f;

	if((m_childA != nullptr) && (m_childB != nullptr))
	{
		return m_childA->calcWeight() / m_childB->calcWeight();
	}
	else
	{
		return 0.0f;
	}
}

// ----------------------------------------------------------------------

void BoxTreeNode::assignIndices ( void )
{
	int counter = 0;

	assignIndices(counter);
}

void BoxTreeNode::assignIndices ( int & counter )
{
	m_index = counter;
	counter++;

	if(m_childA) m_childA->assignIndices(counter);
	if(m_childB) m_childB->assignIndices(counter);
}

// ----------------------------------------------------------------------

void BoxTreeNode::recursePackInto ( BoxTreeNode * & nodeCursor, BoxTreeNode * base ) const
{
	packInto(nodeCursor,base);

	nodeCursor++;

	if(m_childA) m_childA->recursePackInto(nodeCursor,base);
	if(m_childB) m_childB->recursePackInto(nodeCursor,base);
}

void BoxTreeNode::packInto ( BoxTreeNode * node, BoxTreeNode * base ) const
{
	node->m_box = m_box;
	node->m_index = m_index;

	node->m_childA = m_childA ? (base + m_childA->m_index) : nullptr;
	node->m_childB = m_childB ? (base + m_childB->m_index) : nullptr;

	node->m_userId = m_userId;
}

// ----------------------------------------------------------------------

void BoxTreeNode::deleteChildren ( void )
{
	if(m_childA) m_childA->deleteChildren();
	if(m_childB) m_childB->deleteChildren();

	delete m_childA;
	m_childA = nullptr;

	delete m_childB;
	m_childB = nullptr;
}

// ----------------------------------------------------------------------

void BoxTreeNode::write ( Iff & iff, BoxTreeNode * base )
{
	iff.insertChunkFloatVector( m_box.getMax() );
	iff.insertChunkFloatVector( m_box.getMin() );

	iff.insertChunkData(m_index);
	iff.insertChunkData(m_userId);

	int indexA = ( m_childA ? m_childA - base : -1 );
	int indexB = ( m_childB ? m_childB - base : -1 );

	iff.insertChunkData(indexA);
	iff.insertChunkData(indexB);
}

// ----------

void BoxTreeNode::read ( Iff & iff, BoxTreeNode * base )
{
	Vector max = iff.read_floatVector();
	Vector min = iff.read_floatVector();

	m_box = AxialBox(min,max);

	m_index = iff.read_int32();

	m_userId = iff.read_int32();

	int indexA = iff.read_int32();
	int indexB = iff.read_int32();

	m_childA = ( indexA != -1 ? base + indexA : nullptr );
	m_childB = ( indexB != -1 ? base + indexB : nullptr );
}

// ----------------------------------------------------------------------

bool BoxTreeNode::findClosest ( Vector const & V, float maxDistance, float & outDistance, int & outIndex ) const
{
	float dist = Distance3d::Distance2PointABox(V,m_box);

	if(dist >= maxDistance)
	{
		// This node is too far away - don't recurse

		return false;
	}
	
	// ----------
	// Leaf node case - this leaf node is closer 

	if((m_childA == nullptr) && (m_childB == nullptr))
	{
		outDistance = dist;
		outIndex = m_userId;

		return true;
	}

	// ----------
	// Branch node case - recurse through both children and return the closer (if there is one)

	float distanceA = REAL_MAX;
	float distanceB = REAL_MAX;

	int indexA = -1;
	int indexB = -1;

	bool closerA = m_childA->findClosest(V,maxDistance,distanceA,indexA);

	// reduce maxDistance for the child B search so we don't waste time searching through descendants of B that are farther than the closest descendant found in A

	if(closerA) maxDistance = distanceA;

	bool closerB = m_childB->findClosest(V,maxDistance,distanceB,indexB);

	if(closerA || closerB)
	{
		if(distanceA < distanceB)
		{
			outDistance = distanceA;
			outIndex = indexA;
		}
		else
		{
			outDistance = distanceB;
			outIndex = indexB;
		}

		return true;
	}
	else
	{
		return false;
	}
}

// ======================================================================

void BoxTree::install()
{
	BoxTreeNode::install();
}

// ----------------------------------------------------------------------

BoxTree::BoxTree()
: m_flatNodes(nullptr),
  m_root(nullptr),
  m_testCounter(0)
{
}

BoxTree::~BoxTree()
{
	clear();
}

// ----------------------------------------------------------------------

inline BoxTreeNode const * BoxTree::getRoot ( void ) const
{
	return m_root;
}

// ----------------------------------------------------------------------

void BoxTree::build ( BoxVec const & boxes )
{
	if(boxes.empty()) return;

	// ----------

	BoxTreeNodePVec nodes(boxes.size(),nullptr);

	int boxcount = boxes.size();

	for(int i = 0; i < boxcount; i++)
	{
		nodes[i] = new BoxTreeNode(boxes[i],i);
	}

	// ----------
	// Build the tree, brute-force bottom-up (slooooow)

	while(nodes.size() > 1)
	{
		IntPair closestPair = findClosestPair(nodes);

		int A = closestPair.first;
		int B = closestPair.second;

		BoxTreeNode * newNode = new BoxTreeNode( nodes[A], nodes[B] );

		erase(nodes,A,B);

		nodes.push_back(newNode);
	}

	// ----------

	m_root = nodes[0];

	pack();

	m_root->deleteChildren();
	delete m_root;
	m_root = &m_flatNodes->front();
}

// ----------------------------------------------------------------------

void BoxTree::drawDebugShapes ( DebugShapeRenderer * renderer ) const
{
	UNREF(renderer);

#ifdef _DEBUG

	if(renderer == nullptr) return;

	if(m_root) m_root->drawDebugShapes( renderer, VectorArgb::solidWhite, 0 );

#endif
}

// ----------------------------------------------------------------------

int BoxTree::getNodeCount ( void ) const
{
	if(m_flatNodes) return m_flatNodes->size();

	else if(m_root) return m_root->getNodeCount();

	else return 0;
}

// ----------

float BoxTree::calcWeight ( void ) const
{
	if(m_root) 
		return m_root->calcWeight();
	else
		return 0.0f;
}

// ----------

float BoxTree::calcBalance ( void ) const
{
	if(m_root)
		return m_root->calcBalance();
	else
		return 1.0f;
}

// ----------------------------------------------------------------------

template< class TestShape >
static inline void templateTestOverlapRecurse( BoxTreeNode const * node, TestShape const & testShape, IdVec & outIds )
{
	if(Overlap3d::Test(testShape,node->m_box))
	{
		if(node->m_userId != -1) 
		{
			outIds.push_back(node->m_userId);
		}

		if(node->m_childA) templateTestOverlapRecurse(node->m_childA,testShape,outIds);
		if(node->m_childB) templateTestOverlapRecurse(node->m_childB,testShape,outIds);
	}
}

template< class TestShape >
static inline bool templateTestOverlap( BoxTree const & tree, TestShape const & testShape, IdVec & outIds )
{
	if(tree.getRoot() == nullptr) return false;

	int oldSize = outIds.size();

	templateTestOverlapRecurse( tree.getRoot(), testShape, outIds );

	return oldSize != static_cast<int>(outIds.size());
}

// ----------
// Slightly different recursion - test the bounding box of the shape on nodes,
// only test against the actual shape on leaf nodes

template< class TestShape >
static inline void templateTestOverlapRecurse2( BoxTreeNode const * node, TestShape const & testShape, AxialBox const & shapeBounds, IdVec & outIds )
{
	if(Overlap3d::Test(shapeBounds,node->m_box))
	{
		if(node->m_userId != -1) 
		{
			if(Overlap3d::Test(testShape,node->m_box))
			{
				outIds.push_back(node->m_userId);
			}
		}

		if(node->m_childA) templateTestOverlapRecurse(node->m_childA,testShape,outIds);
		if(node->m_childB) templateTestOverlapRecurse(node->m_childB,testShape,outIds);
	}
}

template< class TestShape >
static inline bool templateTestOverlap2( BoxTree const & tree, TestShape const & testShape, IdVec & outIds )
{
	if(tree.getRoot() == nullptr) return false;

	int oldSize = outIds.size();

	AxialBox shapeBounds = Containment3d::EncloseABox(testShape);

	templateTestOverlapRecurse( tree.getRoot(), testShape, shapeBounds, outIds );

	return oldSize != outIds.size();
}

// ----------

bool BoxTree::testOverlap ( AxialBox const & box,      IdVec & outIds ) const   { return templateTestOverlap(*this,box,outIds); }
bool BoxTree::testOverlap ( Line3d const & line,       IdVec & outIds ) const   { return templateTestOverlap(*this,line,outIds); }
bool BoxTree::testOverlap ( Ray3d const & ray,         IdVec & outIds ) const   { return templateTestOverlap(*this,ray,outIds); }
bool BoxTree::testOverlap ( Segment3d const & segment, IdVec & outIds ) const   { return templateTestOverlap(*this,segment,outIds); }

// ----------------------------------------------------------------------

bool BoxTree::findClosest ( Vector const & V, float maxDistance, float & outDistance, int & outId ) const
{
	return getRoot()->findClosest(V,maxDistance,outDistance,outId);
}

// ----------------------------------------------------------------------

void BoxTree::write ( Iff & iff )
{
	iff.insertForm(TAG_BTRE);

	iff.insertForm(TAG_0000);

	// ----------

	iff.insertChunk(TAG_NODS);
	{
		int nodeCount = m_flatNodes->size();

		iff.insertChunkData(nodeCount);

		BoxTreeNode * base = &m_flatNodes->front();

		for( int i = 0; i < nodeCount; i++ )
		{
			m_flatNodes->at(i).write(iff,base);
		}
	}
	iff.exitChunk(TAG_NODS);

	// ----------

	iff.exitForm(TAG_0000);
	
	iff.exitForm(TAG_BTRE);
}

// ----------------------------------------------------------------------

void BoxTree::read ( Iff & iff )
{
	iff.enterForm(TAG_BTRE);

	switch (iff.getCurrentName())
	{
		case TAG_0000:
			read_0000(iff);
			break;

		default:
			FATAL(true,("FloorMesh::Invalid version"));
			break;
	}

	iff.exitForm(TAG_BTRE);

	if(m_flatNodes)
	{
		m_root = &m_flatNodes->front();
	}
}

// ----------------------------------------------------------------------

void BoxTree::read_0000 ( Iff & iff )
{
	clear();

	iff.enterForm(TAG_0000);

	// ----------

	iff.enterChunk(TAG_NODS);
	{
		int nodeCount = iff.read_int32();

		m_flatNodes = new BoxTreeNodeVec(nodeCount);

		BoxTreeNode * base = &m_flatNodes->front();

		for(int i = 0; i < nodeCount; i++)
		{
			m_flatNodes->at(i).read(iff,base);
		}
	}
	iff.exitChunk(TAG_NODS);

	// ----------

	iff.exitForm(TAG_0000);
}

// ----------------------------------------------------------------------

void BoxTree::clear ( void )
{
	if(m_root && !isFlat())
	{
		m_root->deleteChildren();

		delete m_root;
		m_root = nullptr;
	}

	delete m_flatNodes;
	m_flatNodes = nullptr;
}

// ----------------------------------------------------------------------

void BoxTree::pack ( void )
{
	if(!m_root) return;

	if(m_flatNodes) return;

	int nodeCount = getNodeCount();

	m_flatNodes = new BoxTreeNodeVec();
	m_flatNodes->resize(nodeCount);

	BoxTreeNode * nodeCursor = &m_flatNodes->front();

	m_root->assignIndices();
	m_root->recursePackInto(nodeCursor,nodeCursor);
}

// ----------------------------------------------------------------------
