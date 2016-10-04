// ======================================================================
//
// MountValidScaleRangeTable.cpp
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/MountValidScaleRangeTable.h"

#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/CrcString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/LessPointerComparator.h"
#include "sharedFoundation/PersistentCrcString.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedFoundation/TemporaryCrcString.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"

#include <algorithm>
#include <iterator>
#include <map>

// ======================================================================

namespace MountValidScaleRangeTableNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class Saddle
	{
	public:

		Saddle(int seatingCapacity, float appearanceScaleMin, float appearanceScaleMax);

		int   getSeatingCapacity() const;
		float getAppearanceScaleMin() const;
		float getAppearanceScaleMax() const;

	private:

		// Disabled.
		Saddle();
		Saddle &operator =(Saddle const&);

	private:

		int const    m_seatingCapacity;
		float const  m_appearanceScaleMin;
		float const  m_appearanceScaleMax;

	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	typedef std::vector<Saddle*>  SaddleVector;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class MountableCreature
	{
	public:

		explicit MountableCreature(CrcString const &creatureAppearanceName);
		~MountableCreature();

		void addValidScaleRangeForSaddleCapacity(int saddleCapacity, float validScaleMin, float validScaleMax);

		CrcString const &getCreatureAppearanceName() const;

		int  getNumberOfSeatConfigurations() const;
		void getSaddleCapacityAndRangeForSeatConfigurationIndex(int seatConfigurationIndex, int &saddleCapacity, float &validScaleMin, float &validScaleMax) const;

	private:

		// Disabled.
		MountableCreature();
		MountableCreature &operator =(MountableCreature const&);

	private:

		PersistentCrcString const  m_creatureAppearanceName;
		SaddleVector               m_saddles;

	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	typedef std::map<CrcString const*, MountableCreature*, LessPointerComparator>  MountableCreatureTable;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	void                     remove();
	int                      getRequiredColumnNumberFromNamedTable(char const *filename, DataTable const *table, char const *columnName);
	void                     loadTableData(char const *filename);
	MountableCreature const *findMountableCreatureFromAppearanceName(CrcString const &creatureAppearanceName);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool                    s_installed;
	MountableCreatureTable  s_mountableCreatureTable;
}

using namespace MountValidScaleRangeTableNamespace;

// ======================================================================
// class MountValidScaleRangeTableNamespace::Saddle
// ======================================================================

MountValidScaleRangeTableNamespace::Saddle::Saddle(int seatingCapacity, float appearanceScaleMin, float appearanceScaleMax) :
	m_seatingCapacity(seatingCapacity),
	m_appearanceScaleMin(appearanceScaleMin),
	m_appearanceScaleMax(appearanceScaleMax)
{
}

// ----------------------------------------------------------------------

int MountValidScaleRangeTableNamespace::Saddle::getSeatingCapacity() const
{
	return m_seatingCapacity;
}

// ----------------------------------------------------------------------

float MountValidScaleRangeTableNamespace::Saddle::getAppearanceScaleMin() const
{
	return m_appearanceScaleMin;
}

// ----------------------------------------------------------------------

float MountValidScaleRangeTableNamespace::Saddle::getAppearanceScaleMax() const
{
	return m_appearanceScaleMax;
}

// ======================================================================
// class MountValidScaleRangeTableNamespace::MountableCreature
// ======================================================================

MountValidScaleRangeTableNamespace::MountableCreature::MountableCreature(CrcString const &creatureAppearanceName) :
	m_creatureAppearanceName(creatureAppearanceName),
	m_saddles()
{
}

// ----------------------------------------------------------------------

MountValidScaleRangeTableNamespace::MountableCreature::~MountableCreature()
{
	std::for_each(m_saddles.begin(), m_saddles.end(), PointerDeleter());
}

// ----------------------------------------------------------------------

void MountValidScaleRangeTableNamespace::MountableCreature::addValidScaleRangeForSaddleCapacity(int saddleCapacity, float validScaleMin, float validScaleMax)
{
#ifdef _DEBUG
	//-- FATAL if we already have an entry for the given saddle capacity.
	SaddleVector::iterator const endIt = m_saddles.end();
	for (SaddleVector::iterator it = m_saddles.begin(); it != endIt; ++it)
	{
		NOT_NULL(*it);
		DEBUG_FATAL((*it)->getSeatingCapacity() == saddleCapacity, ("MountValidScaleRangeTable: data for mount appearance [%s] has multiple entries for saddle capacity [%d], please fix data by removing the duplicate entry.", getCreatureAppearanceName().getString(), saddleCapacity));
	}
#endif

	m_saddles.push_back(new Saddle(saddleCapacity, validScaleMin, validScaleMax));
}

// ----------------------------------------------------------------------

CrcString const &MountValidScaleRangeTableNamespace::MountableCreature::getCreatureAppearanceName() const
{
	return m_creatureAppearanceName;
}

// ----------------------------------------------------------------------

int MountValidScaleRangeTableNamespace::MountableCreature::getNumberOfSeatConfigurations() const
{
	return static_cast<int>(m_saddles.size());
}

// ----------------------------------------------------------------------

void MountValidScaleRangeTableNamespace::MountableCreature::getSaddleCapacityAndRangeForSeatConfigurationIndex(int seatConfigurationIndex, int &saddleCapacity, float &validScaleMin, float &validScaleMax) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, seatConfigurationIndex, getNumberOfSeatConfigurations());
	
	Saddle const *const saddle = m_saddles[static_cast<SaddleVector::size_type>(seatConfigurationIndex)];
	NOT_NULL(saddle);

	saddleCapacity = saddle->getSeatingCapacity();
	validScaleMin  = saddle->getAppearanceScaleMin();
	validScaleMax  = saddle->getAppearanceScaleMax();
}

// ======================================================================
// namespace MountValidScaleRangeTable
// ======================================================================

void MountValidScaleRangeTableNamespace::remove()
{
	std::for_each(s_mountableCreatureTable.begin(), s_mountableCreatureTable.end(), PointerDeleterPairSecond());
	s_mountableCreatureTable.clear();

	s_installed = false;
}

// ----------------------------------------------------------------------

int MountValidScaleRangeTableNamespace::getRequiredColumnNumberFromNamedTable(char const *filename, DataTable const *table, char const *columnName)
{
	NOT_NULL(columnName);

	int const columnNumber = table->findColumnNumber(columnName);
	FATAL(columnNumber < 0, ("failed to find column name [%s] in MountValidScaleRangeTable file [%s].", columnName, filename));

	return columnNumber;
}

// ----------------------------------------------------------------------

void MountValidScaleRangeTableNamespace::loadTableData(char const *filename)
{
	NOT_NULL(filename);

	//-- Load the data table.
	DataTable *const table = DataTableManager::getTable(filename, true);
	FATAL(!table, ("MountValidScaleRangeTable data file [%s] failed to open.", filename));

	//-- Find required data column numbers.
	int const appearanceNameColumnNumber = getRequiredColumnNumberFromNamedTable(filename, table, "appearance_name");
	int const saddleCapacityColumnNumber = getRequiredColumnNumberFromNamedTable(filename, table, "saddle_capacity");
	int const validScaleMinColumnNumber  = getRequiredColumnNumberFromNamedTable(filename, table, "valid_scale_min");
	int const validScaleMaxColumnNumber  = getRequiredColumnNumberFromNamedTable(filename, table, "valid_scale_max");

	//-- Loop through data, constructing entries as necessary.
	int const rowCount = table->getNumRows();
	for (int rowIndex = 0; rowIndex < rowCount; ++rowIndex)
	{
		//-- Get creature appearance name.
		std::string const &mountableCreatureAppearanceName = table->getStringValue(appearanceNameColumnNumber, rowIndex);
		TemporaryCrcString const mountableCreatureAppearanceNameCrc(mountableCreatureAppearanceName.c_str(), true);

		//-- Find or create new MountableCreature instance for this creature name.
		MountableCreature *mountableCreature = nullptr;

		MountableCreatureTable::iterator lowerBoundIt = s_mountableCreatureTable.lower_bound((const CrcString*)&mountableCreatureAppearanceNameCrc);
		bool const mountableCreatureEntryExists = ((lowerBoundIt != s_mountableCreatureTable.end()) && !s_mountableCreatureTable.key_comp()(static_cast<CrcString const*>(&mountableCreatureAppearanceNameCrc), lowerBoundIt->first));
		
		if (mountableCreatureEntryExists)
			mountableCreature = lowerBoundIt->second;
		else
		{
			mountableCreature = new MountableCreature(mountableCreatureAppearanceNameCrc);
			MountableCreatureTable::iterator insertedIterator = s_mountableCreatureTable.insert(lowerBoundIt, MountableCreatureTable::value_type(&mountableCreature->getCreatureAppearanceName(), mountableCreature));
			UNREF(insertedIterator);
			DEBUG_FATAL(insertedIterator == s_mountableCreatureTable.end(), ("inserting new MountableCreature with appearance name [%s] failed, return iterator == map.end().", mountableCreatureAppearanceNameCrc.getString()));
		}
		NOT_NULL(mountableCreature);

		//-- Get seating configuration and valid appearance scale range for the configuration.
		int const saddleCapacity = table->getIntValue(saddleCapacityColumnNumber, rowIndex);
		DEBUG_FATAL(saddleCapacity < 1, ("invalid saddle capacity [%d] for creature appearance [%s] in file [%s].", saddleCapacity, mountableCreatureAppearanceName.c_str(), filename));

		float const appearanceScaleMin = table->getFloatValue(validScaleMinColumnNumber, rowIndex);
		float const appearanceScaleMax = table->getFloatValue(validScaleMaxColumnNumber, rowIndex);
		DEBUG_FATAL(appearanceScaleMin < 0.0f, ("invalid appearance scale min [%g] for creature appearance [%s] in file [%s].", appearanceScaleMin, mountableCreatureAppearanceName.c_str(), filename));
		DEBUG_FATAL(appearanceScaleMax < appearanceScaleMin, ("invalid appearance scale max [%g] given scale min [%g] for creature appearance [%s] in file [%s].", appearanceScaleMax, appearanceScaleMin, mountableCreatureAppearanceName.c_str(), filename));

		mountableCreature->addValidScaleRangeForSaddleCapacity(saddleCapacity, appearanceScaleMin, appearanceScaleMax);
	}

	DataTableManager::close(filename);
}

// ----------------------------------------------------------------------

MountValidScaleRangeTableNamespace::MountableCreature const *MountValidScaleRangeTableNamespace::findMountableCreatureFromAppearanceName(CrcString const &creatureAppearanceName)
{
	MountableCreatureTable::iterator const findIt = s_mountableCreatureTable.find(&creatureAppearanceName);
	if (findIt != s_mountableCreatureTable.end())
		return  findIt->second;
	else
	{
		DEBUG_WARNING(true, ("'datatables/mount/valid_scale_range.iff' missing entry for creature appearance name '%s'", creatureAppearanceName.getString()));
		return nullptr;
	}
}

// ======================================================================
// class MountValidScaleRangeTable: PUBLIC STATIC
// ======================================================================

void MountValidScaleRangeTable::install(char const *const filename)
{
	InstallTimer const installTimer("MountValidScaleRangeTable::install");

	DEBUG_FATAL(s_installed, ("MountValidScaleRangeTable is already installed."));

	loadTableData(filename);

	s_installed = true;
	ExitChain::add(MountValidScaleRangeTableNamespace::remove, "MountValidScaleRangeTable");
}

// ----------------------------------------------------------------------

MountValidScaleRangeTable::MountabilityStatus MountValidScaleRangeTable::doesCreatureSupportScaleAndSaddleCapacity(CrcString const &creatureAppearanceName, float creatureScale, int saddleCapacity)
{
	DEBUG_FATAL(!s_installed, ("MountValidScaleRangeTable not installed."));

	//-- Find MountableCreature entry for specified creature.
	MountableCreature const *const mountableCreature = findMountableCreatureFromAppearanceName(creatureAppearanceName);
	if (!mountableCreature)
	{
		// No entries for specified creature appearance.  This implies the specified creature appearance name is not mountable.
		return MS_speciesUnmountable;
	}

	//-- Find mountable creature's valid scale range for the given seat count.
	int    entrySaddleCapacity = 0;
	float  entryScaleMin       = 0.0f;
	float  entryScaleMax       = 0.0f;

	int const seatConfigurationCount = mountableCreature->getNumberOfSeatConfigurations();
	for (int seatConfigurationIndex = 0; seatConfigurationIndex < seatConfigurationCount; ++seatConfigurationIndex)
	{
		//-- Retrieve seat configuration data.
		mountableCreature->getSaddleCapacityAndRangeForSeatConfigurationIndex(seatConfigurationIndex, entrySaddleCapacity, entryScaleMin, entryScaleMax);

		//-- Validate seat configuration data.
		DEBUG_FATAL(entrySaddleCapacity < 1, ("invalid seat count [%d] retrieved from mountable creature data for appearance [%s].", entrySaddleCapacity, creatureAppearanceName.getString()));
		DEBUG_FATAL(entryScaleMin < 0.0f, ("invalid minimum appearance scale [%g] retrieved from mountable creature data for appearance [%s].", entryScaleMin, creatureAppearanceName.getString()));
		DEBUG_FATAL(entryScaleMax < entryScaleMin, ("invalid maximum appearance scale [%g], less than minimum appearance scale [%g], retrieved from mountable creature data for appearance [%s].", entryScaleMax, entryScaleMin, creatureAppearanceName.getString()));

		if (entrySaddleCapacity == saddleCapacity)
		{
			// Found a range specification for the seat count specified by the caller. Validate that the creature scale is within range.
			bool const scaleAndSeatCountSupported = (entryScaleMin <= creatureScale) && (entryScaleMax >= creatureScale);
			return scaleAndSeatCountSupported ? MS_creatureMountable: MS_speciesMountableScaleOutOfRange;
		}
	}

	//-- The specified saddle capacity was not supported.
	return MS_speciesMountableSeatingCapacityUnsupported;
}

// ----------------------------------------------------------------------

int MountValidScaleRangeTable::getCreatureCount()
{
	return static_cast<int>(s_mountableCreatureTable.size());
}

// ----------------------------------------------------------------------

CrcString const &MountValidScaleRangeTable::getCreatureName(int index)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getCreatureCount());
	
	MountableCreatureTable::const_iterator it = s_mountableCreatureTable.begin();
	std::advance(it, index);

	MountableCreature const *const mountableCreature = it->second;
	if (mountableCreature)
		return mountableCreature->getCreatureAppearanceName();
	else
		return PersistentCrcString::empty;
}

// ----------------------------------------------------------------------

bool MountValidScaleRangeTable::getScaleRangeForCreatureAndSaddleCapacity(CrcString const &creatureAppearanceName, int saddleCapacity, float &minimumScale, float &maximumScale)
{
	//-- Find MountableCreature entry for specified creature.
	MountableCreature const *const mountableCreature = findMountableCreatureFromAppearanceName(creatureAppearanceName);
	if (!mountableCreature)
	{
		// No entries for specified creature appearance.  This implies the specified creature appearance name is not mountable.
		return false;
	}

	//-- Find mountable creature's valid scale range for the given seat count.
	int    entrySaddleCapacity = 0;
	float  entryScaleMin       = 0.0f;
	float  entryScaleMax       = 0.0f;

	int const seatConfigurationCount = mountableCreature->getNumberOfSeatConfigurations();
	for (int seatConfigurationIndex = 0; seatConfigurationIndex < seatConfigurationCount; ++seatConfigurationIndex)
	{
		//-- Retrieve seat configuration data.
		mountableCreature->getSaddleCapacityAndRangeForSeatConfigurationIndex(seatConfigurationIndex, entrySaddleCapacity, entryScaleMin, entryScaleMax);

		//-- Validate seat configuration data.
		DEBUG_FATAL(entrySaddleCapacity < 1, ("invalid seat count [%d] retrieved from mountable creature data for appearance [%s].", entrySaddleCapacity, creatureAppearanceName.getString()));
		DEBUG_FATAL(entryScaleMin < 0.0f, ("invalid minimum appearance scale [%g] retrieved from mountable creature data for appearance [%s].", entryScaleMin, creatureAppearanceName.getString()));
		DEBUG_FATAL(entryScaleMax < entryScaleMin, ("invalid maximum appearance scale [%g], less than minimum appearance scale [%g], retrieved from mountable creature data for appearance [%s].", entryScaleMax, entryScaleMin, creatureAppearanceName.getString()));

		if (entrySaddleCapacity == saddleCapacity)
		{
			// Found a range specification for the seat count specified by the caller.
			minimumScale = entryScaleMin;
			maximumScale = entryScaleMax;
			return true;
		}
	}

	//-- The specified saddle capacity was not supported.
	return false;
}

// ======================================================================
