#ifndef __GENERIC_RATE_LIMITING_MECHANISM_H__
#define __GENERIC_RATE_LIMITING_MECHANISM_H__

#include "expirationQueue.h"
#include "refptr.h"
#include "rateLimit.h"

#define KEY_PAIR_COMP_OPER(__oper__)							\
	bool operator __oper__ (const KeyPair & rhs) const			\
	{															\
		if (mFixedKey != rhs.mFixedKey) {						\
			return (mFixedKey __oper__ rhs.mFixedKey);			\
		} else {												\
			return (mArbitraryKey __oper__ rhs.mArbitraryKey);	\
		}														\
	}

namespace soe
{
	// create a class with the same interface in order to use different limit and counter types for customized behavior
	class RateCounter
	{
	public:
		RateCounter() : mCount(0), mLimit(0), mIndex(0), mInterval(0) { }
		RateCounter(limit_t count, limit_t limit, interval_t interval, size_t index)
            : mCount(count), mLimit(limit), mIndex(index), mInterval(interval) { }
		virtual ~RateCounter() { }

		unsigned Write(unsigned char * stream, unsigned size) const
        { 
			unsigned bytesTotal = 0;
			unsigned bytes = 0;

			bytesTotal += (bytes = soe::Write(stream+bytesTotal, size-bytesTotal, mCount));
			if (!bytes) return 0;

			bytesTotal += (bytes = soe::Write(stream+bytesTotal, size-bytesTotal, mLimit));
			if (!bytes) return 0;

			bytesTotal += (bytes = soe::Write(stream+bytesTotal, size-bytesTotal, mInterval));
			if (!bytes) return 0;

			bytesTotal += (bytes = soe::Write(stream+bytesTotal, size-bytesTotal, mIndex));
			if (!bytes) return 0;

			return bytesTotal;
        }

        unsigned Read(const unsigned char * stream, unsigned size)
        {
			unsigned bytesTotal = 0;
			unsigned bytes = 0;

			bytesTotal += (bytes = soe::Read(stream+bytesTotal, size-bytesTotal, mCount));
			if (!bytes) return 0;

			bytesTotal += (bytes = soe::Read(stream+bytesTotal, size-bytesTotal, mLimit));
			if (!bytes) return 0;

			bytesTotal += (bytes = soe::Read(stream+bytesTotal, size-bytesTotal, mInterval));
			if (!bytes) return 0;

			bytesTotal += (bytes = soe::Read(stream+bytesTotal, size-bytesTotal, mIndex));
			if (!bytes) return 0;

			return bytesTotal;
        }

        void Initialize(const RateLimits_t & rateLimits)
		{
			if (mIndex < rateLimits.size()) {
				mLimit = rateLimits[mIndex].mLimit;
				mInterval = rateLimits[mIndex].mInterval;
			} else {
				mLimit = 0;
				mInterval = 0;
			}
			// reset count
			mCount = 0;
		}

		limit_t GetCount() const { return mCount; }
		limit_t GetLimit() const { return mLimit; }
		interval_t GetInterval() const { return mInterval; }
		size_t GetIndex() const { return mIndex; }

		limit_t Consume(const RateLimits_t & rateLimits, limit_t count)
		{
			mCount += count;

			return (mLimit - mCount);
		}

		bool Expire(const RateLimits_t & rateLimits)
		{
			bool remove = false;

			if (mCount > mLimit) {
				// raise to next rate limit if count was exceeded
				if (mIndex < rateLimits.size()) {
					mIndex++;
				} else {
					// can't go up; make sure we go down if limit vector has shrunk
					mIndex = rateLimits.size() - 1;
				}
			} else {
				if (mIndex > 0) {
					// drop to previous rate limit if count was not exceeded
					mIndex--;
				} else {
					// remove counter if unused and on initial rate limit
					if (mCount == 0) {
						remove = true;
					}
				}
			}
			// reinitialize
			Initialize(rateLimits);

			return remove;
		}


	protected:
		limit_t mCount;
		limit_t mLimit;
		interval_t mInterval;
		size_t mIndex;
	private:
	};
	typedef std::vector<RateCounter> RateCounters_t;

	inline unsigned Read(const unsigned char * stream, unsigned size, RateCounter & data, unsigned maxLen, unsigned version = 0)
	{
		return data.Read(stream, size);
	}

	inline unsigned Write(unsigned char * stream, unsigned size, const RateCounter & data, unsigned version = 0)
	{
		return data.Write(stream, size);
	}

	inline int Print(char * stream, unsigned size, const RateCounter & data, unsigned maxDepth = INT_MAX)
	{
		return 0;
	}

	template<typename FixedKeyType, typename ArbitraryKeyType, typename LimitType = RateLimits_t, typename CounterType = RateCounter, typename TimeType = time_t>
	class GenericRateLimitingMechanism
	{
	public:
        typedef std::vector<FixedKeyType> FixedKeyTypes_t;
        typedef std::vector<ArbitraryKeyType> ArbitraryKeyTypes_t;
        typedef std::vector<LimitType> LimitTypes_t;
        typedef std::vector<CounterType> CounterTypes_t;

		GenericRateLimitingMechanism() { }
		virtual ~GenericRateLimitingMechanism() { }

		void SetRateLimit(const FixedKeyType & fixedKey, const LimitType & rateLimit);
		void UnsetRateLimit(const FixedKeyType & fixedKey);
		const LimitType * GetRateLimit(const FixedKeyType & fixedKey) const;
		const LimitType * GetNextRateLimit(FixedKeyType & fixedKey) const;

		const CounterType * GetRateCounter(const FixedKeyType & fixedKey, const ArbitraryKeyType & arbitraryKey) const;
		unsigned GetRateCountersAndFixedKeys(const ArbitraryKeyType & arbitraryKey, CounterTypes_t & counters, FixedKeyTypes_t & fixedKeys) const;
		void PutRateCounter(const FixedKeyType & fixedKey, const ArbitraryKeyType & arbitraryKey, const CounterType & counter);
		void PutRateCountersAndFixedKeys(const ArbitraryKeyType & arbitraryKey, const CounterTypes_t & counters, const FixedKeyTypes_t & fixedKeys);
		bool RemoveRateCounter(const FixedKeyType & fixedKey, const ArbitraryKeyType & arbitraryKey);

		unsigned RemoveArbitraryRateCounters(const FixedKeyType & fixedKey);
		unsigned RemoveFixedRateCounters(const ArbitraryKeyType & arbitraryKey);
		unsigned RemoveAllRateCounters();

		limit_t ConsumeRateAllowance(const FixedKeyType & fixedKey, const ArbitraryKeyType & arbitraryKey, limit_t amount, TimeType when);

		void ProcessRateCounters(TimeType when);

	private:
		class KeyPair
		{
		public:
			KeyPair()
			{ }

			KeyPair(const FixedKeyType & fixedKey, const ArbitraryKeyType & arbitraryKey)
				: mFixedKey(fixedKey)
				, mArbitraryKey(arbitraryKey)
			{ }

			virtual ~KeyPair()
			{ }

			KEY_PAIR_COMP_OPER(<)
			KEY_PAIR_COMP_OPER(<=)
			KEY_PAIR_COMP_OPER(==)
			KEY_PAIR_COMP_OPER(>=)
			KEY_PAIR_COMP_OPER(>)

			FixedKeyType mFixedKey;
			ArbitraryKeyType mArbitraryKey;
		};
		class RateInfo
		{
		public:
			RateInfo()
			{ }

			virtual ~RateInfo()
			{ }

			KeyPair mKeyPair;
			CounterType mCounter;
		};

		typedef ref_ptr<RateInfo> RateInfoRefPtr;

		// map types
		typedef std::map<FixedKeyType, LimitType> RateLimitByFixedKeyMap_t;
		typedef std::map<KeyPair, RateInfoRefPtr> RateCounterByKeyPairMap_t;

		// expiration queue type
		typedef expiration_queue<KeyPair, TimeType> RateCounterExpirationQueue_t;

		// maps
		RateLimitByFixedKeyMap_t mRateLimits;
		RateCounterByKeyPairMap_t mRateCounters;

		// queue
		RateCounterExpirationQueue_t mExpirationQueue;
	};

	template<typename FixedKeyType, typename ArbitraryKeyType, typename LimitType, typename CounterType, typename TimeType>
	void GenericRateLimitingMechanism<FixedKeyType, ArbitraryKeyType, LimitType, CounterType, TimeType>::SetRateLimit(const FixedKeyType & fixedKey, const LimitType & rateLimit)
	{
		mRateLimits[fixedKey] = rateLimit;
	}

	template<typename FixedKeyType, typename ArbitraryKeyType, typename LimitType, typename CounterType, typename TimeType>
	void GenericRateLimitingMechanism<FixedKeyType, ArbitraryKeyType, LimitType, CounterType, TimeType>::UnsetRateLimit(const FixedKeyType & fixedKey)
	{
		mRateLimits.erase(fixedKey);
	}

	template<typename FixedKeyType, typename ArbitraryKeyType, typename LimitType, typename CounterType, typename TimeType>
	const LimitType * GenericRateLimitingMechanism<FixedKeyType, ArbitraryKeyType, LimitType, CounterType, TimeType>::GetRateLimit(const FixedKeyType & fixedKey) const
	{
		LimitType const * rateLimit = nullptr;
		typename RateLimitByFixedKeyMap_t::const_iterator limIter = mRateLimits.find(fixedKey);

		if (limIter != mRateLimits.end()) {
			rateLimit = &(limIter->second);
		}
		
		return rateLimit; 
	}

	template<typename FixedKeyType, typename ArbitraryKeyType, typename LimitType, typename CounterType, typename TimeType>
	const LimitType * GenericRateLimitingMechanism<FixedKeyType, ArbitraryKeyType, LimitType, CounterType, TimeType>::GetNextRateLimit(FixedKeyType & fixedKey) const
	{
		LimitType const * rateLimit = nullptr;
		typename RateLimitByFixedKeyMap_t::const_iterator limIter = mRateLimits.upper_bound(fixedKey);

		if (limIter != mRateLimits.end()) {
			fixedKey = limIter->first;
			rateLimit = &(limIter->second);
		}

		return rateLimit; 
	}

	template<typename FixedKeyType, typename ArbitraryKeyType, typename LimitType, typename CounterType, typename TimeType>
	const CounterType * GenericRateLimitingMechanism<FixedKeyType, ArbitraryKeyType, LimitType, CounterType, TimeType>::GetRateCounter(const FixedKeyType & fixedKey, const ArbitraryKeyType & arbitraryKey) const
	{
		const CounterType * counter = nullptr;
		typename RateCounterByKeyPairMap_t::const_iterator counterIter = mRateCounters.find(KeyPair(fixedKey, arbitraryKey));
		if (counterIter != mRateCounters.end()) {
			counter = &(counterIter->second->mCounter);
		}

		return counter;
	}

	template<typename FixedKeyType, typename ArbitraryKeyType, typename LimitType, typename CounterType, typename TimeType>
	unsigned GenericRateLimitingMechanism<FixedKeyType, ArbitraryKeyType, LimitType, CounterType, TimeType>::GetRateCountersAndFixedKeys(const ArbitraryKeyType & arbitraryKey, CounterTypes_t & counters, FixedKeyTypes_t & fixedKeys) const
	{
		typename RateCounterByKeyPairMap_t::const_iterator counterIter;
		typename RateLimitByFixedKeyMap_t::const_iterator limitIter;
		KeyPair keyPair;

		keyPair.mArbitraryKey = arbitraryKey;
		for (limitIter = mRateLimits.begin(); limitIter != mRateLimits.end(); limitIter++)
		{
			keyPair.mFixedKey = limitIter->first;
			counterIter = mRateCounters.find(keyPair);
			if (counterIter != mRateCounters.end()) {
				fixedKeys.push_back(counterIter->first.mFixedKey);
				counters.push_back(counterIter->second->mCounter);
			}
		}

		return counters.size();
	}

	template<typename FixedKeyType, typename ArbitraryKeyType, typename LimitType, typename CounterType, typename TimeType>
	void GenericRateLimitingMechanism<FixedKeyType, ArbitraryKeyType, LimitType, CounterType, TimeType>::PutRateCounter(const FixedKeyType & fixedKey, const ArbitraryKeyType & arbitraryKey, const CounterType & counter)
	{
		KeyPair keyPair(fixedKey, arbitraryKey);
		RateInfoRefPtr info = mRateCounters[keyPair];

		info->mKeyPair = keyPair;
		info->mCounter = counter;
	}

	template<typename FixedKeyType, typename ArbitraryKeyType, typename LimitType, typename CounterType, typename TimeType>
	void GenericRateLimitingMechanism<FixedKeyType, ArbitraryKeyType, LimitType, CounterType, TimeType>::PutRateCountersAndFixedKeys(const ArbitraryKeyType & arbitraryKey, const CounterTypes_t & counters, const FixedKeyTypes_t & fixedKeys)
	{
		unsigned count = counters.size() < fixedKeys.size() ? counters.size() : fixedKeys.size();
		KeyPair keyPair;

		keyPair.mArbitraryKey = arbitraryKey;
		for (unsigned index =0; index < count; index++){
			keyPair.mFixedKey = fixedKeys[index];
			RateInfoRefPtr rateInfoRefPtr = mRateCounters[keyPair];

			rateInfoRefPtr->mKeyPair = keyPair;
			rateInfoRefPtr->mCounter = counters[index];
		}
	}

	template<typename FixedKeyType, typename ArbitraryKeyType, typename LimitType, typename CounterType, typename TimeType>
	bool GenericRateLimitingMechanism<FixedKeyType, ArbitraryKeyType, LimitType, CounterType, TimeType>::RemoveRateCounter(const FixedKeyType & fixedKey, const ArbitraryKeyType & arbitraryKey)
	{
		bool removed = false;
		typename RateCounterByKeyPairMap_t::iterator counterIter = mRateCounters.find(KeyPair(fixedKey, arbitraryKey));
		if (counterIter != mRateCounters.end()) {
			mExpirationQueue.erase(counterIter->first);
			mRateCounters.erase(counterIter);
			removed = true;
		}

		return removed;
	}

	template<typename FixedKeyType, typename ArbitraryKeyType, typename LimitType, typename CounterType, typename TimeType>
	unsigned GenericRateLimitingMechanism<FixedKeyType, ArbitraryKeyType, LimitType, CounterType, TimeType>::RemoveArbitraryRateCounters(const FixedKeyType & fixedKey)
	{
		unsigned numRemoved = 0;
		typename RateCounterByKeyPairMap_t::iterator counterIter = mRateCounters.begin();
		std::list<KeyPair> keysToRemove;
		for (; counterIter != mRateCounters.end(); counterIter++)
		{
			if (counterIter->first.mFixedKey == fixedKey) {
				keysToRemove.push_back(counterIter->first);
			}
		}
		for (typename std::list<KeyPair>::iterator remIter = keysToRemove.begin(); remIter != keysToRemove.end(); remIter++)
		{
			mExpirationQueue.erase(*remIter);
			mRateCounters.erase(*remIter);
			numRemoved++;
		}

		return numRemoved;
	}

	template<typename FixedKeyType, typename ArbitraryKeyType, typename LimitType, typename CounterType, typename TimeType>
	unsigned GenericRateLimitingMechanism<FixedKeyType, ArbitraryKeyType, LimitType, CounterType, TimeType>::RemoveFixedRateCounters(const ArbitraryKeyType & arbitraryKey)
	{
		unsigned numRemoved = 0;
		typename RateCounterByKeyPairMap_t::iterator counterIter = mRateCounters.begin();
		std::list<KeyPair> keysToRemove;
		for (; counterIter != mRateCounters.end(); counterIter++)
		{
			if (counterIter->first.mArbitraryKey == arbitraryKey) {
				keysToRemove.push_back(counterIter->first);
			}
		}
		for (typename std::list<KeyPair>::iterator remIter = keysToRemove.begin(); remIter != keysToRemove.end(); remIter++)
		{
			mExpirationQueue.erase(*remIter);
			mRateCounters.erase(*remIter);
			numRemoved++;
		}

		return numRemoved;
	}

	template<typename FixedKeyType, typename ArbitraryKeyType, typename LimitType, typename CounterType, typename TimeType>
	unsigned GenericRateLimitingMechanism<FixedKeyType, ArbitraryKeyType, LimitType, CounterType, TimeType>::RemoveAllRateCounters()
	{
		unsigned numRemoved = mRateCounters.size();

		mExpirationQueue.clear();
		mRateCounters.clear();

		return numRemoved;
	}

	template<typename FixedKeyType, typename ArbitraryKeyType, typename LimitType, typename CounterType, typename TimeType>
	limit_t GenericRateLimitingMechanism<FixedKeyType, ArbitraryKeyType, LimitType, CounterType, TimeType>::ConsumeRateAllowance(const FixedKeyType & fixedKey, const ArbitraryKeyType & arbitraryKey, limit_t amount, TimeType when)
	{
		limit_t remainingAllowance = INT_MAX;
		const LimitType * rateLimit = GetRateLimit(fixedKey);

		if (rateLimit) {
			typename RateCounterByKeyPairMap_t::iterator counterIter = mRateCounters.find(KeyPair(fixedKey, arbitraryKey));
			RateInfoRefPtr info;
			KeyPair keyPair(fixedKey, arbitraryKey);

			if (counterIter == mRateCounters.end()) {
				info = mRateCounters[keyPair];
				info->mKeyPair = keyPair;
				info->mCounter.Initialize(*rateLimit);
				mExpirationQueue.insert(keyPair, when + info->mCounter.GetInterval());
			} else {
				info = counterIter->second;
			}
			remainingAllowance = info->mCounter.Consume(*rateLimit, amount);
		}

		return remainingAllowance;
	}

	template<typename FixedKeyType, typename ArbitraryKeyType, typename LimitType, typename CounterType, typename TimeType>
	void GenericRateLimitingMechanism<FixedKeyType, ArbitraryKeyType, LimitType, CounterType, TimeType>::ProcessRateCounters(TimeType when)
	{
		while (mExpirationQueue.contains_expired_items(when))
		{
			KeyPair top = mExpirationQueue.top();
			typename RateLimitByFixedKeyMap_t::const_iterator limIter = mRateLimits.find(top.mFixedKey);
			RateInfoRefPtr & info = mRateCounters[top];

			if ((limIter == mRateLimits.end()) || (info->mCounter.Expire(limIter->second))) {
				// clear cache of unused counters
				mRateCounters.erase(top);
				mExpirationQueue.erase(top);
			} else {
				// reset expiration time
				mExpirationQueue.insert(top, when + info->mCounter.GetInterval());
			}
		}
	}

	enum ESingleMemberClass
	{
		eSingleClassMember = 0
	};

	template<typename FixedKeyType, typename LimitType = RateLimits_t, typename CounterType = RateCounter, typename TimeType = time_t>
	class GenericRateLimitingMechanismFixedOnly : public GenericRateLimitingMechanism<FixedKeyType, ESingleMemberClass, LimitType, CounterType, TimeType>
	{
	public:
		GenericRateLimitingMechanismFixedOnly() { }
		virtual ~GenericRateLimitingMechanismFixedOnly() { }

		const CounterType * GetRateCounter(const FixedKeyType & fixedKey) const
		{
			return GenericRateLimitingMechanism<FixedKeyType, ESingleMemberClass>::GetRateCounter(fixedKey, eSingleClassMember);
		}

		unsigned GetRateCountersAndFixedKeys(typename GenericRateLimitingMechanism<FixedKeyType, ESingleMemberClass, LimitType, CounterType, TimeType>::CounterTypes_t & counters, typename GenericRateLimitingMechanism<FixedKeyType, ESingleMemberClass, LimitType, CounterType, TimeType>::FixedKeyTypes_t & fixedKeys) const
		{
			return GenericRateLimitingMechanism<FixedKeyType, ESingleMemberClass>::GetRateCountersAndFixedKeys(eSingleClassMember, counters, fixedKeys);
		}

		void PutRateCounter(const FixedKeyType & fixedKey, const CounterType & counter)
		{
			GenericRateLimitingMechanism<FixedKeyType, ESingleMemberClass>::PutRateCounter(fixedKey, eSingleClassMember, counter);
		}

		void PutRateCountersAndFixedKeys(const typename GenericRateLimitingMechanism<FixedKeyType, ESingleMemberClass, LimitType, CounterType, TimeType>::CounterTypes_t & counters, const typename GenericRateLimitingMechanism<FixedKeyType, ESingleMemberClass, LimitType, CounterType, TimeType>::FixedKeyTypes_t & fixedKeys)
		{
			return GenericRateLimitingMechanism<FixedKeyType, ESingleMemberClass>::PutRateCountersAndFixedKeys(eSingleClassMember, counters, fixedKeys);
		}

		bool RemoveRateCounter(const FixedKeyType & fixedKey)
		{
			return GenericRateLimitingMechanism<FixedKeyType, ESingleMemberClass>::RemoveRateCounter(fixedKey, eSingleClassMember);
		}

		limit_t ConsumeRateAllowance(const FixedKeyType & fixedKey, limit_t amount, TimeType when)
		{
			return GenericRateLimitingMechanism<FixedKeyType, ESingleMemberClass>::ConsumeRateAllowance(fixedKey, eSingleClassMember, amount, when);
		}
	};

	template<typename ArbitraryKeyType, typename LimitType = RateLimits_t, typename CounterType = RateCounter, typename TimeType = time_t>
	class GenericRateLimitingMechanismArbitraryOnly : public GenericRateLimitingMechanism<ESingleMemberClass, ArbitraryKeyType, LimitType, CounterType, TimeType>
	{
	public:
		GenericRateLimitingMechanismArbitraryOnly() { }
		virtual ~GenericRateLimitingMechanismArbitraryOnly() { }

		void SetRateLimit(const LimitType & rateLimit)
		{
			GenericRateLimitingMechanism<ESingleMemberClass, ArbitraryKeyType>::SetRateLimit(eSingleClassMember, rateLimit);
		}

		void UnsetRateLimit()
		{
			GenericRateLimitingMechanism<ESingleMemberClass, ArbitraryKeyType>::UnsetRateLimit(eSingleClassMember);
		}

		const LimitType * GetRateLimit() const
		{
			return GenericRateLimitingMechanism<ESingleMemberClass, ArbitraryKeyType>::GetRateLimit(eSingleClassMember);
		}

		const CounterType * GetRateCounter(const ArbitraryKeyType & arbitraryKey) const
		{
			return GenericRateLimitingMechanism<ESingleMemberClass, ArbitraryKeyType>::GetRateCounter(eSingleClassMember, arbitraryKey);
		}

		void PutRateCounter(const ArbitraryKeyType & arbitraryKey, const CounterType & counter)
		{
			GenericRateLimitingMechanism<ESingleMemberClass, ArbitraryKeyType>::PutRateCounter(eSingleClassMember, arbitraryKey, counter);
		}

		bool RemoveRateCounter(const ArbitraryKeyType & arbitraryKey)
		{
			return GenericRateLimitingMechanism<ESingleMemberClass, ArbitraryKeyType>::RemoveRateCounter(eSingleClassMember, arbitraryKey);
		}

		limit_t ConsumeRateAllowance(const ArbitraryKeyType & arbitaryKey, limit_t amount, TimeType when)
		{
			return GenericRateLimitingMechanism<ESingleMemberClass, ArbitraryKeyType>::ConsumeRateAllowance(eSingleClassMember, arbitaryKey, amount, when);
		}
	};
};

#endif // __GENERIC_RATE_LIMITING_MECHANISM_H__

