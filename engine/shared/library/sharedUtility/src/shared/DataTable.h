// ======================================================================
//
// DataTable.h
// 
// copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_DataTable_H
#define INCLUDED_DataTable_H

#include "sharedFoundation/Tag.h"
#include "DataTableCell.h"
#include "DataTableColumnType.h"

#include <string>
#include <vector>
#include <unordered_map>

class Iff;

class DataTable
{
public:
	DataTable();
	virtual ~DataTable();

	bool                doesColumnExist(const std::string &) const;
	const std::string & getColumnName(int column) const;
	int                 findColumnNumber(const std::string & column) const;
	DataTableColumnType const &getDataTypeForColumn(const std::string& column) const;
	DataTableColumnType const &getDataTypeForColumn(int column) const;

	int                 getIntValue(const std::string & column, int row) const;
	int                 getIntValue(int column, int row) const;
	int                 getIntDefaultForColumn(const std::string & column) const;
	int                 getIntDefaultForColumn(int column) const;
	float               getFloatValue(const std::string & column, int row) const;
	float               getFloatValue(int column, int row) const;
	float               getFloatDefaultForColumn(const std::string & column) const;
	float               getFloatDefaultForColumn(int column) const;
	const char *        getStringValue(const std::string & column, int row) const;
	const char *        getStringValue(int column, int row) const;
	std::string         getStringDefaultForColumn(const std::string & column) const;
	std::string         getStringDefaultForColumn(int column) const;

	void                getIntColumn(const std::string& column, std::vector<int>& returnVector) const;
	void                getIntColumn(const std::string& column, std::vector<long>& returnVector) const;
	void                getFloatColumn(const std::string& column, std::vector<float>& returnVector) const;
	void                getStringColumn(const std::string& column, std::vector<const char *>& returnVector) const;
	void                getIntColumn(int column, std::vector<int>& returnVector) const;
	void                getIntColumn(int column, std::vector<long>& returnVector) const;
	void                getFloatColumn(int column, std::vector<float>& returnVector) const;
	void                getStringColumn(int column, std::vector<const char *>& returnVector) const;

	int                 getNumColumns() const;
	int                 getNumRows() const;

	void                load(Iff &);

	// Search a column of the table for a given value and return the index of the first
	// matching row. Warning - this is a linear search, so it can be slow.

	int                 searchColumnString(int column, const std::string & searchValue) const;
	int                 searchColumnFloat(int column, float searchValue) const;
	int                 searchColumnInt(int column, int searchValue) const;

	std::string const & getName() const;

	static Tag          getDataTableIffId() { return m_dataTableIffId; }

protected:

	const std::vector<std::string> &_columns() const { return m_columns; }

private:

	const DataTableCell * getDataTableCell(int column, int row) const;
	void _readCell(Iff & iff, int column, int row);

	static DataTableColumnType getDataType(const std::string &type);
	void                load_0000(Iff &);
	void                load_0001(Iff &);
	void buildColumnIndexMap();

	typedef std::vector<const DataTableColumnType *> DataTableColumnTypeVector;
	typedef std::unordered_map<std::string /*column name*/, int /*column index*/> ColumnIndexMap;

	int                           m_numRows;
	int                           m_numCols;
	DataTableCell const *         m_cells;
	std::vector<std::string>      m_columns;
	mutable std::vector<void *>   m_index;
	DataTableColumnTypeVector     m_types;
	ColumnIndexMap *              m_columnIndexMap;
	std::string                   m_name;

	static const Tag              m_dataTableIffId;

private:
	
	DataTable(DataTable const &);
	DataTable & operator=(DataTable const &);
};

inline int DataTable::getNumColumns() const
{
	return static_cast<int>(m_columns.size());
}

inline int DataTable::getNumRows() const
{
	return m_numRows;
}

inline const DataTableCell * DataTable::getDataTableCell(int column, int row) const
{
	return m_cells + (row * m_numCols + column);
}

//----------------------------------------------------------------------

inline std::string const & DataTable::getName() const
{
	return m_name;
}

#endif
