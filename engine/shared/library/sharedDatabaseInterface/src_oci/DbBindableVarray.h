// ======================================================================
//
// DbBindableVarray.h
// copyright (c) 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_DbBindableVarray_H
#define INCLUDED_DbBindableVarray_H

// ======================================================================

#include "sharedDatabaseInterface/Bindable.h"

// ======================================================================

struct OCIColl;
typedef OCIColl OCIArray;
struct OCIType;
class NetworkId;

// ======================================================================

namespace DB
{
	class Session;

	// ======================================================================

/**
 * Bindable array type.  Note:  only usable in OCI.  In ODBC, calling
 * any function on this class will FATAL.
 */
	class BindableVarray : public Bindable
	{
	  public:
		BindableVarray();
		~BindableVarray();

		bool create(DB::Session *session, const std::string &name, const std::string &schema);

		void free();
		void clear();

		OCIArray ** getBuffer();
		OCIType   * getTDO();
	
	  protected:
		bool      m_initialized;
		OCIType  *m_tdo;
		OCIArray *m_data;
		Session  *m_session;
	};

// ======================================================================

	class BindableVarrayNumber : public BindableVarray
	{
	  public:
		bool push_back(bool IsNULL, int value);
		bool push_back(bool IsNULL, double value);
		bool push_back(bool IsNULL, long int value);
		bool push_back(bool IsNULL, int64 value);
		bool push_back(int value);
		bool push_back(double value);
		bool push_back(long int value);
		bool push_back(int64 value);

		virtual std::string outputValue() const;
	};

// ======================================================================

	class BindableVarrayString : public BindableVarray
	{
	  public:
		BindableVarrayString();
		bool create(DB::Session *session, const std::string &name, const std::string &schema, size_t maxLength);
		
		bool push_back(bool IsNULL, const Unicode::String &value);
		bool push_back(bool IsNULL, const std::string &value);
		bool push_back(bool IsNULL, bool value);
		bool push_back(bool IsNULL, const NetworkId &value);
		bool push_back(const Unicode::String &value);
		bool push_back(const std::string &value);
		bool push_back(bool value);
		bool push_back(const NetworkId &value);

		virtual std::string outputValue() const;

	  private:
		size_t m_maxLength;
	};

// ======================================================================

} //namespace

// ======================================================================

#endif
