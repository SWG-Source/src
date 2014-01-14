#ifndef	_Singleton_H
#define	_Singleton_H

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
		while(stillRunning)
		{
			// do stuff
			MySingleton::getInstance()->foo();
		}
	}
	\endverbatim

	@author	Justin Randall
---------------------------------------------------------------------*/

template<class ValueType>
class Singleton
{
public:
	static ValueType &	getInstance();

protected:
	Singleton();
	virtual ~Singleton() = 0;

	static bool installed;
	static ValueType * instance;
};

//----------------------------------------------------------------------

template<class ValueType> bool Singleton<ValueType>::installed = false;

//----------------------------------------------------------------------

template<class ValueType> ValueType * Singleton<ValueType>::instance = 0;

//----------------------------------------------------------------------

template<class ValueType>
inline Singleton<ValueType>::Singleton()
{
}

//----------------------------------------------------------------------

template<class ValueType>
inline Singleton<ValueType>::~Singleton()
{
}

//---------------------------------------------------------------------
/**
	@brief This is the well-known interface to retrieve an instance of a
	singleton.

	Example:
	\verbatim
	class MySingleton : public Singleton<MySingleton>
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
inline ValueType & Singleton<ValueType>::getInstance()
{
	if (!installed)
	{
		//-- this is a dirty hack to get around the fact that msvc is an
		//-- approximation of an obsolete c++ compiler

		static ValueType v;
		instance = &v;
		installed = true;
	}

	return *instance;
}

//---------------------------------------------------------------------

#endif	// _Singleton_H
