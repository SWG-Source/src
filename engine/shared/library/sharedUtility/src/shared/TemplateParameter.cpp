//========================================================================
//
// TemplateParameter.cpp - base and simple classes for template parameters
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "sharedUtility/FirstSharedUtility.h"
#include "sharedUtility/TemplateParameter.h"
#include "UnicodeUtils.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/DataResource.h"
#include "sharedFoundation/DynamicVariable.h"
#include "sharedFoundation/DynamicVariableList.h"
#include "sharedMath/Vector.h"

//========================================================================
// class IntegerParam

/**
 * Class constructor.
 */
IntegerParam::IntegerParam(void) : TemplateBase<int, int>(),
	m_dataDeltaType(' ')
{
}	// IntegerParam::IntegerParam

/**
 * Class destructor.
 */
IntegerParam::~IntegerParam()
{
}	// IntegerParam::~IntegerParam

/**
 * Returns the minimum value that a getValue() can return. This function will fatal 
 * if the data type is a weighted list.
 *
 * @return the minimum getValue() return value.
 */
int IntegerParam::getMinValue(void) const
{
	switch (m_dataType)
	{
		case SINGLE:
			return static_cast<int32>(m_dataSingle);
		case RANGE:
			NOT_NULL(m_data.range);
			return static_cast<int32>(m_data.range->min_value);
		case DIE_ROLL:
			NOT_NULL(m_data.dieRoll);
			return static_cast<int32>(m_data.dieRoll->base + m_data.dieRoll->num_dice);
		case NONE:
		case WEIGHTED_LIST:
		default:
			DEBUG_FATAL(true, ("getting min value for IntegerParam data type %d",
				m_dataType));
			break;
	}
	return 0;
}	// IntegerParam::getMinValue

/**
 * Returns the maximum value that a getValue() can return. This function will fatal 
 * if the data type is a weighted list.
 *
 * @return the maximum getValue() return value.
 */
int IntegerParam::getMaxValue(void) const
{
	switch (m_dataType)
	{
		case SINGLE:
			return static_cast<int32>(m_dataSingle);
		case RANGE:
			NOT_NULL(m_data.range);
			return static_cast<int32>(m_data.range->max_value);
		case DIE_ROLL:
			NOT_NULL(m_data.dieRoll);
			return static_cast<int32>(m_data.dieRoll->base + m_data.dieRoll->num_dice * 
				m_data.dieRoll->die_sides);
		case NONE:
		case WEIGHTED_LIST:
		default:
			DEBUG_FATAL(true, ("getting max value for IntegerParam data type %d",
				m_dataType));
			break;
	}
	return 0;
}	// IntegerParam::getMaxValue

/**
 * Loads the param data from an iff file.
 *
 * @param file		the file to load from
 */
void IntegerParam::loadFromIff(Iff &file)
{
	DataTypeId dataType = static_cast<DataTypeId>(file.read_int8());
	m_dataDeltaType = file.read_int8();
	switch (dataType)
	{
		case SINGLE:
			setValue(file.read_int32());
			m_loaded = true;
			break;
		case WEIGHTED_LIST:
			setValue(new WeightedList);
			loadWeightedListFromIff(file);
			break;
		case RANGE:
			{
				int32 min_value = file.read_int32();
				int32 max_value = file.read_int32();
				setValue(min_value, max_value);
				m_loaded = true;
			}
			break;
		case DIE_ROLL:
			{
				int32 num_dice = file.read_int32();
				int32 die_sides = file.read_int32();
				int32 base = file.read_int32();
				setValue(num_dice, die_sides, base);
				m_loaded = true;
			}
			break;
		case NONE:
			cleanData();
			break;
		default:
			DEBUG_FATAL(true, ("loaded unknown data type %d for template int "
				"param", m_dataType));
			break;
	}
}	// IntegerParam::loadFromIff

/**
 * Saves the param data to an iff file.
 *
 * @param file		the file to save to
 */
void IntegerParam::saveToIff(Iff &file) const
{
int32 intData;

	int8 type = static_cast<int8>(m_dataType);
	file.insertChunkData(&type, sizeof(type));
	type = m_dataDeltaType;
	file.insertChunkData(&type, sizeof(type));
	switch (m_dataType)
	{
		case SINGLE:
			intData = static_cast<int32>(m_dataSingle);
			file.insertChunkData(&intData, sizeof(intData));
			break;
		case WEIGHTED_LIST:
			saveWeightedListToIff(file);
			break;
		case RANGE:
			NOT_NULL(m_data.range);
			intData = static_cast<int32>(m_data.range->min_value);
			file.insertChunkData(&intData, sizeof(intData));
			intData = static_cast<int32>(m_data.range->max_value);
			file.insertChunkData(&intData, sizeof(intData));
			break;
		case DIE_ROLL:
			NOT_NULL(m_data.dieRoll);
			intData = static_cast<int32>(m_data.dieRoll->num_dice);
			file.insertChunkData(&intData, sizeof(intData));
			intData = static_cast<int32>(m_data.dieRoll->die_sides);
			file.insertChunkData(&intData, sizeof(intData));
			intData = static_cast<int32>(m_data.dieRoll->base);
			file.insertChunkData(&intData, sizeof(intData));
			break;
		case NONE:
			break;
		default:
			DEBUG_FATAL(true, ("saving unknown data type %d for template int "
				"param", m_dataType));
			break;
	}
}	// IntegerParam::saveToIff


//========================================================================
// class FloatParam

/**
 * Class constructor.
 */
FloatParam::FloatParam(void) : TemplateBase<float, float>(),
	m_dataDeltaType(' ')
{
}	// FloatParam::FloatParam

/**
 * Class destructor.
 */
FloatParam::~FloatParam()
{
}	// FloatParam::~FloatParam

/**
 * Returns the minimum value that a getValue() can return. This function will fatal 
 * if the data type is a weighted list.
 *
 * @return the minimum getValue() return value.
 */
float FloatParam::getMinValue(void) const
{
	switch (m_dataType)
	{
		case SINGLE:
			return m_dataSingle;
		case RANGE:
			NOT_NULL(m_data.range);
			return m_data.range->min_value;
		case NONE:
		case DIE_ROLL:
		case WEIGHTED_LIST:
		default:
			DEBUG_FATAL(true, ("getting min value for FloatParam data type %d",
				m_dataType));
			break;
	}
	return 0;
}	// FloatParam::getMinValue

/**
 * Returns the maximum value that a getValue() can return. This function will fatal 
 * if the data type is a weighted list.
 *
 * @return the maximum getValue() return value.
 */
float FloatParam::getMaxValue(void) const
{
	switch (m_dataType)
	{
		case SINGLE:
			return m_dataSingle;
		case RANGE:
			NOT_NULL(m_data.range);
			return m_data.range->max_value;
		case NONE:
		case DIE_ROLL:
		case WEIGHTED_LIST:
		default:
			DEBUG_FATAL(true, ("getting max value for FloatParam data type %d",
				m_dataType));
			break;
	}
	return 0;
}	// FloatParam::getMaxValue

/**
 * Loads the param data from an iff file.
 *
 * @param file		the file to load from
 */
void FloatParam::loadFromIff(Iff &file)
{
	DataTypeId dataType = static_cast<DataTypeId>(file.read_int8());
	m_dataDeltaType = file.read_int8();
	switch (dataType)
	{
		case SINGLE:
			setValue(file.read_float());
			m_loaded = true;
			break;
		case WEIGHTED_LIST:
			setValue(new WeightedList);
			loadWeightedListFromIff(file);
			break;
		case RANGE:
			{
				float min_value = file.read_float();
				float max_value = file.read_float();
				setValue(min_value, max_value);
				m_loaded = true;
			}
			break;
		case NONE:
			cleanData();
			break;
		case DIE_ROLL:
		default:
			DEBUG_FATAL(true, ("loaded unknown data type %d for template float "
				"param", m_dataType));
			break;
	}
}	// FloatParam::loadFromIff

/**
 * Saves the param data to an iff file.
 *
 * @param file		the file to save to
 */
void FloatParam::saveToIff(Iff &file) const
{
	int8 type = static_cast<int8>(m_dataType);
	file.insertChunkData(&type, sizeof(type));
	type = m_dataDeltaType;
	file.insertChunkData(&type, sizeof(type));
	switch (m_dataType)
	{
		case SINGLE:
			file.insertChunkData(&m_dataSingle, sizeof(m_dataSingle));
			break;
		case WEIGHTED_LIST:
			saveWeightedListToIff(file);
			break;
		case RANGE:
			NOT_NULL(m_data.range);
			file.insertChunkData(&m_data.range->min_value, sizeof(m_data.range->min_value));
			file.insertChunkData(&m_data.range->max_value, sizeof(m_data.range->max_value));
			break;
		case NONE:
			break;
		case DIE_ROLL:
		default:
			DEBUG_FATAL(true, ("saving unknown data type %d for template float "
				"param", m_dataType));
			break;
	}
}	// FloatParam::saveToIff


//========================================================================
// class BoolParam

/**
 * Class constructor.
 */
BoolParam::BoolParam(void) : TemplateBase<bool, bool>()
{
}	// BoolParam::BoolParam

/**
 * Class destructor.
 */
BoolParam::~BoolParam()
{
}	// BoolParam::~BoolParam

/**
 * Loads the param data from an iff file.
 *
 * @param file		the file to load from
 */
void BoolParam::loadFromIff(Iff &file)
{
	DataTypeId dataType = static_cast<DataTypeId>(file.read_int8());
	switch (dataType)
	{
		case SINGLE:
			setValue(file.read_bool8());
			m_loaded = true;
			break;
		case WEIGHTED_LIST:
			setValue(new WeightedList);
			loadWeightedListFromIff(file);
			break;
		case NONE:
			cleanData();
			break;
		case RANGE:
		case DIE_ROLL:
		default:
			DEBUG_FATAL(true, ("loaded unknown data type %d for template bool "
				"param", m_dataType));
			break;
	}
}	// BoolParam::loadFromIff

/**
 * Saves the param data to an iff file.
 *
 * @param file		the file to save to
 */
void BoolParam::saveToIff(Iff &file) const
{
	int8 type = static_cast<int8>(m_dataType);
	file.insertChunkData(&type, sizeof(type));
	switch (m_dataType)
	{
		case SINGLE:
			file.insertChunkData(&m_dataSingle, sizeof(m_dataSingle));
			break;
		case WEIGHTED_LIST:
			saveWeightedListToIff(file);
			break;
		case NONE:
			break;
		case RANGE:
		case DIE_ROLL:
		default:
			DEBUG_FATAL(true, ("saving unknown data type %d for template bool "
				"param", m_dataType));
			break;
	}
}	// BoolParam::saveToIff


//========================================================================
// class StringParam

/**
 * Class constructor.
 */
StringParam::StringParam(void) : TemplateBase<std::string, const std::string &>()
{
}	// StringParam::StringParam

/**
 * Class destructor.
 */
StringParam::~StringParam()
{
}	// StringParam::~StringParam

/**
 * Does cleanup for a single-value parameter.
 */
void StringParam::cleanSingleParam(void)
{
	m_dataSingle = "";
}	// StringParam::cleanSingleParam

/**
 * Loads the param data from an iff file.
 *
 * @param file		the file to load from
 */
void StringParam::loadFromIff(Iff &file)
{
	DataTypeId dataType = static_cast<DataTypeId>(file.read_int8());
	switch (dataType)
	{
		case SINGLE:
			{
				char *string = file.read_string();
				setValue(string);
				delete[] string;
				// if the string appears to be a template name, check that it has '/'
				// instead of '\' and change the string if they are wrong
				if (m_dataSingle.size() >= 7 && 
					m_dataSingle.find(".iff", m_dataSingle.size() - 4) == m_dataSingle.size() - 4 &&
					m_dataSingle.find('\\') != m_dataSingle.npos)
				{
					DEBUG_WARNING(true, ("StringParam::loadFromIff file %s loading "
						"string <%s> that appears to be a template name with "
						"backward slashes. We are changing them to forward slashes.",
						file.getFileName(), m_dataSingle.c_str()));
					size_t slashPos = m_dataSingle.find('\\');
					while (slashPos != m_dataSingle.npos)
					{
						m_dataSingle[slashPos] = '/';
						slashPos = m_dataSingle.find('\\');
					}

				}
				m_loaded = true;
			}
			break;
		case WEIGHTED_LIST:
			setValue(new WeightedList);
			loadWeightedListFromIff(file);
			break;
		case NONE:
			cleanData();
			break;
		case RANGE:
		case DIE_ROLL:
		default:
			DEBUG_FATAL(true, ("loaded unknown data type %d for template string "
				"param", m_dataType));
			break;
	}
}	// StringParam::loadFromIff

/**
 * Saves the param data to an iff file.
 *
 * @param file		the file to save to
 */
void StringParam::saveToIff(Iff &file) const
{
	int8 type = static_cast<int8>(m_dataType);
	file.insertChunkData(&type, sizeof(type));
	switch (m_dataType)
	{
		case SINGLE:
			file.insertChunkString(m_dataSingle.c_str());
			break;
		case WEIGHTED_LIST:
			saveWeightedListToIff(file);
			break;
		case NONE:
			break;
		case RANGE:
		case DIE_ROLL:
		default:
			DEBUG_FATAL(true, ("saving unknown data type %d for template string "
				"param", m_dataType));
			break;
	}
}	// StringParam::saveToIff


//========================================================================
// struct VectorParamData

/**
 * Modifies a vector by the info in this param.
 *
 * @param pos		the vector to modify
 */
void VectorParamData::adjustVector(Vector &pos) const
{
	pos.x += x.getValue();
	if (!ignoreY)
		pos.y += y.getValue();
	pos.z += z.getValue();
	float rad = radius.getValue();
	if (rad > 0.0f)
	{
		if (ignoreY)
		{
			float phi = Random::randomReal(0.0, PI_TIMES_2);
			pos.x += rad * cos(phi);
			pos.z += rad * sin(phi);
		}
		else
		{
			// uniform random point on a sphere from 
			// http://www.cs.cmu.edu/~mws/rpos.html
			float y = Random::randomReal(-rad, rad);
			float phi = Random::randomReal(0.0, PI_TIMES_2);
			float ctheta = cos(asin(y / rad)) * rad;
			pos.x += ctheta * cos(phi);
			pos.z += ctheta * sin(phi);
			pos.y += y;
		}
	}
}	// VectorParamData::adjustVector


//========================================================================
// class VectorParam

/**
 * Class constructor.
 */
VectorParam::VectorParam(void) : TemplateBase<VectorParamData, const VectorParamData &>()
{
}	// VectorParam::VectorParam

/**
 * Class destructor.
 */
VectorParam::~VectorParam()
{
}	// VectorParam::~VectorParam

/**
 * Does cleanup for a single-value parameter.
 */
void VectorParam::cleanSingleParam(void)
{
	m_dataSingle.x.cleanData();
	m_dataSingle.y.cleanData();
	m_dataSingle.z.cleanData();
	m_dataSingle.radius.cleanData();
}	// VectorParam::cleanSingleParam

/**
 * Loads the param data from an iff file.
 *
 * @param file		the file to load from
 */
void VectorParam::loadFromIff(Iff &file)
{
	DataTypeId dataType = static_cast<DataTypeId>(file.read_int8());
	switch (dataType)
	{
		case SINGLE:
			if (m_dataType == WEIGHTED_LIST)
				cleanData();
			m_dataType = SINGLE;
			m_dataSingle.ignoreY = file.read_bool8();
			m_dataSingle.x.loadFromIff(file);
			if (!m_dataSingle.ignoreY)
				m_dataSingle.y.loadFromIff(file);
			m_dataSingle.z.loadFromIff(file);
			m_dataSingle.radius.loadFromIff(file);
			m_loaded = true;
			break;
		case WEIGHTED_LIST:
			setValue(new WeightedList);
			loadWeightedListFromIff(file);
			break;
		case NONE:
			cleanData();
			break;
		case RANGE:
		case DIE_ROLL:
		default:
			DEBUG_FATAL(true, ("loaded unknown data type %d for template vector "
				"param", m_dataType));
			break;
	}
}	// VectorParam::loadFromIff

/**
 * Saves the param data to an iff file.
 *
 * @param file		the file to save to
 */
void VectorParam::saveToIff(Iff &file) const
{
	int8 type = static_cast<int8>(m_dataType);
	file.insertChunkData(&type, sizeof(type));
	switch (m_dataType)
	{
		case SINGLE:
			file.insertChunkData(&m_dataSingle.ignoreY, 
				sizeof(m_dataSingle.ignoreY));
			m_dataSingle.x.saveToIff(file);
			if (!m_dataSingle.ignoreY)
				m_dataSingle.y.saveToIff(file);
			m_dataSingle.z.saveToIff(file);
			m_dataSingle.radius.saveToIff(file);
			break;
		case WEIGHTED_LIST:
			saveWeightedListToIff(file);
			break;
		case NONE:
			break;
		case RANGE:
		case DIE_ROLL:
		default:
			DEBUG_FATAL(true, ("saving unknown data type %d for template vector "
				"param", m_dataType));
			break;
	}
}	// VectorParam::saveToIff


//========================================================================
// class StringIdParam

/**
 * Class constructor.
 */
StringIdParam::StringIdParam(void) : TemplateBase<StringIdParamData, 
	StringIdParamData>()
{
}	// StringIdParam::StringIdParam

/**
 * Class destructor.
 */
StringIdParam::~StringIdParam()
{
}	// StringIdParam::~StringIdParam

StringId StringIdParam::getValue(void) const
{
	if (m_dataType == SINGLE)
	{
		return StringId(m_dataSingle.table.getValue(), 
			m_dataSingle.index.getValue());
	}
	else if (m_dataType == WEIGHTED_LIST)
	{
		int weight = Random::random(1, 100);
		WeightedList::const_iterator end = m_data.weightedList->end();
		for (WeightedList::const_iterator iter = m_data.weightedList->begin();
			iter != end;
			++iter)
		{
			weight -= (*iter).weight;
			if (weight <= 0)
			{
				const StringIdParam *param = dynamic_cast<const StringIdParam *>
					((*iter).value);
				return param->getValue();
			}
		}
		DEBUG_FATAL(true, ("weighted list does not equal 100"));
	}
	DEBUG_FATAL(true, ("invalid type for string id param"));
	return StringId("", 0);
}	// StringIdParam::getValue

/**
 * Does cleanup for a single-value parameter.
 */
void StringIdParam::cleanSingleParam(void)
{
	m_dataSingle.table.cleanData();
	m_dataSingle.index.cleanData();
}	// StringIdParam::cleanSingleParam

/**
 * Loads the param data from an iff file.
 *
 * @param file		the file to load from
 */
void StringIdParam::loadFromIff(Iff &file)
{
	DataTypeId dataType = static_cast<DataTypeId>(file.read_int8());
	switch (dataType)
	{
		case SINGLE:
			{
				StringIdParamData data;
				data.table.loadFromIff(file);
				data.index.loadFromIff(file);
				setValue(data);
				m_loaded = true;
			}
			break;
		case WEIGHTED_LIST:
			setValue(new WeightedList);
			loadWeightedListFromIff(file);
			break;
		case NONE:
			cleanData();
			break;
		case RANGE:
		case DIE_ROLL:
		default:
			DEBUG_FATAL(true, ("loaded unknown data type %d for template stringId "
				"param", m_dataType));
			break;
	}
}	// StringIdParam::loadFromIff

/**
 * Saves the param data to an iff file.
 *
 * @param file		the file to save to
 */
void StringIdParam::saveToIff(Iff &file) const
{
	int8 type = static_cast<int8>(m_dataType);
	file.insertChunkData(&type, sizeof(type));
	switch (m_dataType)
	{
		case SINGLE:
			m_dataSingle.table.saveToIff(file);
			m_dataSingle.index.saveToIff(file);
			break;
		case WEIGHTED_LIST:
			saveWeightedListToIff(file);
			break;
		case NONE:
			break;
		case RANGE:
		case DIE_ROLL:
		default:
			DEBUG_FATAL(true, ("saving unknown data type %d for template stringId "
				"param", m_dataType));
			break;
	}
}	// StringIdParam::saveToIff


//========================================================================
// class TriggerVolumeParam

/**
 * Class constructor.
 */
TriggerVolumeParam::TriggerVolumeParam(void) : TemplateBase<TriggerVolumeParamData, 
	TriggerVolumeParamData>()
{
}	// TriggerVolumeParam::TriggerVolumeParam

/**
 * Class destructor.
 */
TriggerVolumeParam::~TriggerVolumeParam()
{
}	// TriggerVolumeParam::~TriggerVolumeParam

TriggerVolumeData TriggerVolumeParam::getValue(void) const
{
	if (m_dataType == SINGLE)
	{
		return TriggerVolumeData(m_dataSingle.name.getValue(), 
			m_dataSingle.radius.getValue());
	}
	else if (m_dataType == WEIGHTED_LIST)
	{
		int weight = Random::random(1, 100);
		WeightedList::const_iterator end = m_data.weightedList->end();
		for (WeightedList::const_iterator iter = m_data.weightedList->begin();
			iter != end;
			++iter)
		{
			weight -= (*iter).weight;
			if (weight <= 0)
			{
				const TriggerVolumeParam *param = dynamic_cast<
					const TriggerVolumeParam *>((*iter).value);
				return param->getValue();
			}
		}
		DEBUG_FATAL(true, ("weighted list does not equal 100"));
	}
	DEBUG_FATAL(true, ("invalid type for trigger volume param"));
	return TriggerVolumeData("", 0);
}	// TriggerVolumeParam::getValue

/**
 * Does cleanup for a single-value parameter.
 */
void TriggerVolumeParam::cleanSingleParam(void)
{
	m_dataSingle.name.cleanData();
	m_dataSingle.radius.cleanData();
}	// TriggerVolumeParam::cleanSingleParam

/**
 * Loads the param data from an iff file.
 *
 * @param file		the file to load from
 */
void TriggerVolumeParam::loadFromIff(Iff &file)
{
	DataTypeId dataType = static_cast<DataTypeId>(file.read_int8());
	switch (dataType)
	{
		case SINGLE:
			{
				TriggerVolumeParamData data;
				data.name.loadFromIff(file);
				data.radius.loadFromIff(file);
				setValue(data);
				m_loaded = true;
			}
			break;
		case WEIGHTED_LIST:
			setValue(new WeightedList);
			loadWeightedListFromIff(file);
			break;
		case NONE:
			cleanData();
			break;
		case RANGE:
		case DIE_ROLL:
		default:
			DEBUG_FATAL(true, ("loaded unknown data type %d for template "
				"triggerVolume param", m_dataType));
			break;
	}
}	// TriggerVolumeParam::loadFromIff

/**
 * Saves the param data to an iff file.
 *
 * @param file		the file to save to
 */
void TriggerVolumeParam::saveToIff(Iff &file) const
{
	int8 type = static_cast<int8>(m_dataType);
	file.insertChunkData(&type, sizeof(type));
	switch (m_dataType)
	{
		case SINGLE:
			m_dataSingle.name.saveToIff(file);
			m_dataSingle.radius.saveToIff(file);
			break;
		case WEIGHTED_LIST:
			saveWeightedListToIff(file);
			break;
		case NONE:
			break;
		case RANGE:
		case DIE_ROLL:
		default:
			DEBUG_FATAL(true, ("saving unknown data type %d for template "
				"triggerVolume param", m_dataType));
			break;
	}
}	// TriggerVolumeParam::saveToIff


//========================================================================
// class DynamicVariableParamData

/**
 * Class constructor.
 *
 * @param name		name of the dynamic variable
 * @param type		type of the dynamic variable
 */
DynamicVariableParamData::DynamicVariableParamData(const std::string &name, 
	DynamicVariableParamData::DataType type) :
	m_name(name),
	m_type(UNKNOWN)
{
	switch (type)
	{
		case INTEGER:
			m_data.iparam = new IntegerParam();
			NOT_NULL(m_data.iparam);
			break;
		case FLOAT:
			m_data.fparam = new FloatParam();
			NOT_NULL(m_data.fparam);
			break;
		case STRING:
			m_data.sparam = new StringParam();
			NOT_NULL(m_data.sparam);
			break;
		case LIST:
			m_data.lparam = new std::vector<DynamicVariableParamData *>();
			NOT_NULL(m_data.lparam);
			break;
		default:
			DEBUG_FATAL(true, ("DynamicVariableParamData constructed with bad type"));
			break;
	}
	m_type = type;
}	// DynamicVariableParamData::DynamicVariableParamData

/**
 * Class destructor.
 */
DynamicVariableParamData::~DynamicVariableParamData()
{
	switch (m_type)
	{
		case DynamicVariableParamData::INTEGER:
			delete m_data.iparam;
			break;
		case DynamicVariableParamData::FLOAT:
			delete m_data.fparam;
			break;
		case DynamicVariableParamData::STRING:
			delete m_data.sparam;
			break;
		case DynamicVariableParamData::LIST:
			{
				int count = m_data.lparam->size();
				for (int i = 0; i < count; ++i)
				{
					DynamicVariableParamData *temp = m_data.lparam->at(i);
					m_data.lparam->at(i) = nullptr;
					delete temp;
				}
				delete m_data.lparam;
			}
			break;
		case DynamicVariableParamData::UNKNOWN:
		default:
			break;
	}
	m_type = DynamicVariableParamData::UNKNOWN;
}	// DynamicVariableParam::~DynamicVariableParam

/**
 * Loads the param data from an iff file.
 *
 * @param file		the file to load from
 */
void DynamicVariableParamData::loadFromIff(Iff &file)
{
	file.read_string(m_name);
	m_type = static_cast<DynamicVariableParamData::DataType>(file.read_int8());
	switch (m_type)
	{
		case INTEGER:
			m_data.iparam = new IntegerParam;
			NOT_NULL(m_data.iparam);
			m_data.iparam->loadFromIff(file);
			break;
		case FLOAT:
			m_data.fparam = new FloatParam;
			NOT_NULL(m_data.fparam);
			m_data.fparam->loadFromIff(file);
			break;
		case STRING:
			m_data.sparam = new StringParam;
			NOT_NULL(m_data.sparam);
			m_data.sparam->loadFromIff(file);
			break;
		case LIST:
			{
				m_data.lparam = new std::vector<DynamicVariableParamData *>;
				NOT_NULL(m_data.lparam);
				int32 count = file.read_int32();
				for (int i = 0; i < count; ++i)
				{
					DynamicVariableParamData *newData = new DynamicVariableParamData;
					newData->loadFromIff(file);
					m_data.lparam->push_back(newData);
				}
			}
			break;
		case UNKNOWN:
		default:
			DEBUG_FATAL(true, ("read unknown DynamicVariable type %d for template DynamicVariable "
				"param", m_type));
			break;
	}
}	// DynamicVariableParamData::loadFromIff

/**
 * Saves the param data to an iff file.
 *
 * @param file		the file to save to
 */
void DynamicVariableParamData::saveToIff(Iff &file) const
{
int8 data8;

	file.insertChunkString(m_name.c_str());
	data8 = static_cast<int8>(m_type);	
	file.insertChunkData(&data8, sizeof(data8));
	switch (m_type)
	{
		case INTEGER:
			NOT_NULL(m_data.iparam);
			m_data.iparam->saveToIff(file);
			break;
		case FLOAT:
			NOT_NULL(m_data.fparam);
			m_data.fparam->saveToIff(file);
			break;
		case STRING:
			NOT_NULL(m_data.sparam);
			m_data.sparam->saveToIff(file);
			break;
		case LIST:
			{
				NOT_NULL(m_data.lparam);
				int32 count = m_data.lparam->size();
				file.insertChunkData(&count, sizeof(count));
				for (int i = 0; i < count; ++i)
				{
					const DynamicVariableParamData *temp = m_data.lparam->at(i);
					NOT_NULL(temp);
					temp->saveToIff(file);
				}
			}
			break;
		case UNKNOWN:
		default:
			DEBUG_FATAL(true, ("write unknown DynamicVariable type %d for template DynamicVariable "
				"param", m_type));
			break;
	}
}	// DynamicVariableParamData::saveToIff

/**
 * Fills an DynamicVariable list with data from this param.
 *
 * @param list		the list to fill
 */
void DynamicVariableParamData::getDynamicVariableList(DynamicVariableList &list, const DynamicVariableList::NestedList &context) const
{
	DEBUG_FATAL(m_type != LIST, ("trying to load DynamicVariable list with non-list param"));
	
	int count = m_data.lparam->size();
	for (int i = 0; i < count; ++i)
	{
		const DynamicVariableParamData *item = m_data.lparam->at(i);
		NOT_NULL(item);
		switch (item->m_type)
		{
			case INTEGER:
				NOT_NULL(item->m_data.iparam);
				list.setItem(context.getContextName()+item->m_name, item->m_data.iparam->getValue());
				break;
			case FLOAT:
				NOT_NULL(item->m_data.fparam);
				list.setItem(context.getContextName()+item->m_name, item->m_data.fparam->getValue());
				break;
			case STRING:
				NOT_NULL(item->m_data.sparam);
				list.setItem(context.getContextName()+item->m_name, Unicode::utf8ToWide(
					item->m_data.sparam->getValue()));
				break;
			case LIST:
				{
					NOT_NULL(item->m_data.lparam);
					DynamicVariableList::NestedList subList(context, item->m_name);
					item->getDynamicVariableList(list,subList);
				}
				break;
			case UNKNOWN:
			default:
				DEBUG_FATAL(true, ("unknown data type in DynamicVariableParamData::"
					"getDynamicVariableList"));
				break;			
		}
	}
}	// DynamicVariableParamData::getDynamicVariableList


//========================================================================
// class DynamicVariableParam

/**
 * Class constructor.
 */
DynamicVariableParam::DynamicVariableParam(void) : 
	TemplateBase<DynamicVariableParamData, const DynamicVariableParamData &>(),
	m_extendBaseList(false)
{
	m_dataType = SINGLE;
}	// DynamicVariableParam::DynamicVariableParam

/**
 * Class destructor.
 */
DynamicVariableParam::~DynamicVariableParam()
{
}	// DynamicVariableParam::~DynamicVariableParam

/**
 * Does cleanup for a single-value parameter.
 */
void DynamicVariableParam::cleanSingleParam(void)
{
	m_dataSingle.m_name = "";
	switch (m_dataSingle.m_type)
	{
		case DynamicVariableParamData::INTEGER:
			delete m_dataSingle.m_data.iparam;
			m_dataSingle.m_data.iparam = nullptr;
			break;
		case DynamicVariableParamData::FLOAT:
			delete m_dataSingle.m_data.fparam;
			m_dataSingle.m_data.fparam = nullptr;
			break;
		case DynamicVariableParamData::STRING:
			delete m_dataSingle.m_data.sparam;
			m_dataSingle.m_data.sparam = nullptr;
			break;
		case DynamicVariableParamData::LIST:
			{
				std::vector<DynamicVariableParamData *>::iterator iter;
				for (iter = m_dataSingle.m_data.lparam->begin();
					iter != m_dataSingle.m_data.lparam->end();
					++iter)
				{
					delete *iter;
					*iter = nullptr;
				}
			}
			delete m_dataSingle.m_data.lparam;
			m_dataSingle.m_data.lparam = nullptr;
			break;
		case DynamicVariableParamData::UNKNOWN:
		default:
			break;
	}
	m_dataSingle.m_type = DynamicVariableParamData::UNKNOWN;
}	// DynamicVariableParam::cleanSingleParam

/**
 * Loads the param data from an iff file.
 *
 * @param file		the file to load from
 */
void DynamicVariableParam::loadFromIff(Iff &file)
{
	m_extendBaseList = file.read_bool8();

	DataTypeId dataType = static_cast<DataTypeId>(file.read_int8());

	if (dataType == NONE)	// empty list
	{
		cleanData();
		m_loaded = true;
		return;
	}

	m_dataType = dataType;
	switch (m_dataType)
	{
		case SINGLE:
			m_dataSingle.loadFromIff(file);
			m_loaded = true;
			break;
		case WEIGHTED_LIST:
			setValue(new WeightedList);
			loadWeightedListFromIff(file);
			break;
		case NONE:
			break;
		case RANGE:
		case DIE_ROLL:
		default:
			DEBUG_FATAL(true, ("loaded unknown data type %d for template "
				"DynamicVariable param", m_dataType));
			break;
	}
}	// DynamicVariableParam::loadFromIff

/**
 * Saves the param data to an iff file.
 *
 * @param file		the file to save to
 */
void DynamicVariableParam::saveToIff(Iff &file) const
{
	// check for empty list
	if (m_dataType == SINGLE && 
		m_dataSingle.m_type == DynamicVariableParamData::UNKNOWN)
	{
		// save as type NONE, extending the base template
		bool extendBaseList = true;
		int8 type = NONE;
		file.insertChunkData(&extendBaseList, sizeof(bool));
		file.insertChunkData(&type, sizeof(type));
		return;
	}

	int8 type = static_cast<int8>(m_dataType);
	file.insertChunkData(&m_extendBaseList, sizeof(bool));
	file.insertChunkData(&type, sizeof(type));
	switch (m_dataType)
	{
		case SINGLE:
			m_dataSingle.saveToIff(file);
			break;
		case WEIGHTED_LIST:
			saveWeightedListToIff(file);
			break;
		case NONE:
			break;
		case RANGE:
		case DIE_ROLL:
		default:
			DEBUG_FATAL(m_dataType != SINGLE, ("trying to save bad template "
				"DynamicVariable param type %d", m_dataType));
			break;
	}
}	// DynamicVariableParam::saveToIff

/**
 * Fills an DynamicVariable list with data from this param.
 *
 * @param list		the list to fill
 */
void DynamicVariableParam::getDynamicVariableList(DynamicVariableList &list) const
{
	switch (m_dataType)
	{
		case SINGLE:
			m_dataSingle.getDynamicVariableList(list, DynamicVariableList::NestedList(list));
			break;
		case WEIGHTED_LIST:
			{
				int weight = Random::random(1, 100);
				WeightedList::const_iterator end = m_data.weightedList->end();
				WeightedList::const_iterator iter;
				for (iter = m_data.weightedList->begin(); iter != end; ++iter)
				{
					weight -= (*iter).weight;
					if (weight <= 0)
					{
						dynamic_cast<const DynamicVariableParam *>
							((*iter).value)->getDynamicVariableList(list);
						break;
					}
				}
				if (iter == end)
					DEBUG_FATAL(true, ("weighted list does not equal 100"));
			}
			break;
		case NONE:
			break;
		case RANGE:
		case DIE_ROLL:
		default:
			DEBUG_FATAL(true, ("Unknown data type %d for template param", m_dataType));
			break;
	}
}	// DynamicVariableParam::getDynamicVariableList


//========================================================================
