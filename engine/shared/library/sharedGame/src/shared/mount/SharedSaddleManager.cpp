// ======================================================================
//
// SharedSaddleManager.cpp
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/SharedSaddleManager.h"

#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/PersistentCrcString.h"
#include "sharedGame/GameObjectTypes.h"
#include "sharedGame/SharedObjectTemplate.h"
#include "sharedObject/Appearance.h"
#include "sharedObject/AppearanceTemplate.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/Object.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"

#include <algorithm>
#include <map>
#include <string>
#include <vector>

#ifdef _DEBUG
#include "sharedDebug/DebugFlags.h"
#endif

// ======================================================================

namespace SharedSaddleManagerNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	char const * const cs_logicalSaddleNameMapFile = "datatables/mount/logical_saddle_name_map.iff";
	char const * const cs_saddleAppearanceMapFile = "datatables/mount/saddle_appearance_map.iff";
	char const * const cs_riderPoseMapFile = "datatables/mount/rider_pose_map.iff";

	class LogicalSaddleMapEntry
	{
	public:

		virtual ~LogicalSaddleMapEntry();

		virtual CrcString const &getAppearanceTemplateName() const = 0;
		virtual CrcString const *getLogicalSaddleName() const = 0;

	protected:

		LogicalSaddleMapEntry();

	private:

		// Disabled.
		LogicalSaddleMapEntry(LogicalSaddleMapEntry const&);             //lint -esym(754, LogicalSaddleMapEntry::LogicalSaddleMapEntry) // unreferenced local member // defensive hiding
		LogicalSaddleMapEntry & operator=(LogicalSaddleMapEntry const&);

	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class PersistentLogicalSaddleMapEntry: public LogicalSaddleMapEntry
	{
	public:

		PersistentLogicalSaddleMapEntry(char const *mountAppearanceTemplateName, char const *logicalSaddleName);

		virtual CrcString const &getAppearanceTemplateName() const;
		virtual CrcString const *getLogicalSaddleName() const;

	private:

		// Disabled.
		PersistentLogicalSaddleMapEntry();
		PersistentLogicalSaddleMapEntry(PersistentLogicalSaddleMapEntry const&);             //lint -esym(754, PersistentLogicalSaddleMapEntry::PersistentLogicalSaddleMapEntry) // unreferenced local member // defensive hiding
		PersistentLogicalSaddleMapEntry &operator =(PersistentLogicalSaddleMapEntry const&);

	private:

		PersistentCrcString const m_mountAppearanceTemplateName;
		PersistentCrcString const m_logicalSaddleName;

	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class SaddleAppearanceMapEntry
	{
	public:

		virtual ~SaddleAppearanceMapEntry();

		virtual CrcString const &getLogicalSaddleName() const = 0;
		virtual int              getSeatingCapacity() const = 0;
		virtual CrcString const *getSaddleAppearanceName() const = 0;
		virtual CrcString const *getClientDataFilename  () const = 0;

	protected:

		SaddleAppearanceMapEntry();

	private:

		// Disabled.
		SaddleAppearanceMapEntry(SaddleAppearanceMapEntry const&);             //lint -esym(754, SaddleAppearanceMapEntry::SaddleAppearanceMapEntry) // unrefereced local member // defensive hiding
		SaddleAppearanceMapEntry &operator =(SaddleAppearanceMapEntry const&);

	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class PersistentSaddleAppearanceMapEntry: public SaddleAppearanceMapEntry
	{
	public:

		PersistentSaddleAppearanceMapEntry(char const *logicalSaddleName, int seatingCapacity, char const *saddleAppearanceName, const char * clientDataFilename);

		virtual CrcString const &getLogicalSaddleName() const;
		virtual int              getSeatingCapacity() const;
		virtual CrcString const *getSaddleAppearanceName() const;
		virtual CrcString const *getClientDataFilename  () const;

	private:

		// Disabled.
		PersistentSaddleAppearanceMapEntry();
		PersistentSaddleAppearanceMapEntry(PersistentSaddleAppearanceMapEntry const&);             //lint -esym(754, PersistentSaddleAppearanceMapEntry::PersistentSaddleAppearanceMapEntry) // unrefereced local member // defensive hiding
		PersistentSaddleAppearanceMapEntry &operator =(PersistentSaddleAppearanceMapEntry const&);

	private:

		PersistentCrcString const m_logicalSaddleName;
		int                 const m_seatingCapacity;
		PersistentCrcString const m_saddleAppearanceName;
		PersistentCrcString const m_clientDataFilename;
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class RiderPoseMapEntry
	{
	public:

		virtual ~RiderPoseMapEntry();

		virtual CrcString const &getSaddleAppearanceName() const = 0;
		virtual int              getRiderSeatIndex() const = 0;
		virtual CrcString const *getRiderPoseName() const = 0;

	protected:

		RiderPoseMapEntry();

	private:

		RiderPoseMapEntry(RiderPoseMapEntry const&);             //lint -esym(754, RiderPoseMapEntry::RiderPoseMapEntry) // local member not referenced // defensive hiding
		RiderPoseMapEntry &operator =(RiderPoseMapEntry const&);

	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class PersistentRiderPoseMapEntry : public RiderPoseMapEntry
	{
	public:

		PersistentRiderPoseMapEntry(char const *saddleAppearanceName, int riderSeatIndex, char const *riderPoseName);

		virtual CrcString const &getSaddleAppearanceName() const;
		virtual int              getRiderSeatIndex() const;
		virtual CrcString const *getRiderPoseName() const;

	private:

		// Disabled.
		PersistentRiderPoseMapEntry();
		PersistentRiderPoseMapEntry(PersistentRiderPoseMapEntry const&);             //lint -esym(754, PersistentRiderPoseMapEntry::PersistentRiderPoseMapEntry) // local member not referenced // defensive hiding
		PersistentRiderPoseMapEntry &operator =(PersistentRiderPoseMapEntry const&);

	private:

		PersistentCrcString const m_saddleAppearanceName;
		int const                 m_riderSeatIndex;
		PersistentCrcString const m_riderPoseName;

	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class TemporaryRiderPoseMapEntry: public RiderPoseMapEntry
	{
	public:

		TemporaryRiderPoseMapEntry(CrcString const &saddleAppearanceName, int riderSeatIndex);

		virtual CrcString const &getSaddleAppearanceName() const;
		virtual int              getRiderSeatIndex() const;
		virtual CrcString const *getRiderPoseName() const;

	private:

		// Disabled.
		TemporaryRiderPoseMapEntry();
		TemporaryRiderPoseMapEntry(TemporaryRiderPoseMapEntry const&);             //lint -esym(754, TemporaryRiderPoseMapEntry::TemporaryRiderPoseMapEntry) // local member not referenced // defensive hiding
		TemporaryRiderPoseMapEntry &operator =(TemporaryRiderPoseMapEntry const&);

	private:

		CrcString const &m_saddleAppearanceName;
		int const        m_riderSeatIndex;

	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	typedef PersistentCrcString const Key;
	typedef std::map<Key /* AppearanceTemplateName */, LogicalSaddleMapEntry *>    LogicalSaddleMapEntryVector;
	typedef std::map<Key /* LogicalSaddleName */, SaddleAppearanceMapEntry *>  SaddleAppearanceMapEntryVector;
	typedef std::multimap<Key /* SaddleAppearanceName */, RiderPoseMapEntry *> RiderPoseMapEntryVector;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	void    remove();

	int     getRequiredColumnNumberFromNamedTable(char const *filename, DataTable const *table, char const *columnName);
	void    loadLogicalSaddleNameMapTableData(char const *filename);
	void    loadSaddleAppearanceMapTableData(char const *filename);
	void    loadRiderPoseMapTableData(char const *filename);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	ConstCharCrcString const cs_defaultCoveringLogicalAppearanceName("lookup/mnt_wr_default_covering");
	ConstCharCrcString const cs_saddleHardpointName("saddle");

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool                            s_installed;
	LogicalSaddleMapEntryVector     s_logicalSaddleMapTable;
	SaddleAppearanceMapEntryVector  s_saddleAppearanceMapTable;
	RiderPoseMapEntryVector         s_riderPoseMapTable;

	template <typename theMapType> void clearMap(theMapType & map)
	{
		typename theMapType::iterator ii = map.begin();
		typename theMapType::iterator iiEnd = map.end();

		for (; ii != iiEnd; ++ii)
		{
			delete ii->second;
		}

		map.clear();
	}
}

using namespace SharedSaddleManagerNamespace;

// ======================================================================
// class SharedSaddleManagerNamespace::LogicalSaddleMapEntry
// ======================================================================

SharedSaddleManagerNamespace::LogicalSaddleMapEntry::~LogicalSaddleMapEntry()
{
}

// ======================================================================

SharedSaddleManagerNamespace::LogicalSaddleMapEntry::LogicalSaddleMapEntry()
{
}

// ======================================================================
// class SharedSaddleManagerNamespace::PersistentLogicalSaddleMapEntry
// ======================================================================

SharedSaddleManagerNamespace::PersistentLogicalSaddleMapEntry::PersistentLogicalSaddleMapEntry(char const * const mountAppearanceTemplateName, char const * const logicalSaddleName) :
	LogicalSaddleMapEntry(),
	m_mountAppearanceTemplateName(mountAppearanceTemplateName, true),
	m_logicalSaddleName(logicalSaddleName, true)
{
}

// ----------------------------------------------------------------------

CrcString const &SharedSaddleManagerNamespace::PersistentLogicalSaddleMapEntry::getAppearanceTemplateName() const
{
	return m_mountAppearanceTemplateName;
}

// ----------------------------------------------------------------------

CrcString const *SharedSaddleManagerNamespace::PersistentLogicalSaddleMapEntry::getLogicalSaddleName() const
{
	return &m_logicalSaddleName;
}

// ======================================================================
// class SharedSaddleManagerNamespace::SaddleAppearanceMapEntry
// ======================================================================

SharedSaddleManagerNamespace::SaddleAppearanceMapEntry::~SaddleAppearanceMapEntry()
{
}

// ======================================================================

SharedSaddleManagerNamespace::SaddleAppearanceMapEntry::SaddleAppearanceMapEntry()
{
}

// ======================================================================
// class SharedSaddleManagerNamespace::PersistentSaddleAppearanceMapEntry
// ======================================================================

SharedSaddleManagerNamespace::PersistentSaddleAppearanceMapEntry::PersistentSaddleAppearanceMapEntry(char const *logicalSaddleName, int seatingCapacity, char const *saddleAppearanceName, const char * clientDataFilename) :
	SaddleAppearanceMapEntry(),
	m_logicalSaddleName(logicalSaddleName, true),
	m_seatingCapacity(seatingCapacity),
	m_saddleAppearanceName(saddleAppearanceName, true),
	m_clientDataFilename (clientDataFilename, true)
{
}

// ----------------------------------------------------------------------

CrcString const &SharedSaddleManagerNamespace::PersistentSaddleAppearanceMapEntry::getLogicalSaddleName() const
{
	return m_logicalSaddleName;
}

// ----------------------------------------------------------------------

int SharedSaddleManagerNamespace::PersistentSaddleAppearanceMapEntry::getSeatingCapacity() const
{
	return m_seatingCapacity;
}

// ----------------------------------------------------------------------

CrcString const *SharedSaddleManagerNamespace::PersistentSaddleAppearanceMapEntry::getSaddleAppearanceName() const
{
	return &m_saddleAppearanceName;
}

// ----------------------------------------------------------------------

CrcString const *SharedSaddleManagerNamespace::PersistentSaddleAppearanceMapEntry::getClientDataFilename() const
{
	return &m_clientDataFilename;
}

// ======================================================================
// class SharedSaddleManagerNamespace::RiderPoseMapEntry
// ======================================================================

SharedSaddleManagerNamespace::RiderPoseMapEntry::~RiderPoseMapEntry()
{
}

// ======================================================================

SharedSaddleManagerNamespace::RiderPoseMapEntry::RiderPoseMapEntry()
{
}

// ======================================================================
// class SharedSaddleManagerNamespace::PersistentRiderPoseMapEntry
// ======================================================================

SharedSaddleManagerNamespace::PersistentRiderPoseMapEntry::PersistentRiderPoseMapEntry(char const *saddleAppearanceName, int riderSeatIndex, char const *riderPoseName) :
	RiderPoseMapEntry(),
	m_saddleAppearanceName(saddleAppearanceName, true),
	m_riderSeatIndex(riderSeatIndex),
	m_riderPoseName(riderPoseName, true)
{
}

// ----------------------------------------------------------------------

CrcString const &SharedSaddleManagerNamespace::PersistentRiderPoseMapEntry::getSaddleAppearanceName() const
{
	return m_saddleAppearanceName;
}

// ----------------------------------------------------------------------

int SharedSaddleManagerNamespace::PersistentRiderPoseMapEntry::getRiderSeatIndex() const
{
	return m_riderSeatIndex;
}

// ----------------------------------------------------------------------

CrcString const *SharedSaddleManagerNamespace::PersistentRiderPoseMapEntry::getRiderPoseName() const
{
	return &m_riderPoseName;
}

// ======================================================================
// class SharedSaddleManagerNamespace::TemporaryRiderPoseMapEntry
// ======================================================================

SharedSaddleManagerNamespace::TemporaryRiderPoseMapEntry::TemporaryRiderPoseMapEntry(CrcString const &saddleAppearanceName, int riderSeatIndex) :
	RiderPoseMapEntry(),
	m_saddleAppearanceName(saddleAppearanceName),
	m_riderSeatIndex(riderSeatIndex)
{
}

// ----------------------------------------------------------------------

CrcString const &SharedSaddleManagerNamespace::TemporaryRiderPoseMapEntry::getSaddleAppearanceName() const
{
	return m_saddleAppearanceName;
}

// ----------------------------------------------------------------------

int SharedSaddleManagerNamespace::TemporaryRiderPoseMapEntry::getRiderSeatIndex() const
{
	return m_riderSeatIndex;
}

// ----------------------------------------------------------------------

CrcString const *SharedSaddleManagerNamespace::TemporaryRiderPoseMapEntry::getRiderPoseName() const
{
	return nullptr;
}

// ======================================================================
// namespace SharedSaddleManagerNamespace
// ======================================================================

void SharedSaddleManagerNamespace::remove()
{
	DEBUG_FATAL(!s_installed, ("SharedSaddleManager not installed."));
	s_installed = false;

	SharedSaddleManagerNamespace::clearMap(s_logicalSaddleMapTable);
	SharedSaddleManagerNamespace::clearMap(s_saddleAppearanceMapTable);
	SharedSaddleManagerNamespace::clearMap(s_riderPoseMapTable);
}

// ----------------------------------------------------------------------

int SharedSaddleManagerNamespace::getRequiredColumnNumberFromNamedTable(char const *filename, DataTable const *table, char const *columnName)
{
	NOT_NULL(columnName);

	int const columnNumber = table->findColumnNumber(columnName);
	FATAL(columnNumber < 0, ("failed to find column name [%s] in file [%s].", columnName, filename));

	return columnNumber;
}

// ----------------------------------------------------------------------

void SharedSaddleManagerNamespace::loadLogicalSaddleNameMapTableData(char const *filename)
{
	NOT_NULL(filename);

	//-- Load the data table.
	DataTable *const table = DataTableManager::getTable(filename, true);
	FATAL(!table, ("SharedSaddleManager: failed to open data file [%s].", filename));

	//-- Find required data column numbers.
	int const appearanceTemplateNameColumnNumber = getRequiredColumnNumberFromNamedTable(filename, table, "sat_name");
	int const logicalSaddleNameColumnNumber  = getRequiredColumnNumberFromNamedTable(filename, table, "logical_saddle_name");

	//-- Loop through data, constructing entries as necessary.
	int const rowCount = table->getNumRows();

	for (int rowIndex = 0; rowIndex < rowCount; ++rowIndex)
	{
		//-- Get entry data.
		std::string const & appearanceTemplateName = table->getStringValue(appearanceTemplateNameColumnNumber, rowIndex);
		std::string const & logicalSaddleName      = table->getStringValue(logicalSaddleNameColumnNumber, rowIndex);

		//-- Create map entry, add to vector.

		PersistentLogicalSaddleMapEntry * const entry = new PersistentLogicalSaddleMapEntry(appearanceTemplateName.c_str(), logicalSaddleName.c_str());
		Key const key(appearanceTemplateName.c_str(), true);
		s_logicalSaddleMapTable[key] = entry;
	}

	DataTableManager::close(filename);
}

// ----------------------------------------------------------------------

void SharedSaddleManagerNamespace::loadSaddleAppearanceMapTableData(char const *filename)
{
	NOT_NULL(filename);

	//-- Load the data table.
	DataTable *const table = DataTableManager::getTable(filename, true);
	FATAL(!table, ("SharedSaddleManager: failed to open data file [%s].", filename));

	//-- Find required data column numbers.
	int const logicalSaddleNameColumnNumber    = getRequiredColumnNumberFromNamedTable(filename, table, "logical_saddle_name");
	int const seatingCapacityColumnNumber      = getRequiredColumnNumberFromNamedTable(filename, table, "saddle_capacity");
	int const saddleAppearanceNameColumnNumber = getRequiredColumnNumberFromNamedTable(filename, table, "saddle_appearance_name");
	int const clientDataFilenameColumnNumber   = getRequiredColumnNumberFromNamedTable(filename, table, "client_data_filename");

	//-- Loop through data, constructing entries as necessary.
	int const rowCount = table->getNumRows();

	for (int rowIndex = 0; rowIndex < rowCount; ++rowIndex)
	{
		//-- Get entry data.
		std::string const &logicalSaddleName    = table->getStringValue(logicalSaddleNameColumnNumber, rowIndex);
 		int         const  seatingCapacity      = table->getIntValue(seatingCapacityColumnNumber, rowIndex);
		std::string const &saddleAppearanceName = table->getStringValue(saddleAppearanceNameColumnNumber, rowIndex);
		std::string const &clientDataFilename   = table->getStringValue(clientDataFilenameColumnNumber,   rowIndex);

		//-- Create map entry, add to vector.
		PersistentSaddleAppearanceMapEntry * const entry = new PersistentSaddleAppearanceMapEntry(logicalSaddleName.c_str(), seatingCapacity, saddleAppearanceName.c_str(), clientDataFilename.c_str ());
		Key const key(logicalSaddleName.c_str(), true);
		s_saddleAppearanceMapTable[key] = entry;
	}

	DataTableManager::close(filename);
}

// ----------------------------------------------------------------------

void SharedSaddleManagerNamespace::loadRiderPoseMapTableData(char const * const filename)
{
	NOT_NULL(filename);

	//-- Load the data table.
	DataTable *const table = DataTableManager::getTable(filename, true);
	FATAL(!table, ("SharedSaddleManager: failed to open data file [%s].", filename));

	//-- Find required data column numbers.
	int const saddleAppearanceNameColumnNumber = getRequiredColumnNumberFromNamedTable(filename, table, "saddle_appearance_name");
	int const riderSeatIndexColumnNumber       = getRequiredColumnNumberFromNamedTable(filename, table, "seat_index");
	int const riderPoseNameColumnNumber        = getRequiredColumnNumberFromNamedTable(filename, table, "rider_pose");

	//-- Loop through data, constructing entries as necessary.
	int const rowCount = table->getNumRows();

	for (int rowIndex = 0; rowIndex < rowCount; ++rowIndex)
	{
		//-- Get entry data.
		std::string const &saddleAppearanceName = table->getStringValue(saddleAppearanceNameColumnNumber, rowIndex);
 		int         const  riderSeatIndex       = table->getIntValue(riderSeatIndexColumnNumber, rowIndex);
		std::string const &riderPoseName        = table->getStringValue(riderPoseNameColumnNumber, rowIndex);

		//-- Create map entry, add to vector.
		PersistentRiderPoseMapEntry * const entry = new PersistentRiderPoseMapEntry(saddleAppearanceName.c_str(), riderSeatIndex, riderPoseName.c_str());
		Key const key(saddleAppearanceName.c_str(), true);
		s_riderPoseMapTable.insert(std::pair<Key, PersistentRiderPoseMapEntry *>(key, entry));
	}

	DataTableManager::close(filename);
}

// ======================================================================
// class SharedSaddleManager: PUBLIC STATIC
// ======================================================================

void SharedSaddleManager::install()
{
	InstallTimer const installTimer("SharedSaddleManager::install");

	DEBUG_FATAL(s_installed, ("SharedSaddleManager already installed."));

	loadLogicalSaddleNameMapTableData(cs_logicalSaddleNameMapFile);
	loadSaddleAppearanceMapTableData(cs_saddleAppearanceMapFile);
	loadRiderPoseMapTableData(cs_riderPoseMapFile);

	s_installed = true;
	ExitChain::add(SharedSaddleManagerNamespace::remove, "SharedSaddleManager");
}

// ----------------------------------------------------------------------

CrcString const * SharedSaddleManager::getLogicalSaddleNameForMountAppearance(CrcString const & mountAppearanceTemplateName)
{
	DEBUG_FATAL(!s_installed, ("SharedSaddleManager not installed."));

	LogicalSaddleMapEntryVector::const_iterator ii = s_logicalSaddleMapTable.find(Key(mountAppearanceTemplateName));
	if (ii != s_logicalSaddleMapTable.end())
	{
		LogicalSaddleMapEntry const * const entry = ii->second;
		NOT_NULL(entry);
		return entry->getLogicalSaddleName();
	}

	return 0;
}

// ----------------------------------------------------------------------

CrcString const *SharedSaddleManager::getSaddleAppearanceForLogicalSaddleName(CrcString const & logicalSaddleName)
{
	DEBUG_FATAL(!s_installed, ("SharedSaddleManager not installed."));

	SaddleAppearanceMapEntryVector::const_iterator ii = s_saddleAppearanceMapTable.find(Key(logicalSaddleName));
	if (ii != s_saddleAppearanceMapTable.end())
	{
		SaddleAppearanceMapEntry const * const entry = ii->second;
		NOT_NULL(entry);
		return entry->getSaddleAppearanceName();
	}

	return 0;
}

// ----------------------------------------------------------------------

int SharedSaddleManager::getSaddleSeatingCapacityForLogicalSaddleName(CrcString const & logicalSaddleName)
{
	DEBUG_FATAL(!s_installed, ("SharedSaddleManager not installed."));

	SaddleAppearanceMapEntryVector::const_iterator ii = s_saddleAppearanceMapTable.find(Key(logicalSaddleName));
	if (ii != s_saddleAppearanceMapTable.end())
	{
		SaddleAppearanceMapEntry const * const entry = ii->second;
		NOT_NULL(entry);
		return entry->getSeatingCapacity();
	}

	return 0;
}

// ----------------------------------------------------------------------

CrcString const *SharedSaddleManager::getClientDataFilenameForLogicalSaddleName(CrcString const &logicalSaddleName)
{
	DEBUG_FATAL(!s_installed, ("SharedSaddleManager not installed."));

	SaddleAppearanceMapEntryVector::const_iterator ii = s_saddleAppearanceMapTable.find(Key(logicalSaddleName));
	if (ii != s_saddleAppearanceMapTable.end())
	{
		SaddleAppearanceMapEntry const * const entry = ii->second;
		NOT_NULL(entry);
		return entry->getClientDataFilename();
	}

	return 0;
}

// ----------------------------------------------------------------------

CrcString const *SharedSaddleManager::getRiderPoseForSaddleAppearanceNameAndRiderSeatIndex(CrcString const &saddleAppearanceName, int riderSeatIndex)
{
 	DEBUG_FATAL(!s_installed, ("SharedSaddleManager not installed."));

	typedef std::pair<RiderPoseMapEntryVector::const_iterator, RiderPoseMapEntryVector::const_iterator> Range;

	Range const range = s_riderPoseMapTable.equal_range(Key(saddleAppearanceName));

	RiderPoseMapEntryVector::const_iterator ii = range.first;

	for (; ii != range.second; ++ii)
	{
		RiderPoseMapEntry const * const entry = ii->second;
		NOT_NULL(entry);

		if (entry->getRiderSeatIndex() == riderSeatIndex)
		{
			return entry->getRiderPoseName();
		}
	}

	return 0;
}

// ======================================================================
