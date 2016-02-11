//========================================================================
//
// DynamicVariable.cpp - generic name->data mapping system
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "sharedFoundation/FirstSharedFoundation.h"
#include "sharedFoundation/DynamicVariable.h"

#include "StringId.h"
#include "sharedFoundation/DynamicVariableList.h"
#include "sharedFoundation/DynamicVariableLocationData.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedMath/Quaternion.h"
#include "sharedMath/Transform.h"
#include "sharedMathArchive/VectorArchive.h"
#include "unicodeArchive/UnicodeArchive.h"
#include "UnicodeUtils.h"

//========================================================================

namespace DynamicVariableNamespace
{
	typedef void (*pf) (void const * const);

	void freeIntArray(void const * const data)
	{
		delete reinterpret_cast<std::vector<int> const *>(data);
	}

	void freeRealArray(void const * const data)
	{
		delete reinterpret_cast<std::vector<float> const *>(data);
	}

	void freeStdString(void const * const data)
	{
		delete reinterpret_cast<std::string const *>(data);
	}

	void freeUnicodeStringArray(void const * const data)
	{
		delete reinterpret_cast<std::vector<Unicode::String> const *>(data);
	}

	void freeStdStringArray(void const * const data)
	{
		delete reinterpret_cast<std::vector<std::string> const *>(data);
	}

	void freeNetworkIdArray(void const * const data)
	{
		delete reinterpret_cast<std::vector<NetworkId> const *>(data);
	}

	void freeLocation(void const * const data)
	{
		delete reinterpret_cast<DynamicVariableLocationData const *>(data);
	}

	void freeLocationArray(void const * const data)
	{
		delete reinterpret_cast<std::vector<DynamicVariableLocationData> const *>(data);
	}

	void freeStringId(void const * const data)
	{
		delete reinterpret_cast<StringId const *>(data);
	}

	void freeStringIdArray(void const * const data)
	{
		delete reinterpret_cast<std::vector<StringId> const *>(data);
	}

	void freeTransform(void const * const data)
	{
		delete reinterpret_cast<Transform const *>(data);
	}

	void freeTransformArray(void const * const data)
	{
		delete reinterpret_cast<std::vector<Transform> const *>(data);
	}

	void freeVector(void const * const data)
	{
		delete reinterpret_cast<Vector const *>(data);
	}

	void freeVectorArray(void const * const data)
	{
		delete reinterpret_cast<std::vector<Vector> const *>(data);
	}
};

//========================================================================
// class DynamicVariable

DynamicVariable::DynamicVariable() :
m_type(INT),
m_value(),
m_position(-1),
m_cachedValueDirty(true)
{
	m_cachedValue[0] = nullptr;
	m_cachedValue[1] = nullptr;
}

// ----------------------------------------------------------------------

DynamicVariable::~DynamicVariable()
{
	// release cache
	if (m_cachedValue[1] && (m_type != NETWORK_ID))
	{
		DynamicVariableNamespace::pf f;
		*(reinterpret_cast<void **>(&f)) = m_cachedValue[1];
		(*f)(m_cachedValue[0]);
	}
}

// ----------------------------------------------------------------------

DynamicVariable::DynamicVariable(const DynamicVariable &rhs) :
		m_type(rhs.m_type),
		m_value(rhs.m_value),
		m_position(rhs.m_position),
		m_cachedValueDirty(true)
{
	m_cachedValue[0] = nullptr;
	m_cachedValue[1] = nullptr;
}

// ----------------------------------------------------------------------

DynamicVariable::DynamicVariableType DynamicVariable::getType(void) const
{
	return m_type;
}

// ----------------------------------------------------------------------

DynamicVariable& DynamicVariable::operator=(const DynamicVariable &rhs)
{
	// release cache if type is changing
	if (m_cachedValue[1])
	{
		if (m_type != rhs.m_type)
		{
			if (m_type != NETWORK_ID)
			{
				DynamicVariableNamespace::pf f;
				*(reinterpret_cast<void **>(&f)) = m_cachedValue[1];
				(*f)(m_cachedValue[0]);
			}

			m_cachedValue[0] = nullptr;
			m_cachedValue[1] = nullptr;
		}
	}

	m_type = rhs.m_type;
	m_value = rhs.m_value;
	m_position = rhs.m_position;
	m_cachedValueDirty = true;

	return *this;
}

// ----------------------------------------------------------------------

bool DynamicVariable::operator!=(const DynamicVariable &rhs) const
{
	return ((m_type != rhs.m_type) || (m_value != rhs.m_value));
}

// ----------------------------------------------------------------------

const Unicode::String &DynamicVariable::getPackedValueString() const
{
	return m_value;
}

// ----------------------------------------------------------------------

void DynamicVariable::load(int position, int typeId, const Unicode::String &packedValueString)
{
	// release cache if type is changing
	if (m_cachedValue[1])
	{
		if (m_type != static_cast<DynamicVariableType>(typeId))
		{
			if (m_type != NETWORK_ID)
			{
				DynamicVariableNamespace::pf f;
				*(reinterpret_cast<void **>(&f)) = m_cachedValue[1];
				(*f)(m_cachedValue[0]);
			}

			m_cachedValue[0] = nullptr;
			m_cachedValue[1] = nullptr;
		}
	}

	m_position = position;
	m_type = static_cast<DynamicVariableType>(typeId);
	m_value = packedValueString;
	m_cachedValueDirty = true;
}

// ----------------------------------------------------------------------

int DynamicVariable::getPosition() const
{
	return m_position;
}

// ----------------------------------------------------------------------

void DynamicVariable::setPosition(int newPosition)
{
	m_position = newPosition;
}

// ======================================================================
// DynamicVariable Constructors

DynamicVariable::DynamicVariable(int value) :
		m_type(INT),
		m_value(),
		m_position(-1),
		m_cachedValueDirty(true)
{
	m_cachedValue[0] = nullptr;
	m_cachedValue[1] = nullptr;

	pack(value, m_value);
}

// ----------------------------------------------------------------------

DynamicVariable::DynamicVariable(const std::vector<int> & value) :
		m_type(INT_ARRAY),
		m_value(),
		m_position(-1),
		m_cachedValueDirty(true)
{
	m_cachedValue[0] = nullptr;
	m_cachedValue[1] = nullptr;

	pack(value, m_value);
}

// ----------------------------------------------------------------------

DynamicVariable::DynamicVariable(float value) :
		m_type(REAL),
		m_value(),
		m_position(-1),
		m_cachedValueDirty(true)
{
	m_cachedValue[0] = nullptr;
	m_cachedValue[1] = nullptr;

	pack(value, m_value);
}

// ----------------------------------------------------------------------

DynamicVariable::DynamicVariable(const std::vector<float> & value) :
		m_type(REAL_ARRAY),
		m_value(),
		m_position(-1),
		m_cachedValueDirty(true)
{
	m_cachedValue[0] = nullptr;
	m_cachedValue[1] = nullptr;

	pack(value, m_value);
}

// ----------------------------------------------------------------------

DynamicVariable::DynamicVariable(const Unicode::String &value) :
		m_type(STRING),
		m_value(value),
		m_position(-1),
		m_cachedValueDirty(true)
{
	m_cachedValue[0] = nullptr;
	m_cachedValue[1] = nullptr;
}

// ----------------------------------------------------------------------

DynamicVariable::DynamicVariable(const std::string &value) :
		m_type(STRING),
		m_value(Unicode::utf8ToWide(value)),
		m_position(-1),
		m_cachedValueDirty(true)
{
	m_cachedValue[0] = nullptr;
	m_cachedValue[1] = nullptr;
}

// ----------------------------------------------------------------------

DynamicVariable::DynamicVariable(const std::vector<Unicode::String> & value) :
		m_type(STRING_ARRAY),
		m_value(),
		m_position(-1),
		m_cachedValueDirty(true)
{
	m_cachedValue[0] = nullptr;
	m_cachedValue[1] = nullptr;

	pack(value, m_value);
}

// ----------------------------------------------------------------------

DynamicVariable::DynamicVariable(const NetworkId & value) :
		m_type(NETWORK_ID),
		m_value(Unicode::narrowToWide(value.getValueString())),
		m_position(-1),
		m_cachedValueDirty(true)
{
	m_cachedValue[0] = nullptr;
	m_cachedValue[1] = nullptr;
}

// ----------------------------------------------------------------------

DynamicVariable::DynamicVariable(const std::vector<NetworkId> & value) :
		m_type(NETWORK_ID_ARRAY),
		m_value(),
		m_position(-1),
		m_cachedValueDirty(true)
{
	m_cachedValue[0] = nullptr;
	m_cachedValue[1] = nullptr;

	pack(value, m_value);
}

// ----------------------------------------------------------------------

DynamicVariable::DynamicVariable(const DynamicVariableLocationData & value) :
		m_type(LOCATION),
		m_value(),
		m_position(-1),
		m_cachedValueDirty(true)
{
	m_cachedValue[0] = nullptr;
	m_cachedValue[1] = nullptr;

	pack(value, m_value);
}

// ----------------------------------------------------------------------

DynamicVariable::DynamicVariable(const std::vector<DynamicVariableLocationData> & value) :
		m_type(LOCATION_ARRAY),
		m_value(),
		m_position(-1),
		m_cachedValueDirty(true)
{
	m_cachedValue[0] = nullptr;
	m_cachedValue[1] = nullptr;

	pack(value, m_value);
}

// ----------------------------------------------------------------------

DynamicVariable::DynamicVariable(const StringId &value) :
		m_type(STRING_ID),
		m_value(Unicode::narrowToWide(value.getTable() + ' ' + value.getText())),
		m_position(-1),
		m_cachedValueDirty(true)
{
	m_cachedValue[0] = nullptr;
	m_cachedValue[1] = nullptr;
}

// ----------------------------------------------------------------------

DynamicVariable::DynamicVariable(const std::vector<StringId> &value) :
		m_type(STRING_ID_ARRAY),
		m_value(),
		m_position(-1),
		m_cachedValueDirty(true)
{
	m_cachedValue[0] = nullptr;
	m_cachedValue[1] = nullptr;

	pack(value, m_value);
}

// ----------------------------------------------------------------------

DynamicVariable::DynamicVariable(const Transform &value) :
		m_type(TRANSFORM),
		m_value(),
		m_position(-1),
		m_cachedValueDirty(true)
{
	m_cachedValue[0] = nullptr;
	m_cachedValue[1] = nullptr;

	pack(value, m_value);
}

// ----------------------------------------------------------------------

DynamicVariable::DynamicVariable(const std::vector<Transform> &value) :
		m_type(TRANSFORM_ARRAY),
		m_value(),
		m_position(-1),
		m_cachedValueDirty(true)
{
	m_cachedValue[0] = nullptr;
	m_cachedValue[1] = nullptr;

	pack(value, m_value);
}

// ----------------------------------------------------------------------

DynamicVariable::DynamicVariable(const Vector &value) :
		m_type(VECTOR),
		m_value(),
		m_position(-1),
		m_cachedValueDirty(true)
{
	m_cachedValue[0] = nullptr;
	m_cachedValue[1] = nullptr;

	pack(value, m_value);
}

// ----------------------------------------------------------------------

DynamicVariable::DynamicVariable(const std::vector<Vector> &value) :
		m_type(VECTOR_ARRAY),
		m_value(),
		m_position(-1),
		m_cachedValueDirty(true)
{
	m_cachedValue[0] = nullptr;
	m_cachedValue[1] = nullptr;

	pack(value, m_value);
}

// ======================================================================
// DynamicVariable getters

bool DynamicVariable::get(int & value) const
{
	if (m_type != INT)
		return false;

	if (m_cachedValueDirty)
	{
#ifdef _DEBUG
		FATAL((sizeof(int) > sizeof(m_cachedValue[0])),("DynamicVariable cache size mismatch for type INT"));
#endif

		*(reinterpret_cast<int *>(&(m_cachedValue[0]))) = atoi(Unicode::wideToNarrow(m_value).c_str());

		m_cachedValueDirty = false;
	}

	value = *(reinterpret_cast<int *>(&(m_cachedValue[0])));
	return true;
}

// ----------------------------------------------------------------------

bool DynamicVariable::get(std::vector<int> &value) const
{
	if (m_type != INT_ARRAY)
		return false;

	if (m_cachedValueDirty)
	{
		if (!m_cachedValue[0])
		{
			m_cachedValue[0] = new std::vector<int>();
			m_cachedValue[1] = (void*)DynamicVariableNamespace::freeIntArray;
		}

		std::vector<int> & cachedValue = *(reinterpret_cast<std::vector<int> *>(m_cachedValue[0]));
		cachedValue.clear();

		static const int BUFSIZE = 15;
		char buffer[BUFSIZE];
		char *bufpos=buffer;
		std::string packedString=Unicode::wideToNarrow(m_value);

		for(std::string::const_iterator i = packedString.begin();
			i != packedString.end() && bufpos < buffer + BUFSIZE; ++i)
		{
			if ((*i)==':')
			{
				*bufpos='\0';
				cachedValue.push_back(atoi(buffer));
				bufpos=buffer;
			}
			else
			{
				*(bufpos++)=*i;
			}
		}
		if (bufpos >= buffer + BUFSIZE)
		{
			WARNING_STRICT_FATAL(true, ("DynamicVariable::get int array, "
				"tried to overrun buffer!"));
			value.clear();
			return false;
		}

		m_cachedValueDirty = false;
	}

	value = *(reinterpret_cast<std::vector<int> *>(m_cachedValue[0]));
	return true;
}

// ----------------------------------------------------------------------

bool DynamicVariable::get(float & value) const
{
	if (m_type != REAL)
		return false;

	if (m_cachedValueDirty)
	{
#ifdef _DEBUG
		FATAL((sizeof(float) > sizeof(m_cachedValue[0])),("DynamicVariable cache size mismatch for type REAL"));
#endif

		sscanf(Unicode::wideToNarrow(m_value).c_str(),"%f",&value);
		*(reinterpret_cast<float *>(&(m_cachedValue[0]))) = value;

		m_cachedValueDirty = false;
	}

	value = *(reinterpret_cast<float *>(&(m_cachedValue[0])));
	return true;
}

// ----------------------------------------------------------------------

bool DynamicVariable::get(std::vector<float> & value) const
{
	if (m_type != REAL_ARRAY)
		return false;

	if (m_cachedValueDirty)
	{
		if (!m_cachedValue[0])
		{
			m_cachedValue[0] = new std::vector<float>();
			m_cachedValue[1] = (void*)DynamicVariableNamespace::freeRealArray;
		}

		std::vector<float> & cachedValue = *(reinterpret_cast<std::vector<float> *>(m_cachedValue[0]));
		cachedValue.clear();

		static const int BUFSIZE = 30;
		char buffer[BUFSIZE];
		char *bufpos=buffer;
		std::string packedString=Unicode::wideToNarrow(m_value);

		for(std::string::iterator i = packedString.begin();
			i != packedString.end() && bufpos < buffer + BUFSIZE; ++i)
		{
			if ((*i)==':')
			{
				*bufpos='\0';
				real temp;
				sscanf(buffer,"%f",&temp);
				cachedValue.push_back(temp);
				bufpos=buffer;
			}
			else
			{
				*(bufpos++)=*i;
			}
		}
		if (bufpos >= buffer + BUFSIZE)
		{
			WARNING_STRICT_FATAL(true, ("DynamicVariable::get float array, "
				"tried to overrun buffer!"));
			value.clear();
			return false;
		}

		m_cachedValueDirty = false;
	}

	value = *(reinterpret_cast<std::vector<float> *>(m_cachedValue[0]));
	return true;
}

// ----------------------------------------------------------------------

bool DynamicVariable::get(Unicode::String & value) const
{
	if (m_type != STRING)
		return false;

	value = m_value;
	return true;
}

// ----------------------------------------------------------------------

bool DynamicVariable::get(std::string & value) const
{
	if (m_type != STRING)
		return false;

	if (m_cachedValueDirty)
	{
		if (!m_cachedValue[0])
		{
			m_cachedValue[0] = new std::string();
			m_cachedValue[1] = (void*)DynamicVariableNamespace::freeStdString;
		}

		std::string & cachedValue = *(reinterpret_cast<std::string *>(m_cachedValue[0]));
		cachedValue = Unicode::wideToUTF8(m_value);

		m_cachedValueDirty = false;
	}

	value = *(reinterpret_cast<std::string *>(m_cachedValue[0]));
	return true;
}

// ----------------------------------------------------------------------

bool DynamicVariable::get(std::vector<Unicode::String> & value) const
{
	if (m_type != STRING_ARRAY)
		return false;

	if (m_cachedValueDirty)
	{
		if (!m_cachedValue[0])
		{
			m_cachedValue[0] = new std::vector<Unicode::String>();
			m_cachedValue[1] = (void*)DynamicVariableNamespace::freeUnicodeStringArray;
		}

		std::vector<Unicode::String> & cachedValue = *(reinterpret_cast<std::vector<Unicode::String> *>(m_cachedValue[0]));
		cachedValue.clear();

		Unicode::String buffer;
		for(Unicode::String::const_iterator i=m_value.begin(); i!=m_value.end(); ++i)
		{
			if ((*i)=='\\')
			{
				++i;
				buffer+=*i;
			}
			else if ((*i)==':')
			{
				cachedValue.push_back(buffer);
				buffer.clear();
			}
			else
			{
				buffer+=*i;
			}
		}

		m_cachedValueDirty = false;
	}

	value = *(reinterpret_cast<std::vector<Unicode::String> *>(m_cachedValue[0]));
	return true;
}

// ----------------------------------------------------------------------

bool DynamicVariable::get(NetworkId & value) const
{
	if (m_type != NETWORK_ID)
		return false;

	if (m_cachedValueDirty)
	{
#ifdef _DEBUG
		FATAL((sizeof(NetworkId::NetworkIdType) > sizeof(m_cachedValue)),("DynamicVariable cache size mismatch for type NETWORK_ID"));
#endif

		value = NetworkId(Unicode::wideToNarrow(m_value));
		*(reinterpret_cast<NetworkId::NetworkIdType *>(&(m_cachedValue[0]))) = value.getValue();

		m_cachedValueDirty = false;
	}

	value = NetworkId(*(reinterpret_cast<NetworkId::NetworkIdType *>(&(m_cachedValue[0]))));
	return true;
}

// ----------------------------------------------------------------------

bool DynamicVariable::get(std::vector<NetworkId> & value) const
{
	if (m_type != NETWORK_ID_ARRAY)
		return false;

	if (m_cachedValueDirty)
	{
		if (!m_cachedValue[0])
		{
			m_cachedValue[0] = new std::vector<NetworkId>();
			m_cachedValue[1] = (void*)DynamicVariableNamespace::freeNetworkIdArray;
		}

		std::vector<NetworkId> & cachedValue = *(reinterpret_cast<std::vector<NetworkId> *>(m_cachedValue[0]));
		cachedValue.clear();

		static const int BUFSIZE = 32;
		char buffer[BUFSIZE];
		char *bufpos=buffer;
		std::string packedString=Unicode::wideToNarrow(m_value);

		for (std::string::iterator i = packedString.begin();
			i != packedString.end() && bufpos < buffer + BUFSIZE; ++i)
		{
			if ((*i)==':')
			{
				*bufpos='\0';
				cachedValue.push_back(NetworkId(buffer));
				bufpos=buffer;
			}
			else
			{
				*(bufpos++)=*i;
			}
		}
		if (bufpos >= buffer + BUFSIZE)
		{
			WARNING_STRICT_FATAL(true, ("DynamicVariable::get networkId array, "
				"tried to overrun buffer!"));
			value.clear();
			return false;
		}

		m_cachedValueDirty = false;
	}

	value = *(reinterpret_cast<std::vector<NetworkId> *>(m_cachedValue[0]));
	return true;
}

// ----------------------------------------------------------------------

bool DynamicVariable::get(DynamicVariableLocationData & value) const
{
	if (m_type != LOCATION)
		return false;

	if (m_cachedValueDirty)
	{
		if (!m_cachedValue[0])
		{
			m_cachedValue[0] = new DynamicVariableLocationData();
			m_cachedValue[1] = (void*)DynamicVariableNamespace::freeLocation;
		}

		DynamicVariableLocationData & cachedValue = *(reinterpret_cast<DynamicVariableLocationData *>(m_cachedValue[0]));

		static const int BUFSIZE = 256;
		char tempScene[BUFSIZE];
		char tempCell[BUFSIZE];
		std::string data(Unicode::wideToNarrow(m_value));
		const char * bufptrStart = data.c_str();
		char * bufptrEnd = nullptr;
		cachedValue.pos.x = static_cast<float>(strtod(bufptrStart, &bufptrEnd));
		bufptrStart = bufptrEnd;
		while (*bufptrStart != '\0' && isspace(*bufptrStart))
			++bufptrStart;
		cachedValue.pos.y = static_cast<float>(strtod(bufptrStart, &bufptrEnd));
		bufptrStart = bufptrEnd;
		while (*bufptrStart != '\0' && isspace(*bufptrStart))
			++bufptrStart;
		cachedValue.pos.z = static_cast<float>(strtod(bufptrStart, &bufptrEnd));
		bufptrStart = bufptrEnd;

		// handle an empty scene name
		if (*bufptrStart != '\0' && isspace(*bufptrStart) && isspace(*(bufptrStart+1)))
		{
			tempScene[0] = '\0';

			while (*bufptrStart != '\0' && isspace(*bufptrStart))
				++bufptrStart;
		}
		else
		{
			while (*bufptrStart != '\0' && isspace(*bufptrStart))
				++bufptrStart;
			bufptrEnd = const_cast<char *>(strchr(bufptrStart, ' '));
			if (bufptrEnd == nullptr || bufptrEnd - bufptrStart >= BUFSIZE)
			{
				WARNING_STRICT_FATAL(true, ("DynamicVariable::get location buffer overflow in scene"));
				return false;
			}
			memcpy(tempScene, bufptrStart, bufptrEnd - bufptrStart);
			tempScene[bufptrEnd - bufptrStart] = '\0';
			bufptrStart = bufptrEnd + 1;
		}

		int cellLen = strlen(bufptrStart);
		if (cellLen >= BUFSIZE)
		{
			WARNING_STRICT_FATAL(true, ("DynamicVariable::get location buffer overflow in cell"));
			return false;
		}
		memcpy(tempCell, bufptrStart, cellLen);
		tempCell[cellLen] = '\0';

		cachedValue.scene = tempScene;
		cachedValue.cell = NetworkId(tempCell);

		m_cachedValueDirty = false;
	}

	value = *(reinterpret_cast<DynamicVariableLocationData *>(m_cachedValue[0]));
	return true;
}

// ----------------------------------------------------------------------

bool DynamicVariable::get(std::vector<DynamicVariableLocationData> & value) const
{
	if (m_type != LOCATION_ARRAY)
		return false;

	if (m_cachedValueDirty)
	{
		if (!m_cachedValue[0])
		{
			m_cachedValue[0] = new std::vector<DynamicVariableLocationData>();
			m_cachedValue[1] = (void*)DynamicVariableNamespace::freeLocationArray;
		}

		std::vector<DynamicVariableLocationData> & cachedValue = *(reinterpret_cast<std::vector<DynamicVariableLocationData> *>(m_cachedValue[0]));
		cachedValue.clear();

		static const int BUFSIZE = 256;
		static const int VARBUFSIZE = BUFSIZE * 3;
		char buffer[VARBUFSIZE];
		char *bufpos=buffer;
		std::string packedString=Unicode::wideToNarrow(m_value);

		for(std::string::iterator i = packedString.begin();
			i != packedString.end() && bufpos < buffer + VARBUFSIZE; ++i)
		{
			if ((*i)==':')
			{
				*bufpos='\0';
				DynamicVariableLocationData temp;
				char tempScene[BUFSIZE];
				char tempCell[BUFSIZE];

				const char * bufptrStart = buffer;
				char * bufptrEnd = nullptr;
				temp.pos.x = static_cast<float>(strtod(bufptrStart, &bufptrEnd));
				bufptrStart = bufptrEnd;
				while (*bufptrStart != '\0' && isspace(*bufptrStart))
					++bufptrStart;
				temp.pos.y = static_cast<float>(strtod(bufptrStart, &bufptrEnd));
				bufptrStart = bufptrEnd;
				while (*bufptrStart != '\0' && isspace(*bufptrStart))
					++bufptrStart;
				temp.pos.z = static_cast<float>(strtod(bufptrStart, &bufptrEnd));
				bufptrStart = bufptrEnd;

				// handle an empty scene name
				if (*bufptrStart != '\0' && isspace(*bufptrStart) && isspace(*(bufptrStart+1)))
				{
					tempScene[0] = '\0';

					while (*bufptrStart != '\0' && isspace(*bufptrStart))
						++bufptrStart;
				}
				else
				{
					while (*bufptrStart != '\0' && isspace(*bufptrStart))
						++bufptrStart;
					bufptrEnd = const_cast<char *>(strchr(bufptrStart, ' '));
					if (bufptrEnd == nullptr || bufptrEnd - bufptrStart >= BUFSIZE)
					{
						WARNING_STRICT_FATAL(true, ("DynamicVariable::get location array buffer overflow in scene"));
						return false;
					}
					memcpy(tempScene, bufptrStart, bufptrEnd - bufptrStart);
					tempScene[bufptrEnd - bufptrStart] = '\0';
					bufptrStart = bufptrEnd + 1;
				}

				int cellLen = strlen(bufptrStart);
				if (cellLen >= BUFSIZE)
				{
					WARNING_STRICT_FATAL(true, ("DynamicVariable::get location array buffer overflow in cell"));
					return false;
				}
				memcpy(tempCell, bufptrStart, cellLen);
				tempCell[cellLen] = '\0';

				temp.scene = tempScene;
				temp.cell = NetworkId(tempCell);
				cachedValue.push_back(temp);
				bufpos=buffer;
			}
			else
			{
				*(bufpos++)=*i;
			}
		}
		if (bufpos >= buffer + VARBUFSIZE)
		{
			WARNING_STRICT_FATAL(true, ("DynamicVariable::get location array, "
				"tried to overrun buffer!"));
			value.clear();
			return false;
		}

		m_cachedValueDirty = false;
	}

	value = *(reinterpret_cast<std::vector<DynamicVariableLocationData> *>(m_cachedValue[0]));
	return true;
}

// ----------------------------------------------------------------------

bool DynamicVariable::get(StringId &value) const
{
	if (m_type != STRING_ID)
		return false;

	if (m_cachedValueDirty)
	{
		if (!m_cachedValue[0])
		{
			m_cachedValue[0] = new StringId();
			m_cachedValue[1] = (void*)DynamicVariableNamespace::freeStringId;
		}

		StringId & cachedValue = *(reinterpret_cast<StringId *>(m_cachedValue[0]));
		std::string temp(Unicode::wideToNarrow(m_value));
		size_t space=temp.find(' ');
		cachedValue=StringId(temp.substr(0,space),temp.substr(space+1));

		m_cachedValueDirty = false;
	}

	value = *(reinterpret_cast<StringId *>(m_cachedValue[0]));
	return true;
}

// ----------------------------------------------------------------------

bool DynamicVariable::get(std::vector<StringId> &value) const
{
	if (m_type != STRING_ID_ARRAY)
		return false;

	if (m_cachedValueDirty)
	{
		if (!m_cachedValue[0])
		{
			m_cachedValue[0] = new std::vector<StringId>();
			m_cachedValue[1] = (void*)DynamicVariableNamespace::freeStringIdArray;
		}

		std::vector<StringId> & cachedValue = *(reinterpret_cast<std::vector<StringId> *>(m_cachedValue[0]));
		cachedValue.clear();

		std::string packedString(Unicode::wideToNarrow(m_value));

		for (size_t pos=0; pos < packedString.length();)
		{
			size_t const space=packedString.find(' ',pos);
			std::string table(packedString,pos,(space-pos));
			size_t const colon=packedString.find(':',space);
			std::string text(packedString,space+1,(colon-space-1));
			pos=colon+1;

			cachedValue.push_back(StringId(table,text));
		}

		m_cachedValueDirty = false;
	}

	value = *(reinterpret_cast<std::vector<StringId> *>(m_cachedValue[0]));
	return true;
}

// ----------------------------------------------------------------------

bool DynamicVariable::get(Transform &value) const
{
	if (m_type != TRANSFORM)
		return false;

	if (m_cachedValueDirty)
	{
		if (!m_cachedValue[0])
		{
			m_cachedValue[0] = new Transform();
			m_cachedValue[1] = (void*)DynamicVariableNamespace::freeTransform;
		}

		Transform & cachedValue = *(reinterpret_cast<Transform *>(m_cachedValue[0]));
		Quaternion q;
		Vector p;
		sscanf(Unicode::wideToNarrow(m_value).c_str(), "%f %f %f %f %f %f %f", &q.w, &q.x, &q.y, &q.z, &p.x, &p.y, &p.z);
		cachedValue.setPosition_p(p);
		q.getTransformPreserveTranslation(&cachedValue);

		m_cachedValueDirty = false;
	}

	value = *(reinterpret_cast<Transform *>(m_cachedValue[0]));
	return true;
}

// ----------------------------------------------------------------------

bool DynamicVariable::get(std::vector<Transform> & value) const
{
	if (m_type != TRANSFORM_ARRAY)
		return false;

	if (m_cachedValueDirty)
	{
		if (!m_cachedValue[0])
		{
			m_cachedValue[0] = new std::vector<Transform>();
			m_cachedValue[1] = (void*)DynamicVariableNamespace::freeTransformArray;
		}

		std::vector<Transform> & cachedValue = *(reinterpret_cast<std::vector<Transform> *>(m_cachedValue[0]));
		cachedValue.clear();

		static const int BUFSIZE = 256;
		char buffer[BUFSIZE];
		char *bufpos = buffer;
		std::string packedString = Unicode::wideToNarrow(m_value);

		for (std::string::iterator i = packedString.begin();
			i != packedString.end() && bufpos < buffer + BUFSIZE; ++i)
		{
			if ((*i) == ':')
			{
				*bufpos = '\0';
				Quaternion q;
				Vector p;
				sscanf(buffer, "%f %f %f %f %f %f %f", &q.w, &q.x, &q.y, &q.z, &p.x, &p.y, &p.z);
				Transform temp;
				temp.setPosition_p(p);
				q.getTransformPreserveTranslation(&temp);
				cachedValue.push_back(temp);
				bufpos = buffer;
			}
			else
			{
				*(bufpos++) = *i;
			}
		}
		if (bufpos >= buffer + BUFSIZE)
		{
			WARNING_STRICT_FATAL(true, ("DynamicVariable::get transform array, "
				"tried to overrun buffer!"));
			value.clear();
			return false;
		}

		m_cachedValueDirty = false;
	}

	value = *(reinterpret_cast<std::vector<Transform> *>(m_cachedValue[0]));
	return true;
}

// ----------------------------------------------------------------------

bool DynamicVariable::get(Vector &value) const
{
	if (m_type != VECTOR)
		return false;

	if (m_cachedValueDirty)
	{
		if (!m_cachedValue[0])
		{
			m_cachedValue[0] = new Vector();
			m_cachedValue[1] = (void*)DynamicVariableNamespace::freeVector;
		}

		Vector & cachedValue = *(reinterpret_cast<Vector *>(m_cachedValue[0]));
		sscanf(Unicode::wideToNarrow(m_value).c_str(), "%f %f %f", &cachedValue.x, &cachedValue.y, &cachedValue.z);

		m_cachedValueDirty = false;
	}

	value = *(reinterpret_cast<Vector *>(m_cachedValue[0]));
	return true;
}

// ----------------------------------------------------------------------

bool DynamicVariable::get(std::vector<Vector> & value) const
{
	if (m_type != VECTOR_ARRAY)
		return false;

	if (m_cachedValueDirty)
	{
		if (!m_cachedValue[0])
		{
			m_cachedValue[0] = new std::vector<Vector>();
			m_cachedValue[1] = (void*)DynamicVariableNamespace::freeVectorArray;
		}

		std::vector<Vector> & cachedValue = *(reinterpret_cast<std::vector<Vector> *>(m_cachedValue[0]));
		cachedValue.clear();

		static const int BUFSIZE = 256;
		char buffer[BUFSIZE];
		char *bufpos = buffer;
		std::string packedString = Unicode::wideToNarrow(m_value);

		for (std::string::iterator i = packedString.begin();
			i != packedString.end() && bufpos < buffer + BUFSIZE; ++i)
		{
			if ((*i) == ':')
			{
				*bufpos = '\0';
				Vector temp;
				sscanf(buffer, "%f %f %f", &temp.x, &temp.y, &temp.z);
				cachedValue.push_back(temp);
				bufpos = buffer;
			}
			else
			{
				*(bufpos++) = *i;
			}
		}
		if (bufpos >= buffer + BUFSIZE)
		{
			WARNING_STRICT_FATAL(true, ("DynamicVariable::get vector array, "
				"tried to overrun buffer!"));
			value.clear();
			return false;
		}

		m_cachedValueDirty = false;
	}

	value = *(reinterpret_cast<std::vector<Vector> *>(m_cachedValue[0]));
	return true;
}

// ----------------------------------------------------------------------

int DynamicVariable::getUTF8Length() const
{
	std::string utf8;
	Unicode::wideToUTF8(m_value, utf8);
	return utf8.size();
}

// ----------------------------------------------------------------------

#define GET_UTF_LENGTH(T) \
int DynamicVariable::getUTF8Length(const T &value) \
{ \
	Unicode::String packedData; \
	pack(value, packedData); \
	std::string utf8; \
	IGNORE_RETURN(Unicode::wideToUTF8(packedData, utf8)); \
	return utf8.size(); \
}

GET_UTF_LENGTH(int)
GET_UTF_LENGTH(std::vector<int>)
GET_UTF_LENGTH(float)
GET_UTF_LENGTH(std::vector<float>)
GET_UTF_LENGTH(Unicode::String)
GET_UTF_LENGTH(std::string)
GET_UTF_LENGTH(std::vector<Unicode::String>)
GET_UTF_LENGTH(NetworkId)
GET_UTF_LENGTH(std::vector<NetworkId>)
GET_UTF_LENGTH(DynamicVariableLocationData)
GET_UTF_LENGTH(std::vector<DynamicVariableLocationData>)
GET_UTF_LENGTH(StringId)
GET_UTF_LENGTH(std::vector<StringId>)
GET_UTF_LENGTH(Transform)
GET_UTF_LENGTH(std::vector<Transform>)
GET_UTF_LENGTH(Vector)
GET_UTF_LENGTH(std::vector<Vector>)


// ----------------------------------------------------------------------

void DynamicVariable::pack(int value, Unicode::String & packedData)
{
	char buffer[30];
	_itoa(value,buffer,10);
	packedData = Unicode::narrowToWide(buffer);
}

// ----------------------------------------------------------------------

void DynamicVariable::pack(const std::vector<int> & value, Unicode::String & packedData)
{
	packedData.clear();
	char buffer[15];
	for(std::vector<int>::const_iterator i = value.begin(); i != value.end(); ++i)
	{
		sprintf(buffer,"%i:",*i);
		packedData += Unicode::narrowToWide(buffer);
	}
}

// ----------------------------------------------------------------------

void DynamicVariable::pack(float value, Unicode::String & packedData)
{
	char buffer[30];
	sprintf(buffer,"%f",value);
	packedData = Unicode::narrowToWide(buffer);
}

// ----------------------------------------------------------------------

void DynamicVariable::pack(const std::vector<float> & value, Unicode::String & packedData)
{
	packedData.clear();
	char buffer[30];
	for(std::vector<float>::const_iterator i = value.begin(); i != value.end(); ++i)
	{
		sprintf(buffer,"%f:",*i);
		packedData += Unicode::narrowToWide(buffer);
	}
}

// ----------------------------------------------------------------------

void DynamicVariable::pack(const Unicode::String &value, Unicode::String & packedData)
{
	packedData = value;
}

// ----------------------------------------------------------------------

void DynamicVariable::pack(const std::string &value, Unicode::String & packedData)
{
	packedData = Unicode::utf8ToWide(value);
}

// ----------------------------------------------------------------------

void DynamicVariable::pack(const std::vector<Unicode::String> & value, Unicode::String & packedData)
{
	packedData.clear();
	for(std::vector<Unicode::String>::const_iterator i = value.begin(); i != value.end(); ++i)
	{
		for (Unicode::String::const_iterator j =(*i).begin(); j!= (*i).end(); ++j)
		{
			if ((*j)==':')
				packedData += Unicode::narrowToWide("\\:");
			else if ((*j)=='\\')
				packedData += Unicode::narrowToWide("\\\\");
			else
				packedData += (*j);
		}
		packedData += ':';
	}
}

// ----------------------------------------------------------------------

void DynamicVariable::pack(const NetworkId & value, Unicode::String & packedData)
{
	packedData = Unicode::narrowToWide(value.getValueString());
}

// ----------------------------------------------------------------------

void DynamicVariable::pack(const std::vector<NetworkId> & value, Unicode::String & packedData)
{
	std::string temp;
	for(std::vector<NetworkId>::const_iterator i = value.begin(); i != value.end(); ++i)
	{
		temp += (*i).getValueString()+':';
	}
	packedData = Unicode::narrowToWide(temp);
}

// ----------------------------------------------------------------------

void DynamicVariable::pack(const DynamicVariableLocationData & value, Unicode::String & packedData)
{
	char temp[256];
	sprintf(temp,"%f %f %f %s %s",value.pos.x, value.pos.y,value.pos.z, value.scene.c_str(), value.cell.getValueString().c_str());
	temp[255] = '\0';
	packedData = Unicode::narrowToWide(temp);
}

// ----------------------------------------------------------------------

void DynamicVariable::pack(const std::vector<DynamicVariableLocationData> & value, Unicode::String & packedData)
{
	packedData.clear();
	for (std::vector<DynamicVariableLocationData>::const_iterator i = value.begin(); i != value.end(); ++i)
	{
		char temp[256];
		sprintf(temp,"%f %f %f %s %s",(*i).pos.x, (*i).pos.y,(*i).pos.z, (*i).scene.c_str(), (*i).cell.getValueString().c_str());
		temp[255] = '\0';
		packedData += Unicode::narrowToWide(std::string(temp)+':');
	}
}

// ----------------------------------------------------------------------

void DynamicVariable::pack(const StringId &value, Unicode::String & packedData)
{
	packedData = Unicode::narrowToWide(value.getTable() + ' ' + value.getText());
}

// ----------------------------------------------------------------------

void DynamicVariable::pack(const std::vector<StringId> &value, Unicode::String & packedData)
{
	packedData.clear();
	for(std::vector<StringId>::const_iterator i = value.begin(); i != value.end(); ++i)
		packedData += Unicode::narrowToWide(i->getTable()+' '+i->getText()+':');
}

// ----------------------------------------------------------------------

void DynamicVariable::pack(const Transform &value, Unicode::String & packedData)
{
	Quaternion q(value);
	Vector p(value.getPosition_p());
	char temp[256];
	snprintf(temp, 255, "%.12g %.12g %.12g %.12g %.12g %.12g %.12g",
		static_cast<double>(q.w), static_cast<double>(q.x), static_cast<double>(q.y), static_cast<double>(q.z),
		static_cast<double>(p.x), static_cast<double>(p.y), static_cast<double>(p.z));
	temp[255] = '\0';
	packedData = Unicode::narrowToWide(temp);
}

// ----------------------------------------------------------------------

void DynamicVariable::pack(const std::vector<Transform> &value, Unicode::String & packedData)
{
	packedData.clear();
	for (std::vector<Transform>::const_iterator i = value.begin(); i != value.end(); ++i)
	{
		Quaternion q(*i);
		Vector p((*i).getPosition_p());
		char temp[256];
		snprintf(temp, 255, "%.12g %.12g %.12g %.12g %.12g %.12g %.12g:",
			static_cast<double>(q.w), static_cast<double>(q.x), static_cast<double>(q.y), static_cast<double>(q.z),
			static_cast<double>(p.x), static_cast<double>(p.y), static_cast<double>(p.z));
		temp[255] = '\0';
		packedData += Unicode::narrowToWide(temp);
	}
}

// ----------------------------------------------------------------------

void DynamicVariable::pack(const Vector &value, Unicode::String & packedData)
{
	char temp[256];
	snprintf(temp, 255, "%.12g %.12g %.12g", static_cast<double>(value.x), static_cast<double>(value.y), static_cast<double>(value.z));
	temp[255] = '\0';
	packedData = Unicode::narrowToWide(temp);
}

// ----------------------------------------------------------------------

void DynamicVariable::pack(const std::vector<Vector> &value, Unicode::String & packedData)
{
	packedData.clear();
	for (std::vector<Vector>::const_iterator i = value.begin(); i != value.end(); ++i)
	{
		char temp[256];
		snprintf(temp, 255, "%.12g %.12g %.12g:", static_cast<double>((*i).x), static_cast<double>((*i).y), static_cast<double>((*i).z));
		temp[255] = '\0';
		packedData += Unicode::narrowToWide(temp);
	}
}

// ======================================================================
// DynamicVariable Archive

namespace Archive
{
	void get(ReadIterator & source, DynamicVariable & target)
	{
		int typeInt;
		get(source,typeInt);

		// release cache if type is changing
		if (target.m_cachedValue[1])
		{
			if (target.m_type != static_cast<DynamicVariable::DynamicVariableType>(typeInt))
			{
				if (target.m_type != DynamicVariable::NETWORK_ID)
				{
					DynamicVariableNamespace::pf f;
					*(reinterpret_cast<void **>(&f)) = target.m_cachedValue[1];
					(*f)(target.m_cachedValue[0]);
				}

				target.m_cachedValue[0] = nullptr;
				target.m_cachedValue[1] = nullptr;
			}
		}

		target.m_type = static_cast<DynamicVariable::DynamicVariableType>(typeInt);
		get(source,target.m_value);
		get(source,target.m_position);
		target.m_cachedValueDirty = true;
	}

	void put(ByteStream & target, const DynamicVariable & source)
	{
		put(target,static_cast<int>(source.m_type));
		put(target,source.m_value);
		put(target,source.m_position);
	}

	void get(ReadIterator & source, DynamicVariableLocationData & target)
	{
		Archive::get(source, target.pos);
		Archive::get(source, target.scene);
		Archive::get(source, target.cell);
	}

	void put(ByteStream & target, const DynamicVariableLocationData & source)
	{
		Archive::put(target, source.pos);
		Archive::put(target, source.scene);
		Archive::put(target, source.cell);
	}
}

// ======================================================================
