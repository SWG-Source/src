#ifndef	_Singleton2_H
#define	_Singleton2_H

//TODO:  This is a temporary one-off from Singleton to experiment
// with ways of supporting derived classes.  If we like it, we
// should replace Singleton with it.  Otherwise, we should delete
// it.

//---------------------------------------------------------------------
#pragma warning (disable : 4514) // unreferenced inline function removed
#include <assert.h>

//---------------------------------------------------------------------
/**
	Singleton                                         Object Creational

  @brief Intent
	Ensure a class has only one instance, and provide a global point of
	access to it.

  Applicability
	Use the singleton when:
	- there must be exactly one instance of a class, and it must be 
	  accessibleto clients from a well-known access point
	- when the sole instance should be extensible by subclassing, and
	  and clients should be able to use an extended instance without 
	  modifying their code


  Design Patterns, pg. 127, Addison-Wessley Professional Computing 
	Series, Erich Gamma, Richard Helm, Ralph Johnson and John Vlissides

  The Singleton template ensures that classes inheriting it's 
  interface are instantiated only once. This implementation differs
  from the Singleton in Design patterns in two ways:

  It is not instantiated as a static somewhere in userland -- rather 
  the Singleton client must invoke Singleton<ValueType>::install()
  and a corresponding remove(). The Singleton will install itself
  on the first call to getInstance() if it is
  not installed()'d. 

  Example:
	\verbatim
	class MySingleton : public Singleton<MySingleton>
	{
	public:
		MySingleton() : i(0)
		{
		};

		~MySingleton()
		{
		};

		void    foo()
		{
			i ++;
		};

	private:
		int     i;
	};

	void someMainLoop()
	{
		MySingleton::install();

		while(stillRunning)
		{
			// do stuff
			MySingleton::getInstance()->foo();
		}
		
		MySingleton::remove();
	}
	\endverbatim

	@author	Justin Randall
---------------------------------------------------------------------*/

template<class ValueType>
class Singleton2
{
public:
	static ValueType &	getInstance();
	
	// install takes a parameter and has it's implementation inside
	// the class declaration because that's the only way to make it work
	// in WIN32
	template<class LeafType>
		static void	install(LeafType*)
		{
			assert(!instance);
			instance = new LeafType;
		}

	static void			remove();

protected:
	Singleton2();
	virtual ~Singleton2() = 0;

private:
	static ValueType *	instance;
};

//---------------------------------------------------------------------
/**
	@brief Instantiate the one and only instance of the ValueType singleton.


	Check that it is being invoked via the Singleton::install() method,
	and that no instance is already present!
*/
template<class ValueType>
inline Singleton2<ValueType>::Singleton2()
{
	// The singleton should **ONLY** be instantiated via the install()
	// method in the template base class, NEVER EVER EVER by the
	// client! Additionally, install() should only be invoked ONCE.
	//
	// Singleton<ValueType>::install()
	//
	assert(instance == 0);
}

//---------------------------------------------------------------------
/**
	@brief Remove the one and only instance of the ValueType singleton.

	Ensure that it is installed, and that there is already an
	instance. It's not necessarily critical to have an instance or
	to be installed at removal, but invoking remove() when the
	singleton has not been installed may be indicative of poor
	client implementation of this class.
*/
template<class ValueType>
inline Singleton2<ValueType>::~Singleton2()
{
	assert(instance != nullptr);
	instance = 0;
}

//---------------------------------------------------------------------
/**
	@brief This is the well-known interface to retrieve an instance of a
	singleton.

	Example:
	\verbatim
	class MySingleton : public Singleton<MySignleton>
	{
	 // ... blah blah blah
	 void foo();
	};

	MySingleton::getInstance()->foo();
	\endverbatim

	If the Singleton has not been installed, getInstance() will
	install the singleton for the user. Remove must still be invoked,
	however, to insure it is cleaned up.
*/
template<class ValueType>
inline ValueType & Singleton2<ValueType>::getInstance()
{
	assert(instance != nullptr);
	return *instance;
}

//---------------------------------------------------------------------
/**
	The remove() methid is the only proper way to destroy a 
	ValueType singleton. Destruction via the base or derived dtor
	will result in assertions in debug mode, and undefined behavior
	in non-debug mode.
*/
template<class ValueType>
inline void Singleton2<ValueType>::remove()
{
	delete instance;
	instance = 0;
}

//---------------------------------------------------------------------
// a templated instance has a unique signature, and it is perfectly
// safe to declare a static variable in a multiply included header
template<class ValueType>
ValueType *	Singleton2<ValueType>::instance = 0;

//---------------------------------------------------------------------

#endif	// _Singleton_H
