// ======================================================================
//
// ValueDictionaryArchive.cpp
// Copyright 2004 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedUtility/FirstSharedUtility.h"
#include "sharedUtility/ValueDictionaryArchive.h"

#include "Archive/Archive.h"

#include "sharedUtility/ValueDictionary.h"
#include "sharedUtility/ValueTypeBool.h"
#include "sharedUtility/ValueTypeFloat.h"
#include "sharedUtility/ValueTypeObjId.h"
#include "sharedUtility/ValueTypeSignedInt.h"
#include "sharedUtility/ValueTypeString.h"

//----------------------------------------------------------------------

namespace Archive
{
	void get (ReadIterator & source, ValueDictionary & target)
	{
		// pointer to a function that can unpack a value type; each value
		// type class should have a function that can unpack itself
		typedef ValueTypeBase * (*pf) (ReadIterator &);

		// a map that contains the different fuctions that can be
		// called to unpack the various value type objects
		typedef std::map<std::string, pf> ValueObjectUnpackHandlerMap;

		// static map that contains the different functions that
		// can be called to unpack the various value type objects
		static ValueObjectUnpackHandlerMap valueObjectUnpackHandlerMap;

		// first time initialization of the map; all data types
		// that we expect to unpack MUST have an entry in the map
		if (valueObjectUnpackHandlerMap.empty())
		{
			// we do reinterpret_cast here because C++ doesn't seem
			// to understand class hierarchy when it comes to "pointer
			// to function" data types; ValueTypeSignedInt * (*pf) (ReadIterator &)
			// should be accepted for ValueTypeBase * (*pf) (ReadIterator &)
			// if ValueTypeSignedInt is derived from ValueTypeBase
			valueObjectUnpackHandlerMap[ValueTypeBool::ms_type] = reinterpret_cast<pf>(ValueTypeBool::unpack);
			valueObjectUnpackHandlerMap[ValueTypeFloat::ms_type] = reinterpret_cast<pf>(ValueTypeFloat::unpack);
			valueObjectUnpackHandlerMap[ValueTypeObjId::ms_type] = reinterpret_cast<pf>(ValueTypeObjId::unpack);
			valueObjectUnpackHandlerMap[ValueTypeSignedInt::ms_type] = reinterpret_cast<pf>(ValueTypeSignedInt::unpack);
			valueObjectUnpackHandlerMap[ValueTypeString::ms_type]     = reinterpret_cast<pf>(ValueTypeString::unpack);
		}

		target.clear();

		int32 count;
		get(source, count);

		std::string name;
		std::string type;
		ValueTypeBase * value = nullptr;
		ValueObjectUnpackHandlerMap::const_iterator iterFind;
		for (int i = 0; i < static_cast<int>(count); ++i)
		{
			get(source, name); // element name
			get(source, type); // element data type

			// find the handler function that can unpack the data type
			iterFind = valueObjectUnpackHandlerMap.find(type);
			FATAL(iterFind == valueObjectUnpackHandlerMap.end(), ("Value dictionary has no unpack handler for element name (%s), element data type (%s).", name.c_str(), type.c_str()));

			// call the handler function to unpack the data
			value = (*(iterFind->second))(source);
			FATAL(!value, ("Value dictionary unpack failed for element name (%s), element data type (%s).", name.c_str(), type.c_str()));

			target.insert(name, *value);

			delete value;
			value = nullptr;
		}
	}

	void put (ByteStream & target, const ValueDictionary & source)
	{
		int32 count = static_cast<int32>(source.get().size());
		put(target, count);

		for (DictionaryValueMap::const_iterator iter = source.get().begin(); iter != source.get().end(); ++iter)
		{
			put(target, iter->first);             // element name
			put(target, iter->second->getType()); // element data type

			iter->second->pack(target);           // element data
		}
	}
}

//======================================================================
