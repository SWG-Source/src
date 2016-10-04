#ifndef STATISTICS_H
#define STATISTICS_H

#include <time.h>
#include "Platform.h"

#ifdef EXTERNAL_DISTRO
namespace NAMESPACE 
{

#endif
namespace Base 
{
    template <class TYPE, uint32 COUNT>
    class CStatistic
    {
        public:
            CStatistic(uint32 freq=0) :
				mSampleFrequency(freq),
				mLastSampleTime(0),
				mSample(0),
				mSampleTotal(0),
				mAggregateTotal(0),
				mAggregateMaximum(0),
				mAggregateMinimum(0),
				mAverageTotal(0),
				mAverageIndex(0),
				mAverageCount(0)
			{
			}

            bool Sample(TYPE value)
			{ 
				bool commit = false;

				if (!mLastSampleTime)
					mLastSampleTime = time(nullptr);
				
				if (!mSampleFrequency || mLastSampleTime + mSampleFrequency < (unsigned)time(nullptr))
				{
					mSampleTotal++;
					mAggregateTotal += value;
					if (value > mAggregateMaximum)
						mAggregateMaximum = value;
					if (value < mAggregateMinimum)
						mAggregateMinimum = value;

					if (mAverageIndex == COUNT) 
						mAverageIndex = 0; 
					if (mAverageCount > COUNT)
						mAverageTotal -= mAverageData[mAverageIndex];

					mAverageData[mAverageIndex++] = value; 
					mAverageTotal += value;

					if (mAverageCount < COUNT)
						mAverageCount++;

					commit = true;
					mSample = 0;
				}

				mSample += value;
				
				return commit;
			}
			
			TYPE GetSample(uint32 age = 0)
			{
				if (age > mAverageCount)
					return 0;

				uint32 index = mAverageIndex;
				if (age > index)
					index = COUNT - (age - index);
				else
					index -= age;

				return mAverageData[index];
			}
            
			double GetAverage()
			{
				return (double)mAverageTotal/mAverageCount;
			}
            
			double GetAggregateAverage()
			{
				return (double)mAggregateTotal/mSampleTotal;
			}
            
			TYPE GetMaximum()
			{
				return mAggregateMaximum;
			}
            
			TYPE GetMinimum()
			{
				return mAggregateMinimum;
			}

        private:
            uint32	mSampleFrequency;
			time_t	mLastSampleTime;
			TYPE	mSample;
			uint32	mSampleTotal;
			TYPE	mAggregateTotal;
			TYPE	mAggregateMaximum;
			TYPE	mAggregateMinimum;
			
			TYPE    mAverageData[COUNT];
			TYPE	mAverageTotal;
            uint32  mAverageIndex;
            uint32  mAverageCount;
    };

    class CStatisticTimer
    {
        public:
            CStatisticTimer(bool running = true);

            void Start()    { if (mRunning) return; mStart = getTimer(); mRunning = true; }
            void Stop()     { if (!mRunning) return; mTotal += getTimer()-mStart; mRunning = false; }
            void Reset()    { mTotal = 0; mStart = getTimer(); }

            double  GetTime();
            uint64  GetFraction(uint32 fraction=1000);

        private:
            uint64  mTotal;
            uint64  mStart;
            bool    mRunning;
    };

};
#ifdef EXTERNAL_DISTRO
};
#endif

#endif // STATISTICS_H
