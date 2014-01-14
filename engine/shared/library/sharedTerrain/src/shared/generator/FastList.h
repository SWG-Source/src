//===================================================================
//
// FastList.h
// asommers 
//
// copyright 2001, sony online entertainment
//
//--
//
// FastList is an unsorted automatic memory based list completely avoiding memory allocation
//
//===================================================================

#ifndef INCLUDED_FastList_H
#define INCLUDED_FastList_H

//===================================================================

template<class T, int n>
class FastList
{
public:

	FastList ();
	~FastList ();

	int       getNumberOfElements ();
	const T&  operator[] (int index) const;
	T&        operator[] (int index);

	bool      exists (const T& element) const;

	void      add (const T& element);
	void      addIfNotExists (const T& element);

private:

	//-- disable heap'ed fastlist
	void* operator new (size_t size);

	FastList (const FastList<T, n>& rhs);
	FastList& operator= (const FastList<T, n>& rhs);

private:

	int numberOfElements;
	T   data [n];
};

//===================================================================

template<class T, int n>
inline FastList<T, n>::FastList () :
	numberOfElements (0)
{
}

//-------------------------------------------------------------------

template<class T, int n>
inline FastList<T, n>::~FastList ()
{
}

//-------------------------------------------------------------------

template<class T, int n>
inline int FastList<T, n>::getNumberOfElements ()
{
	return numberOfElements;
}

//-------------------------------------------------------------------

template<class T, int n>
inline const T& FastList<T, n>::operator[] (int index) const
{
	DEBUG_FATAL (index < 0 || index >= numberOfElements, ("index out of range"));
	return data [index];
}

//-------------------------------------------------------------------

template<class T, int n>
inline T& FastList<T, n>::operator[] (int index)
{
	DEBUG_FATAL (index < 0 || index >= numberOfElements, ("index out of range"));
	return data [index];
}

//-------------------------------------------------------------------

template<class T, int n>
inline bool FastList<T, n>::exists (const T& element) const
{
	int i;
	for (i = 0; i < numberOfElements; ++i)
		if (data [i] == element)
			return true;

	return false;
}

//-------------------------------------------------------------------

template<class T, int n>
inline void FastList<T, n>::add (const T& element)
{
	DEBUG_FATAL (numberOfElements == n, ("can't add to a full list"));

	data [numberOfElements++] = element;
}

//-------------------------------------------------------------------

template<class T, int n>
inline void FastList<T, n>::addIfNotExists (const T& element)
{
	if (!exists (element))
		add (element);
}

//===================================================================

#endif
