//========================================================================
//
// DynamicVariable.h - generic name->data mapping system
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef _INCLUDED_DynamicVariable_H
#define _INCLUDED_DynamicVariable_H

#include "Unicode.h"

class DynamicVariable;
class DynamicVariableLocationData;
class NetworkId;
class StringId;
class Transform;
class Vector;

//========================================================================

namespace Archive
{
	class ReadIterator;
	class ByteStream;
	
	void get(ReadIterator & source, DynamicVariable & target);
	void put(ByteStream & target, const DynamicVariable & source);
}

//========================================================================
// class DynamicVariable - base class for dynamic variables

class DynamicVariable
{
  public:
	enum status {OK, BAD_TYPE, NOT_PRESENT};
	enum DynamicVariableType {
		INT, INT_ARRAY,					//lint !e578 INT hides INT in windef.h
		REAL, REAL_ARRAY, 
		STRING, STRING_ARRAY,
		NETWORK_ID, NETWORK_ID_ARRAY,
		LOCATION, LOCATION_ARRAY,
		LIST,
		STRING_ID, STRING_ID_ARRAY,
		TRANSFORM, TRANSFORM_ARRAY,
		VECTOR, VECTOR_ARRAY
	};
	enum { MAX_DYNAMIC_VARIABLE_NAME_LEN = 128 };			// this includes final '\0'
	enum { MAX_DYNAMIC_VARIABLE_STRING_VALUE_LEN = 128 };	// this includes final '\0'
	enum { MAX_DYNAMIC_VARIABLE_UTF8_LENGTH = 1000 };

  public:
	DynamicVariable();
	~DynamicVariable();
	DynamicVariable(const DynamicVariable &rhs);
	DynamicVariable& operator=(const DynamicVariable &rhs);
	bool operator!=(const DynamicVariable &rhs) const;

	int getPosition() const;
	void setPosition(int newPosition);

	explicit DynamicVariable(int value);
	explicit DynamicVariable(const std::vector<int> & value);
	explicit DynamicVariable(float value);
	explicit DynamicVariable(const std::vector<float> & value);
	explicit DynamicVariable(const Unicode::String &value);
	explicit DynamicVariable(const std::string &value);
	explicit DynamicVariable(const std::vector<Unicode::String> & value);
	explicit DynamicVariable(const NetworkId & value);
	explicit DynamicVariable(const std::vector<NetworkId> & value);
	explicit DynamicVariable(const DynamicVariableLocationData & value);
	explicit DynamicVariable(const std::vector<DynamicVariableLocationData> & value);
	explicit DynamicVariable(const StringId &value);
	explicit DynamicVariable(const std::vector<StringId> &value);
	explicit DynamicVariable(const Transform &value);
	explicit DynamicVariable(const std::vector<Transform> &value);
	explicit DynamicVariable(const Vector &value);
	explicit DynamicVariable(const std::vector<Vector> &value);

	bool get(int & value) const;
	bool get(std::vector<int> & value) const;
	bool get(float & value) const;
	bool get(std::vector<float> & value) const;
	bool get(Unicode::String & value) const;
	bool get(std::string & value) const;
	bool get(std::vector<Unicode::String> & value) const;
	bool get(NetworkId & value) const;
	bool get(std::vector<NetworkId> & value) const;
	bool get(DynamicVariableLocationData & value) const;
	bool get(std::vector<DynamicVariableLocationData> & value) const;
	bool get(StringId &value) const;
	bool get(std::vector<StringId> &value) const;
	bool get(Transform &value) const;
	bool get(std::vector<Transform> &value) const;
	bool get(Vector &value) const;
	bool get(std::vector<Vector> &value) const;

	       int getUTF8Length() const;
	static int getUTF8Length(const int & value);
	static int getUTF8Length(const std::vector<int> & value);
	static int getUTF8Length(const float & value);
	static int getUTF8Length(const std::vector<float> & value);
	static int getUTF8Length(const Unicode::String &value);
	static int getUTF8Length(const std::string &value);
	static int getUTF8Length(const std::vector<Unicode::String> & value);
	static int getUTF8Length(const NetworkId & value);
	static int getUTF8Length(const std::vector<NetworkId> & value);
	static int getUTF8Length(const DynamicVariableLocationData & value);
	static int getUTF8Length(const std::vector<DynamicVariableLocationData> & value);
	static int getUTF8Length(const StringId &value);
	static int getUTF8Length(const std::vector<StringId> &value);
	static int getUTF8Length(const Transform &value);
	static int getUTF8Length(const std::vector<Transform> &value);
	static int getUTF8Length(const Vector &value);
	static int getUTF8Length(const std::vector<Vector> &value);

public:
	// functions used for persistence:
	DynamicVariableType getType() const;
	const Unicode::String &getPackedValueString() const;
	void load(int position, int typeId, const Unicode::String &packedValueString);

private:

	static void pack(int value, Unicode::String & packedData);
	static void pack(const std::vector<int> & value, Unicode::String & packedData);
	static void pack(float value, Unicode::String & packedData);
	static void pack(const std::vector<float> & value, Unicode::String & packedData);
	static void pack(const Unicode::String &value, Unicode::String & packedData);
	static void pack(const std::string &value, Unicode::String & packedData);
	static void pack(const std::vector<Unicode::String> & value, Unicode::String & packedData);
	static void pack(const NetworkId & value, Unicode::String & packedData);
	static void pack(const std::vector<NetworkId> & value, Unicode::String & packedData);
	static void pack(const DynamicVariableLocationData & value, Unicode::String & packedData);
	static void pack(const std::vector<DynamicVariableLocationData> & value, Unicode::String & packedData);
	static void pack(const StringId &value, Unicode::String & packedData);
	static void pack(const std::vector<StringId> &value, Unicode::String & packedData);
	static void pack(const Transform &value, Unicode::String & packedData);
	static void pack(const std::vector<Transform> &value, Unicode::String & packedData);
	static void pack(const Vector &value, Unicode::String & packedData);
	static void pack(const std::vector<Vector> &value, Unicode::String & packedData);

private:
	DynamicVariableType m_type;
	Unicode::String m_value;
	int m_position;

	// caching the value so we don't constantly convert them from the string representation
	//
	// if the value can fit in m_cachedValue, we directly store it there
	// int uses m_cachedValue[0], m_cachedValue[1] = nullptr
	// float uses m_cachedValue[0], m_cachedValue[1] = nullptr
	// NetworkId uses both m_cachedValue[0] and m_cachedValue[1]
	//
	// if not, we allocate storage for the value and store the pointer to it
	// in m_cachedValue[0] and store the pointer to the function to free the
	// storage in m_cachedValue[1]
	//
	mutable bool m_cachedValueDirty;
	mutable void * m_cachedValue[2];

	friend void Archive::get(Archive::ReadIterator & source, DynamicVariable & target);
	friend void Archive::put(Archive::ByteStream & target, const DynamicVariable & source);
};

//========================================================================

#endif	// _INCLUDED_DynamicVariable_H

