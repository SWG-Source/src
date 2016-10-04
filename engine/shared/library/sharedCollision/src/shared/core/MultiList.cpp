// ======================================================================
//
// MultiList.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedCollision/FirstSharedCollision.h"
#include "sharedCollision/MultiList.h"
#include "sharedCollision/BaseClass.h"

// ======================================================================

MultiListHandle::MultiListHandle( BaseClass * object, int color )
: m_color(color),
  m_object(object),
  m_head(nullptr),
  m_tail(nullptr)
{
}

MultiListHandle::~MultiListHandle()
{
	IGNORE_RETURN( detach() );

    m_object = nullptr;
    m_head = nullptr;
    m_tail = nullptr;
}

// ----------------------------------------------------------------------

MultiListNode * MultiListHandle::getHead ( void )
{
	return m_head;
}

MultiListNode const * MultiListHandle::getHead ( void ) const
{
	return m_head;
}

int MultiListHandle::getColor ( void ) const
{
	return m_color;
}

// ----------------------------------------------------------------------

MultiListNode *	MultiListHandle::detachHead ( void )
{
    if(m_head)
    {
        return m_head->detach();
    }
    else
    {
        return nullptr;
    }
}

// ----------

MultiListHandle * MultiListHandle::detach ( void )
{
	destroyNodes();

	return this;
}

// ----------

bool MultiListHandle::isConnected ( void ) const
{
	return m_head != nullptr;
}

// ----------

void MultiListHandle::clear ( void )
{
	IGNORE_RETURN( detach() );
}

// ----------

void MultiListHandle::erase ( MultiListNode * node )
{
	DEBUG_FATAL(node->m_list[m_color] != this, ("MultiListHandle::erase - Node is not part of this list"));

	IGNORE_RETURN( node->detach() );
	
	destroyNode(node);
} //lint !e1762 // function could be const

// ----------

bool MultiListHandle::isEmpty ( void ) const
{
	return m_head == nullptr;
}

// ----------

void MultiListHandle::insert( MultiListNode * node, MultiListNode * prev, MultiListNode * next )
{
	// verify that the nodes are all distinct
	DEBUG_FATAL( prev == node, ("MultiListHandle::insert - Prev and Node are the same"));
	DEBUG_FATAL( next == node, ("MultiListHandle::insert - Next and node are the same "));
	DEBUG_FATAL( (prev || next) && (prev == next), ("MultiListHandle::insert - Prev and Next are the same"));

	// verify that the node to insert is not nullptr
	DEBUG_FATAL( node == nullptr, ("MultiListHandle::insert - Cannot insert nullptr node"));

	// verify that the nodes we're inserting between are adjacent
	DEBUG_FATAL( prev && (prev->m_next[m_color] != next), ("MultiListHandle::insert - Prev and Next are not adjacent"));
	DEBUG_FATAL( next && (next->m_prev[m_color] != prev), ("MultiListHandle::insert - Prev and Next are not adjacent"));

	// and are a part of this list
	DEBUG_FATAL( prev && (prev->m_list[m_color] != this), ("MultiListHandle::insert - Prev is not part of this list"));
	DEBUG_FATAL( next && (next->m_list[m_color] != this), ("MultiListHandle::insert - Next is not part of this list"));

    NOT_NULL(node);

	// ----------

	node->detach(m_color);

	node->m_prev[m_color] = prev;
	node->m_next[m_color] = next;

	if(prev) prev->m_next[m_color] = node;
	if(next) next->m_prev[m_color] = node;

	node->m_list[m_color] = this;

	if(m_head == next) m_head = node;
	if(m_tail == prev) m_tail = node;
}

// ----------

void MultiListHandle::connectTo( MultiListHandle & handle, BaseClass * data )
{
	DEBUG_FATAL( m_color == handle.m_color, ("MultiListHandle::connectTo - Cannot connect two handles of the same color\n"));

	MultiListNode * newNode = createNode(data);

	insert(newNode);

	handle.insert(newNode);
}

// ----------

void MultiListHandle::insert( MultiListNode * node )
{
	insert( node, nullptr, m_head );
}

// ----------

MultiListNode * MultiListHandle::detach ( MultiListNode * node )
{
    if(node)
    {
	    MultiListNode * next = node->m_next[m_color];
	    MultiListNode * prev = node->m_prev[m_color];

	    if( next ) next->m_prev[m_color] = prev;
	    if( prev ) prev->m_next[m_color] = next;

	    if( m_head == node ) m_head = next;
	    if( m_tail == node ) m_tail = prev;

	    // ----------

	    node->m_next[m_color] = nullptr;
	    node->m_prev[m_color] = nullptr;
	    node->m_list[m_color] = nullptr;
    }

	return node;
}

// ----------

bool MultiListHandle::disconnect( MultiListHandle & handle )
{
	MultiListNode * deadNode = find(handle);

	if(deadNode)
	{
		IGNORE_RETURN( deadNode->detach() );

		destroyNode(deadNode);

		return true;
	}
	else
	{
		return false;
	}
}

// ----------

MultiListNode * MultiListHandle::find ( MultiListHandle & testHandle )
{
	MultiListNode * cursor = m_head;

	while(cursor)
	{
		MultiListHandle * handle = cursor->m_list[testHandle.m_color];

		if(handle == &testHandle)
		{
			return cursor;
		}

		cursor = cursor->m_next[m_color];
	}

	return nullptr;
} //lint !e1764 // testHandle could be made const ref

MultiListNode const * MultiListHandle::find ( MultiListHandle const & testHandle ) const
{
	MultiListNode const * cursor = m_head;

	while(cursor)
	{
		MultiListHandle const * handle = cursor->m_list[testHandle.m_color];

		if(handle == &testHandle)
		{
			return cursor;
		}

		cursor = cursor->m_next[m_color];
	}

	return nullptr;
}

// ----------

BaseClass * MultiListHandle::getObject ( void )
{
	return m_object;
}

BaseClass const * MultiListHandle::getObject ( void ) const
{
	return m_object;
}

// ----------------------------------------------------------------------

MultiListNode * MultiListHandle::createNode ( BaseClass * data ) const
{
	MultiListNode * newNode = new MultiListNode();

	newNode->setData(data);

	return newNode;
}

// ----------

void MultiListHandle::destroyNode( MultiListNode * node ) const
{
	delete node;
}

// ----------

void MultiListHandle::destroyNodes ( void )
{
	while(m_head)
	{
		MultiListNode * deadNode = m_head->detach();

		destroyNode(deadNode);
	}
}

// ======================================================================

MultiListNode::MultiListNode()
: m_data(nullptr)
{
	for(int i = 0; i < nColors; i++)
	{
		m_next[i] = nullptr;
		m_prev[i] = nullptr;
		m_list[i] = nullptr;
	}
}

MultiListNode::~MultiListNode()
{
	IGNORE_RETURN( detach() );

	BaseClass * data = m_data;
	m_data = nullptr;
	delete data;
}

// ----------

MultiListNode * MultiListNode::detach ( void )
{
	for(int i = 0; i < nColors; i++)
	{
		detach(i);
	}

	return this;
}

// ----------
	
void MultiListNode::detach ( int color )
{
	if(m_list[color])
		IGNORE_RETURN( m_list[color]->detach(this) );
}

// ----------

MultiListNode * MultiListNode::getNext ( int color )
{
	return m_next[color];
} //lint !e1762 // function could be const

MultiListNode const * MultiListNode::getNext ( int color ) const
{
	return m_next[color];
}

// ----------

BaseClass * MultiListNode::getObject ( int color )
{
	if(m_list[color])
		return m_list[color]->getObject();
	else
		return nullptr;
} //lint !e1762 // function could be const


BaseClass const * MultiListNode::getObject ( int color ) const
{
	if(m_list[color])
		return m_list[color]->getObject();
	else
		return nullptr;
}

// ----------

BaseClass * MultiListNode::getData ( void )
{
	return m_data;
}

BaseClass const * MultiListNode::getData ( void ) const
{
	return m_data;
}

void MultiListNode::setData( BaseClass * data )
{
	if(m_data == data) return;

	BaseClass * deadData = m_data;
	m_data = nullptr;
	delete deadData;
	m_data = data;
}

// ----------------------------------------------------------------------

void MultiListNode::swapNext ( int color )
{
	if(!m_next[color]) return;

	MultiListNode * A = this;
	MultiListNode * B = m_next[color];

	MultiListNode * prev = A->m_prev[color];
	MultiListNode * next = B->m_next[color];

	B->m_prev[color] = prev;
	B->m_next[color] = A;

	A->m_prev[color] = B;
	A->m_next[color] = next;
}

// ======================================================================

/*
MultiList::MultiList()
: m_free(0,nullptr)
{
}

// ----------

bool MultiList::connect ( MultiListHandle * red, MultiListHandle * blue )
{
	return connect(red,blue,nullptr);
}

// ----------

bool MultiList::connect ( MultiListHandle & red, MultiListHandle & blue, BaseClass * data )
{
	DEBUG_FATAL( red.m_color == blue.m_color, ("MultiList::connect - Cannot connect two handles of the same color"));

	// ----------

	if(connected(red,blue)) return false;

	insert(red);
	insert(blue);

	red.connectTo(blue);

	return true;
}

// ----------

bool MultiList::connected( MultiListHandle & red, MultiListHandle & blue )
{
	return red.find(blue) != nullptr;
}

// ----------

bool MultiList::disconnect( MultiListHandle & red, MultiListHandle & blue )
{
	return red.disconnect(blue);
}

// ----------

MultiListNode * MultiList::getFreeNode ( void )
{
	if(m_free.m_head == nullptr)
	{
		return new MultiListNode();
	}
	else
	{
		return m_free.detachHead();
	}
}

// ----------

void MultiList::addFreeNode( MultiListNode * freeNode )
{
	freeNode->detach();

	m_free.insert(freeNode);		
}

// ----------

void MultiList::insert( MultiListNode * node, MultiListNode * prev, MultiListNode * next, int color )
{
	DEBUG_FATAL( prev == node, ("MultiList::insert - Prev and Node are the same"));
	DEBUG_FATAL( next == node, ("MultiList::insert - Next and Node are the same"));
	DEBUG_FATAL( prev && (prev->m_next[color] != next), ("MultiList::insert - Prev and Next are not adjacent"));
	DEBUG_FATAL( next && (next->m_prev[color] != prev), ("MultiList::insert - Prev and Next are not adjacent"));

	node->detach();

	if(prev) 
	{
		prev->m_next[color] = node;
		node->m_prev[color] = prev;
	}

	if(next) 
	{
		next->m_prev[color] = node;
		node->m_next[color] = next;
	}
}

// ----------

void MultiList::insert( MultiListHandle * node, MultiListHandle * prev, MultiListHandle * next )
{
	// verify that the nodes are all distinct
	DEBUG_FATAL( prev == node, ("MultiList::insert - Prev and Node are the same"));
	DEBUG_FATAL( next == node, ("MultiList::insert - Next and Node are the same"));
	DEBUG_FATAL( prev == next, ("MultiList::insert - Prev and Next are the same"));

	// verify that the node to insert is not nullptr
	DEBUG_FATAL( node == nullptr, ("MultiList::insert - Cannot insert a nullptr node"));

	// verify that the two nodes are adjacent
	DEBUG_FATAL( prev && (prev->m_next != next), ("MultiList::insert - Prev and Next are not adjacent"));
	DEBUG_FATAL( next && (next->m_prev != prev), ("MultiList::insert - Prev and Next are not adjacent"));

	// and are a part of this list
	DEBUG_FATAL( prev && (prev->m_list != this), ("MultiList::insert - Prev is not part of this list"));
	DEBUG_FATAL( next && (next->m_list != this), ("MultiList::insert - Next is not part of this list"));

	// and are the same color as the node
	DEBUG_FATAL( prev && (prev->m_color != node->m_color), ("MultiList::insert - Prev and Node are different colors"));
	DEBUG_FATAL( next && (next->m_color != node->m_color), ("MultiList::insert - Next and Node are different colors"));

	// ----------

	node->detach();

	node->m_prev = prev;
	node->m_next = next;

	if(prev) prev->m_next = node;
	if(next) next->m_prev = node;

	node->m_list = this;

	int color = node->m_color;

	if(m_head[color] == next) m_head[color] = node;
	if(m_tail[color] == prev) m_tail[color] = node;
}

// ----------

MultiListHandle * MultiList::detach( MultiListHandle * node )
{
	DEBUG_FATAL( node == nullptr, ("MultiList::detach - Cannot detach nullptr node"));
	DEBUG_FATAL( node->m_list != this, ("MultiList::detach - Node is not part of this list"));

	// ----------

	int color = node->m_color;

	MultiListHandle * next = node->m_next;
	MultiListHandle * prev = node->m_prev;

	if(next) next->m_prev = prev;
	if(prev) prev->m_next = next;

	if(m_head[color] == node) m_head[color] = node->m_next;
	if(m_tail[color] == node) m_tail[color] = node->m_prev;

	node->m_prev = nullptr;
	node->m_next = nullptr;
	node->m_list = nullptr;

	return node;
}

// ----------

void MultiList::insert( MultiListHandle * node )
{
	DEBUG_FATAL(node == nullptr, ("MultiList::insert - Cannot insert nullptr node"));

	// ----------

	int color = node->m_color;

	insert(node,nullptr,m_head[color]);
}
*/

// ======================================================================
