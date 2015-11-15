#ifndef _LOG_EVENT_H
#define _LOG_EVENT_H

//-----------------------------------------------------------------------------

namespace DB
{
	class Session;
}

//-----------------------------------------------------------------------------

namespace DB
{

	/**
	 * This is an abstract base class for events that can be logged.
	 *
	 * To create a LogEvent for a particular purpose, derive from this class.
	 * Override the save function to do whatever is needed to record the
	 * particular event.
	 */
	class LogEvent
	{
	  public:
		/**
		 * Default constructor stored the current system time in eventTime.
		 */
		LogEvent();
		virtual ~LogEvent(){return;};
	  public:
		/**
		 * Saves the LogEvent to the database.
		 *
		 * Override this function in derived classes to save specific data.
		 */
		virtual bool save(DB::Session *session) =0;

	  protected:
		/**
		 * The time the event occurred, in seconds since the epoch.
		 */
		time_t eventTime;
	};

}

//-----------------------------------------------------------------------------

#endif
