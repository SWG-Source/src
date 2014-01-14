// ======================================================================
//
// DataTableWriter.h
// 
// copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_DataTableWriter_H
#define INCLUDED_DataTableWriter_H

#include "DataTableColumnType.h"
#include <list>
#include <string>
#include <vector>

class Iff;
class XmlTreeNode;

//----------------------------------------------------------------------

class DataTableWriter
{
public:

	DataTableWriter();
	~DataTableWriter();

	void setOutputPath (const char * path);
	void loadFromSpreadsheet (const char * filename);

	bool save (const char * outputFileName, bool optional = false) const;
	bool save (bool optional = false) const;
  	void save (Iff & iff) const;

	bool saveTable( const char* tableName, const char* fileName = 0, bool optional = false ) const;

	void getTableNames(std::vector<std::string> & v) const;
	void getTableOutputFileName( const char* tableName, std::string& fileName ) const;

	static bool isXmlFile(const char * filename);

private:

	class NamedDataTable;

	typedef std::list<NamedDataTable *> NamedDataTableList;

	bool _checkIntegrity(const NamedDataTable * ndt) const;

	void _loadFromSpreadsheetTab (const char * filename);
	void _loadFromSpreadsheetXml (const char * filename);
	void _loadWorksheet (NamedDataTable * ndt, XmlTreeNode nodeWorksheet);

	void _loadDataTableSchema(NamedDataTable * ndt, XmlTreeNode parentNode);
	void _loadDataTableRow(NamedDataTable * ndt, XmlTreeNode rowNode);

	void _loadColumnNames(NamedDataTable * ndt, const char* row);
	void _loadTypes(NamedDataTable * ndt, const char* row);
	void _loadRow(NamedDataTable * ndt, const char* row, int index);

	void _saveColumns(NamedDataTable * ndt, Iff& iff) const;
	void _saveTypes(NamedDataTable * ndt, Iff& iff) const;
	void _saveRows(NamedDataTable * ndt, Iff& iff) const;

	bool _writeTable(NamedDataTable * ndt, const char * outputFile, bool optional) const;
  	void _saveTableToIff(Iff & iff, NamedDataTable * ndt) const;
	
	static DataTableCell *_getNewCell(DataTableColumnType const &columnType, char const *contents);
	
	NamedDataTableList m_tables;
	std::string m_path;
};

//----------------------------------------------------------------------

#endif
