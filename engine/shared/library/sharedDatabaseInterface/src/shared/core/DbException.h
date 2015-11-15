// ======================================================================
//
// DbException.h
// copyright (c) 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_DbException_H
#define INCLUDED_DbException_H

// ======================================================================

#include <string>
// ======================================================================

namespace DB
{
	class Exception
	{
	  public:
		Exception(int errorNumber, const std::string &errorDescription, const std::string &contextDescription);
		Exception(int errorNumber, const std::string &errorDescription);

	  public:
		int                 getErrorNumber() const;
		const std::string & getErrorDescription() const;
		const std::string & getContextDescription() const;

	  private:
		int m_errorNumber;
		std::string m_errorDescription;
		std::string m_contextDescription;
	};

	// ======================================================================
	
	inline int Exception::getErrorNumber() const
	{
		return m_errorNumber;
	}

	// ----------------------------------------------------------------------

	inline const std::string & Exception::getErrorDescription() const
	{
		return m_errorDescription;
	}

	// ----------------------------------------------------------------------

	inline const std::string & Exception::getContextDescription() const
	{
		return m_contextDescription;
	}

	// ======================================================================

} // namespace

// ======================================================================

#endif
