// ======================================================================
//
// AutoDeltaPackedMap.h
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_AutoDeltaPackedMap_H
#define INCLUDED_AutoDeltaPackedMap_H

// ======================================================================

#include <stdio.h>
#include "AutoDeltaMap.h"

// ======================================================================

namespace Archive
{
	int countCharacter(const std::string &str, char c);
	//These are implemented in sharedFoundation/NetworkIdArchive
#ifdef WIN32
	void get(ReadIterator & source, unsigned __int64 & target);
	void put(ByteStream & target, const unsigned __int64 & source);
#else
	void get(ReadIterator & source, unsigned long long int & target);
	void put(ByteStream & target, const unsigned long long int & source);
#endif
	/**
	 * An AutoDeltaPackedMap is an AutoDeltaMap that will be packed into
	 * a single value for storage.  It functions as an AutoDeltaMap in
	 * all respects except that packDeltas() will send the entire map
	 * on the network.
	 */
	template<class KeyType, typename ValueType, typename ObjectType=DefaultObjectType>
	class AutoDeltaPackedMap : public AutoDeltaMap<KeyType, ValueType, ObjectType>
	{
	public:
		AutoDeltaPackedMap();
		AutoDeltaPackedMap(const AutoDeltaPackedMap & source);
		~AutoDeltaPackedMap();

		static void     unpack(ReadIterator & source, std::string & buffer);
		static void     pack(ByteStream & target, const std::string & buffer);
		
		virtual void    packDelta(ByteStream & target) const;
		virtual void    unpackDelta(ReadIterator & source);

		void setOnChanged(ObjectType * owner, void (ObjectType::*onChanged)());

	private:
		static void     internal_unpack(ReadIterator & source, std::string & buffer, const char * format);
		static void     internal_pack(ByteStream & target, const std::string & buffer, const char * format);

		void onChanged();

		std::pair<ObjectType *, void (ObjectType::*)()> *onChangedCallback;

	private:
		AutoDeltaPackedMap & operator=(const AutoDeltaPackedMap &rhs);
	};

	template<class KeyType, typename ValueType, typename ObjectType>
	inline AutoDeltaPackedMap<KeyType, ValueType, ObjectType>::AutoDeltaPackedMap() :
			AutoDeltaMap<KeyType, ValueType, ObjectType>(),
			onChangedCallback(0)
	{
	}		

	template<class KeyType, typename ValueType, typename ObjectType>
	inline AutoDeltaPackedMap<KeyType, ValueType, ObjectType>::AutoDeltaPackedMap(const AutoDeltaPackedMap<KeyType, ValueType, ObjectType> & source) :
			AutoDeltaMap<KeyType, ValueType, ObjectType>(source),
			onChangedCallback(0)
	{
	}		

	template<class KeyType, typename ValueType, typename ObjectType>
	inline AutoDeltaPackedMap<KeyType, ValueType, ObjectType>::~AutoDeltaPackedMap()
	{
		delete onChangedCallback;
	}		

	template<class KeyType, typename ValueType, typename ObjectType>
	inline void AutoDeltaPackedMap<KeyType, ValueType, ObjectType>::packDelta(ByteStream & target) const
	{
		AutoDeltaMap<KeyType, ValueType, ObjectType>::pack(target);
	}
	
	template<class KeyType, typename ValueType, typename ObjectType>
	inline void AutoDeltaPackedMap<KeyType, ValueType, ObjectType>::unpackDelta(ReadIterator & source)
	{
		AutoDeltaMap<KeyType, ValueType, ObjectType>::unpack(source);
		onChanged();
	}

	template<class KeyType, typename ValueType, typename ObjectType>
	inline void AutoDeltaPackedMap<KeyType, ValueType, ObjectType>::internal_pack(ByteStream & target, const std::string & buffer, const char* format)
	{
		char temp[200];
		typename AutoDeltaMap<KeyType, ValueType, ObjectType>::Command c;

		Archive::put(target, countCharacter(buffer,':'));
		Archive::put(target, static_cast<size_t>(0)); // baselineCommandCount
		
		int tempPos = 0;
		for (std::string::const_iterator i=buffer.begin(); i!=buffer.end(); ++i)
		{
			if (*i==':')
			{
				temp[tempPos]='\0';
				sscanf(temp,format,&c.key, &c.value);
				Archive::put(target, static_cast<unsigned char>(AutoDeltaMap<KeyType, ValueType, ObjectType>::Command::ADD));
				Archive::put(target, c.key);
				Archive::put(target, c.value);
				tempPos=0;
			}
			else
			{
				temp[tempPos++]=*i;
			}
		}
	}

	template<class KeyType, typename ValueType, typename ObjectType>
	inline void AutoDeltaPackedMap<KeyType, ValueType, ObjectType>::internal_unpack(ReadIterator & source, std::string & buffer, const char *format)
	{
		char temp[200];
		
		typename AutoDeltaMap<KeyType, ValueType, ObjectType>::Command c;
		size_t commandCount;
		size_t baselineCommandCount;

		Archive::get(source, commandCount);
		Archive::get(source, baselineCommandCount);

		if (commandCount==0)
		{
			buffer=' '; // An empty map is represented as a single space, because a completely empty string is used to mean "no change"
		}
		else
		{
			for (size_t i = 0; i < commandCount; ++i)
			{
				Archive::get(source, c.cmd);
				Archive::get(source, c.key);
				Archive::get(source, c.value);
				
#ifdef WIN32
				_snprintf(temp, sizeof(temp)-1, format, c.key, c.value);
#else
				snprintf(temp, sizeof(temp)-1, format, c.key, c.value);
#endif
			
				temp[sizeof(temp)-1]='\0';
				buffer+=temp;
			}
		}
	}
	
	template<class KeyType, typename ValueType, typename ObjectType>
	inline void AutoDeltaPackedMap<KeyType, ValueType, ObjectType>::onChanged()
	{
		if (onChangedCallback && onChangedCallback->first)
		{
			ObjectType &owner = *onChangedCallback->first;
			void (ObjectType::*cb)() = onChangedCallback->second;
			(owner.*cb)();
		}
	}

	template<class KeyType, typename ValueType, typename ObjectType>
	inline void AutoDeltaPackedMap<KeyType, ValueType, ObjectType>::setOnChanged(ObjectType * owner, void (ObjectType::*cb)())
	{
		delete onChangedCallback;
		onChangedCallback = new std::pair<ObjectType *, void (ObjectType::*)()>;
		onChangedCallback->first = owner;
		onChangedCallback->second = cb;
	}

	// ======================================================================
	// pack-to-string functions for specific types

	template<>
	inline void AutoDeltaPackedMap<int, float>::pack(ByteStream & target, const std::string & buffer)
	{
		internal_pack(target, buffer, "%i %f");
	}

	template<>
	inline void AutoDeltaPackedMap<int, float>::unpack(ReadIterator & source, std::string & buffer)
	{
		internal_unpack(source, buffer, "%i %.2f:");
	}

	template<>
	inline void AutoDeltaPackedMap<int, int>::pack(ByteStream & target, const std::string & buffer)
	{
		internal_pack(target, buffer, "%i %i");
	}

	template<>
	inline void AutoDeltaPackedMap<int, int>::unpack(ReadIterator & source, std::string & buffer)
	{
		internal_unpack(source, buffer, "%i %i:");
	}

	template<>
	inline void AutoDeltaPackedMap<int, unsigned long>::pack(ByteStream & target, const std::string & buffer)
	{
		internal_pack(target, buffer, "%i %u");
	}

	template<>
	inline void AutoDeltaPackedMap<int, unsigned long>::unpack(ReadIterator & source, std::string & buffer)
	{
		internal_unpack(source, buffer, "%i %u:");
	}

#ifdef WIN32
	template<>
	inline void AutoDeltaPackedMap<unsigned long, unsigned __int64>::pack(ByteStream & target, const std::string & buffer)
	{
		internal_pack(target, buffer, "%lu %I64u");
	}

	template<>
	inline void AutoDeltaPackedMap<unsigned long, unsigned __int64>::unpack(ReadIterator & source, std::string & buffer)
	{
		internal_unpack(source, buffer, "%lu %I64u:");
	}

#else
	template<>
	inline void AutoDeltaPackedMap<unsigned long, unsigned long long int>::pack(ByteStream & target, const std::string & buffer)
	{
		internal_pack(target, buffer, "%lu %llu");
	}

	template<>
	inline void AutoDeltaPackedMap<unsigned long, unsigned long long int>::unpack(ReadIterator & source, std::string & buffer)
	{
		internal_unpack(source, buffer, "%lu %llu:");
	}
#endif

} //namespace

// ======================================================================

#endif
