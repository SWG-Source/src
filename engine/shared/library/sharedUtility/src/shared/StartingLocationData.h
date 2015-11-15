//======================================================================
//
// StartingLocationData.h
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_StartingLocationData_H
#define INCLUDED_StartingLocationData_H

#include <string>

//======================================================================

class StartingLocationData
{
public:

	std::string name;
	std::string planet;
	float       x;
	float       y;
	float       z;
	std::string cellId;
	std::string image;
	std::string descriptionId;

	StartingLocationData ();
};

//======================================================================

#endif
