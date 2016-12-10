// ======================================================================
//
// Watcher.h
// copyright 1998    Bootprint Entertainment
// Copyright 2000-01 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// @todo How do I get this into doxygen?  It's information that really spans multiple classes
//
// The Watcher system allows pointers to objects to be automatically
// reset to nullptr when the object watching them is destoyed.
// 
// For something <T> to be watchable, it must provide a routine of the form:
//
//   	WatchedByList &T::getWatchedByList() const;
//
// This allows things to be watchable without having to derive from a common
// base class.  The WatchedByList returned by the object must have exactly
// the same lifespan as the object itself.  The easiest way to accomplish
// that is to make the WatchedByList a member of the object.  Since it is
// possible to watch const objects, it's very likely that the member variable
// will need to be made mutable.
//
// ======================================================================

#ifndef INCLUDED_Watcher_H
#define INCLUDED_Watcher_H

// ======================================================================

class WatchedByList;

// ======================================================================

// Base class for Watcher and ConstWatcher.  This should never be used 
// directly by the application.  
class BaseWatcher
{
	friend class WatchedByList;

protected:

	explicit BaseWatcher(void *newValue);
	virtual ~BaseWatcher();

	void reset();
	void addTo(WatchedByList &watchedByList);
	void removeFrom(WatchedByList &watchedByList);

private:

	/// Disabled
	BaseWatcher();

	/// Disabled
	BaseWatcher(const BaseWatcher &);

protected:

	void *m_data;
};

// Watcher to a non-const object.
template <typename T>
class Watcher : public BaseWatcher
{
public:

	explicit Watcher(T *data=nullptr);
	Watcher(const Watcher &newValue);
	~Watcher();  //lint !e1509 // Warning -- base class destructor for class is not virtual

	Watcher<T> & operator = (T *newValue);
	Watcher<T> & operator = (const Watcher<T> &);

	// Equality and less operators for watchers. Without these operators,
	// std::find will coerce the pointer type to a watcher and the resulting constructor
	// call will add an element to the watched object's watchedBy list, potentially
	// causing a reallocation. Then, when the find is done, the temporary object is
	// destroyed and the watcher is removed from the watchedBy list.

	bool operator == ( T const * const rhs ) const;
	bool operator <  ( T const * const rhs ) const;

	bool operator == ( Watcher<T> const & rhs ) const;
	bool operator <  ( Watcher<T> const & rhs ) const;
	
	T *getPointer() const;
	operator T*() const;
	T *operator ->() const;
};

// Watcher to a const object.
template <typename T>
class ConstWatcher : public BaseWatcher
{
public:

	explicit ConstWatcher(const T *data=nullptr);
	ConstWatcher(const ConstWatcher &newValue);
	~ConstWatcher();  //lint !e1509 // Warning -- base class destructor for class is not virtual

	ConstWatcher<T> &operator =(const T *newValue);
	ConstWatcher<T> &operator =(const ConstWatcher<T> &);
	
	// Equality and less operators for watchers. Without these operators,
	// std::find will coerce the pointer type to a watcher and the resulting constructor
	// call will add an element to the watched object's watchedBy list, potentially
	// causing a reallocation. Then, when the find is done, the temporary object is
	// destroyed and the watcher is removed from the watchedBy list.

	bool operator == ( T const * const rhs ) const;
	bool operator <  ( T const * const rhs ) const;

	bool operator == ( ConstWatcher<T> const & rhs ) const;
	bool operator <  ( ConstWatcher<T> const & rhs ) const;
	
	const T *getPointer() const;
	operator const T*() const;
	const T *operator ->() const;
};

// Keeps track of whom is watching it.
class WatchedByList
{
	friend class BaseWatcher;

public:

	static void install();

public:

	WatchedByList();
	~WatchedByList();

	void nullWatchers();

protected:

	void add(BaseWatcher &baseWatcher);
	void remove(BaseWatcher &baseWatcher);

private:

	/// Disabled
	WatchedByList(const WatchedByList &);

	/// Disabled
	WatchedByList &operator &(const WatchedByList &);

private:

	typedef std::vector<BaseWatcher *> List;

private:

	List *m_list;
};

// ======================================================================
/**
 * Construct a BaseWatcher
 */

inline BaseWatcher::BaseWatcher(void *data)
: m_data(data)
{
}

// ----------------------------------------------------------------------
/**
 * Reset a BaseWatcher.
 *
 * This routine is only called when the object being pointed to is being destroyed.
 */

inline void BaseWatcher::reset()
{
	m_data = nullptr;
}

// ----------------------------------------------------------------------
/**
 * Destroy a BaseWatcher
 *
 * This routine will walk the singly linked list of watchers and remove
 * this watcher from the list.
 */

inline BaseWatcher::~BaseWatcher()
{
	m_data = nullptr;
}

// ----------------------------------------------------------------------
/**
 * Add this BaseWatcher to the WatchedByList.
 * 
 * This routine exists so we don't need make Watcher<T> a friend of WatchedByList.
 * @internal
 */

inline void BaseWatcher::addTo(WatchedByList &watchedByList)
{
	watchedByList.add(*this);
}

// ----------------------------------------------------------------------
/**
 * Remove this BaseWatcher from the WatchedByList.
 * 
 * This routine exists so we don't need make Watcher<T> a friend of WatchedByList.
 * @internal
 */

inline void BaseWatcher::removeFrom(WatchedByList &watchedByList)
{
	watchedByList.remove(*this);
}

// ======================================================================
/**
 * Convert the data pointer to the object pointer type.
 * 
 * @return Pointer to the object the Watcher points to
 */

template <typename T>
inline T *Watcher<T>::getPointer() const
{
	return reinterpret_cast<T *>(m_data);
}

// ----------------------------------------------------------------------
/**
 * Construct a Watcher.
 * 
 * @param data Initial value for the data pointer.
 */

template <typename T>
inline Watcher<T>::Watcher(T *data)
: BaseWatcher(data)
{
	if (m_data)
		addTo(getPointer()->getWatchedByList());
}

// ----------------------------------------------------------------------
/**
 * Copy-construct a Watcher.
 * 
 * @param rhs Watcher to copy the data pointer from.
 */

template <typename T>
inline Watcher<T>::Watcher(const Watcher &rhs)
: BaseWatcher(rhs.m_data)
{
	if (m_data)
		addTo(getPointer()->getWatchedByList());
}

// ----------------------------------------------------------------------
/**
 * Destroy a Watcher.
 */

template <typename T>
inline Watcher<T>::~Watcher()
{
	if (m_data)
		removeFrom(getPointer()->getWatchedByList());
}

// ----------------------------------------------------------------------
/**
 * Set the value of Watcher from a pointer.
 * 
 * @param newValue  Pointer to copy the data from.
 * @return The watcher itself.
 */

template <typename T>
inline Watcher<T> & Watcher<T>::operator = (T *data)
{
	if (data != m_data)
	{
		if (m_data)
			removeFrom(getPointer()->getWatchedByList());
			
		m_data = data;

		if (m_data)
			addTo(getPointer()->getWatchedByList());
	}

	return *this;
}

// ----------------------------------------------------------------------
/**
 * Assign one watcher to another, effectively making the assignee watch
 * the same object as the assigner.
 * 
 * @return The object itself
 */

template<typename T>
inline Watcher<T> & Watcher<T>::operator = (const Watcher<T> & rhs)
{
	return operator = (rhs.getPointer());
}

// ----------------------------------------------------------------------
/**
 * Equality operator for watchers, which allows us to compare a watcher
 * and a pointer without coercing the pointer to a watcher also. 
 *
 * A watcher is considered to be equal to a pointer P if it's watching 
 * the object pointed to by P.
 * 
 * @return True if the watcher is watching the object pointed to by rhs.
 */

template<typename T>
inline bool Watcher<T>::operator == ( T const * const rhs ) const
{
	return getPointer() == rhs;
}

// ----------------------------------------------------------------------
/**
 * Less operator for watchers, which allows us to compare a watcher
 * and a pointer without coercing the pointer to a watcher also.
 *
 * Asking if a watcher is less than a pointer doesn't make much sense,
 * but it's required by the STL associative containers.
 * 
 * @return True if the watcher's pointer is numerically less than rhs
 */

template<typename T>
inline bool Watcher<T>::operator <  ( T const * const rhs ) const
{
	return getPointer() < rhs;
}

// ----------------------------------------------------------------------
/**
 * Equality operator for watchers. 
 *
 * Two watchers are considered to be equal if they watch the same object.
 * 
 * @return True if the two watchers are watching the same object
 */

template<typename T>
inline bool Watcher<T>::operator == ( Watcher<T> const & rhs ) const
{
	return getPointer() == rhs.getPointer();
}

// ----------------------------------------------------------------------
/**
 * Less operator for watchers.
 *
 * Asking if a watcher is less than another watcher doesn't make much 
 * sense, but it's required by the STL associative containers.
 * 
 * @return True if the watcher's pointer is numerically less than rhs
 */

template<typename T>
inline bool Watcher<T>::operator < ( Watcher<T> const & rhs ) const
{
	return getPointer() < rhs.getPointer();
}

// ----------------------------------------------------------------------
/**
 * Cast the Watcher to the object pointer type.
 * 
 * @return Pointer to the object the Watcher points to.
 */

template <typename T>
inline Watcher<T>::operator T *() const
{
	return reinterpret_cast<T *>(m_data);
}

// ----------------------------------------------------------------------
/**
 * Dereference the Watcher to the object.
 * 
 * @return Pointer to the object the watcher points to.
 */

template <typename T>
inline T *Watcher<T>::operator ->() const
{
	return reinterpret_cast<T *>(m_data);
}

// ======================================================================
/**
 * Convert the data pointer to the object pointer type.
 * 
 * @return Pointer to the object the ConstWatcher points to
 */

template <typename T>
inline const T *ConstWatcher<T>::getPointer() const
{
	return reinterpret_cast<const T *>(m_data);
}

// ----------------------------------------------------------------------
/**
 * Construct a Watcher.
 * 
 * @param data Initial value for the data pointer.
 */

template <typename T>
inline ConstWatcher<T>::ConstWatcher(const T *data)
: BaseWatcher(const_cast<void *>(reinterpret_cast<const void *>(data)))
{
	if (m_data)
		addTo(getPointer()->getWatchedByList());
}

// ----------------------------------------------------------------------
/**
 * Copy-construct a ConstWatcher.
 * 
 * @param rhs ConstWatcher to copy the data pointer from.
 */

template <typename T>
inline ConstWatcher<T>::ConstWatcher(const ConstWatcher &rhs)
: BaseWatcher(rhs.m_data)
{
	if (m_data)
		addTo(getPointer()->getWatchedByList());
}

// ----------------------------------------------------------------------
/**
 * Destroy a ConstWatcher.
 */

template <typename T>
inline ConstWatcher<T>::~ConstWatcher()
{
	if (m_data)
		removeFrom(getPointer()->getWatchedByList());
}

// ----------------------------------------------------------------------
/**
 * Set the value of ConstWatcher from a pointer.
 * 
 * @param newValue  Pointer to copy the data from.
 * @return The ConstWatcher itself.
 */

template <typename T>
inline ConstWatcher<T> &ConstWatcher<T>::operator =(const T *data)
{
	if (m_data != static_cast<const void *>(data))
	{
		if (m_data)
			removeFrom(getPointer()->getWatchedByList());
			
		m_data = const_cast<void *>(reinterpret_cast<const void *>(data));

		if (m_data)
			addTo(getPointer()->getWatchedByList());
	}

	return *this;
}

// ----------------------------------------------------------------------
/**
 * Assign one ConstWatcher to another, effectively making the assignee watch
 * the same object as the assigner.
 * 
 * @return The object itself
 */

template<typename T>
inline ConstWatcher<T> & ConstWatcher<T>::operator = (const ConstWatcher<T> & rhs)
{
	return operator = (rhs.getPointer());
}

// ----------------------------------------------------------------------
/**
 * Equality operator for watchers, which allows us to compare a watcher
 * and a pointer without coercing the pointer to a watcher also. 
 *
 * A watcher is considered to be equal to a pointer P if it's watching 
 * the object pointed to by P.
 * 
 * @return True if the watcher is watching the object pointed to by rhs.
 */

template<typename T>
inline bool ConstWatcher<T>::operator == ( T const * const rhs ) const
{
	return getPointer() == rhs;
}

// ----------------------------------------------------------------------
/**
 * Less operator for watchers, which allows us to compare a watcher
 * and a pointer without coercing the pointer to a watcher also.
 *
 * Asking if a watcher is less than a pointer doesn't make much sense,
 * but it's required by the STL associative containers.
 * 
 * @return True if the watcher's pointer is numerically less than rhs
 */

template<typename T>
inline bool ConstWatcher<T>::operator <  ( T const * const rhs ) const
{
	return getPointer() < rhs;
}

// ----------------------------------------------------------------------
/**
 * Equality operator for watchers. 
 *
 * Two watchers are considered to be equal if they watch the same object.
 * 
 * @return True if the two watchers are watching the same object
 */

template<typename T>
inline bool ConstWatcher<T>::operator == ( ConstWatcher<T> const & rhs ) const
{
	return getPointer() == rhs.getPointer();
}

// ----------------------------------------------------------------------
/**
 * Less operator for watchers.
 *
 * Asking if a watcher is less than another watcher doesn't make much 
 * sense, but it's required by the STL associative containers.
 * 
 * @return True if the watcher's pointer is numerically less than rhs
 */

template<typename T>
inline bool ConstWatcher<T>::operator < ( ConstWatcher<T> const & rhs ) const
{
	return getPointer() < rhs.getPointer();
}

// ----------------------------------------------------------------------
/**
 * Cast the ConstWatcher to the object pointer type.
 * 
 * @return Pointer to the object the ConstWatcher points to.
 */

template <typename T>
inline ConstWatcher<T>::operator const T *() const
{
	return reinterpret_cast<const T *>(m_data);
}

// ----------------------------------------------------------------------
/**
 * Dereference the ConstWatcher to the object.
 * 
 * @return Pointer to the object the ConstWatcher points to.
 */

template <typename T>
inline const T *ConstWatcher<T>::operator ->() const
{
	return reinterpret_cast<const T *>(m_data);
}

// ----------------------------------------------------------------------
/**
 * Construct a WatchedByList.
 *
 * This list of watchers remains nullptr until someone first watches the object.
 */

inline WatchedByList::WatchedByList()
: m_list(nullptr)
{
}

// ======================================================================

#endif
