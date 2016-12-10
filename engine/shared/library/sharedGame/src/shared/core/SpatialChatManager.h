//======================================================================
//
// SpatialChatManager.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_SpatialChatManager_H
#define INCLUDED_SpatialChatManager_H

//======================================================================

class SpatialChatManager
{
public:
	static void                          install             ();
	static void                          remove              ();

	static uint32                        getChatTypeByName   (const std::string & name);
	static uint32                        getDefaultChatType  ();
	static bool                          getChatNameByType   (uint32 type, std::string & name);
	static const std::vector<std::string> & getSkillsProviding  (uint32 type);

	static bool                          isPrivate           (uint32 type);
	static uint16                        getVolume           (uint32 type);

private:
	
	static void                          load                (const std::string & filename);

	static uint32                        ms_defaultChatType;
};

//----------------------------------------------------------------------

inline uint32 SpatialChatManager::getDefaultChatType ()
{
	return ms_defaultChatType;
}

//======================================================================

#endif
