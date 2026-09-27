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
		// Integer values are bound at the width of their own type. The
		// overloads name the fundamental types rather than int32_t/int64_t so
		// that every signed integer type has an exact match on both ILP32 and
		// LP64. Unsigned types have no overload (an unsigned argument is
		// ambiguous): a uint32 column value is pushed as its column, below,
		// so it is stored in the column's own encoding.
		bool push_back(bool IsNULL, short value);
		bool push_back(bool IsNULL, int value);
		bool push_back(bool IsNULL, long value);
		bool push_back(bool IsNULL, long long value);
		bool push_back(bool IsNULL, double value);
		bool push_back(short value);
		bool push_back(int value);
		bool push_back(long value);
		bool push_back(long long value);
		bool push_back(double value);

		// Push a column's value in its database form.
		bool push_back(bool IsNULL, BindableLong const & column)   { return push_back(IsNULL, column.getValue()); }
		bool push_back(bool IsNULL, BindableUint32 const & column) { return push_back(IsNULL, column.getDatabaseValue()); }
		bool push_back(bool IsNULL, BindableInt64 const & column)  { return push_back(IsNULL, column.getValue()); }
		bool push_back(bool IsNULL, BindableDouble const & column) { return push_back(IsNULL, column.getValue()); }
		bool push_back(BindableLong const & column)   { return push_back(column.getValue()); }
		bool push_back(BindableUint32 const & column) { return push_back(column.getDatabaseValue()); }
		bool push_back(BindableInt64 const & column)  { return push_back(column.getValue()); }
		bool push_back(BindableDouble const & column) { return push_back(column.getValue()); }

		virtual std::string outputValue() const;

	  private:
		template <typename T> bool appendInteger(bool IsNULL, T value);
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
