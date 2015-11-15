// ServerClock.h
// copyright 2001 Verant Interactive
// Author: Justin Randall

//-----------------------------------------------------------------------

class ServerClock
{
public:
	static ServerClock &getInstance();
	
	~ServerClock();
	const unsigned long getGameTimeSeconds () const;
	const unsigned long getServerFrame() const;
	const unsigned long getSubtractInterval() const;
	void                incrementServerFrame();
	void                setSubtractInterval(const unsigned long newSubtractInterval);
	void                setGameTimeSeconds (unsigned long newGameTime);
	bool                isSet              () const;
	std::string         getDebugPrintableTimeframe (const unsigned long timeInSeconds);

   static const unsigned long cms_endOfTime;
   
protected:
	ServerClock();

private:
	unsigned long serverFrame;
	unsigned long subtractInterval;
	mutable time_t lastTime;
};

//-----------------------------------------------------------------------

inline const unsigned long ServerClock::getServerFrame() const
{
	return serverFrame;
}

//-----------------------------------------------------------------------

inline const unsigned long ServerClock::getSubtractInterval() const
{
	return subtractInterval;
}

// ----------------------------------------------------------------------

inline bool ServerClock::isSet() const
{
	return subtractInterval!=0;
}

//-----------------------------------------------------------------------
