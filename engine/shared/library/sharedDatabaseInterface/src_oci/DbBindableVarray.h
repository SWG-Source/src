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
#include "sharedDatabaseInterface/BindableNetworkId.h"

#include <cstdint>

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
		// Integers are pushed at a fixed width, 32 or 64 bits, the same on
		// every platform. There is no overload for long, whose width depends
		// on the platform, nor for unsigned types: a uint32 value is pushed
		// as its BindableUint32 column, below, which supplies the column's
		// signed 32-bit encoding.
		bool push_back(bool IsNULL, int32_t value);
		bool push_back(bool IsNULL, int64_t value);
		bool push_back(bool IsNULL, double value);
		bool push_back(int32_t value);
		bool push_back(int64_t value);
		bool push_back(double value);

		// Push a column's value in its database form, at the column's own
		// width. A NULL element is pushed as NULL without reading the
		// column's value, which is meaningless while the column is NULL.
		// The one-argument forms push NULL exactly when the column is NULL.
		bool push_back(bool IsNULL, BindableInt32 const & column)  { return IsNULL ? push_back(true, int32_t(0)) : push_back(false, column.getValue()); }
		bool push_back(bool IsNULL, BindableUint32 const & column) { return IsNULL ? push_back(true, int32_t(0)) : push_back(false, column.getDatabaseValue()); }
		bool push_back(bool IsNULL, BindableInt64 const & column)  { return IsNULL ? push_back(true, int64_t(0)) : push_back(false, static_cast<int64_t>(column.getValue())); }
		bool push_back(bool IsNULL, BindableDouble const & column) { return IsNULL ? push_back(true, 0.0) : push_back(false, column.getValue()); }
		bool push_back(BindableInt32 const & column)  { return push_back(column.isNull(), column); }
		bool push_back(BindableUint32 const & column) { return push_back(column.isNull(), column); }
		bool push_back(BindableInt64 const & column)  { return push_back(column.isNull(), column); }
		bool push_back(BindableDouble const & column) { return push_back(column.isNull(), column); }

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
		// Push a column in its database form; a NULL column is pushed as
		// NULL without reading its value.
		bool push_back(bool IsNULL, BindableNetworkId const & column);
		bool push_back(bool IsNULL, BindableBool const & column);
		bool push_back(BindableNetworkId const & column) { return push_back(column.isNull(), column); }
		bool push_back(BindableBool const & column)      { return push_back(column.isNull(), column); }
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
