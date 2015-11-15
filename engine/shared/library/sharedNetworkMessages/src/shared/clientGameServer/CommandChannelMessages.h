// CommandChannelMessages.h
// copyright 2000 Verant Interactive
// Author: Justin Randall

#ifndef	_COMMAND_CHANNEL_MESSAGES_H
#define	_COMMAND_CHANNEL_MESSAGES_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "Unicode.h"
#include "sharedMath/Vector.h"
#include <string>
#include <vector>


//-----------------------------------------------------------------------

class CmdSceneReady : public GameNetworkMessage
{
public:

	static const char * const MessageType;

	CmdSceneReady	();
	explicit CmdSceneReady	(Archive::ReadIterator & source);
	virtual				~CmdSceneReady	();
private:
	CmdSceneReady(const CmdSceneReady & source);
	CmdSceneReady & operator = (const CmdSceneReady & rhs);
};

//-----------------------------------------------------------------------

class CmdStartScene : public GameNetworkMessage
{
public:

	static const char * const MessageType;

    CmdStartScene	(const NetworkId objectId, 
		const std::string & newSceneName, 
		const Vector newStartPosition,
		const float newStartYaw,
		const std::string & templateName,
		const int64 serverTime,
		const int32 serverEpoch,
		bool disableWorldSnapshot);
	
    explicit CmdStartScene	(Archive::ReadIterator & source);
    virtual	~CmdStartScene	();
	
	bool                        getDisableWorldSnapshot() const;
    const NetworkId             getObjectId       () const;
    const std::string &         getSceneName      () const;
    const Vector&               getStartPosition  () const;
    const float                 getStartYaw       () const;
    const std::string &         getTemplateName   () const;
	const int64                 getTimeSeconds    () const;
	const int32                 getServerEpoch    () const;
	
private:
	Archive::AutoVariable<bool>         disableWorldSnapshot;
    Archive::AutoVariable<NetworkId>    objectId;
    Archive::AutoVariable<std::string>  sceneName;
    Archive::AutoVariable<Vector>       startPosition;
	Archive::AutoVariable<float>        startYaw;
    Archive::AutoVariable<std::string>  templateName;
	Archive::AutoVariable<int64>        timeSeconds;
	Archive::AutoVariable<int32>        serverEpoch;
	
    CmdStartScene();
    CmdStartScene(const CmdStartScene&);
    CmdStartScene& operator= (const CmdStartScene&);
};

//-----------------------------------------------------------------------

inline bool CmdStartScene::getDisableWorldSnapshot() const
{
	return disableWorldSnapshot.get();
}

//-----------------------------------------------------------------------

inline const NetworkId CmdStartScene::getObjectId() const
{
	return objectId.get(); //lint !e1037 // const and non const conversion operators are irrelevent here, the return value is const
}

//-----------------------------------------------------------------------

inline const std::string & CmdStartScene::getSceneName() const
{
	return sceneName.get(); //lint !e1037 
}

//-----------------------------------------------------------------------

inline const Vector& CmdStartScene::getStartPosition() const
{
	return startPosition.get();
}

//-----------------------------------------------------------------------

inline const float CmdStartScene::getStartYaw() const
{
	return startYaw.get();
}

//-----------------------------------------------------------------------

inline const std::string & CmdStartScene::getTemplateName() const
{
	return templateName.get(); //lint !e1037 // const and non const conversion operators are irrelevent here, the return value is const
}

//-----------------------------------------------------------------------

inline const int64 CmdStartScene::getTimeSeconds() const
{
	return timeSeconds.get();
}

//-----------------------------------------------------------------------

inline const int32 CmdStartScene::getServerEpoch() const
{
	return serverEpoch.get();
}

//-----------------------------------------------------------------------

#endif	// _COMMAND_CHANNEL_MESSAGES_H
