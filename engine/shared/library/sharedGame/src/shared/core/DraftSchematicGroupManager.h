//======================================================================
//
// DraftSchematicGroupManager.h
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_DraftSchematicGroupManager_H
#define INCLUDED_DraftSchematicGroupManager_H

//======================================================================

class DraftSchematicGroupManager
{
public:

	typedef stdvector<std::string>::fwd                StringVector;
	typedef stdvector<std::pair<uint32, uint32> >::fwd SchematicVector;

	static void                         getGroups             (StringVector & sv);
	static bool                         getSchematicsForGroup (const std::string & group, SchematicVector & sv);
};

//======================================================================

#endif
