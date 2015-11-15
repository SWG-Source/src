// ======================================================================
//
// DBBindableBitArray.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_DBBindableBitArray_H
#define INCLUDED_DBBindableBitArray_H

// ======================================================================

#include "sharedDatabaseInterface/DbBindableString.h"
#include "sharedFoundation/BitArray.h"

// ======================================================================

namespace DB {

    /**
	 * Bindable BitArray class
	 *
	 * Template parameter S is the maximum size of the BitArray
	 * The BitArray will be stored in the database using (S+3)/4 chars
	 * Each char is a hex value for 4 bits from the BitArray
     */
	template<int S>
	class BindableBitArray : public BindableString<int((S+3)>>2)>
	{
	public:
		BindableBitArray();
		explicit BindableBitArray(const BitArray &bitArray);
		
		void setValue(const BitArray &bitArray);
		BindableBitArray &operator=(const BitArray &bitArray); 
		
		void getValue(BitArray &bitArray) const;
		BitArray getValue() const;
	};
	
	// ----------------------------------------------------------------------
	
	template<int S>
	BindableBitArray<S>::BindableBitArray()
	{
	}
	
	// ----------------------------------------------------------------------
	
	template<int S>
	BindableBitArray<S>::BindableBitArray(const BitArray &bitArray)
	{
		setValue(bitArray);
	}
	
	// ----------------------------------------------------------------------
	
	template<int S>
	void BindableBitArray<S>::setValue(const BitArray &bitArray)
	{
		std::string buf;
		bitArray.getAsDbTextString(buf, int((S+3)>>2));

		// if no bits are set in the BitArray, we must
		// put a character into buf, because BindableString
		// won't save an empty string; empty string means
		// don't overwrite current value in DB
		if (buf.empty())
			buf.push_back('0');

		BindableString<int((S+3)>>2)>::setValue(buf);
	}
	
	// ----------------------------------------------------------------------
	
	template<int S>
	BindableBitArray<S> &BindableBitArray<S>::operator=(const BitArray &bitArray)
	{
		setValue(bitArray);
		return *this;
	}
	
	// ----------------------------------------------------------------------
	
	template<int S>
	void BindableBitArray<S>::getValue(BitArray &bitArray) const
	{
		bitArray=getValue();
	}
	
	// ----------------------------------------------------------------------
	
	template<int S>
	BitArray BindableBitArray<S>::getValue() const
	{
		// Each char in the string is a hex value from 0-F
		char buf[((S+3)>>2) + 1];
		BindableString<int((S+3)>>2)>::getValue(buf,((S+3)>>2));
		BitArray temp;

		// indicator is the number of chars read from the database
		if (this->indicator > 0)
		{
			buf[this->indicator] = 0;
			temp.setFromDbTextString(buf);
		}

		return temp;
	}
	
} // end namespace DB
// ======================================================================

#endif
