// ======================================================================
//
// TaskGetStructures.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_TaskGetCharacters_H
#define INCLUDED_TaskGetCharacters_H

// ======================================================================

#include <vector>

#include "sharedDatabaseInterface/DbTaskRequest.h"
#include "sharedFoundation/NetworkId.h"
#include "Unicode.h"

//-----------------------------------------------------------------------

class TaskGetStructures:public DB::TaskRequest
{
  public:
	struct StructureRec
	{
		std::string     location;
		std::string     objectTemplate;
		NetworkId       structureId;
		real            x;
		real            y;
		real            z;
		uint32		deleted;
	};
	
	typedef std::vector<StructureRec*> StructuresType;
	
 	TaskGetStructures(NetworkId & characterId, uint32 toolId, uint32 loginServerId);
	virtual ~TaskGetStructures();
	
	virtual bool        process             (DB::Session *session);
	virtual void        onComplete          ();

  protected:
	StructuresType structures;

  private:	
	NetworkId m_characterId;
  	uint32 m_toolId;
	uint32 m_loginServerId;
	
	TaskGetStructures();
	TaskGetStructures(const TaskGetStructures&);
	TaskGetStructures& operator=(const TaskGetStructures& rhs);
};

//-----------------------------------------------------------------------

#endif
