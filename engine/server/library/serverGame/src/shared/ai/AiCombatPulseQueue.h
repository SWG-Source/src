#include "sharedFoundation/NetworkId.h"

class NetworkId;
class TangibleObject;

class AiCombatPulseQueue
{
public:
	static void install();
	static void remove();

	static void schedule(TangibleObject* const object, int waitTimeMs = 0, unsigned long currentFrameTimeMs = 0);
	static void setAiPerFrame(int amount);
	static void setAiMaxWaitTimeMs(unsigned int amount);
	static void alter(real time);
};


