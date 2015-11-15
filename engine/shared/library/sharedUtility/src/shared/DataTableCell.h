// ======================================================================
//
// DataTableCell.h
// 
// copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_DataTableCell_H
#define INCLUDED_DataTableCell_H

// ======================================================================

class MemoryBlockManager;

// ======================================================================

class DataTableCell
{
public:

	static void install();
	static void remove();
	static void *operator new(size_t size, void *p) { UNREF(size); return p; }
	static void *operator new(size_t size);
	static void operator delete(void *pointer);

	enum CellType {
		 CT_string
		,CT_int
		,CT_float
	};

	explicit DataTableCell(int         value);
	explicit DataTableCell(float       value);
	explicit DataTableCell(const char *value);
	~DataTableCell();

	CellType    getType()           const { return m_type; }
	int         getIntValue()       const { return m_value.m_i; }
	float       getFloatValue()     const { return m_value.m_f; }
	const char *getStringValue()    const;
	int         getStringValueCrc() const { return m_value.m_s.m_crc; }

private:
	const CellType m_type;
	struct
	{
		union
		{
			int   m_i;
			float m_f;
			struct
			{
				const char *m_sz;
				int         m_crc;
			} m_s;
		};
	} m_value;


	DataTableCell &operator=(const DataTableCell &);

	static MemoryBlockManager *ms_memoryBlockManager;
};

inline const char *DataTableCell::getStringValue() const 
{ 
	if (m_value.m_s.m_sz)
	{
		return m_value.m_s.m_sz; 
	}
	else
	{
		return "";
	}
}

// ======================================================================

#endif
