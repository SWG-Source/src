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
		void setNull();

		int *getIndicator(); //TODO:  enforce that only QueryImpl uses this

		virtual std::string outputValue() const = 0; // For debugging or reporting errors, output the value(s) stored in this Bindable object
		
	  protected:
		explicit Bindable(int _indicator);

		/**
		 * The size of the data, or -1 for nullptr.
		 */
		int indicator;
	};
	
} 

// ======================================================================

#endif
