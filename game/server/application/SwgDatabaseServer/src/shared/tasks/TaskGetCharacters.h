// ======================================================================
//
// TaskGetCharacters.h
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

class TaskGetCharacters:public DB::TaskRequest
{
  public:
	struct CharacterRec
	{
		Unicode::String characterName;
		std::string     location;
		std::string     objectTemplate;
		NetworkId       characterId;
		NetworkId       containerId;
		real            x;
		real            y;
		real            z;
	};
	
	typedef std::vector<CharacterRec*> CharactersType;
	
 	TaskGetCharacters(uint32 stationId);
	virtual ~TaskGetCharacters();
	
	virtual bool        process             (DB::Session *session);
	virtual void        onComplete          ();

  protected:
	CharactersType characters;

  private:	
	uint32 m_stationId;
	
	TaskGetCharacters();
	TaskGetCharacters(const TaskGetCharacters&);
	TaskGetCharacters& operator=(const TaskGetCharacters& rhs);
};

//-----------------------------------------------------------------------

#endif
