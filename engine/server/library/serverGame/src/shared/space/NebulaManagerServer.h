//======================================================================
//
// NebulaManagerServer.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_NebulaManagerServer_H
#define INCLUDED_NebulaManagerServer_H

//======================================================================

class NebulaLightningData;
class ServerObject;

//----------------------------------------------------------------------

class NebulaManagerServer
{
public:

	static void loadScene(std::string const & sceneId);
	static void clear();

	static void update(float elapsedTime);

	static void enqueueLightning(NebulaLightningData const & nebulaLightningData);
	static void handleEnvironmentalDamage(ServerObject & victim, int nebulaId);

private:

	static void generateLightningEvents(float elapsedTime);
	static void handleEnqueuedLightningEvents();
};

//======================================================================

#endif
