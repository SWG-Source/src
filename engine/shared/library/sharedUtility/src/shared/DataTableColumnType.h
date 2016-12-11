// ======================================================================
//
// DataTableColumnType.h
//
// copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_DataTableColumnType_H
#define INCLUDED_DataTableColumnType_H

#include <string>

class DataTableCell;

class DataTableColumnType
{
public:
	explicit DataTableColumnType(std::string const &desc);
	~DataTableColumnType ();
	DataTableColumnType(DataTableColumnType const &rhs);
	DataTableColumnType &operator=(DataTableColumnType const &rhs);

	enum DataType
	{
		DT_Int,
		DT_Float,
		DT_String,
		DT_Unknown,
		DT_Comment,
		// specializations of DT_Int basic type
		DT_HashString,
		DT_Enum,
		DT_Bool,		
		// specializations of DT_String basic type
		DT_PackedObjVars,
		DT_BitVector,
	};

	DataType getType() const;
	DataType getBasicType() const;
	std::string const &getTypeSpecString() const;
	bool mangleValue(std::string &value) const;
	DataTableCell const *getDefaultCell() const;
	bool areUniqueCellsRequired() const;

private:
	bool lookupEnum(std::string const &label, int &result) const;
	bool lookupBitVector(std::string const &label, int &result) const;

private:
	DataTableColumnType();
	void createDefaultCell();

	std::string m_typeSpecString;
	DataType m_type;
	DataType m_basicType;
	std::string m_defaultValue;
	typedef std::map<std::string, int> StringIntMap;
	StringIntMap * m_enumMap;
	DataTableCell * m_defaultCell;
};

inline const DataTableCell * DataTableColumnType::getDefaultCell() const
{
	return m_defaultCell;
}

#endif // INCLUDED_DataTableColumnType_H

