//========================================================================
//
// TemplateParameter.h - base and classes for template parameters
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef _INCLUDED_TemplateParameter_H
#define _INCLUDED_TemplateParameter_H

#include "sharedFile/Iff.h"
#include "sharedFoundation/DataResourceList.h"
#include "sharedFoundation/DynamicVariableList.h"
#include "sharedFoundation/Tag.h"
#include "sharedRandom/Random.h"
#include "StringId.h"

#include <string>
#include <vector>

class Vector;

//========================================================================
template <class DataType, class ReturnType>
class TemplateBase
{
public:
	struct WeightedValue
	{
		TemplateBase<DataType, ReturnType> *value;
		int weight;
	};
	typedef std::vector<WeightedValue> WeightedList;

	struct Range
	{
		DataType min_value;
		DataType max_value;

		Range(void) {}
		Range(const Range &s) :
			min_value(s.min_value),
			max_value(s.max_value) {}
		Range & operator=(const Range &s)
		{
			min_value = s.min_value;
			max_value = s.max_value;
			return *this;
		}
	};

	struct DieRoll
	{
		DataType num_dice;
		DataType die_sides;
		DataType base;

		DieRoll(void) {}
		DieRoll(const DieRoll &s) :
			num_dice(s.num_dice),
			die_sides(s.die_sides),
			base(s.base) {}
		DieRoll & operator =(const DieRoll &s)
		{
			num_dice = s.num_dice;
			die_sides = s.die_sides;
			base = s.base;
			return *this;
		}
	};

	enum DataTypeId
	{
		NONE,
		SINGLE,
		WEIGHTED_LIST,
		RANGE,              // only applies to numerical types
		DIE_ROLL            // only applies to integral types
	};

	DataTypeId   getType(void) const;

public:

	void cleanData(void);

	virtual void loadFromIff(Iff &file) = 0;
	virtual void saveToIff(Iff &file)  const = 0;

	ReturnType getValue(void) const;

	const Range * getRawRange(void) const;
	const DieRoll * getRawDieRoll(void) const;
	const WeightedList * getRawWeightedList(void) const;

	void setValue(const DataType & value);
	void setValue(WeightedList *list);

	bool isLoaded(void) const;

protected:

	DataType   m_dataSingle;			// storage for simple-type data
	DataTypeId m_dataType;              // the type of data for this param
	union
	{
		Range *range;
		DieRoll *dieRoll;
		WeightedList *weightedList;
	} m_data;							// storage for complex-type data
	bool m_loaded;						// flag that this parameter has been loaded

	TemplateBase(void);
	TemplateBase(const TemplateBase<DataType, ReturnType> &);
	TemplateBase<DataType, ReturnType> & operator =(const TemplateBase<DataType, ReturnType> &);
	virtual ~TemplateBase();

	virtual void              cleanSingleParam(void);
	void              loadWeightedListFromIff(Iff &file);
	void              saveWeightedListToIff(Iff &file) const;

	virtual TemplateBase<DataType, ReturnType> * createNewParam(void) = 0;

	virtual ReturnType   getSingle(void) const;
	virtual ReturnType   getRange(void) const;
	virtual ReturnType   getDieRoll(void) const;
	void         setValue(const DataType & min_value, const DataType & max_value);
	void         setValue(const DataType & num_dice, const DataType & die_sides, const DataType & base);
};

template <class DataType, class ReturnType>
inline TemplateBase<DataType, ReturnType>::TemplateBase(void) :
	m_dataType(NONE),
	m_loaded(false),
	m_data()
{
}	// TemplateBase::TemplateBase(void)

template <class DataType, class ReturnType>
inline TemplateBase<DataType, ReturnType>::TemplateBase(const TemplateBase<
	DataType, ReturnType> & source) : m_loaded(false)
{
	m_dataType = source.m_dataType;
	switch (m_dataType)
	{
	case SINGLE:
		m_dataSingle = source.m_dataSingle;
		break;
	case WEIGHTED_LIST:
		m_data.weightedList = new WeightedList(*source.m_data.weightedList);
		break;
	case RANGE:
		m_data.range = new Range(*source.m_data.range);
		break;
	case DIE_ROLL:
		m_data.dieRoll = new DieRoll(*source.m_data.dieRoll);
		break;
	case NONE:
	default:
		break;
	}
}	// TemplateBase::TemplateBase(const TemplateBase &)

template <class DataType, class ReturnType>
inline TemplateBase<DataType, ReturnType> & TemplateBase<DataType, ReturnType>::
operator =(const TemplateBase<DataType, ReturnType> &source)
{
	cleanData();
	m_dataType = source.m_dataType;
	switch (m_dataType)
	{
	case SINGLE:
		m_dataSingle = source.m_dataSingle;
		break;
	case WEIGHTED_LIST:
		m_data.weightedList = new WeightedList(*source.m_data.weightedList);
		break;
	case RANGE:
		m_data.range = new Range(*source.m_data.range);
		break;
	case DIE_ROLL:
		m_data.dieRoll = new DieRoll(*source.m_data.dieRoll);
		break;
	case NONE:
	default:
		break;
	}
	return *this;
}	// TemplateBase::operator =

/**
 * Frees up any memory assiciated with the paramater.
 */
template <class DataType, class ReturnType>
inline void TemplateBase<DataType, ReturnType>::cleanData(void)
{
	switch (m_dataType)
	{
	case SINGLE:
		cleanSingleParam();
		break;
	case WEIGHTED_LIST:
	{
		typename WeightedList::iterator end = m_data.weightedList->end();
		for (typename WeightedList::iterator iter = m_data.weightedList->begin();
			iter != end;
			++iter)
		{
			delete (*iter).value;
			(*iter).value = nullptr;
		}
		delete m_data.weightedList;
		m_data.weightedList = nullptr;
	}
	break;
	case RANGE:
		delete m_data.range;
		m_data.range = nullptr;
		break;
	case DIE_ROLL:
		delete m_data.dieRoll;
		m_data.dieRoll = nullptr;
		break;
	case NONE:
	default:
		break;
	}
	m_dataType = NONE;
	m_loaded = false;
}	// TemplateBase::cleanData

template <class DataType, class ReturnType>
inline TemplateBase<DataType, ReturnType>::~TemplateBase()
{
	cleanData();
}	// TemplateBase::~TemplateBase()

template <class DataType, class ReturnType>
inline bool TemplateBase<DataType, ReturnType>::isLoaded(void) const
{
	return m_loaded;
}	// TemplateBase::isLoaded

template <class DataType, class ReturnType>
inline ReturnType TemplateBase<DataType, ReturnType>::getValue(void) const
{
	static DataType dummyReturn;

	switch (m_dataType)
	{
	case SINGLE:
		return getSingle();
	case WEIGHTED_LIST:
	{
		int weight = Random::random(1, 100);
		typename WeightedList::const_iterator end = m_data.weightedList->end();
		for (typename WeightedList::const_iterator iter = m_data.weightedList->begin();
			iter != end;
			++iter)
		{
			weight -= (*iter).weight;
			if (weight <= 0)
			{
				return dynamic_cast<const TemplateBase<DataType, ReturnType> *>
					((*iter).value)->getValue();
			}
		}
		DEBUG_FATAL(true, ("weighted list does not equal 100"));
	}
	break;
	case RANGE:
		return getRange();
	case DIE_ROLL:
		return getDieRoll();
	case NONE:
	default:
		DEBUG_FATAL(true, ("Unknown data type %d for template param", m_dataType));
		break;
	}
	return dummyReturn;
}	// TemplateBase::getValue

template <class DataType, class ReturnType>
inline const typename TemplateBase<DataType, ReturnType>::Range * TemplateBase<DataType, ReturnType>::getRawRange(void) const
{
	if (m_dataType == RANGE)
		return m_data.range;
	return nullptr;
}

template <class DataType, class ReturnType>
inline const typename TemplateBase<DataType, ReturnType>::DieRoll * TemplateBase<DataType, ReturnType>::getRawDieRoll(void) const
{
	if (m_dataType == DIE_ROLL)
		return m_data.dieRoll;
	return nullptr;
}

template <class DataType, class ReturnType>
inline const typename TemplateBase<DataType, ReturnType>::WeightedList * TemplateBase<DataType, ReturnType>::getRawWeightedList(void) const
{
	if (m_dataType == WEIGHTED_LIST)
		return m_data.weightedList;
	return nullptr;
}

template <class DataType, class ReturnType>
inline typename TemplateBase<DataType, ReturnType>::DataTypeId TemplateBase<DataType, ReturnType>::getType(void) const
{
	return (m_dataType);
}

template <class DataType, class ReturnType>
inline ReturnType TemplateBase<DataType, ReturnType>::getSingle(void) const
{
	return m_dataSingle;
}	// TemplateBase::getSingle

template <class DataType, class ReturnType>
inline ReturnType TemplateBase<DataType, ReturnType>::getRange(void) const
{
	static DataType dummyReturn;

	DEBUG_FATAL(true, ("getRange not supported"));
	return dummyReturn;
}	// TemplateBase::getRange

template <class DataType, class ReturnType>
inline ReturnType TemplateBase<DataType, ReturnType>::getDieRoll(void) const
{
	static DataType dummyReturn;

	DEBUG_FATAL(true, ("getDieRoll not supported"));
	return dummyReturn;
}	// TemplateBase::getDieRoll

template <class DataType, class ReturnType>
inline void TemplateBase<DataType, ReturnType>::setValue(const DataType & value)
{
	cleanData();
	m_dataType = SINGLE;
	m_dataSingle = value;
	m_loaded = true;
}

template <class DataType, class ReturnType>
inline void TemplateBase<DataType, ReturnType>::setValue(const DataType & min_value,
	const DataType & max_value)
{
	cleanData();
	m_dataType = RANGE;
	m_data.range = new Range();
	m_data.range->min_value = min_value;
	m_data.range->max_value = max_value;
	m_loaded = true;
}

template <class DataType, class ReturnType>
inline void TemplateBase<DataType, ReturnType>::setValue(const DataType & num_dice,
	const DataType & die_sides, const DataType & base)
{
	cleanData();
	m_dataType = DIE_ROLL;
	m_data.dieRoll = new DieRoll;
	m_data.dieRoll->num_dice = num_dice;
	m_data.dieRoll->die_sides = die_sides;
	m_data.dieRoll->base = base;
	m_loaded = true;
}

// note that the parameter becomes responsible for deleting the list
template <class DataType, class ReturnType>
inline void TemplateBase<DataType, ReturnType>::setValue(WeightedList *list)
{
	NOT_NULL(list);
	cleanData();
	m_dataType = WEIGHTED_LIST;
	m_data.weightedList = list;
	m_loaded = true;
}

/**
 * Does cleanup for a single-value parameter.
 */
template <class DataType, class ReturnType>
inline void TemplateBase<DataType, ReturnType>::cleanSingleParam(void)
{
}	// TemplateBase::cleanSingleParam

/**
 * Loads a weighted list from an iff file.
 *
 * @param file		the file to load from
 */
template <class DataType, class ReturnType>
inline void TemplateBase<DataType, ReturnType>::loadWeightedListFromIff(Iff &file)
{
	WeightedValue weightedValue;

	NOT_NULL(m_data.weightedList);

	int32 count = file.read_int32();
	if (count <= 0)
		return;
	for (int i = 0; i < count; ++i)
	{
		weightedValue.weight = file.read_int32();
		weightedValue.value = createNewParam();
		weightedValue.value->loadFromIff(file);
		m_data.weightedList->push_back(weightedValue);
	}
	m_loaded = true;
}	// TemplateBase::loadWeightedListFromIff

/**
 * Saves a weighted list to an iff file.
 *
 * @param file		the file to save to
 */
template <class DataType, class ReturnType>
inline void TemplateBase<DataType, ReturnType>::saveWeightedListToIff(Iff &file) const
{
	int32 intData;

	NOT_NULL(m_data.weightedList);

	intData = static_cast<int32>(m_data.weightedList->size());
	file.insertChunkData(&intData, sizeof(intData));

	typename WeightedList::const_iterator end = m_data.weightedList->end();
	for (typename WeightedList::const_iterator iter = m_data.weightedList->begin();
		iter != end;
		++iter)
	{
		intData = static_cast<int32>((*iter).weight);
		file.insertChunkData(&intData, sizeof(intData));
		(*iter).value->saveToIff(file);
	}
}	// TemplateBase::saveWeightedListToIff

//========================================================================
// class IntegerParam

class IntegerParam : public TemplateBase<int, int>
{
public:
	IntegerParam(void);
	virtual ~IntegerParam();

	virtual void loadFromIff(Iff &file);
	virtual void saveToIff(Iff &file) const;

	char getDeltaType(void) const;
	void setDeltaType(char type);

	int getMinValue(void) const;
	int getMaxValue(void) const;

	void setValue(const int & value);
	void setValue(WeightedList *list);
	void setValue(const int & min_value, const int & max_value);
	void setValue(const int & num_dice, const int & die_sides, const int & base);

	const DieRoll * getDieRollStruct(void) const;
	const Range * getRangeStruct(void) const;

protected:
	virtual TemplateBase<int, int> * createNewParam(void);
	virtual int                      getRange(void) const;
	virtual int                      getDieRoll(void) const;

private:
	char m_dataDeltaType;		// if '+' or '-', this param is a delta on a
								// derived template param
};

inline char IntegerParam::getDeltaType(void) const
{
	return m_dataDeltaType;
}

inline void IntegerParam::setDeltaType(char type)
{
	m_dataDeltaType = type;
}

inline TemplateBase<int, int> *IntegerParam::createNewParam(void)
{
	return new IntegerParam;
}

inline int IntegerParam::getRange(void) const
{
	DEBUG_FATAL(m_dataType != RANGE, ("getRange on non-range integer param"));
	NOT_NULL(m_data.range);

	return Random::random(m_data.range->min_value, m_data.range->max_value);
}

inline int IntegerParam::getDieRoll(void) const
{
	DEBUG_FATAL(m_dataType != DIE_ROLL, ("getDieRoll on non-die integer param"));
	NOT_NULL(m_data.dieRoll);

	int result = m_data.dieRoll->base;
	for (int i = 0; i < m_data.dieRoll->num_dice; ++i)
		result += Random::random(1, m_data.dieRoll->die_sides);
	return result;
}

inline void IntegerParam::setValue(const int & value)
{
	TemplateBase<int, int>::setValue(value);
}

inline void IntegerParam::setValue(WeightedList *list)
{
	TemplateBase<int, int>::setValue(list);
}

inline void IntegerParam::setValue(const int & min_value, const int & max_value)
{
	TemplateBase<int, int>::setValue(min_value, max_value);
}

inline void IntegerParam::setValue(const int & num_dice, const int & die_sides, const int & base)
{
	TemplateBase<int, int>::setValue(num_dice, die_sides, base);
}

inline const IntegerParam::DieRoll * IntegerParam::getDieRollStruct(void) const
{
	if (m_dataType == DIE_ROLL)
	{
		return m_data.dieRoll;
	}
	else
	{
		return nullptr;
	}
}

inline const IntegerParam::Range * IntegerParam::getRangeStruct(void) const
{
	if (m_dataType == RANGE)
	{
		return m_data.range;
	}
	else
	{
		return nullptr;
	}
}

//========================================================================
// class FloatParam

class FloatParam : public TemplateBase<float, float>
{
public:
	FloatParam(void);
	virtual ~FloatParam();

	virtual void loadFromIff(Iff &file);
	virtual void saveToIff(Iff &file) const;

	char getDeltaType(void) const;
	void setDeltaType(char type);

	float getMinValue(void) const;
	float getMaxValue(void) const;

	void setValue(const float & value);
	void setValue(WeightedList *list);
	void setValue(const float & min_value, const float & max_value);

	const Range * getRangeStruct(void) const;

protected:
	virtual TemplateBase<float, float> * createNewParam(void);
	virtual float                        getRange(void) const;

private:
	char m_dataDeltaType;		// if '+' or '-', this param is a delta on a
								// derived template param
};

inline char FloatParam::getDeltaType(void) const
{
	return m_dataDeltaType;
}

inline void FloatParam::setDeltaType(char type)
{
	m_dataDeltaType = type;
}

inline TemplateBase<float, float> *FloatParam::createNewParam(void)
{
	return new FloatParam;
}

inline float FloatParam::getRange(void) const
{
	DEBUG_FATAL(m_dataType != RANGE, ("getRange on non-range float param"));
	NOT_NULL(m_data.range);

	return Random::randomReal(m_data.range->min_value, m_data.range->max_value);
}

inline void FloatParam::setValue(const float & value)
{
	TemplateBase<float, float>::setValue(value);
}

inline void FloatParam::setValue(WeightedList *list)
{
	TemplateBase<float, float>::setValue(list);
}

inline void FloatParam::setValue(const float & min_value, const float & max_value)
{
	TemplateBase<float, float>::setValue(min_value, max_value);
}

inline const FloatParam::Range * FloatParam::getRangeStruct() const
{
	if (m_dataType == RANGE)
	{
		return m_data.range;
	}
	else
	{
		return nullptr;
	}
}

//========================================================================
// class BoolParam

class BoolParam : public TemplateBase<bool, bool>
{
public:
	BoolParam(void);
	virtual ~BoolParam();

	virtual void loadFromIff(Iff &file);
	virtual void saveToIff(Iff &file) const;

protected:
	virtual TemplateBase<bool, bool> * createNewParam(void);
};

inline TemplateBase<bool, bool> *BoolParam::createNewParam(void)
{
	return new BoolParam;
}

//========================================================================
//

class StringParam : public TemplateBase<std::string, const std::string &>
{
public:
	StringParam(void);
	virtual ~StringParam();

	virtual void cleanSingleParam(void);

	virtual void loadFromIff(Iff &file);
	virtual void saveToIff(Iff &file) const;

protected:
	virtual TemplateBase<std::string, const std::string &> *createNewParam(void);
};

inline TemplateBase<std::string, const std::string &> *StringParam::createNewParam(void)
{
	return new StringParam;
}

//========================================================================
// class VectorParam

struct VectorParamData
{
	bool ignoreY;
	FloatParam x;
	FloatParam y;
	FloatParam z;
	FloatParam radius;

	void adjustVector(Vector &pos) const;
};

class VectorParam : public TemplateBase<VectorParamData, const VectorParamData &>
{
public:
	VectorParam(void);
	virtual ~VectorParam();

	virtual void cleanSingleParam(void);

	virtual void loadFromIff(Iff &file);
	virtual void saveToIff(Iff &file) const;

protected:
	virtual TemplateBase<VectorParamData, const VectorParamData &> *createNewParam(void);
};

inline TemplateBase<VectorParamData, const VectorParamData &> *VectorParam::createNewParam(void)
{
	return new VectorParam;
}

//========================================================================
// class StringId param

struct StringIdParamData
{
	StringParam table;
	StringParam index;

	StringIdParamData(void) : table(), index() {}
};

class StringIdParam : public TemplateBase<StringIdParamData, StringIdParamData>
{
public:
	StringIdParam(void);
	virtual ~StringIdParam();

	virtual void cleanSingleParam(void);

	virtual void loadFromIff(Iff &file);
	virtual void saveToIff(Iff &file) const;

	StringId getValue(void) const;

protected:
	virtual TemplateBase<StringIdParamData, StringIdParamData> *createNewParam(void);
};

inline TemplateBase<StringIdParamData, StringIdParamData> *StringIdParam::createNewParam(void)
{
	return new StringIdParam;
}

//========================================================================
// class TriggerVolume param

class TriggerVolumeData
{
	friend class TriggerVolumeParam;
public:
	TriggerVolumeData(void) : m_name(nullptr), m_radius(0.0f) {}
	TriggerVolumeData(const std::string & name, float radius) : m_name(&name), m_radius(radius) {}

	const std::string & getName(void) const;
	float               getRadius(void) const;

private:
	const std::string *m_name;
	float m_radius;
};

inline const std::string & TriggerVolumeData::getName(void) const
{
	NOT_NULL(m_name);
	return *m_name;
}	// TriggerVolumeData::getName

inline float TriggerVolumeData::getRadius(void) const
{
	return m_radius;
}	// TriggerVolumeData::getRadius

struct TriggerVolumeParamData
{
	StringParam name;
	FloatParam radius;

	TriggerVolumeParamData(void) : name(), radius() {}

private:
	// no copying
//	TriggerVolumeParamData(const TriggerVolumeParamData &);
//	TriggerVolumeParamData & operator =(const TriggerVolumeParamData &);
};

class TriggerVolumeParam : public TemplateBase<TriggerVolumeParamData,
	TriggerVolumeParamData>
{
public:
	TriggerVolumeParam(void);
	virtual ~TriggerVolumeParam();

	virtual void cleanSingleParam(void);

	virtual void loadFromIff(Iff &file);
	virtual void saveToIff(Iff &file) const;

	TriggerVolumeData getValue(void) const;

protected:
	virtual TemplateBase<TriggerVolumeParamData, TriggerVolumeParamData> *createNewParam(void);
};

inline TemplateBase<TriggerVolumeParamData, TriggerVolumeParamData> *TriggerVolumeParam::createNewParam(void)
{
	return new TriggerVolumeParam;
}

//========================================================================
// class DynamicVariableParamData - used by class DynamicVariableParam

class DynamicVariableParam;
class DynamicVariableList;

class DynamicVariableParamData
{
public:
	std::string m_name;
	enum DataType
	{
		UNKNOWN,
		INTEGER,
		FLOAT,
		STRING,
		LIST
	} m_type;
	union
	{
		IntegerParam *iparam;
		FloatParam *fparam;
		StringParam *sparam;
		std::vector<DynamicVariableParamData *> *lparam;
	} m_data;

	DynamicVariableParamData(void);
	DynamicVariableParamData(const std::string &name, DataType type);
	virtual ~DynamicVariableParamData();
	void loadFromIff(Iff &file);
	void saveToIff(Iff &file) const;
	void getDynamicVariableList(DynamicVariableList &list, const DynamicVariableList::NestedList &context) const;

private:
	DynamicVariableParamData(const DynamicVariableParamData &);
	DynamicVariableParamData & operator =(const DynamicVariableParamData &);
};

inline DynamicVariableParamData::DynamicVariableParamData(void) :
	m_name(),
	m_type(UNKNOWN)
{
	memset(&m_data, 0, sizeof(m_data));
}

//========================================================================
//

class DynamicVariableParam : public TemplateBase<DynamicVariableParamData, const DynamicVariableParamData &>
{
public:
	DynamicVariableParam(void);
	virtual ~DynamicVariableParam();

	virtual void cleanSingleParam(void);

	virtual void loadFromIff(Iff &file);
	virtual void saveToIff(Iff &file) const;

	void setExtendingBaseList(bool flag);
	bool isExtendingBaseList(void) const;
	void setIsLoaded(void);

	void getDynamicVariableList(DynamicVariableList &list) const;

protected:
	virtual TemplateBase<DynamicVariableParamData, const DynamicVariableParamData &> *createNewParam(void);

private:
	bool m_extendBaseList;

	DynamicVariableParam(const DynamicVariableParam &);
	DynamicVariableParam & operator =(const DynamicVariableParam &);
};

inline TemplateBase<DynamicVariableParamData, const DynamicVariableParamData &> *
DynamicVariableParam::createNewParam(void)
{
	return new DynamicVariableParam();
}

inline void DynamicVariableParam::setExtendingBaseList(bool flag)
{
	m_extendBaseList = flag;
}

inline bool DynamicVariableParam::isExtendingBaseList(void) const
{
	return m_extendBaseList;
}

inline void DynamicVariableParam::setIsLoaded(void)
{
	m_loaded = true;
}

//========================================================================
//

template <class SP>
class StructParam : public TemplateBase<SP *, SP *>
{
public:
	StructParam(void);
	virtual ~StructParam();

	bool isInitialized(void) const;

	virtual void cleanSingleParam(void);

	virtual void loadFromIff(Iff &file);
	virtual void saveToIff(Iff &file) const;

protected:
	virtual TemplateBase<SP *, SP *> *createNewParam(void);
};

template <class SP>
inline bool StructParam<SP>::isInitialized(void) const
{
	return (this->m_dataType != TemplateBase<SP *, SP *>::NONE);
}	// StructParam<SP>::isInitialized

template <class SP>
inline TemplateBase<SP *, SP *> *StructParam<SP>::createNewParam(void)
{
	return new StructParam;
}	// StructParam<SP>::createNewParam

/**
 * Class constructor.
 */
template <class SP>
inline StructParam<SP>::StructParam(void) : TemplateBase<SP *, SP *>()
{
	this->m_dataSingle = nullptr;
}	// StructParam<SP>::StructParam

/**
 * Class destructor.
 */
template <class SP>
inline StructParam<SP>::~StructParam()
{
	if (this->m_dataType == TemplateBase<SP *, SP *>::SINGLE)
	{
		delete this->m_dataSingle;
		this->m_dataSingle = nullptr;
		this->m_dataType = TemplateBase<SP *, SP *>::NONE;
	}
}	// StructParam<SP>::~StructParam

/**
 * Does cleanup for a single-value parameter.
 */
template <class SP>
inline void StructParam<SP>::cleanSingleParam(void)
{
	delete this->m_dataSingle;
	this->m_dataSingle = nullptr;
}	// StructParam<SP>::cleanSingleParam

/**
 * Loads the param data from an iff file.
 *
 * @param file		the file to load from
 */
template <class SP>
inline void StructParam<SP>::loadFromIff(Iff &file)
{
	typename StructParam<SP>::DataTypeId dataType = static_cast<typename StructParam<SP>::DataTypeId>(file.read_int8());
	switch (dataType)
	{
	case TemplateBase<SP *, SP *>::SINGLE:
	{
		Tag id = file.read_int32();
		SP * structTemplate = DataResourceList<SP>::fetch(id);
		NOT_NULL(structTemplate);
		// we need to exit the chunk because the iff class doesn't
		// support chunk nesting
		file.exitChunk();
		structTemplate->loadFromIff(file);
		// we need to enter a fake chunk because whoever called this
		// function assumes we are still in a chunk
		file.enterChunk();
		this->setValue(structTemplate);
		this->m_loaded = true;
	}
	break;
	case TemplateBase<SP *, SP *>::WEIGHTED_LIST:
		this->setValue(new typename TemplateBase<SP *, SP *>::WeightedList);
		this->loadWeightedListFromIff(file);
		break;
	case TemplateBase<SP *, SP *>::NONE:
		this->cleanData();
		break;
	case TemplateBase<SP *, SP *>::RANGE:
	case TemplateBase<SP *, SP *>::DIE_ROLL:
	default:
		DEBUG_FATAL(true, ("loaded unknown data type %d for template struct param", this->m_dataType));
		break;
	}
}	// StructParam<SP>::loadFromIff

/**
 * Saves the param data to an iff file.
 *
 * @param file		the file to save to
 */
template <class SP>
inline void StructParam<SP>::saveToIff(Iff &file) const
{
	int8 type = static_cast<int8>(this->m_dataType);
	file.insertChunkData(&type, sizeof(type));
	switch (this->m_dataType)
	{
	case TemplateBase<SP *, SP *>::SINGLE:
	{
		int32 tag = this->m_dataSingle->getId();
		file.insertChunkData(&tag, sizeof(tag));
		// we need to exit the chunk because the iff class doesn't
		// support chunk nesting
		file.exitChunk();
		this->m_dataSingle->saveToIff(file);
		// we need to insert a fake chunk because whoever called this
		// function assumes we are still in a chunk
		file.insertChunk(TAG(X, X, X, X));
	}
	break;
	case TemplateBase<SP *, SP *>::WEIGHTED_LIST:
		this->saveWeightedListToIff(file);
		break;
	case TemplateBase<SP *, SP *>::NONE:
		break;
	case TemplateBase<SP *, SP *>::RANGE:
	case TemplateBase<SP *, SP *>::DIE_ROLL:
	default:
		DEBUG_FATAL(true, ("saving unknown data type %d for template struct param", this->m_dataType));
		break;
	}
}	// StructParam<SP>::saveToIff

//========================================================================

#endif	// _INCLUDED_TemplateParameter_H
