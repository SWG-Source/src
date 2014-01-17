#ifndef __RATE_LIMIT_H__
#define __RATE_LIMIT_H__

namespace soe
{
	typedef long limit_t;
	typedef long interval_t;

	struct RateLimit
	{
		RateLimit()
			: mLimit(0)
			, mInterval(0)
		{ }

		limit_t mLimit;
		interval_t mInterval;
	};
	typedef std::vector<RateLimit> RateLimits_t;
};

#endif // __RATE_LIMIT_H__
