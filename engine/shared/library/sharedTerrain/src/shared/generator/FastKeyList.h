//===================================================================
//
// FastKeyList.h
// asommers 
//
// copyright 2001, sony online entertainment
//
//--
//
// FastKeyList is a sorted automatic memory based list completely avoiding memory allocation
// T must have a .key parameter which is of type KEY

//===================================================================

#ifndef INCLUDED_FastKeyList_H
#define INCLUDED_FastKeyList_H

//===================================================================

template<class T, class KEY, int MAXSIZE>
class FastKeyList
{
public:

	FastKeyList ();
	~FastKeyList ();

	int       getNumberOfElements ();
	const T&  operator[] (int index) const;

	void      insertIfNotExists (const T& element);

private:

	void      add (const T& element);

private:

	//-- disable heap'ed FastKeyList
	void* operator new (size_t size);

	FastKeyList (const FastKeyList<T, KEY, MAXSIZE>& rhs);
	FastKeyList& operator= (const FastKeyList<T, KEY, MAXSIZE>& rhs);

private:

	int numberOfElements;
	T   data [MAXSIZE];
};

//===================================================================

template<class T, class KEY, int MAXSIZE>
inline FastKeyList<T, KEY, MAXSIZE>::FastKeyList () :
	numberOfElements (0)
{
}

//-------------------------------------------------------------------

template<class T, class KEY, int MAXSIZE>
inline FastKeyList<T, KEY, MAXSIZE>::~FastKeyList ()
{
}

//-------------------------------------------------------------------

template<class T, class KEY, int MAXSIZE>
inline int FastKeyList<T, KEY, MAXSIZE>::getNumberOfElements ()
{
	return numberOfElements;
}

//-------------------------------------------------------------------

template<class T, class KEY, int MAXSIZE>
inline const T& FastKeyList<T, KEY, MAXSIZE>::operator[] (int index) const
{
	DEBUG_FATAL (index < 0 || index >= numberOfElements, ("index out of range"));
	return data [index];
}

//-------------------------------------------------------------------

template<class T, class KEY, int MAXSIZE>
inline void FastKeyList<T, KEY, MAXSIZE>::add (const T& element)
{
	DEBUG_FATAL (numberOfElements == MAXSIZE, ("can't add to a full list"));

	data [numberOfElements++] = element;
}

//-------------------------------------------------------------------

template<class T, class KEY, int MAXSIZE>
inline void FastKeyList<T, KEY, MAXSIZE>::insertIfNotExists(const T& element)
{
	//-- search for where to insert
	int i = 0;
	for (; i < numberOfElements; ++i)
	{
		if (data[i].key == element.key)
		{
			return;
		}
		else if (data[i].key > element.key)
		{
			break;
		}
	}

	//-- is element to add at end?
	if (i == numberOfElements)
	{
		add (element);
	}
	else
	{
		DEBUG_FATAL (numberOfElements == MAXSIZE, ("can't add to a full list"));

		//-- make room
		int j;
		for (j = numberOfElements; j >= i; --j)
			data [j] = data [j - 1];

		data [i] = element;
		numberOfElements++;
	}
}

//===================================================================

#endif
