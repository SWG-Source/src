// ServerClock.h
// copyright 2001 Verant Interactive
// Author: Justin Randall

//-----------------------------------------------------------------------

class ServerClock
{
public:
	static ServerClock &getInstance();
	
	~ServerClock();
	const uint32_t getGameTimeSeconds () const;
	const uint32_t getServerFrame() const;
	const uint32_t getSubtractInterval() const;
	void                incrementServerFrame();
	void                setSubtractInterval(const uint32_t newSubtractInterval);
	void                setGameTimeSeconds (uint32_t newGameTime);
	bool                isSet              () const;
	std::string         getDebugPrintableTimeframe (const uint32_t timeInSeconds);

   static const int32_t cms_endOfTime;
   
protected:
	ServerClock();

private:
	uint32_t serverFrame;
	uint32_t subtractInterval;
	mutable time_t lastTime;
};

//-----------------------------------------------------------------------

inline const uint32_t ServerClock::getServerFrame() const
{
	return serverFrame;
}

//-----------------------------------------------------------------------

inline const uint32_t ServerClock::getSubtractInterval() const
{
	return subtractInterval;
}

// ----------------------------------------------------------------------

inline bool ServerClock::isSet() const
{
	return subtractInterval!=0;
}

//-----------------------------------------------------------------------
