// ======================================================================
//
// DebugInfoManager.h
// Copyright 2004 Sony Online Entertainment Inc
// All Rights Reserved
//
// ======================================================================

#ifndef INCLUDED_DebugInfoManager_H
#define INCLUDED_DebugInfoManager_H

// ======================================================================

class Transform;
class Vector;

// ======================================================================

class DebugInfoManager
{
public:
	typedef std::map<std::string, Unicode::String> PropertySection;
	typedef std::map<std::string, PropertySection> PropertyMap;

public:
	static void addProperty(PropertyMap & propertyMap, std::string const & section, std::string const & propertyName, int propertyValue);
	static void addProperty(PropertyMap & propertyMap, std::string const & section, std::string const & propertyName, unsigned int propertyValue);
	static void addProperty(PropertyMap & propertyMap, std::string const & section, std::string const & propertyName, unsigned long propertyValue);
	static void addProperty(PropertyMap & propertyMap, std::string const & section, std::string const & propertyName, float propertyValue);
	static void addProperty(PropertyMap & propertyMap, std::string const & section, std::string const & propertyName, bool propertyValue);
	static void addProperty(PropertyMap & propertyMap, std::string const & section, std::string const & propertyName, std::string const & propertyValue);
	static void addProperty(PropertyMap & propertyMap, std::string const & section, std::string const & propertyName, char const * propertyValue);
	static void addProperty(PropertyMap & propertyMap, std::string const & section, std::string const & propertyName, Transform const & propertyValue);
	static void addProperty(PropertyMap & propertyMap, std::string const & section, std::string const & propertyName, Vector const & propertyValue);
	static void addProperty(PropertyMap & propertyMap, std::string const & section, std::string const & propertyName, Unicode::String const & propertyValue);

private:
	DebugInfoManager();
	~DebugInfoManager();
	DebugInfoManager(DebugInfoManager const &);
	DebugInfoManager & operator=(DebugInfoManager const &);
};

// ======================================================================

#endif
