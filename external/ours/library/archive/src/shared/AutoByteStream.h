
#ifndef	_INCLUDED_AutoByteStream_H
#define	_INCLUDED_AutoByteStream_H

//---------------------------------------------------------------------
#pragma warning ( disable : 4786)

#include "Archive.h"
#include <cassert>
#include <list>
#include <vector>

namespace Archive {
//---------------------------------------------------------------------

class ByteStream;
class AutoVariableBase;

//---------------------------------------------------------------------
/**
	@brief AutoAchive is a specialization of ByteStream that automates packing
	and unpacking.

	To use a AutoByteStream, the client declares AutoVariables for
	members it owns and wants to persist. If the client class derives
	from some parent which does not provide Auto variables:

	   Declare a AutoVariable that shadows the parent class' normal
	   integral type, override pack and unpack. During the pack 
	   operation, the subclass sets the AutoVariable shadowing the
	   parent value, then invokes AutoByteStream::pack(). Durin the
	   unpack operation, invoke AutoByteStream::unpack(), then invoke
	   the parent's setter to assign the parent's value to the 
	   AutoVariable's value.

	Example Usage:
	\code
	class MyAutoByteStream : public AutoByteStream
	{
	public:
		MyAutoByteStream();
		~MyAutoByteStream() {};

        const float getFloatValue() const;
		const int   getIntValue() const;
		void        setValue(const float value);
		void        setValue(const int value);

	private:
		AutoVariable<int>	        i;
	};

	MyAutoByteStream::MyAutoByteStream() :
	i(0)
	{
		addVariable(i);
	}

	const int MyAutoByteStream::getIntValue() const
	{
		return i;	// conversion take care of in the AutoVariable template
	}

	void MyAutoByteStream::setValue(const int newValue)
	{
		i = newValue;
	}

	void foo()
	{
		MyAutoArchvie a;
		
		a.setValue(42);
		a.setValue(3.1415f);
		receiveNetworkMessage(a.pack());
	}

	void receiveNetworkMessage(const ByteStream & source)
	{
		MyAutoByteStream b

		b << source;

		assert(b.getIntValue() == 42);
		assert(b.getFloatValue() == 3.1415f);
	}
	\endcode

	@see AutoVariable<ValueType>

	@author Justin Randall
*/
class AutoByteStream 
{
public:
	AutoByteStream();
	virtual ~AutoByteStream();
	void                 addVariable(AutoVariableBase & newVariable);
	virtual const unsigned int getItemCount() const;
	virtual void         pack(ByteStream & target) const;
	virtual void         unpack(ReadIterator & source);
protected:
	/**
		@brief a list of member variables that are tracked by this
		AutoByteStream object
	*/
	std::vector<AutoVariableBase *>	members;
private:
	/** disabled */
	AutoByteStream(const AutoByteStream & source);
};

//---------------------------------------------------------------------
/**
	@brief A AutoVariableBase is an abstract class which defines a pack
	and unpack interface for the AutoByteStream.

	AutoVariables derive from this class.

	@see AutoVariable<ValueType>
	@see AutoByteStream
	@see ByteStream

	@author Justin Randall
*/
class AutoVariableBase
{
public:
	AutoVariableBase();
	virtual ~AutoVariableBase();
	/** pure virtual */
	virtual void     pack(ByteStream & target) const = 0;

	/** pure virtual */
	virtual void     unpack(ReadIterator & source) = 0;
};


//----------------------------------------------------------------------

inline void get(ReadIterator &source, AutoVariableBase & target)
{
	target.unpack (source);
}

//----------------------------------------------------------------------

inline void put(ByteStream &target, const AutoVariableBase & source)
{
	source.pack (target);
}

//-----------------------------------------------------------------------
/**
	A stand-alone, authoritative Auto variable template

	The AutoVariable contains an instance of a ValueType. It behaves
	much like the ValueType it encapsulates, but also provides a
	serialization interface for AutoByteStreams. 

	The use of a AutoVariable is intended to behave similarly to
	the actual ValueType it tracks, adding features for automatic
	archival and retrieval.

	A AutoVariable template should be used in a AutoByteStream 
	derived class to automatically pack and unpack data when the
	ByteStream is packed or unpacked. 

	Refer to AutoByteStream for example useage of a AutoVariable.

	@see AutoByteStream
	@see AutoVariableBase

	@author Justin Randall	
*/
template<class ValueType>
class AutoVariable : public AutoVariableBase
{
public:
	AutoVariable();
	explicit AutoVariable(const ValueType & source);
	virtual ~AutoVariable();

	const ValueType & get() const;
	virtual void      pack(ByteStream & target) const;
	void              set(const ValueType & rhs);
	virtual void      unpack(ReadIterator & source);

private:
	ValueType        value;
};

//---------------------------------------------------------------------
/**
	@brief construct an AutoVariable 
*/
template<class ValueType>
AutoVariable<ValueType>::AutoVariable() :
AutoVariableBase(),
value()
{
}

//---------------------------------------------------------------------
/**
	ValueType copy constructor.

	Motivation: Provide an implicit conversion for the AutoVariable
	that mimicks the ValueType it represents.

	Invoking this copy constructor merely assigns the source value
	to the AutoVariable value member.

	@param source  The source value which will
	                                     employ ValueType's assignment
                                         operator to set
										 AutoVariable::value to the
										 source value.

	@author Justin Randall
*/
template<class ValueType>
AutoVariable<ValueType>::AutoVariable(const ValueType & source) :
AutoVariableBase(),
value(source)
{
}

//---------------------------------------------------------------------
/**
	@brief Destroy the AutoVariable object

	Doesn't do anything special

	@author Justin Randall
*/
template<class ValueType>
AutoVariable<ValueType>::~AutoVariable()
{
}

//---------------------------------------------------------------------
/**
	@brief Retrieve the current value of the AutoVariable

	@return A const reference to the current value

	Example:
	\code
	AutoVariable<int> i;
	i.set(10);
	int j = i.get();
	\endcode

	@author Justin Randall

	@see AutoVariableBase
*/
template<class ValueType>
const ValueType & AutoVariable<ValueType>::get() const
{
	return value;
}

//---------------------------------------------------------------------
/**
	Pack the current value of the AutoVariable

	The current value of the AutoVariable is packed at the write
	position of the target ByteStream.

	The target ByteStream is typically a AutoByteStream, as it has
	protected access to the AutoVariable::pack() method.

	@param target    an ByteStream to receive the value at its
	                           current write position

	@see AutoVariable
	@see AutoVariableBase
	@see AutoByteStream

	@author Justin Randall
*/
template<class ValueType>
void AutoVariable<ValueType>::pack(ByteStream & target) const
{
	put(target, value);
}

//---------------------------------------------------------------------
/**
	@brief set the current value of a AutoVariable

	@param rhs   the new value

	@author Justin Randall
*/
template<class ValueType>
void AutoVariable<ValueType>::set(const ValueType & rhs)
{
	value = rhs;
}

//---------------------------------------------------------------------
/**
	Unpack a value from a source ByteStream and set the current value.

	A new value is extracted from the source ByteStream at its current 
	read position and applied to the AutoVariable value member.

	The source ByteStream is typically a AutoByteStream, as it has 
	protected access to the AutoVariable::unpack() method.

	@param source   The ByteStream supplying the new 
	                                value for the 

	@see AutoByteStream
	@see AutoVariableBase

	@author Justin Randall
*/
template<class ValueType>
void AutoVariable<ValueType>::unpack(ReadIterator & source)
{
	using Archive::get;
	get(source, value);
}

//-----------------------------------------------------------------------
/**
	@brief Like autovariable, but tracks an array (vector) of values

	If an array needs to be ByteStreamd automatically, it must be
	a vector.

*/
template<class ValueType>
class AutoArray : public AutoVariableBase
{
public:
	AutoArray();
	AutoArray(const AutoArray & source);
	~AutoArray();

	typedef std::vector<ValueType> DataVector;
	std::vector<ValueType> &       get();
	const std::vector<ValueType> & get() const;
	void                           set(const std::vector<ValueType> & source);
	
	virtual void pack(ByteStream & target) const;
	virtual void unpack(ReadIterator & source);

private:
	std::vector<ValueType>	array;
};

//-----------------------------------------------------------------------
/**
	@brief AutoArray ctor

	Instantiate an AutoArray of ValueType objects

	@author Justin Randall
*/
template<class ValueType>
inline AutoArray<ValueType>::AutoArray() 
{
}

//-----------------------------------------------------------------------
/**
	@brief AutoArray copy constructor

	Deep copy an AutoArray of ValueType objects

	@author Justin Randall
*/
template<class ValueType>
inline AutoArray<ValueType>::AutoArray(const AutoArray & source) :
array(source.array)
{
}

//-----------------------------------------------------------------------
/**
	@brief AutoArray dtor

	Doesn't do anything special

	@author Justin Randall
*/
template<class ValueType>
inline AutoArray<ValueType>::~AutoArray()
{
}

//-----------------------------------------------------------------------
/**
	@brief AutoArray accessor routine

	@return a const reference to a vector of ValueType objects

	@author Justin Randall
*/
template<class ValueType>
inline const std::vector<ValueType> & AutoArray<ValueType>::get() const
{
	return array;
}

//-----------------------------------------------------------------------
/**
	@brief AutoArray accessor routine

	@return a reference to a vector of ValueType objects

	@author Justin Randall
*/
template<class ValueType>
inline std::vector<ValueType> & AutoArray<ValueType>::get()
{
	return array;
}

//-----------------------------------------------------------------------
/**
	@brief AutoArray accessor routine

	@param source    a const reference to an STL vector of ValueType objects
	                 which are deep copied into this AutoArray's vector of
	                 ValueType objects

	@author Justin Randall
*/
template<class ValueType>
inline void AutoArray<ValueType>::set(const std::vector<ValueType> & source)
{
	array = source;
}

//-----------------------------------------------------------------------
/**
	@brief pack this AutoArray into an Archive::ByteStream

	The Archive::ByteStream receives an integer indicating the number
	of ValueType objects contained by the AutoArray, followed by each
	element of the AutoArray. The data is written at the Archive::ByteStream
	object's current write position.

	@param target  The Archive::ByteStream object that will receive the
	               AutoArray data.

	@see Archive::ByteStream
	@see unpack

	@author Justin Randall
*/
template<class ValueType>
inline void AutoArray<ValueType>::pack(Archive::ByteStream & target) const
{
	unsigned int arraySize = array.size();
	Archive::put(target, arraySize);

	typename std::vector<ValueType>::const_iterator i;
	for(i = array.begin(); i != array.end(); ++i)
	{
		put(target, (*i));
	}
}

//-----------------------------------------------------------------------
/**
	@brief retrieve AutoArray elements from a source Archive::ByteStream
	object.

	This method will reconstruct a populated ValueArray from an 
	Archive::ByteStream object that had an AutoArray of the same 
	ValueType written at the ByteStream's current read position.

	First an integer indicating the number of objects in the stream
	is retrieved, followed by values for each ValueType object that
	was put into the Archive::ByteStream.

	@param source   The source Archive::ByteStream containing AutoArray
	                data at it's current read position.

	@see Archive::ByteStrea
	@see pack

	@author Justin Randall
*/
template<class ValueType>
inline void AutoArray<ValueType>::unpack(Archive::ReadIterator & source) 
{
	using Archive::get;
	unsigned int arraySize;
	Archive::get(source, arraySize);
	ValueType v;

	for(unsigned int i = 0; i < arraySize; ++i)
	{
		get(source, v);
		array.push_back(v);
	}
}	

//-----------------------------------------------------------------------
/**
	@brief Like autovariable, but tracks an array (list) of values

*/
template<class ValueType>
class AutoList : public AutoVariableBase
{
public:
	AutoList();
	AutoList(const AutoList & source);
	~AutoList();

	typedef std::list<ValueType> DataList;
	std::list<ValueType> &       get();
	const std::list<ValueType> & get() const;
	void                         set(const std::list<ValueType> & source);
	
	virtual void pack(ByteStream & target) const;
	virtual void unpack(ReadIterator & source);

private:
	std::list<ValueType>	theList;
};

//-----------------------------------------------------------------------
/**
	@brief AutoList ctor

	Instantiate an AutoList of ValueType objects

	@author Justin Randall
*/
template<class ValueType>
inline AutoList<ValueType>::AutoList() 
{
}

//-----------------------------------------------------------------------
/**
	@brief AutoList copy constructor

	Deep copy an AutoList of ValueType objects

	@author Justin Randall
*/
template<class ValueType>
inline AutoList<ValueType>::AutoList(const AutoList & source) :
theList(source.theList)
{
}

//-----------------------------------------------------------------------
/**
	@brief AutoList dtor

	Doesn't do anything special

	@author Justin Randall
*/
template<class ValueType>
inline AutoList<ValueType>::~AutoList()
{
}

//-----------------------------------------------------------------------
/**
	@brief AutoList accessor routine

	@return a const reference to a list of ValueType objects

	@author Justin Randall
*/
template<class ValueType>
inline const std::list<ValueType> & AutoList<ValueType>::get() const
{
	return theList;
}

//-----------------------------------------------------------------------
/**
	@brief AutoList accessor routine

	@return a reference to a list of ValueType objects

	@author Justin Randall
*/
template<class ValueType>
inline std::list<ValueType> & AutoList<ValueType>::get()
{
	return theList;
}

//-----------------------------------------------------------------------
/**
	@brief AutoList accessor routine

	@param source    a const reference to an STL list of ValueType objects
	                 which are deep copied into this AutoList's list of
	                 ValueType objects

	@author Justin Randall
*/
template<class ValueType>
inline void AutoList<ValueType>::set(const std::list<ValueType> & source)
{
	theList = source;
}

//-----------------------------------------------------------------------
/**
	@brief pack this AutoList into an Archive::ByteStream

	The Archive::ByteStream receives an integer indicating the number
	of ValueType objects contained by the AutoList, followed by each
	element of the AutoList. The data is written at the Archive::ByteStream
	object's current write position.

	@param target  The Archive::ByteStream object that will receive the
	               AutoList data.

	@see Archive::ByteStream
	@see unpack

	@author Justin Randall
*/
template<class ValueType>
inline void AutoList<ValueType>::pack(Archive::ByteStream & target) const
{
	unsigned int arraySize = theList.size();
	Archive::put(target, arraySize);

	typename std::list<ValueType>::const_iterator i;
	for(i = theList.begin(); i != theList.end(); ++i)
	{
		put(target, (*i));
	}
}

//-----------------------------------------------------------------------
/**
	@brief retrieve AutoList elements from a source Archive::ByteStream
	object.

	This method will reconstruct a populated ValueArray from an 
	Archive::ByteStream object that had an AutoList of the same 
	ValueType written at the ByteStream's current read position.

	First an integer indicating the number of objects in the stream
	is retrieved, followed by values for each ValueType object that
	was put into the Archive::ByteStream.

	@param source   The source Archive::ByteStream containing AutoList
	                data at it's current read position.

	@see Archive::ByteStrea
	@see pack

	@author Justin Randall
*/
template<class ValueType>
inline void AutoList<ValueType>::unpack(Archive::ReadIterator & source) 
{
	using Archive::get;
	unsigned int arraySize;
	Archive::get(source, arraySize);
	ValueType v;

	for(unsigned int i = 0; i < arraySize; ++i)
	{
		get(source, v);
		theList.push_back(v);
	}
}

//---------------------------------------------------------------------

}//namespace Archive

#endif	// _INCLUDED_AutoByteStream_H
