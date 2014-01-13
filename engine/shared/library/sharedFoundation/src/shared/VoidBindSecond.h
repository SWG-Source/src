// ======================================================================
//
// VoidBindSecond.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#include <functional>

// ======================================================================

template <class Operation> 
class VoidBinderSecond: public std::unary_function<typename Operation::first_argument_type, void>
{
private:

	typename Operation::second_argument_type  m_boundValue;
	Operation                                 m_operation;

public:

	VoidBinderSecond(const Operation& operation, const typename Operation::second_argument_type &boundValue) 
		: m_operation(operation), m_boundValue(boundValue) {}

	void operator()(const typename Operation::first_argument_type& argument) const
		{
			m_operation(argument, m_boundValue); 
		}
};

// ======================================================================
/**
 * works just like std::bind2nd() when return value of function is void.
 *
 * this is necessary since our STL breaks under MSVC 6 when using void functions
 * with bind2nd.
 */


template <class Operation, typename BindArgumentType>
inline VoidBinderSecond<Operation> VoidBindSecond(const Operation& operation, const BindArgumentType &bindArgument) 
{
  typedef typename Operation::second_argument_type  SecondArgumentType;

  return VoidBinderSecond<Operation>(operation, SecondArgumentType(bindArgument));
}

// ======================================================================
