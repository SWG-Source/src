//======================================================================
//
// StringIdArchive.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "localizationArchive/FirstLocalizationArchive.h"
#include "localizationArchive/StringIdArchive.h"

#include "StringId.h"
#include "Archive/Archive.h"
#include "Archive/ByteStream.h"

//======================================================================

namespace Archive
{
	/**
	* Retrieves a StringId from the a data buffer.
	*
	* @param source		the source buffer
	*/
	void get(ReadIterator & source, StringId & target)
	{
		unsigned long index = 0;
		std::string   table;
		std::string   name;

		Archive::get(source, table);
		Archive::get(source, index);
		Archive::get(source, name);

		target.setTable     (table);
		target.setText      (name);

		//-- getting the index over the wire is potentially problematic
		target.setTextIndex (0);
	}

	//----------------------------------------------------------------------

	/**
	* Add a StringId to a data buffer.
	*
	* @param target		the buffer to fill
	*/

	void put(ByteStream & target, const StringId & source)
	{
		Archive::put(target, source.getTable ());
		Archive::put(target, source.getTextIndex ());
		Archive::put(target, source.getText ());
	}
}

//======================================================================
