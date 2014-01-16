#include "Statistics.h"

#ifdef EXTERNAL_DISTRO
namespace NAMESPACE 
{

#endif
namespace Base 
{

    CStatisticTimer::CStatisticTimer(bool running) :
        mTotal(0),
        mStart(getTimer()),
        mRunning(running)
    {
    }

    double CStatisticTimer::GetTime()
    {
        uint64 total = mTotal;

        if (mRunning)
            total += getTimer()-mStart;

        return (double)((int64)(total/getTimerFrequency()));    
    }

    uint64 CStatisticTimer::GetFraction(uint32 fraction)
    {
        uint64 total = mTotal;

        if (mRunning)
            total += getTimer()-mStart;

        if (fraction == 0)
            return total;
        else
            return total/(getTimerFrequency()/fraction);    
    }

};
#ifdef EXTERNAL_DISTRO
};
#endif

