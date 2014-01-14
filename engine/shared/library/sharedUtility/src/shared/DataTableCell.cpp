// ======================================================================
//
// DataTableCell.cpp
// 
// copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedUtility/FirstSharedUtility.h"
#include "sharedUtility/DataTableCell.h"

#include "sharedFoundation/Crc.h"
#include "sharedFoundation/CrcString.h"
#include "sharedFoundation/MemoryBlockManager.h"


// ======================================================================

MemoryBlockManager *DataTableCell::ms_memoryBlockManager;

// ----------------------------------------------------------------------

void DataTableCell::install()
{
	DEBUG_FATAL(ms_memoryBlockManager, ("DataTableCell is not installed"));
	ms_memoryBlockManager = new MemoryBlockManager("DataTableCell::ms_memoryBlockManager", true, sizeof(DataTableCell), 0, 0, 0);
}

// ----------------------------------------------------------------------

void DataTableCell::remove()
{
	DEBUG_FATAL(!ms_memoryBlockManager,("DataTableCell is not installed"));
	delete ms_memoryBlockManager;
	ms_memoryBlockManager = 0;
}

// ----------------------------------------------------------------------

void *DataTableCell::operator new(size_t size)
{
	UNREF(size);
	DEBUG_FATAL(!ms_memoryBlockManager,("DataTableCell is not installed"));
	DEBUG_FATAL(size != sizeof(DataTableCell),("bad size"));
	return ms_memoryBlockManager->allocate();
}

// ----------------------------------------------------------------------

void DataTableCell::operator delete(void *pointer)
{
	DEBUG_FATAL(!ms_memoryBlockManager,("DataTableCell is not installed"));
	ms_memoryBlockManager->free(pointer);
}

// ----------------------------------------------------------------------

DataTableCell::DataTableCell(int value)
:	 m_type(CT_int)
{
	m_value.m_i=value;
}

// ----------------------------------------------------------------------

DataTableCell::DataTableCell(float value)
:	 m_type(CT_float)
{
	m_value.m_f = value;
}

// ----------------------------------------------------------------------

DataTableCell::DataTableCell(const char *value)
:	 m_type(CT_string)
{
	if (!value || !*value)
	{
		m_value.m_s.m_sz = 0;
		m_value.m_s.m_crc = 0;
	}
	else
	{
		int len = strlen(value) + 1;
		char *s = new char[len];
		
		// normalize copies the output into s, so we can get the crc
		CrcString::normalize(s, value);
		m_value.m_s.m_crc = Crc::calculate(s);

		// copy the correct (unnormalized) string into s (this saves us the allocation of a tmp string)
		strcpy(s, value);
		m_value.m_s.m_sz = s;
	}
}

// ----------------------------------------------------------------------

DataTableCell::~DataTableCell()
{
	if (m_type==CT_string && m_value.m_s.m_sz!=0)
	{
		delete [] const_cast<char *>(m_value.m_s.m_sz);
	#ifdef _DEBUG
		m_value.m_s.m_sz=0;
	#endif
	}
}
