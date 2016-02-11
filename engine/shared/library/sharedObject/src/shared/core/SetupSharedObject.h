// ======================================================================
//
// SetupSharedObject.h
// copyright 1998 Bootprint Entertainment
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SetupSharedObject_H
#define INCLUDED_SetupSharedObject_H

// ======================================================================

class SetupSharedObject
{
private:

	// **** INCREMENT THIS VERSION NUMBER WHENEVER THIS STRUCT CHANAGES ****

	// This enum is private so that a game can't use it directly.  If the
	// struct changes, the game programmers should be forced to look at the
	// changes and update their data and version number manually.

	enum
	{
		DATA_VERSION = 13
	};

public:

	struct Data
	{
	public:

		Data();
		~Data();

	public:

		int             version;

		// Specifies if Object container support should be installed.
		bool            useContainers;

		// Specifies the valid slot definition file.  Must be specified
		// if container support is installed.
		std::string    *slotDefinitionFilename;

		// If loading a slot definition file, this value controls if the
		// hardpoint names associated with a slot are loaded.  This should
		// be set true for clients and false for servers.
		bool            loadAssociatedHardpointNames;

		// specifies if the movement state tables should be installed
		bool            useMovementTable;
		std::string    *movementStateTableFilename;

		bool            useTimedAppearanceTemplates;

		bool            ensureDefaultAppearanceExists;

		// Specifies the name of the customization id manager data file.
		// Must be specified to use appearance customization system.
		// See addCustomizationSupportData().
		std::string    *customizationIdManagerFilename;

		// specifies whether objects should alter their child and contained objects, rather than expecting them to be scheduled separately
		bool            objectsAlterChildrenAndContents;
		
		// Specifies whether or not ObjectTemplateList should load the object template crc table
		bool loadObjectTemplateCrcStringTable;

		// Specifies the name of the POB ejection point transform override filename to use; use nullptr (default) if no ejection point support.
		char const     *pobEjectionTransformFilename;

	private:

		// disabled
		Data(const Data&);
		Data &operator =(const Data&);

	};

public:

	static void setupDefaultGameData(Data &data);
	static void setupDefaultConsoleData(Data &data);
	static void setupDefaultMFCData(Data &data);

	static void addSlotIdManagerData(Data &data, bool loadAssociatedHardpointNames);
	static void addMovementTableData(Data &data);
	static void addCustomizationSupportData(Data &data);
	static void addPobEjectionTransformData(Data &data);
	
	static void install(const Data &data);
};

// ======================================================================

#endif
