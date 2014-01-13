// CommandChannelMessages.cpp
// copyright 2000 Verant Interactive
// Author: Justin Randall


//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/CommandChannelMessages.h"

#include "Archive/ByteStream.h"
#include "Archive/AutoByteStream.h"
#include "unicodeArchive/UnicodeArchive.h"
#include "sharedMathArchive/VectorArchive.h"

//----------------------------------------------------------------------

const char * const CmdSceneReady::MessageType = "CmdSceneReady";

//-----------------------------------------------------------------------

CmdSceneReady::CmdSceneReady() :
GameNetworkMessage(MessageType) 
{
}

//-----------------------------------------------------------------------

CmdSceneReady::CmdSceneReady(Archive::ReadIterator & source) :
GameNetworkMessage(MessageType) 
{
	UNREF(source);
}

//-----------------------------------------------------------------------

CmdSceneReady::~CmdSceneReady()
{
}

//----------------------------------------------------------------------

const char * const CmdStartScene::MessageType = "CmdStartScene";

//-----------------------------------------------------------------------

CmdStartScene::CmdStartScene(const NetworkId newObjectId,
                             const std::string & newSceneName,
                             const Vector newStartPosition,
							 const float newStartYaw,
                             const std::string & newTemplateName,
							 const int64 t,
							 const int32 e,
							 bool disableSnapshot) :
GameNetworkMessage(MessageType),
disableWorldSnapshot(disableSnapshot),
objectId(newObjectId),
sceneName(newSceneName),
startPosition(newStartPosition),
startYaw(newStartYaw),
templateName(newTemplateName),
timeSeconds(t),
serverEpoch(e)
{
	addVariable(disableWorldSnapshot);
	addVariable(objectId);
	addVariable(sceneName);
	addVariable(startPosition);
	addVariable(startYaw);
	addVariable(templateName);
	addVariable(timeSeconds);
	addVariable(serverEpoch);
}

//-----------------------------------------------------------------------

CmdStartScene::CmdStartScene(Archive::ReadIterator & source) :
GameNetworkMessage(MessageType),
disableWorldSnapshot(false),
objectId(NetworkId::cms_invalid),
sceneName(),
startPosition(),
startYaw(0.0f),
templateName(),
timeSeconds(),
serverEpoch()
{
	addVariable(disableWorldSnapshot);
	addVariable(objectId);
	addVariable(sceneName);
	addVariable(startPosition);
	addVariable(startYaw);
	addVariable(templateName);
	addVariable(timeSeconds);
	addVariable(serverEpoch);
	AutoByteStream::unpack(source);
}

//-----------------------------------------------------------------------

CmdStartScene::~CmdStartScene()
{
}

//-----------------------------------------------------------------------

