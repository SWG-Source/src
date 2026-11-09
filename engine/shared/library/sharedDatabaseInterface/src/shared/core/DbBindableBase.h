// ======================================================================
//
// DBBindableBase.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_DBBindableBase_H
#define INCLUDED_DBBindableBase_H

// ======================================================================

namespace DB {

	class Bindable
	{
	  public:
		Bindable();
		//Bindable(const Bindable &rhs); // auto-generated version is fine
		//Bindable &operator=(const Bindable &rhs);
		virtual ~Bindable();
		
		bool isNull() const;

		/**
		 * Make the value NULL. The stored value is reset to the type's null
		 * value (clearValue()), so a NULL never keeps a previous value: OCI
		 * does not write a column's buffer when it fetches a NULL, and array
		 * fetches reuse their rows, so without the reset a NULL column would
		 * read as whatever an earlier row held.
		 */
		void setNull();

		int *getIndicator(); //TODO:  enforce that only QueryImpl uses this

		virtual std::string outputValue() const = 0; // For debugging or reporting errors, output the value(s) stored in this Bindable object
		
	  protected:
		explicit Bindable(int _indicator);

		/** Reset the stored value to the type's null value; see setNull(). */
		virtual void clearValue();

		/**
		 * The size of the data, or -1 for nullptr.
		 */
		int indicator;
	};
	
} 

// ======================================================================

#endif
