// ============================================================================
// 
// AppearanceManager.cpp
// Copyright Sony Online Entertainment, Inc.
//
// ============================================================================

#ifndef INCLUDED_AppearanceManager_H
#define INCLUDED_AppearanceManager_H

//-----------------------------------------------------------------------------
class AppearanceManager
{
public:

	static void install();

	static bool isAppearanceManaged(std::string const &fileName);
	static bool getAppearanceName(std::string &targetName, std::string const &sourceName, int const column);

private:

	static void remove();
};

// ============================================================================

#endif // INCLUDED_AppearanceManager_H
