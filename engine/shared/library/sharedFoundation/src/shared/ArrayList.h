//===================================================================
//
// ArrayList.h
// asommers 10-2-98
//
// copyright 1998, bootprint entertainment
// copyright 2001, sony online entertainment
//
// generic list template
//
// @todo replace instances of ArrayList with std::vector
//
//===================================================================

#ifndef INCLUDED_ArrayList_H
#define INCLUDED_ArrayList_H

//===================================================================

template <class T>
class ArrayList
{
public:

	// construction/destruction
	explicit ArrayList (int newSize=0);
	ArrayList (const ArrayList<T>& rhs);
	~ArrayList (void);

	// allow preallocation of list
	void     preallocate (int newSize, bool numberOfElementsToo=false);
	void     preallocateAndSet (int newSize, const T& newValue);

	// destroy the list if it was created statically
	void     destroy (void);

	// clear out the list
	void     clear (void);

	// add a new item to the list
	void     add (const T& newElement);
	void     addIfNotExist (const T& newElement);
	void     allocateNext (void);
	
	void     insert (int index, const T& newElement);

	// remove an item from the list
	void     remove (const T& element, bool mustExist=false);
	void     removeIndexAndCompactList (int index);

	// delete the last item from the list
	// NOTE: caller should know how to delete the element if necessary
	void     deleteLast (void);

	// returns true if element is already in the list
	bool     existsInList (const T& element) const;
	bool     existsInList (const T& element, int& index) const;

	// returns the number of elements in the list
	int      getNumberOfElements (void) const;

	// returns the index'th item in the list (DOES NOT check for out of bounds)
	T&       getElement (int index);
	const T& getElement (int index) const;
	T&       getLastElement (void);
	const T& getLastElement (void) const;

	// returns the index'th item in the list (DOES NOT check for out of bounds)
	T&       operator[] (int index);
	const T& operator[] (int index) const;

	// 
	ArrayList<T>& operator= (const ArrayList<T>& rhs);

	// swapping
	void     swap (int index1, int index2);

	// stl interface to make transition easier
	const T& back () const;
	T&       back ();
	int      size () const;
	bool     empty () const;
	void     push_back (const T& element);
	void     pop_back ();

private:

	enum
	{
		DEFAULT_SIZE = 16        // initial list m_size
	};
	
private:

	// resizes the list
	void resizeUp (void);

	void copy (const ArrayList<T>& rhs);

private:

	int  m_size;                   // list m_size
	T*   m_data;                   // list m_data
	int  m_numberOfElements;       // number of items in the list
};

//===================================================================

template <class T>
inline ArrayList<T>::ArrayList (int newSize) :
	m_size (newSize),
	m_data (0),
	m_numberOfElements (0)
{
	if (newSize)
		m_data        = new T [m_size];
}

//-------------------------------------------------------------------
	
template <class T>
inline ArrayList<T>::ArrayList (const ArrayList<T>& rhs) :
	m_size (0),
	m_data (0),
	m_numberOfElements (0)
{
	copy (rhs);
}

//-------------------------------------------------------------------
	
template <class T>
inline ArrayList<T>& ArrayList<T>::operator= (const ArrayList<T>& rhs)
{
	if (this != &rhs)
		copy (rhs);

	return *this;
}

//-------------------------------------------------------------------

template <class T>
inline void ArrayList<T>::copy (const ArrayList<T>& rhs)
{
	// only delete and re-new the list if we don't have enough elements to hold the contents of rhs
	if (m_numberOfElements < rhs.m_numberOfElements)
	{
		delete [] m_data;

		m_size = rhs.m_size;
		m_data = new T [m_size];
	}

	// only copy the vital elements
	memcpy (m_data, rhs.m_data, sizeof (T) * rhs.m_numberOfElements);

	m_numberOfElements = rhs.m_numberOfElements;
}

//-------------------------------------------------------------------

template <class T>
inline void ArrayList<T>::destroy (void)
{
	delete [] m_data;
	m_data             = 0;

	m_size             = 0;
	m_numberOfElements = 0;
}

//-------------------------------------------------------------------

template <class T>
inline ArrayList<T>::~ArrayList (void)
{
	destroy ();
}

//-------------------------------------------------------------------

template <class T>
inline void ArrayList<T>::preallocate (int newSize, bool numberOfElementsToo)
{
	// if the m_size we want to preallocate is greater than our current m_size, grow to fit
	if (newSize > m_size)
	{
		delete [] m_data;
		m_data = new T [newSize];
		m_size = newSize;
	}

	if (numberOfElementsToo)
		m_numberOfElements = newSize;
}

//-------------------------------------------------------------------

template <class T>
inline void ArrayList<T>::preallocateAndSet (int newSize, const T& newValue)
{
	preallocate (newSize, true);

	int i;
	for (i = 0; i < newSize; i++)
		m_data [i] = newValue;
}

//-------------------------------------------------------------------

template <class T>
inline void ArrayList<T>::clear (void)
{
	m_numberOfElements = 0;
}

//-------------------------------------------------------------------

template <class T>
inline void ArrayList<T>::add (const T& newElement)
{
	// is the list full? if so, resize it
	if (m_numberOfElements == m_size)
		resizeUp ();

	// add the m_data to the list
	NOT_NULL (m_data);
	m_data [m_numberOfElements++] = newElement;
}

//-------------------------------------------------------------------

template <class T>
inline void ArrayList<T>::addIfNotExist (const T& newElement)
{
	if (existsInList (newElement))
		return;

	add (newElement);
}

//-------------------------------------------------------------------

template <class T>
inline void ArrayList<T>::insert (int index, const T& newElement)
{
	NOT_NULL (m_data);

	//-- if inserting at end, just add it
	if (index == m_numberOfElements)
		add (newElement);
	else
	{
		//-- is the list full? if so, resize it
		if (m_numberOfElements == m_size)
			resizeUp ();

		//-- move everything from index to end of list down
		int i;
		for (i = m_numberOfElements - 1; i >= index; --i)
			m_data [i + 1] = m_data [i];

		//-- insert item into list
		m_data [index] = newElement;
		m_numberOfElements++;
	}
}

//-------------------------------------------------------------------

template <class T>
inline void ArrayList<T>::allocateNext (void)
{
	// is the list full? if so, resize it
	if (m_numberOfElements == m_size)
		resizeUp ();

	m_numberOfElements++;
}

//-------------------------------------------------------------------

template <class T>
inline void ArrayList<T>::remove (const T& element, bool mustExist)
{
	NOT_NULL (m_data);

	// search the list for the element
	int i;
	for (i = 0; i < m_numberOfElements; i++)
	{
		// if we found it
		if (m_data [i] == element)
		{
			// squish list
			int j;
			for (j = i; j < m_numberOfElements-1; j++)
				m_data [j] = m_data [j+1];

			m_numberOfElements--;

			return;
		}
	}

	UNREF (mustExist);
	DEBUG_FATAL (mustExist, ("ArrayList<T>::remove - not found in list"));
}

//-------------------------------------------------------------------

template <class T>
inline void ArrayList<T>::removeIndexAndCompactList (int index)
{
	NOT_NULL (m_data);
	DEBUG_FATAL (index < 0 || index >= m_numberOfElements, ("index %d out of range", index));

	// squish list
	int j;
	for (j = index; j < m_numberOfElements-1; j++)
		m_data [j] = m_data [j+1];

	m_numberOfElements--;
}

//-------------------------------------------------------------------

template <class T>
inline void ArrayList<T>::deleteLast (void)
{
	if (m_numberOfElements > 0)
		m_numberOfElements--;
}

//-------------------------------------------------------------------

template <class T>
inline bool ArrayList<T>::existsInList (const T& element) const
{
	if (m_numberOfElements > 0)
	{
		NOT_NULL (m_data);

		int i;
		for (i = 0; i < m_numberOfElements; i++)
			if (m_data [i] == element)
				return true;
	}

	return false;
}

//-------------------------------------------------------------------

template <class T>
inline bool ArrayList<T>::existsInList (const T& element, int& index) const
{
	if (m_numberOfElements > 0)
	{
		NOT_NULL (m_data);

		int i;
		for (i = 0; i < m_numberOfElements; i++)
			if (m_data [i] == element)
			{
				index = i;

				return true;
			}
	}

	return false;
}

//-------------------------------------------------------------------

template <class T>
inline void ArrayList<T>::resizeUp (void)
{
	// allocate new memory for the list (just double it)
	int newSize = m_size ? m_size * 2 : DEFAULT_SIZE;

	T* newData = new T [newSize];
	
	// copy the contents of the old list into the new list
	if (m_data)
		memcpy (newData, m_data, sizeof (T) * m_size);

	// increase the m_size
	m_size = newSize;

	// zap the old list
	delete [] m_data;

	// point to the new list
	m_data = newData;
}

//-------------------------------------------------------------------

template <class T>
inline int ArrayList<T>::getNumberOfElements (void) const
{
	return m_numberOfElements;
}

//-------------------------------------------------------------------

template <class T>
inline const T& ArrayList<T>::getElement (int index) const
{
	NOT_NULL (m_data);
	DEBUG_FATAL (index < 0 || index >= m_numberOfElements, ("index %d out of range", index));

	return m_data [index];
}

//-------------------------------------------------------------------

template <class T>
inline const T& ArrayList<T>::getLastElement (void) const
{
	NOT_NULL (m_data);
	DEBUG_FATAL (m_numberOfElements == 0, ("ArrayList<T>::getLastElement - list has no elements"));

	return m_data [m_numberOfElements - 1];
}

//-------------------------------------------------------------------

template <class T>
inline const T& ArrayList<T>::operator[] (int index) const
{
	NOT_NULL (m_data);
	DEBUG_FATAL (index < 0 || index >= m_numberOfElements, ("index %d out of range", index));

	return m_data [index];
}

//-------------------------------------------------------------------

template <class T>
inline T& ArrayList<T>::getElement (int index) 
{
	NOT_NULL (m_data);
	DEBUG_FATAL (index < 0 || index >= m_numberOfElements, ("index %d out of range", index));

	return m_data [index];
}

//-------------------------------------------------------------------

template <class T>
inline T& ArrayList<T>::getLastElement (void)
{
	NOT_NULL (m_data);
	DEBUG_FATAL (m_numberOfElements == 0, ("ArrayList<T>::getLastElement - list has no elements"));

	return m_data [m_numberOfElements - 1];
}

//-------------------------------------------------------------------

template <class T>
inline T& ArrayList<T>::operator[] (int index) 
{
	NOT_NULL (m_data);
	DEBUG_FATAL (index < 0 || index >= m_numberOfElements, ("index %d out of range", index));

	return m_data [index];
}

//-------------------------------------------------------------------

template <class T>
inline void ArrayList<T>::swap (int index1, int index2)
{
	NOT_NULL (m_data);
	DEBUG_FATAL (index1 < 0 || index1 >= m_numberOfElements || index2 < 0 || index2 >= m_numberOfElements, ("ArrayList<T>::swap - index1 %d or index2 %d out of range", index1, index2));

	T tmp         = m_data [index1];
	m_data [index1] = m_data [index2];
	m_data [index2] = tmp;
}

//-------------------------------------------------------------------

template <class T>
inline const T& ArrayList<T>::back () const
{
	return getLastElement ();
}

//-------------------------------------------------------------------

template <class T>
inline T& ArrayList<T>::back ()
{
	return getLastElement ();
}

//-------------------------------------------------------------------

template <class T>
inline int ArrayList<T>::size () const
{
	return getNumberOfElements ();
}

//-------------------------------------------------------------------

template <class T>
inline bool ArrayList<T>::empty () const
{
	return getNumberOfElements () == 0;
}

//-------------------------------------------------------------------

template <class T>
inline void ArrayList<T>::push_back (const T& element)
{
	add (element);
}

//-------------------------------------------------------------------

template <class T>
inline void ArrayList<T>::pop_back ()
{
	deleteLast ();
}

//===================================================================

#endif
