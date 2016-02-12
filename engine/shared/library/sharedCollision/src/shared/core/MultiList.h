// ======================================================================
//
// MultiList.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_MultiList_H
#define INCLUDED_MultiList_H

class MultiListNode;
class MultiListHandle;
class BaseClass;

// ======================================================================

class MultiListHandle
{
public:

    MultiListHandle( BaseClass * object, int color );
    virtual ~MultiListHandle();

    // ----------

    MultiListNode *         getHead         ( void );
    MultiListNode const *   getHead         ( void ) const;
    int                     getColor        ( void ) const;

    BaseClass *             getObject       ( void );
    BaseClass const *       getObject       ( void ) const;

    // ----------

    void                    connectTo       ( MultiListHandle & handle, BaseClass * data );
    bool                    disconnect      ( MultiListHandle & handle );
    bool                    isConnected     ( void ) const;

    // ----------

    void                    erase           ( MultiListNode * node );
    void                    clear           ( void );
    
    // ----------
    
    bool                    isEmpty         ( void ) const;

    MultiListNode *         find            ( MultiListHandle & testHandle );
    MultiListNode const *   find            ( MultiListHandle const & testHandle ) const;

protected:

    // ----------

    MultiListNode *         detachHead      ( void );
    MultiListHandle *       detach          ( void );

    // ----------

    MultiListNode *         detach          ( MultiListNode * node );

    void                    insert          ( MultiListNode * node );

    void                    insert          ( MultiListNode * node, 
                                              MultiListNode * prev, 
                                              MultiListNode * next );

    // ----------

    MultiListNode *         createNode      ( BaseClass * data ) const;
    void                    destroyNode     ( MultiListNode * node ) const;
    void                    destroyNodes    ( void );

    // ----------

private:

    friend class MultiListNode;

    // ----------

    int                 m_color;
    BaseClass *         m_object;

    MultiListNode *     m_head;
    MultiListNode *     m_tail;

    MultiListHandle( MultiListHandle const & copy );
    MultiListHandle & operator = ( MultiListHandle const & copy );
};

// ----------------------------------------------------------------------
// Multi-dimensional list node with backpointers

class MultiListNode
{
public:

    MultiListNode();
    virtual ~MultiListNode();

    // ----------

    MultiListNode *         getNext         ( int color );
    MultiListNode const *   getNext         ( int color ) const;

    BaseClass *             getObject       ( int color );
    BaseClass const *       getObject       ( int color ) const;

    BaseClass *             getData         ( void );
    BaseClass const *       getData         ( void ) const;

	void                    swapNext        ( int color );

protected:

    MultiListNode *         detach          ( void );
    void                    detach          ( int color );

    void                    insert          ( MultiListNode * node, 
                                              MultiListNode * prev, 
                                              MultiListNode * next, 
                                              int color );

    void                    clear           ( void );

    void                    setData         ( BaseClass * data );

    // ----------

    friend class MultiListHandle;

    enum
    {
        nColors = 2
    };

    MultiListNode *     m_next[nColors];
    MultiListNode *     m_prev[nColors];
    MultiListHandle *   m_list[nColors];

    BaseClass *         m_data;

    // ----------

private:

    MultiListNode( MultiListNode const & copy );
    MultiListNode & operator = ( MultiListNode const & copy );
};

// ----------------------------------------------------------------------

template<class T>
class MultiListConstIterator
{
public:

    MultiListConstIterator( MultiListHandle const & handle )
    {
        m_node = handle.getHead();
        m_iterateColor = handle.getColor();
        m_valueColor = (m_iterateColor == 0) ? 1 : 0;
    }

    T const * operator * ( void )
    {
        if(m_node)
            return safe_cast<T const *>(m_node->getObject(m_valueColor));
        else
            return nullptr;
    }

    operator bool ( void ) const
    {
        return m_node != nullptr;
    }

    void operator ++ ( void )
    {
        if(m_node) 
            m_node = m_node->getNext(m_iterateColor);
    }

private:

    MultiListNode const *   m_node;
    int                     m_iterateColor;
    int                     m_valueColor;

    MultiListConstIterator();
    MultiListConstIterator( MultiListConstIterator const & copy );
    MultiListConstIterator & operator = ( MultiListConstIterator const & copy );
};

// ----------------------------------------------------------------------

template<class T>
class MultiListIterator
{
public:

    MultiListIterator( MultiListHandle & handle )
    {
        m_node = handle.getHead();
        m_iterateColor = handle.getColor();
        m_valueColor = (m_iterateColor == 0) ? 1 : 0;
    }

    T * operator * ( void )
    {
        if(m_node)
            return safe_cast<T *>(m_node->getObject(m_valueColor));
        else
            return nullptr;
    }

    operator bool ( void ) const
    {
        return m_node != nullptr;
    }

    void operator ++ ( void )
    {
        if(m_node) 
            m_node = m_node->getNext(m_iterateColor);
    }

private:

    MultiListNode *     m_node;
    int                 m_iterateColor;
    int                 m_valueColor;

    MultiListIterator();
    MultiListIterator( MultiListIterator const & copy );
    MultiListIterator & operator = ( MultiListIterator const & copy );
};

// ----------------------------------------------------------------------

template<class T>
class MultiListConstDataIterator
{
public:

	MultiListConstDataIterator( void )
	{
		m_node = nullptr;
		m_color = -1;
	}

    MultiListConstDataIterator( MultiListHandle const & handle )
    {
        m_node = handle.getHead();
        m_color = handle.getColor();
    }

	MultiListConstDataIterator & operator = ( MultiListHandle const & handle )
	{
		m_node = handle.getHead();
		m_color = handle.getColor();

		return *this;
	}

	MultiListConstDataIterator ( MultiListConstDataIterator const & it )
	{
		m_node = it.m_node;
		m_color = it.m_color;
	}

	MultiListConstDataIterator & operator = ( MultiListConstDataIterator const & it )
	{
		m_node = it.m_node;
		m_color = it.m_color;
	}

    T const * operator * ( void ) const
    {
        if(m_node)
            return safe_cast<T const *>(m_node->getData());
        else
            return nullptr;
    }

    operator bool ( void ) const
    {
        return m_node != nullptr;
    }

    void operator ++ ( void )
    {
        if(m_node) 
            m_node = m_node->getNext(m_color);
    }

private:

    MultiListNode const *   m_node;
    int                     m_color;
};

// ----------------------------------------------------------------------

template<class T>
class MultiListDataIterator
{
public:

	MultiListDataIterator( void )
	{
		m_node = nullptr;
		m_color = -1;
	}

    MultiListDataIterator( MultiListHandle & handle )
    {
        m_node = handle.getHead();
        m_color = handle.getColor();
    }

	MultiListDataIterator & operator = ( MultiListHandle & handle )
	{
		m_node = handle.getHead();
		m_color = handle.getColor();

		return *this;
	}

	MultiListDataIterator ( MultiListDataIterator const & it )
	{
		m_node = it.m_node;
		m_color = it.m_color;
	}

	MultiListDataIterator & operator = ( MultiListDataIterator const & it )
	{
		m_node = it.m_node;
		m_color = it.m_color;
	}

    T * operator * ( void ) const
    {
        if(m_node)
            return safe_cast<T *>(m_node->getData());
        else
            return nullptr;
    }

    operator bool ( void ) const
    {
        return m_node != nullptr;
    }

    void operator ++ ( void )
    {
        if(m_node) 
            m_node = m_node->getNext(m_color);
    }

    MultiListNode * getNode ( void )
    {
        return m_node;
    }

private:

    MultiListNode *     m_node;
    int                 m_color;
};

// ======================================================================

#endif // #ifndef INCLUDED_MultiList_H

