#ifndef CLOCK_H
#define CLOCK_H

#ifdef EXTERNAL_DISTRO
namespace NAMESPACE 
{
#endif


#if defined(WIN32)
    typedef __int64 ClockStamp;
#else
    typedef long long ClockStamp;
#endif

/**
 * @brief A Clock can be used as a millisecond timer.
 */
class Clock
{
public:

    /**
     * @brief Creates a clock, must still be started with Clock::start method.
     *
     *      Once created, a clock can be started, and stoped as often as possible.
     */
    Clock();


    /**
     * @brief Starts the timer running.
     */
    void start();

    /**
     * @brief Stops the timer from running (note: can still be started again later).
     */
    void stop();

    /**
     * @brief Tells you if the timer has been in the started state for longer than runTime.
     *
     * @param runTime  The amount of time to test if this timer has ran longer than.
     *
     * @return 'true' if timer has ran for longer than or equal to runTime, false otherwise.
     */
    bool isDone(unsigned runTime);

    /**
     * @brief Resets this clock (as if it were never started).
     */
    void reset();

private:
    ClockStamp m_lastStart;
    unsigned m_totalRunTime;

    ClockStamp getCurTime();
    ClockStamp getElapsedSinceLastStart();
};


#ifdef EXTERNAL_DISTRO
};
#endif


#endif //CLOCK_H


