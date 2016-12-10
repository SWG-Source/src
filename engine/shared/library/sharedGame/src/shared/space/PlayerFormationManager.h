//======================================================================
//
// PlayerFormationManager.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_PlayerFormationManager_H
#define INCLUDED_PlayerFormationManager_H

//======================================================================

class Vector;
class Vector2d;

//----------------------------------------------------------------------

class PlayerFormationManager
{
public:

	typedef std::vector<std::string>  FormationNameList;
	struct FormationLocation;

public:
	static void install();
	static void remove();
	static void loadTables();
	static void getFormationNames(FormationNameList & formationNames);
	static bool isValidFormationName(std::string const & formationName);
	static int getMaximumFormationGroupMembers();
	static bool getPositionOffset(uint32 formationCrc, int index, Vector & /*OUT*/ offset);
	static bool getPositionOffset(uint32 formationCrc, int index, Vector2d & /*OUT*/ offset);

private:
	PlayerFormationManager(PlayerFormationManager const & rhs);
	PlayerFormationManager & operator=(PlayerFormationManager const & rhs);

	static void clear();
	static void loadFormationFile(std::string const & formationFileName);
	static bool getPositionOffsetFormationLocation(uint32 const formationCrc, int const index, FormationLocation & formationLocation);
};

//======================================================================

#endif
