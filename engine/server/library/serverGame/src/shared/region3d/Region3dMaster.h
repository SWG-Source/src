// ======================================================================
//
// Region3dMaster.h
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_Region3dMaster_H
#define INCLUDED_Region3dMaster_H

// ======================================================================

class Region3d;
class Vector;

// ======================================================================

class Region3dMaster
{
public:
	static void install();
	static void reloadRegionTable();
	static Region3d const *getRegionByName(std::string const &regionName);
	static void getRegionsAtPoint(Vector const &point, std::vector<Region3d const *> &regions);
	static Region3d const *getSmallestRegionAtPoint(Vector const &point);

private:
	static void addRegion(Region3d &region);
	static void removeRegion(Region3d &region);
	
private:
	Region3dMaster();
	Region3dMaster(Region3dMaster const &);
	Region3dMaster &operator=(Region3dMaster const &);

friend class Region3d;
};

// ======================================================================

#endif // INCLUDED_Region3dMaster_H

