// ======================================================================
//
// DataTableWriter.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedUtility/FirstSharedUtility.h"
#include "sharedUtility/DataTableWriter.h"

#include "fileInterface/StdioFile.h"
#include "sharedFile/FileNameUtils.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedUtility/DataTable.h"
#include "sharedXml/XmlTreeDocument.h"
#include "sharedXml/XmlTreeDocumentList.h"
#include "sharedXml/XmlTreeNode.h"

#include <map>

// ======================================================================

#ifdef _DEBUG
namespace DataTableWriterNamespace
{
	int s_debugRow = 0;
};
#endif

//----------------------------------------------------------------------------

class DataTableWriter::NamedDataTable
{
	friend class DataTableWriter;

public:

	NamedDataTable();
	~NamedDataTable();

	const std::string& getName() const;
	void setName(const std::string& name);

	int getNumColumns() const { return static_cast<int>(m_columns.size()); }

	const std::string & getColumnName(int column) const
	{
		DEBUG_FATAL(column < 0 || column >= static_cast<int>(m_columns.size()), ("DataTable [%s] getColumnName(): Invalid col number [%d].  Cols=[%d]\n", m_Name.c_str(), column, static_cast<int>(m_columns.size())));
		return m_columns[static_cast<size_t>(column)];
	}

	const DataTableColumnType &getDataTypeForColumn(int column) const
	{
		DEBUG_FATAL(column < 0 || column >= static_cast<int>(m_columns.size()), ("DataTable [%s] getDataTypeForColumn(): Invalid col number [%d] for possible row [%d].  Cols=[%d]\n", m_Name.c_str(), column, DataTableWriterNamespace::s_debugRow, static_cast<int>(m_columns.size())));
		return *m_types[static_cast<size_t>(column)];
	}

	int getNumRows() const { return static_cast<int>(m_rows.size()); }

private:

	typedef std::vector<const DataTableCell *> DataTableRow;
	typedef std::vector<const DataTableColumnType *> DataTableColumnTypeVector;

	std::string m_Name;
	
	std::vector<DataTableRow *>   m_rows;
	std::vector<std::string>      m_columns;
	DataTableColumnTypeVector     m_types;

private:
	NamedDataTable(NamedDataTable const &);
	NamedDataTable & operator=(NamedDataTable const &);
};

//----------------------------------------------------------------------------

namespace DataTableWriterNamespace
{
	std::string &unquotify(std::string &s);

	std::string getTableNameFromTabFile( const char* fileName );

};

using namespace DataTableWriterNamespace;

// ======================================================================

DataTableWriter::NamedDataTable::NamedDataTable()
{
}

// ----------------------------------------------------------------------
	
DataTableWriter::NamedDataTable::~NamedDataTable()
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
	std::vector<DataTableRow *>::iterator i = m_rows.begin();
	for (; i != m_rows.end(); ++i)
	{
		DataTableColumnTypeVector::iterator k = m_types.begin();
		DataTableRow::iterator j = (*i)->begin();
		for(; j != (*i)->end(); ++j, ++k)
		{
			// Only delete cells that are different from the default
			if(*j != (*k)->getDefaultCell())
				delete *j;
		}
		delete *i;
	}

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
	DataTableColumnTypeVector::iterator l = m_types.begin();
	while (l != m_types.end())
	{
		delete *l++;
	}
	m_types.clear();
}

// ----------------------------------------------------------------------

inline const std::string& DataTableWriter::NamedDataTable::getName() const
{
	return m_Name;
}

// ----------------------------------------------------------------------

inline void DataTableWriter::NamedDataTable::setName(const std::string& name)
{
	m_Name = name;
}

// ----------------------------------------------------------------------

std::string & DataTableWriterNamespace::unquotify(std::string &s)
{
	//First strip any beginning end quotes.
	if (s.length() >= 2)
	{
		if (s[0] == '"')
			s.erase(0, 1);
		if (s[s.length()-1] == '"')
			s.erase(s.length()-1, 1);
	}


	//Then deal with imbedded quotes
	//iterate through string
	//if character is a quote
		//if next is a quote erase this one jump past next
		//else erase this one
	//else next

	for (std::string::iterator i = s.begin(); i != s.end();)
	{
		if (*i == '"')
		{
			if ( (i+1) != s.end() && *(i+1) == '"' )
			{
				s.erase(i);
				++i;
			}
			else
			{
				s.erase(i);
			}
		}
		else
		{
			++i;
		}
	}
	return s;

}

// ----------------------------------------------------------------------

std::string DataTableWriterNamespace::getTableNameFromTabFile( const char* fileName )
{
	std::string ret(fileName);

	ret.erase(ret.find_last_of('.'));

	// support both types of naming, regardless of what platform you are on.
	size_t i = ret.find_last_of("\\/");

	if (i != std::string::npos)
		ret.erase(0, i+1);

	return ret;
}


// ----------------------------------------------------------------------

DataTableWriter::DataTableWriter()
{
}

// ----------------------------------------------------------------------

DataTableWriter::~DataTableWriter()
{
	while(!m_tables.empty())
	{
		NamedDataTable * ndt = m_tables.front();
		m_tables.pop_front();
		delete ndt;
	}
}

// ----------------------------------------------------------------------

bool DataTableWriter::isXmlFile(const char * filename)
{
	NOT_NULL(filename);

	const char * xmlExtString = ".XML";
	const int    extLength    = sizeof(xmlExtString)/sizeof(xmlExtString[0]);

	int fileLength = strlen(filename);

	if(fileLength <= extLength)
		return false;

	// Return false if the extension does not match xmlExtString
	return !_strnicmp(filename + fileLength - extLength, xmlExtString, extLength);
}

// ----------------------------------------------------------------------

void DataTableWriter::loadFromSpreadsheet(const char * filename)
{
	NOT_NULL(filename);

	if(!isXmlFile(filename))
		// Original file format
		_loadFromSpreadsheetTab(filename);
	else
		// XML file format
		_loadFromSpreadsheetXml(filename);
}

// ----------------------------------------------------------------------

void DataTableWriter::_loadWorksheet(NamedDataTable * ndt, XmlTreeNode nodeWorksheet)
{
	// Table
	XmlTreeNode nodeTable = nodeWorksheet.getChildNodeNamed("Table");
	FATAL(nodeTable.isNull(), ("Worksheet %s: Table node not found, worksheet has no data", ndt->getName().c_str()));

	XmlTreeNode nodeRow = nodeTable.getChildNodeNamed("Row");
	FATAL(nodeRow.isNull(), ("Worksheet %s: Missing row node, worksheet has no data", ndt->getName().c_str()));

	XmlTreeNode nodeCell = nodeRow.getChildNodeNamed("Cell");
	FATAL(nodeCell.isNull(), ("Worksheet %s: Missing cell node, worksheet has no data", ndt->getName().c_str()));

	// Read the column names
	while(!nodeCell.isNull())
	{
		XmlTreeNode nodeText = nodeCell.getTextChildNodeNamed("Data");
		FATAL(nodeText.isNull(), ("Worksheet %s: Column #%d missing a name", ndt->getName().c_str(), ndt->m_columns.size()+1));

		std::string s = nodeText.getTextValue();
		ndt->m_columns.push_back(s);
		nodeCell = nodeCell.getNextSiblingElementNode();
	}

	nodeRow = nodeRow.getNextSiblingElementNode();
	nodeCell = nodeRow.getChildNodeNamed("Cell");

	// Read the column types
	while(!nodeCell.isNull())
	{
		XmlTreeNode nodeText = nodeCell.getTextChildNodeNamed("Data");
		FATAL(nodeText.isNull(), ("Worksheet %s: Column #%d missing a type", ndt->getName().c_str(), ndt->m_types.size()+1));

		std::string s = nodeText.getTextValue();
		ndt->m_types.push_back(new DataTableColumnType(unquotify(s)));
		nodeCell = nodeCell.getNextSiblingElementNode();
	}

	FATAL(ndt->m_columns.size() != ndt->m_types.size(), ("Worksheet %s: Number of columns %d does not match number of types %d",
		ndt->getName().c_str(), ndt->m_columns.size(), ndt->m_types.size()));

	nodeRow = nodeRow.getNextSiblingElementNode();

	FATAL(nodeRow.isNull(), ("Worksheet %s: No data rows found.", ndt->getName().c_str()));

	// Read the row data
	while(!nodeRow.isNull())
	{
		NamedDataTable::DataTableRow * newRow = new NamedDataTable::DataTableRow;
		int count = 0;

		nodeCell = nodeRow.getChildNodeNamed("Cell");
		while(!nodeCell.isNull())
		{
			const char * c;
			XmlTreeNode nodeText = nodeCell.getTextChildNodeNamed("Data");

			// If the cell has no data node with text, use an empty string
			if(nodeText.isNull())
				c = "";
			else
				c = nodeText.getTextValue();

			int nIndex = 0;

			// Get the cell's index attribute if provide
			nodeCell.getElementAttributeAsInt("Index", nIndex, true);

			FATAL(nIndex && nIndex <= count, ("Worksheet %s: Row %d has an out of order index number. Expected %d, got %d",
				ndt->getName().c_str(), ndt->m_rows.size()+1, count+1, nIndex));

			int colSize = ndt->m_columns.size();
			FATAL(nIndex > colSize || count >= colSize, ("Worksheet %s: Too many columns in row %d. Expected %d, got %d",
				ndt->getName().c_str(), ndt->m_rows.size()+1, colSize, nIndex > colSize ? nIndex : count+1));

			// Skip cells up to index attribute value
			for(; count< (nIndex - 1); ++count)
				newRow->push_back(_getNewCell(ndt->getDataTypeForColumn(count), ""));

			newRow->push_back(_getNewCell(ndt->getDataTypeForColumn(count), c));

			nodeCell = nodeCell.getNextSiblingElementNode();
			count++;
		}

		// Fill in the empty columns
		int oldSize = newRow->size();
		int colSize = ndt->m_columns.size();
		for (int i = 0; i < colSize - oldSize; ++i)
			newRow->push_back(_getNewCell(ndt->getDataTypeForColumn(i+oldSize), ""));

		ndt->m_rows.push_back(newRow);

		// Advance to next row
		nodeRow = nodeRow.getNextSiblingElementNode();
	}
}

// ----------------------------------------------------------------------

void DataTableWriter::_loadFromSpreadsheetXml(const char * filename)
{
	NOT_NULL(filename);

	ConstCharCrcString cc(filename);

	// Open the xml file
	XmlTreeDocument const *const xmlTreeDocument = XmlTreeDocumentList::fetch(cc);

	// Get the root node
	XmlTreeNode rootNode = xmlTreeDocument->getRootTreeNode();

	// Workbook
	FATAL(rootNode.isNull(), ("%s is not a valid XML file. Root node not found.", filename));

	if (_stricmp(rootNode.getName(), "Workbook") == 0)
	{
		// Excel Worksheet
		XmlTreeNode node = rootNode.getChildNodeNamed("Worksheet");
		FATAL(node.isNull(), ("%s is not a valid Excel XML file. Worksheet node not found.", filename));

		while(!node.isNull())
		{
			std::string strName;
			node.getElementAttributeAsString("Name", strName);

			FATAL(!_stricmp(strName.c_str(), "Sheet1"), ("You need to rename the worksheet %s", strName.c_str()));
			FATAL(!_stricmp(strName.c_str(), "Sheet2"), ("You need to rename the worksheet %s", strName.c_str()));
			FATAL(!_stricmp(strName.c_str(), "Sheet3"), ("You need to rename the worksheet %s", strName.c_str()));

			// Skip worksheets with underscore as first character
			if(strName[0] != '_')
			{
				m_tables.push_back(new NamedDataTable());
				NamedDataTable* ndt = m_tables.back();
				ndt->setName(strName);
				_loadWorksheet(ndt, node);
			}

			node = node.getNextSiblingElementNode();
		}
	}
	else // assume this is a dataset
	{
		// look for a schema node
		XmlTreeNode schemaNode = rootNode.getChildNodeNamed("schema");
		XmlTreeNode node(0);

		if (!schemaNode.isNull())
			node = schemaNode.getChildNodeNamed("element");

		if (!node.isNull())
			node = node.getChildNodeNamed("complexType");

		if (!node.isNull())
			node = node.getChildNodeNamed("choice");

		if (!node.isNull())
			node = node.getChildNodeNamed("element");

		FATAL(node.isNull(), ("%s is not a valid DataSet file. No table definitions found.", filename));

		std::map< std::string, NamedDataTable* > namedTables;

		while (!node.isNull())
		{
			std::string strName;
			node.getElementAttributeAsString("Name", strName);

			FATAL( namedTables.find(strName) != namedTables.end(), ("%s is not a valid DataSet file. Duplicate table definition found.", filename));

			// Skip tables with underscore as first character
			if(strName[0] != '_')
			{
				NamedDataTable* table = new NamedDataTable();

				namedTables[strName] = table;
				table->setName(strName);

				_loadDataTableSchema(table, node);
			}
		
			node = node.getNextSiblingElementNode();
		}

		// fill in the table rows
		node = schemaNode.getNextSiblingElementNode();
			
		std::map< std::string, NamedDataTable* >::iterator t = namedTables.end();
		while (!node.isNull())
		{
			std::string rowName = node.getName();

			if (t == namedTables.end() || t->first != rowName)
				t = namedTables.find( rowName );

			if (t != namedTables.end() )
				_loadDataTableRow( t->second, node );

			node = node.getNextSiblingElementNode();
		}

		// fill in the table of tables
		for (t = namedTables.begin(); t != namedTables.end(); ++t)
		{
			m_tables.push_back(t->second);
		}

	}


	// Clean up
	xmlTreeDocument->release();
}


// ----------------------------------------------------------------------

void DataTableWriter::_loadDataTableSchema(NamedDataTable * ndt, XmlTreeNode parentNode)
{
	XmlTreeNode node(0);

	if (!parentNode.isNull())
		node = parentNode.getChildNodeNamed("complexType");

	if (!node.isNull())
		node = node.getChildNodeNamed("sequence");

	if (!node.isNull())
		node = node.getChildNodeNamed("element");

	// read the column names and typespec properties
	while (!node.isNull())
	{
		std::string name;
		std::string type;

		node.getElementAttributeAsString("name", name);
		node.getElementAttributeAsString("typespec", type);

		ndt->m_columns.push_back(name);		
		ndt->m_types.push_back(new DataTableColumnType( type ));

		node = node.getNextSiblingElementNode();
	}
}

// ----------------------------------------------------------------------

void DataTableWriter::_loadDataTableRow(NamedDataTable * ndt, XmlTreeNode rowNode)
{
	NamedDataTable::DataTableRow * newRow = new NamedDataTable::DataTableRow;

	int index = 0;

	const char * c = 0;

	for (index = 0; index < ndt->getNumColumns(); ++index)
	{
		XmlTreeNode cellNode = rowNode.getChildElementNodeNamed( ndt->getColumnName(index).c_str() );

		c = "";

		if (!cellNode.isNull())
		{
			XmlTreeNode textNode = cellNode.getFirstChildTextNode();

			if (!textNode.isNull())
				c = textNode.getTextValue();
		}

		newRow->push_back( _getNewCell(ndt->getDataTypeForColumn(index), c ));
	}

	ndt->m_rows.push_back(newRow);
}


// ----------------------------------------------------------------------

void DataTableWriter::_loadFromSpreadsheetTab(const char * filename)
{
	NamedDataTable * ndt;

	NOT_NULL(filename);

	StdioFile inputFile(filename,"r");
	FATAL(!inputFile.isOpen(), ("Could not open %s.", filename));

	int fileLength = inputFile.length();
	int bufferLength = fileLength + 1;
	char* buffer = new char[bufferLength];
	memset(buffer, 0, bufferLength);

	int bytes_read = inputFile.read(buffer, fileLength);
	FATAL(!bytes_read, ("Didn't read in any bytes when loading %s.", filename));

	FATAL(buffer[bytes_read-1] != '\n', ("No newline at end of file when loading %s.", filename));

	// Terminate the buffer because apparently fread can put garbage in the buffer
	// past the end of the file data
	buffer[bytes_read] = 0;

	const char* newLine = strchr(buffer, '\n');
	FATAL(!newLine, ("No end of line while looking for column row when loading %s.", filename));

	FATAL(!m_tables.empty(), ("TAB file does not support multiple tables when loading %s.", filename));

	m_tables.push_back(new NamedDataTable());
	ndt = m_tables.back();
	ndt->setName(getTableNameFromTabFile(filename).c_str());

	_loadColumnNames(ndt, buffer);
	const char* currentPos = newLine + 1;

	newLine = strchr(currentPos, '\n');
	FATAL(!newLine, ("No end of line while looking for column row when loading %s.", filename));

	_loadTypes(ndt, currentPos);
	currentPos = newLine + 1;

	newLine = strchr(currentPos, '\n');

	for(int count = 0; newLine; ++count)
	{
		_loadRow(ndt, currentPos, count);
		currentPos = newLine + 1;

		newLine = strchr(currentPos, '\n');
	}
	FATAL(ndt->getNumRows() < 1, ("No rows in the table when loading %s.", filename));

	delete [] buffer;
	inputFile.close();

}

// ----------------------------------------------------------------------

bool DataTableWriter::save(const char * outputFileName, bool optional) const
{
	if (!outputFileName || outputFileName[0] != '\0')
	{
		DEBUG_FATAL(true, ("OutputFileName is nullptr or empty."));
		return false;
	}

	if (m_tables.size() > 1)
	{
		DEBUG_FATAL(true, ("OutputFileName not supported on DataTableWriter with multiple tables."));
		return false;
	}

	if (m_tables.size() == 0)
	{
		DEBUG_FATAL(true, ("DataTableWriter is empty."));
		return false;
	}


	return _writeTable( m_tables.front(), outputFileName, optional );
}

// ----------------------------------------------------------------------

void DataTableWriter::save (Iff & iff) const
{
	FATAL(m_tables.size() == 0, ("No table loaded."));
	FATAL(m_tables.size() > 1, ("Multiple tables not supported."));
	NamedDataTable * ndt = *m_tables.begin();

	_saveTableToIff(iff, ndt);
}

// ----------------------------------------------------------------------

bool DataTableWriter::_writeTable(NamedDataTable * ndt, const char * outputFile, bool optional) const
{
	Iff iff(512,true,true);

	if (!FileNameUtils::isWritable(outputFile))
	{
		FATAL(!optional, ("ERROR: The output file is not available for writing: %s", outputFile));
		WARNING(true, ("ERROR: The output file is not available for writing: %s", outputFile));
		return false;
	}

	_saveTableToIff(iff, ndt);

	iff.write(outputFile);
	iff.close();
	return true;
}

// ----------------------------------------------------------------------

void DataTableWriter::_saveTableToIff(Iff & iff, NamedDataTable * ndt) const
{
	NOT_NULL(ndt);

	_checkIntegrity(ndt);
	iff.insertForm(DataTable::getDataTableIffId());
	iff.insertForm(TAG_0001);

	_saveColumns(ndt, iff);
	_saveTypes(ndt, iff);
	_saveRows(ndt, iff);

	iff.exitForm(TAG_0001);
	iff.exitForm();
}

// ----------------------------------------------------------------------

void DataTableWriter::_loadColumnNames(NamedDataTable * ndt, const char* row)
{
	const char* currentPos = row;
	const char* tab = strchr(currentPos, '\t');

	// we want endLine to be the first occurrence of a newline or carriage return
	const char* endLine = strchr(currentPos, '\n');
	const char* carriageRet = strchr(currentPos, '\r');
	endLine = (carriageRet && carriageRet == endLine - 1) ? carriageRet : endLine;

	while(tab && tab < endLine)
	{
		FATAL(currentPos == tab, ("Empty cell in column names"));
		std::string s(currentPos, tab - currentPos);
		ndt->m_columns.push_back(s);
		currentPos = tab + 1;
		tab = strchr(currentPos, '\t');
	}
	//Read last cell in row
	FATAL(endLine <= currentPos, ("Empty cell at end of col. names"));
	std::string s (currentPos, endLine - currentPos);
	ndt->m_columns.push_back(s);

}

// ----------------------------------------------------------------------

void DataTableWriter::_loadTypes(NamedDataTable * ndt, const char* row)
{
	const char* currentPos = row;
	const char* tab = strchr(currentPos, '\t');

	// we want endLine to be the first occurrence of a newline or carriage return
	const char* endLine = strchr(currentPos, '\n');
	const char* carriageRet = strchr(currentPos, '\r');
	endLine = (carriageRet && carriageRet == endLine - 1) ? carriageRet : endLine;

	while(tab && tab < endLine)
	{
		FATAL(currentPos == tab, ("Empty cell in type names"));
		std::string s(currentPos, tab - currentPos);
		ndt->m_types.push_back(new DataTableColumnType(unquotify(s)));
		currentPos = tab + 1;
		tab = strchr(currentPos, '\t');
	}
		//Read last cell in row
	FATAL(endLine <= currentPos, ("Empty cell at end of type names"));
	std::string s (currentPos, endLine - currentPos);
	ndt->m_types.push_back(new DataTableColumnType(unquotify(s)));
	FATAL(ndt->m_types.size() != ndt->m_columns.size(), ("Types != num columns"));

}

// ----------------------------------------------------------------------

DataTableCell *DataTableWriter::_getNewCell(DataTableColumnType const &columnType, char const *buf)
{
	std::string value(buf);
	if (!columnType.mangleValue(unquotify(value)))
	{
		FATAL(true, ("tried to create a cell with an invalid value [%s] for its type [%s])", value.c_str(), columnType.getTypeSpecString().c_str()));
	}

	switch (columnType.getBasicType())
	{
	case DataTableColumnType::DT_Int:
		return new DataTableCell(static_cast<int>(strtol(value.c_str(), nullptr, 0)));
		break;
	case DataTableColumnType::DT_Float:
		return new DataTableCell(static_cast<float>(atof(value.c_str())));
		break;
	case DataTableColumnType::DT_String:
	case DataTableColumnType::DT_Comment:
		return new DataTableCell(value.c_str());
		break;
	default:
		FATAL(true, ("Type with unknown basic type specified in the types row."));
		break;
	}
	return 0;
}

// ----------------------------------------------------------------------

void DataTableWriter::_loadRow(NamedDataTable * ndt, const char* row, int index)
{
#ifdef _DEBUG
	s_debugRow = index + 3;
#endif

	const char* currentPos = row;
	const char* tab = strchr(currentPos, '\t');

	// we want endLine to be the first occurrence of a newline or carriage return
	const char* endLine = strchr(currentPos, '\n');
	const char* carriageRet = strchr(currentPos, '\r');
	endLine = (carriageRet && carriageRet == endLine - 1) ? carriageRet : endLine;

	NamedDataTable::DataTableRow * newRow = new NamedDataTable::DataTableRow;

	char buf[16384];
	int count = 0;
	for(count = 0; tab && tab < endLine; ++count)
	{
		FATAL(tab < currentPos, ("Causality has been broken"));
		if (tab == currentPos)
		{
			//empty cell
			buf[0] = '\0';
		}
		else
		{
			FATAL(tab - currentPos > isizeof(buf) - 1, ("Cells can only be less than %d bytes", sizeof(buf) - 1));
			strncpy(buf, currentPos, tab - currentPos);
			buf[tab - currentPos] = '\0';
		}

		newRow->push_back(_getNewCell(ndt->getDataTypeForColumn(count), buf));

		currentPos = tab + 1;
		tab = strchr(currentPos, '\t');
	}

	//Still need the last cell
	{
		FATAL(endLine < currentPos, ("Empty cell at end of row %d", index));
		if (endLine == currentPos)
		{
			//empty cell
			buf[0] = '\0';
		}
		else
		{
			FATAL(endLine - currentPos > isizeof(buf) - 1, ("Cells can only be less than %d bytes", sizeof(buf) - 1));
			strncpy(buf, currentPos, endLine - currentPos);
			buf[endLine - currentPos] = '\0';
		}

		newRow->push_back(_getNewCell(ndt->getDataTypeForColumn(count), buf));
	}

	//Fill in empty rows:
	int oldSize = static_cast<int>(newRow->size());
	buf[0] = '\0';
	for (int i = 0; i < static_cast<int>(ndt->m_columns.size()) - oldSize; ++i)
	{
		newRow->push_back(_getNewCell(ndt->getDataTypeForColumn(i+oldSize), buf));
	}
	ndt->m_rows.push_back(newRow);
}

// ----------------------------------------------------------------------

void DataTableWriter::_saveColumns(NamedDataTable * ndt, Iff& iff) const
{
	iff.insertChunk(TAG(C,O,L,S));
	NamedDataTable::DataTableColumnTypeVector::iterator j;
	int numCols = 0;
	for (j = ndt->m_types.begin(); j!=ndt->m_types.end(); ++j)
	{
		if ((*j)->getType() != DataTableColumnType::DT_Comment)
			++numCols;
	}
	iff.insertChunkData(&numCols, sizeof(int));
	std::vector<std::string>::iterator i = ndt->m_columns.begin();
	j = ndt->m_types.begin();
	for (; i!= ndt->m_columns.end(); ++i, ++j)
	{
		if ((*j)->getType() != DataTableColumnType::DT_Comment)
			iff.insertChunkString(i->c_str());
	}
	iff.exitChunk(TAG(C,O,L,S));
}

// ----------------------------------------------------------------------

void DataTableWriter::_saveTypes(NamedDataTable * ndt, Iff& iff) const
{
	iff.insertChunk(TAG(T,Y,P,E));
	NamedDataTable::DataTableColumnTypeVector::iterator i = ndt->m_types.begin();
	for (; i!= ndt->m_types.end(); ++i)
	{
		if ((*i)->getType() != DataTableColumnType::DT_Comment)
			iff.insertChunkString((*i)->getTypeSpecString().c_str());
	}

	iff.exitChunk(TAG(T,Y,P,E));

}

// ----------------------------------------------------------------------

void DataTableWriter::_saveRows(NamedDataTable * ndt, Iff& iff) const
{
	iff.insertChunk(TAG(R,O,W,S));

	int numRows = static_cast<int>(ndt->m_rows.size());
	iff.insertChunkData(&numRows, sizeof(int));
	std::vector<NamedDataTable::DataTableRow* >::iterator i = ndt->m_rows.begin();
	for (; i!= ndt->m_rows.end(); ++i)
	{
		std::vector<const DataTableCell *>::iterator j = (*i)->begin();
		int count = 0;
		for (; j != (*i)->end(); ++j)
		{
			switch (ndt->m_types[count]->getBasicType())
			{
			case DataTableColumnType::DT_Int:
				{
				const DataTableCell * cell = *j;
				NOT_NULL(cell);
				int tmp = cell->getIntValue();
				iff.insertChunkData(&tmp, sizeof(int));
				break;
				}
			case DataTableColumnType::DT_Float:
				{
				const DataTableCell * cell = *j;
				NOT_NULL(cell);
				float tmp = cell->getFloatValue();
				iff.insertChunkData(&tmp, sizeof(float));
				break;
				}
			case DataTableColumnType::DT_String:
				{
				const DataTableCell * cell = *j;
				NOT_NULL(cell);
				iff.insertChunkString(cell->getStringValue());
				break;
				}
			case DataTableColumnType::DT_Comment:
				{
				}
				break;
			default:
				FATAL(true, ("bad case"));
				break;
			}
			++count;
		}
		//should we enter a seperator here between rows?
	}

	iff.exitChunk(TAG(R,O,W,S));
}

// ----------------------------------------------------------------------

bool DataTableWriter::_checkIntegrity(const NamedDataTable * ndt) const
{
	FATAL(ndt->m_columns.empty(), ("empty columns"));
	FATAL(ndt->m_columns.size() != ndt->m_types.size(), ("size mismatch"));
	FATAL(ndt->m_rows.empty(), ("empty rows"));
	FATAL(ndt->m_columns.size() != ndt->m_rows[0]->size(), ("size mismatch %d %d", ndt->m_columns.size(), ndt->m_rows[0]->size()));

	// Check unique constraints
	int column = 0;
	for (NamedDataTable::DataTableColumnTypeVector::const_iterator i = ndt->m_types.begin(); i != ndt->m_types.end(); ++i)
	{
		if ((*i)->areUniqueCellsRequired())
		{
			for (std::vector<NamedDataTable::DataTableRow *>::const_iterator j = ndt->m_rows.begin(); j != ndt->m_rows.end(); ++j)
			{
				std::vector<NamedDataTable::DataTableRow *>::const_iterator k = j;
				++k;
				for ( ; k != ndt->m_rows.end(); ++k)
				{
					switch ((*i)->getBasicType())
					{
					case DataTableColumnType::DT_Int:
						FATAL(
							(**j)[column]->getIntValue() == (**k)[column]->getIntValue(),
							(
								"unique constraint not satisfied for column %s, value %d",
								ndt->m_columns[column].c_str(),
								(**j)[column]->getIntValue()
							)
						);
						break;
					case DataTableColumnType::DT_Float:
						FATAL(
							(**j)[column]->getFloatValue() == (**k)[column]->getFloatValue(),
							(
								"unique constraint not satisfied for column %s, value %g",
								ndt->m_columns[column].c_str(),
								static_cast<double>((**j)[column]->getFloatValue())
							)
						);
						break;
					case DataTableColumnType::DT_String:
						FATAL(
							strcmp((**j)[column]->getStringValue(), (**k)[column]->getStringValue())==0,
							(
								"unique constraint not satisfied for column %s, value %s",
								ndt->m_columns[column].c_str(),
								(**j)[column]->getStringValue()
							)
						);
						break;
					default:
						break;
					}
				}
			}
		}
		++column;
	}

	return true;
}

// ----------------------------------------------------------------------

void DataTableWriter::setOutputPath(const char * path)
{
	m_path = path;
}

// ----------------------------------------------------------------------

void DataTableWriter::getTableNames(std::vector<std::string> & v) const
{
	NamedDataTableList::const_iterator t = m_tables.begin();

	for (; t != m_tables.end(); ++t)
	{
		NamedDataTable const * const ndt = *t;
		v.push_back(ndt->getName());
	}
}

// ----------------------------------------------------------------------

bool DataTableWriter::save (bool optional) const
{
	bool retval = true;

	NamedDataTableList::const_iterator i = m_tables.begin();
	for(; i != m_tables.end(); ++i)
	{
		std::string outputFile;
		getTableOutputFileName((*i)->getName().c_str(), outputFile);
		retval = _writeTable(*i, outputFile.c_str(), optional) && retval;
	}

	return retval;

}

// ----------------------------------------------------------------------

void DataTableWriter::getTableOutputFileName( const char* tableName, std::string& fileName ) const
{
	if (tableName != 0)
	{
		fileName = m_path;
		fileName.append(tableName);
		fileName.append(".iff");
	}
}

// ----------------------------------------------------------------------

bool DataTableWriter::saveTable( const char* tableName, const char* fileName, bool optional ) const
{
	bool ret = true;

	NamedDataTableList::const_iterator i;
	for (i = m_tables.begin(); i != m_tables.end(); ++i)
	{
		if (strcmp((*i)->getName().c_str(), tableName) == 0)
			break;
	}

	if (i != m_tables.end())
	{
		if (!fileName || fileName[0] != '\0')
		{
			std::string outputFile;
			getTableOutputFileName((*i)->getName().c_str(), outputFile);

			ret = _writeTable(*i, outputFile.c_str(), optional);
		}
		else
			ret = _writeTable(*i, fileName, optional);
	}

	return ret;
}

// ======================================================================

