// ======================================================================
//
// SceneGlobalData.h
// copyright (c) 2005 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SceneGlobalData_H
#define INCLUDED_SceneGlobalData_H

// ======================================================================

// ======================================================================

// ======================================================================

class SceneGlobalData
{
  public:
	static void install();
	static float getUpdateRadius(std::string const & sceneName);

  private:
	static void remove();
};

// ======================================================================

#endif
