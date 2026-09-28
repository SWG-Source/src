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

	// startTimeMs/endTimeMs are absolute Clock::timeMs() values.  The
	// syncStamp fields of nebulaLightningData are filled in per connection
	// server when the lightning is sent to clients.
	static void enqueueLightning(NebulaLightningData const & nebulaLightningData, uint64_t startTimeMs, uint64_t endTimeMs);
	static void handleEnvironmentalDamage(ServerObject & victim, int nebulaId);

private:

	static void generateLightningEvents(float elapsedTime);
	static void handleEnqueuedLightningEvents();
};

//======================================================================

#endif
