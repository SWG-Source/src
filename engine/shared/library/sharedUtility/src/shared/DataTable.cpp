// ======================================================================
//
// DataTable.cpp
// 
// copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedUtility/FirstSharedUtility.h"
#include "sharedUtility/DataTable.h"

#include "sharedFile/Iff.h"
#include "sharedUtility/DataTableCell.h"

#include <map>
#include <unordered_map>

//----------------------------------------------------------------------------

const Tag DataTable::m_dataTableIffId = TAG(D,T,I,I);

//----------------------------------------------------------------------------

DataTable::DataTable() :
m_numRows(0),
m_numCols(0),
m_cells(0),
m_columns(),
m_index(),
m_types(),
m_columnIndexMap(new ColumnIndexMap(17)),
m_name()
{
}

//----------------------------------------------------------------------------

DataTable::~DataTable()
{
	std::vector<void *>::iterator k = m_index.begin();
	int count = 0;
	for (; k != m_index.end(); ++k, ++count)
	{
		DataTableColumnType::DataType dt = m_types[static_cast<size_t>(count)]->getBasicType();

		switch (dt)
		{
			case DataTableColumnType::DT_Int:
			{
				if (*k)
				{
					delete static_cast<std::multimap<int, int> *>(*k);
					*k = nullptr;
				}
			}
			break;
			case DataTableColumnType::DT_Float:
			{
				if (*k)
				{
					delete static_cast<std::multimap<float, int> *>(*k);
					*k = nullptr;
				}

			}
			break;
			case DataTableColumnType::DT_String:
			{
				if (*k)
				{
					delete static_cast<std::pair<std::multimap<const std::string, int>, std::multimap<int, int> > *>(*k);
					*k = nullptr;
				}

			}
			break;
			case DataTableColumnType::DT_HashString:
			case DataTableColumnType::DT_Enum:
			case DataTableColumnType::DT_Bool:
			case DataTableColumnType::DT_BitVector:
			case DataTableColumnType::DT_Unknown:
			case DataTableColumnType::DT_Comment:
			case DataTableColumnType::DT_PackedObjVars:
			default:
			{
				WARNING_STRICT_FATAL((*k) != nullptr, ("Bad column Index %d. Unsupported datatype.  This may leak memory.", count));
			}
			break;
		}
	}

	if (m_cells)
	{
		for (int row=0;row<m_numRows;++row)
		{
			const DataTableCell *rowCells = m_cells + row * m_numCols;
			for (int col=0;col<m_numCols;++col)
			{
				const DataTableCell &cell = rowCells[col];
				cell.~DataTableCell();
			}
		}

		free((void *)m_cells);
		m_cells=0;
	}

	DataTableColumnTypeVector::iterator l = m_types.begin();
	while(l != m_types.end())
	{
		delete *l++;
	}

	delete m_columnIndexMap;
	m_columnIndexMap = nullptr;
}

//----------------------------------------------------------------------------

bool DataTable::doesColumnExist(const std::string & column) const
{
	DEBUG_FATAL(m_columnIndexMap->empty(), ("DataTable(%x) [%s]: Column index map is empty.\n", this, m_name.c_str()));

	return m_columnIndexMap->find(column) != m_columnIndexMap->end();
}

//----------------------------------------------------------------------------

int DataTable::findColumnNumber(const std::string & column) const
{
	DEBUG_FATAL(m_columnIndexMap->empty(), ("DataTable(%x) [%s]: Column index map is empty.\n", this, m_name.c_str()));

	int count = -1;
	ColumnIndexMap::const_iterator const it = m_columnIndexMap->find(column);

	if (it != m_columnIndexMap->end())
	{
		count = it->second;
	}

	return count;
}

//----------------------------------------------------------------------------
const std::string & DataTable::getColumnName(int column) const
{
	DEBUG_FATAL(column < 0 || column >= getNumColumns(), ("DataTable [%s] getColumnName(): Invalid col number [%d].  Cols=[%d]\n", m_name.c_str(), column, getNumColumns()));
	return m_columns[static_cast<size_t>(column)];
}


//----------------------------------------------------------------------------

DataTableColumnType DataTable::getDataType(const std::string & type)
{
	return DataTableColumnType(type);
}

//----------------------------------------------------------------------------

DataTableColumnType const &DataTable::getDataTypeForColumn(const std::string& column) const
{
	int const columnIndex = findColumnNumber(column);
	DEBUG_FATAL(columnIndex < 0 || columnIndex >= getNumColumns(), ("DataTable [%s] getDataTypeForColumn(): Column name [%s] is invalid\n", m_name.c_str(), column.c_str()));

	return getDataTypeForColumn(columnIndex);
}

//----------------------------------------------------------------------------

DataTableColumnType const &DataTable::getDataTypeForColumn(int column) const
{
	DEBUG_FATAL(column < 0 || column >= getNumColumns(), ("DataTable [%s] getDataTypeForColumn(): Invalid col number [%d].  Cols=[%d]\n", m_name.c_str(), column, getNumColumns()));
	return *m_types[static_cast<size_t>(column)];
}

//----------------------------------------------------------------------------

int DataTable::getIntValue(const std::string & column, int row) const
{
	int const columnIndex = findColumnNumber(column);
	DEBUG_FATAL(columnIndex < 0 || columnIndex >= getNumColumns(), ("DataTable [%s] getIntValue(): Column name [%s] is invalid\n", m_name.c_str(), column.c_str()));
	return getIntValue(columnIndex, row);
}

//----------------------------------------------------------------------------

int DataTable::getIntValue(int column, int row) const
{
	DEBUG_FATAL(row < 0 || row >= getNumRows(), ("DataTable [%s] getIntValue(): Invalid row number [%d].  Rows=[%d]\n", m_name.c_str(), row, getNumRows()));
	DEBUG_FATAL(column < 0 || column >= getNumColumns(), ("DataTable [%s] getIntValue(): Invalid col number [%d].  Cols=[%d]\n", m_name.c_str(), column, getNumColumns()));
	
	// we can return the value of an int column or the crc value of a string column
	if (m_types[static_cast<size_t>(column)]->getBasicType() == DataTableColumnType::DT_Int)
	{
		const DataTableCell *cell = getDataTableCell(column, row);
		DEBUG_FATAL(cell->getType()!=DataTableCell::CT_int, ("Could not convert row %d column %d to int value", row, column));
		return cell->getIntValue();
	}
	else if (m_types[static_cast<size_t>(column)]->getBasicType() == DataTableColumnType::DT_String)
	{
		const DataTableCell *cell = getDataTableCell(column, row);
		DEBUG_FATAL(cell->getType()!=DataTableCell::CT_string, ("Could not convert row %d column %d to string value", row, column));
		return cell->getStringValueCrc();
	}

	DEBUG_FATAL(true, ("DataTable [%s] getIntValue(): Wrong data type [%d] for col [%d].\n", m_name.c_str(), m_types[static_cast<size_t>(column)]->getBasicType(), column));
	return 0;
}

//----------------------------------------------------------------------------

int DataTable::getIntDefaultForColumn(const std::string & column) const
{
	int const columnIndex = findColumnNumber(column);
	DEBUG_FATAL(columnIndex < 0 || columnIndex >= getNumColumns(), ("DataTable [%s] getIntDefaultForColumn(): Invalid col number [%d].  Cols=[%d]\n", m_name.c_str(), columnIndex, getNumColumns()));
	return getIntDefaultForColumn(columnIndex);
}

//----------------------------------------------------------------------------

int DataTable::getIntDefaultForColumn(int column) const
{
	DEBUG_FATAL(column < 0 || column >= getNumColumns(), ("DataTable [%s] getIntDefaultForColumn(): Invalid col number [%d].  Cols=[%d]\n", m_name.c_str(), column, getNumColumns()));
	DEBUG_FATAL(m_types[static_cast<size_t>(column)]->getBasicType() != DataTableColumnType::DT_Int, ("Wrong data type for column %d.", column));
	std::string value;
	IGNORE_RETURN( getDataTypeForColumn(column).mangleValue(value) );
	return atoi(value.c_str());
}

//----------------------------------------------------------------------------

float DataTable::getFloatValue(const std::string & column, int row) const
{
	int const columnIndex = findColumnNumber(column);
	DEBUG_FATAL(columnIndex < 0 || columnIndex >= getNumColumns(), ("DataTable Column [%s] is invalid", column.c_str()));
	return getFloatValue(columnIndex, row);
}

//----------------------------------------------------------------------------

float DataTable::getFloatValue(int column, int row) const
{
	DEBUG_FATAL(row < 0 || row >= getNumRows(), ("Row is invalid."));
	DEBUG_FATAL(column < 0 || column >= getNumColumns(), ("DataTable [%s] getFloatValue(): Invalid col number [%d].  Cols=[%d]\n", m_name.c_str(), column, getNumColumns()));
	DEBUG_FATAL(m_types[static_cast<size_t>(column)]->getBasicType() != DataTableColumnType::DT_Float, ("Wrong data type for column %d.", column));

	const DataTableCell *cell = getDataTableCell(column, row);
	DEBUG_FATAL(cell->getType()!=DataTableCell::CT_float, ("Could not convert row %d column %d to float value", row, column));
	return cell->getFloatValue();

}

//----------------------------------------------------------------------------

float DataTable::getFloatDefaultForColumn(const std::string & column) const
{
	int const columnIndex = findColumnNumber(column);
	DEBUG_FATAL(columnIndex < 0 || columnIndex >= getNumColumns(), ("DataTable Column [%s] is invalid", column.c_str()));
	return getFloatDefaultForColumn(columnIndex);
}

//----------------------------------------------------------------------------

float DataTable::getFloatDefaultForColumn(int column) const
{
	DEBUG_FATAL(column < 0 || column >= getNumColumns(), ("DataTable [%s] getFloatDefaultForColumn(): Invalid col number [%d].  Cols=[%d]\n", m_name.c_str(), column, getNumColumns()));
	DEBUG_FATAL(m_types[static_cast<size_t>(column)]->getBasicType() != DataTableColumnType::DT_Float, ("Wrong data type for column %d.", column));
	std::string value;
	IGNORE_RETURN ( getDataTypeForColumn(column).mangleValue(value) );
	return static_cast<float>(atof(value.c_str()));
}

//----------------------------------------------------------------------------

const char *DataTable::getStringValue(const std::string & column, int row) const
{
	int const columnIndex = findColumnNumber(column);
	DEBUG_FATAL(columnIndex < 0 || columnIndex >= getNumColumns(), ("DataTable Column [%s] is invalid", column.c_str()));
	return getStringValue(columnIndex, row);
}

//----------------------------------------------------------------------------

const char *DataTable::getStringValue(int column, int row) const
{
	DEBUG_FATAL(row < 0 || row >= getNumRows(), ("Row [%d] is invalid.", row));
	DEBUG_FATAL(column < 0 || column >= getNumColumns(), ("Column [%d] is invalid.", column));
	DEBUG_FATAL(m_types[static_cast<size_t>(column)]->getBasicType() != DataTableColumnType::DT_String, ("Wrong data type for column %s (%d). Current data type is %s", getColumnName(column).c_str(), column, getDataTypeForColumn(column).getTypeSpecString().c_str()));

	const DataTableCell * const cell = getDataTableCell(column, row);
	DEBUG_FATAL(cell->getType()!=DataTableCell::CT_string, ("Could not convert row %d column %d to string value", row, column));
	return cell->getStringValue();

}

//----------------------------------------------------------------------------

std::string DataTable::getStringDefaultForColumn(const std::string & column) const
{
	int const columnIndex = findColumnNumber(column);
	DEBUG_FATAL(columnIndex < 0 || columnIndex >= getNumColumns(), ("DataTable Column [%s] is invalid", column.c_str()));
	return getStringDefaultForColumn(columnIndex);
}

//----------------------------------------------------------------------------

std::string DataTable::getStringDefaultForColumn(int column) const
{
	DEBUG_FATAL(column < 0 || column >= getNumColumns(), ("DataTable [%s] getStringDefaultForColumn(): Invalid col number [%d].  Cols=[%d]\n", m_name.c_str(), column, getNumColumns()));

	DEBUG_FATAL(m_types[static_cast<size_t>(column)]->getBasicType() != DataTableColumnType::DT_String, ("Wrong data type for column %d.", column));
	std::string value;
	IGNORE_RETURN( getDataTypeForColumn(column).mangleValue(value) );
	return value;
}

//----------------------------------------------------------------------------

void DataTable::getIntColumn(const std::string& column, std::vector<int>& returnVector) const
{
	int const columnIndex = findColumnNumber(column);
	DEBUG_FATAL(columnIndex < 0 || columnIndex >= getNumColumns(), ("DataTable Column [%s] is invalid", column.c_str()));
	getIntColumn(columnIndex, returnVector);
}

//----------------------------------------------------------------------------

void DataTable::getIntColumn(const std::string& column, std::vector<long>& returnVector) const
{
	int const columnIndex = findColumnNumber(column);
	DEBUG_FATAL(columnIndex < 0 || columnIndex >= getNumColumns(), ("DataTable Column [%s] is invalid", column.c_str()));
	getIntColumn(columnIndex, returnVector);
}

//----------------------------------------------------------------------------

void DataTable::getFloatColumn(const std::string& column, std::vector<float>& returnVector) const
{
	int const columnIndex = findColumnNumber(column);
	DEBUG_FATAL(columnIndex < 0 || columnIndex >= getNumColumns(), ("DataTable Column [%s] is invalid", column.c_str()));
	getFloatColumn(columnIndex, returnVector);
}

//----------------------------------------------------------------------------

void DataTable::getStringColumn(const std::string& column, std::vector<const char *>& returnVector) const
{
	int const columnIndex = findColumnNumber(column);
	DEBUG_FATAL(columnIndex < 0 || columnIndex >= getNumColumns(), ("DataTable Column [%s] is invalid", column.c_str()));
	getStringColumn(columnIndex, returnVector);
}

//----------------------------------------------------------------------------

void DataTable::getIntColumn(int column, std::vector<int>& returnVector) const
{
	returnVector.clear();
	for (int i = 0; i < getNumRows(); ++i)
	{
		returnVector.push_back(getIntValue(column, i));
	}
}

//----------------------------------------------------------------------------

void DataTable::getIntColumn(int column, std::vector<long>& returnVector) const
{
	returnVector.clear();
	for (int i = 0; i < getNumRows(); ++i)
	{
		returnVector.push_back(getIntValue(column, i));
	}
}

//----------------------------------------------------------------------------

void DataTable::getFloatColumn(int column, std::vector<float>& returnVector) const
{
	returnVector.clear();
	for (int i = 0; i < getNumRows(); ++i)
	{
		returnVector.push_back(getFloatValue(column, i));
	}
}

//----------------------------------------------------------------------------

void DataTable::getStringColumn(int column, std::vector<const char *>& returnVector) const
{
	returnVector.clear();
	for (int i = 0; i < getNumRows(); ++i)
	{
		returnVector.push_back(getStringValue(column, i));
	}
}

//----------------------------------------------------------------------------

void DataTable::_readCell(Iff & iff, int column, int row)
{
	DataTableCell *cell = const_cast<DataTableCell *>(getDataTableCell(column, row));
	const DataTableColumnType &typeCol = *m_types[static_cast<size_t>(column)];
	switch (typeCol.getBasicType())
	{
	case DataTableColumnType::DT_Int:
	{
		int tmp = iff.read_int32();
		new (cell) DataTableCell(tmp);

		break;
	}
	case DataTableColumnType::DT_Float:
	{
		float tmp = iff.read_float();
		new (cell) DataTableCell(tmp);
		break;
	}
	case DataTableColumnType::DT_String:
	{
		const int maxLen = 16384;
		char buffer[maxLen];
		buffer[0]=0;
		iff.read_string(buffer, maxLen-1);
		buffer[maxLen-1]=0;
		new (cell) DataTableCell(buffer);
		break;
	}
	case DataTableColumnType::DT_Unknown:
	case DataTableColumnType::DT_HashString:
	case DataTableColumnType::DT_Enum:
	case DataTableColumnType::DT_Bool:
	case DataTableColumnType::DT_BitVector:
	case DataTableColumnType::DT_Comment:
	case DataTableColumnType::DT_PackedObjVars:
	default:
		DEBUG_FATAL(true, ("bad case"));
		break;
	}
}

//----------------------------------------------------------------------------

void DataTable::load(Iff & iff)
{
	IGNORE_RETURN ( iff.enterForm(m_dataTableIffId, false) );

	const Tag version = iff.getCurrentName();
	if (version == TAG_0000)
		load_0000(iff);
	else if (version == TAG_0001)
		load_0001(iff);
	else
	{
		char buffer[5];
		
		ConvertTagToString(version, buffer);
		FATAL(true, ("unknown DataTable file format [%s]", buffer));
	}

	iff.exitForm(m_dataTableIffId, false);

	int count = getNumColumns();
	for (int i = 0; i < count; ++i)
	{
		//initialize the table index used for searching.
		m_index.push_back(nullptr);
	}

	buildColumnIndexMap();

	if (nullptr != iff.getFileName())
		m_name = iff.getFileName();
	else
		m_name.clear();
}

//----------------------------------------------------------------------------

void DataTable::load_0000(Iff & iff)
{
	IGNORE_RETURN ( iff.enterForm(TAG_0000, false) );

	//load columns
	iff.enterChunk(TAG(C,O,L,S));
	m_numCols = iff.read_int32();	
	int i = 0;
	m_columns.reserve(m_numCols);
	std::string tmpString;
	for (i = 0; i < m_numCols; ++i)
	{
		iff.read_string(tmpString);
		m_columns.push_back(tmpString);
		tmpString.clear();
	}

	iff.exitChunk(TAG(C,O,L,S));	
	//load type info
	iff.enterChunk(TAG(T,Y,P,E));
	for (i = 0; i < m_numCols; ++i)
	{
		DataTableColumnType::DataType dt = static_cast<DataTableColumnType::DataType>(iff.read_int32());
		// version 0000 has only Int, Float, String, and no other format information
		switch (dt)
		{
		case DataTableColumnType::DT_Int:
			{
				m_types.push_back(new DataTableColumnType("i"));
			}
			break;
		case DataTableColumnType::DT_Float:
			{
				m_types.push_back(new DataTableColumnType("f"));
			}
			break;
		case DataTableColumnType::DT_String:
			{
				m_types.push_back(new DataTableColumnType("s"));
			}
			break;
		case DataTableColumnType::DT_Unknown:
		case DataTableColumnType::DT_HashString:
		case DataTableColumnType::DT_Enum:
		case DataTableColumnType::DT_Bool:
		case DataTableColumnType::DT_BitVector:
		case DataTableColumnType::DT_Comment:
		case DataTableColumnType::DT_PackedObjVars:

		default:
			{
				FATAL(true, ("unknown column type loaded from version 0000"));
			}
			break;
		}
	}
	iff.exitChunk(TAG(T,Y,P,E));

	//load rows
	iff.enterChunk(TAG(R,O,W,S));
	m_numRows = iff.read_int32();

	const size_t cellSize = m_numRows * m_numCols * sizeof(DataTableCell);
	void *cellMemory = malloc(cellSize);
	m_cells = (DataTableCell *)cellMemory;
	for (i = 0; i < m_numRows; ++i)
	{
		for (int j = 0; j < m_numCols; ++j)
		{
			_readCell(iff, j, i);
		}
	}

	iff.exitChunk(TAG(R,O,W,S));
	iff.exitForm(TAG_0000, false);
}

//----------------------------------------------------------------------------

void DataTable::load_0001(Iff & iff)
{
	IGNORE_RETURN( iff.enterForm(TAG_0001, false) );

	//load columns
	iff.enterChunk(TAG(C,O,L,S));
	m_numCols = iff.read_int32();	
	int i = 0;
	m_columns.reserve(m_numCols);
	std::string tmpString;
	for (i = 0; i < m_numCols; ++i)
	{
		iff.read_string(tmpString);
		m_columns.push_back(tmpString);
		tmpString.clear();
	}

	iff.exitChunk(TAG(C,O,L,S));	
	//load type info
	iff.enterChunk(TAG(T,Y,P,E));
	for (i = 0; i < m_numCols; ++i)
	{
		// version 0001 has a format string for the type
		m_types.push_back(new DataTableColumnType(iff.read_stdstring()));
	}
	iff.exitChunk(TAG(T,Y,P,E));

	//load rows
	iff.enterChunk(TAG(R,O,W,S));
	m_numRows = iff.read_int32();

	const size_t cellSize = m_numRows * m_numCols * sizeof(DataTableCell);
	void *cellMemory = malloc(cellSize);
	m_cells = (DataTableCell *)cellMemory;
	for (i = 0; i < m_numRows; ++i)
	{
		for (int j = 0; j < m_numCols; ++j)
		{
			_readCell(iff, j, i);
		}
	}

	iff.exitChunk(TAG(R,O,W,S));
	iff.exitForm(TAG_0001, false);
}

// ----------------------------------------------------------------------

int DataTable::searchColumnString( int column, const std::string & searchValue ) const
{
	DEBUG_FATAL(column < 0 || column >= getNumColumns(), ("DataTable [%s] searchColumnString(): Invalid col number [%d].  Cols=[%d]\n", m_name.c_str(), column, getNumColumns()));

	int retval = -1;

	void * voidIndex = m_index[ static_cast<size_t>(column) ];
	if (!voidIndex)
	{
		std::pair<std::multimap<const std::string, int>, std::multimap<int, int> > * indexPair = new std::pair<std::multimap<const std::string, int>, std::multimap<int, int> >;

		m_index[static_cast<size_t>(column)] = static_cast<void *>(indexPair);

		//no index has been built yet for this column
		int rowCount = getNumRows();
		for (int i = 0; i < rowCount; ++i)
		{
			const std::string & valueString = getStringValue(column,i);
			IGNORE_RETURN ( indexPair->first.insert(std::pair<const std::string, int>(valueString, i)) );
			int valueCrc = getIntValue(column,i);
			IGNORE_RETURN ( indexPair->second.insert(std::pair<int, int>(valueCrc, i)) );
			if (retval == -1 && valueString == searchValue)
				retval = i;
		}
	}
	else
	{
		std::pair<std::multimap<const std::string, int>, std::multimap<int, int> > * indexPair = static_cast<std::pair<std::multimap<const std::string, int>, std::multimap<int, int> > *>(voidIndex);
		std::multimap<const std::string, int>::iterator iter = indexPair->first.find(searchValue);
		if (iter != indexPair->first.end())
			retval = iter->second;	
	}

	return retval;
}

// ----------

int DataTable::searchColumnFloat( int column, float searchValue ) const
{
	DEBUG_FATAL(column < 0 || column >= getNumColumns(), ("DataTable [%s] searchColumnFloat(): Invalid col number [%d].  Cols=[%d]\n", m_name.c_str(), column, getNumColumns()));

	int retval = -1;

	void * voidIndex = m_index[ static_cast<size_t>(column) ];
	if (!voidIndex)
	{
		std::multimap<float, int> * index = new std::multimap<float, int>;

		m_index[static_cast<size_t>(column)] = static_cast<void *>(index);

		//no index has been built yet for this column
		for (int i = 0; i < getNumRows(); ++i)
		{
			float rowValue = getFloatValue(column,i);
			IGNORE_RETURN ( index->insert(std::pair<float, int>(rowValue, i)) );
			if (retval == -1 && rowValue == searchValue) //lint !e777 //ok to compare floats
				retval = i;
		}
	}
	else
	{
		std::multimap<float, int> * index = static_cast<std::multimap<float, int> *>(voidIndex);
		std::multimap<float, int>::iterator iter = index->find(searchValue);
		if (iter != index->end())
			retval = iter->second;
	}

	return retval;
}

// ----------

int DataTable::searchColumnInt( int column, int searchValue ) const
{
	DEBUG_FATAL(column < 0 || column >= getNumColumns(), ("DataTable [%s] searchColumnInt(): Invalid col number [%d].  Cols=[%d]\n", m_name.c_str(), column, getNumColumns()));

	int retval = -1;
	DataTableColumnType::DataType columnType = m_types[static_cast<size_t>(column)]->getBasicType();

	void * voidIndex = m_index[ static_cast<size_t>(column) ];
	if (!voidIndex)
	{
		if (columnType == DataTableColumnType::DT_Int)
		{
			std::multimap<int, int> * index = new std::multimap<int, int>;

			m_index[static_cast<size_t>(column)] = static_cast<void *>(index);

			//no index has been built yet for this column
			int rowCount = getNumRows();
			for (int i = 0; i < rowCount; ++i)
			{
				int rowValue = getIntValue(column,i);
				IGNORE_RETURN ( index->insert(std::pair<int, int>(rowValue, i)) );
				if (retval == -1 && rowValue == searchValue)
					retval = i;
			}
		}
		else if (columnType == DataTableColumnType::DT_String)
		{
			std::pair<std::multimap<const std::string, int>, std::multimap<int, int> > * indexPair = new std::pair<std::multimap<const std::string, int>, std::multimap<int, int> >;

			m_index[static_cast<size_t>(column)] = static_cast<void *>(indexPair);

			//no index has been built yet for this column
			int rowCount = getNumRows();
			for (int i = 0; i < rowCount; ++i)
			{
				const std::string & valueString = getStringValue(column,i);
				IGNORE_RETURN ( indexPair->first.insert(std::pair<const std::string, int>(valueString, i)) );
				int valueCrc = getIntValue(column,i);
				IGNORE_RETURN ( indexPair->second.insert(std::pair<int, int>(valueCrc, i)) );
				if (retval == -1 && valueCrc == searchValue)
					retval = i;
			}
		}
	}
	else
	{
		if (columnType == DataTableColumnType::DT_Int)
		{
			std::multimap<int, int> * index = static_cast<std::multimap<int, int> *>(voidIndex);
			std::multimap<int, int>::iterator iter = index->find(searchValue);
			if (iter != index->end())
				retval = iter->second;
		}
		else if (columnType == DataTableColumnType::DT_String)
		{
			std::pair<std::multimap<const std::string, int>, std::multimap<int, int> > * indexPair = static_cast<std::pair<std::multimap<const std::string, int>, std::multimap<int, int> > *>(voidIndex);
			std::multimap<int, int>::iterator iter = indexPair->second.find(searchValue);
			if (iter != indexPair->second.end())
				retval = iter->second;
		}
	}

	return retval;

}

// ----------

void DataTable::buildColumnIndexMap()
{
	std::vector<std::string>::const_iterator i = m_columns.begin();
	for (int columnIndex = 0; i != m_columns.end(); ++i, ++columnIndex)
	{
		(*m_columnIndexMap)[*i] = columnIndex;
	}
}

//----------------------------------------------------------------------------

