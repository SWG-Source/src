// ======================================================================
//
// AlterResult.h
// Copyright 2002 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_AlterResult_H
#define INCLUDED_AlterResult_H

// ======================================================================

class AlterResult
{
public:

	static void incorporateAlterResult(float &aggregateAlterResult, float componentAlterResult);
	static void incorporateLongAlterResult(float &aggregateAlterResult, float componentAlterResult);
	static void incorporateExactAlterResult(float &aggregateAlterResult, float componentAlterResult);
	static void formatTime(char *buffer, int bufferLength, float time);

public:

	static float const cms_alterQuickly;
	static float const cms_alterNextFrame;
	static float const cms_keepNoAlter;
	static float const cms_kill;

};

// ======================================================================
/**
 * Incorporate the alter result of a component into an aggregate alter result.
 *
 * This function takes care of some special cases, like ensuring that an aggregate
 * return value of cms_kill does not get overwritten by another component that
 * requests an alter some time in the future or requests to not be killed.
 *
 * If any component desires to be killed, the aggregate alter result will return
 * cms_kill.
 *
 * @param aggregateAlterResult  On input, contains the current aggregate alter result.
 *                              This should be initialized by the caller to cms_keepNoAlter before
 *                              incorporating the results of any component alter results.
 *                              On output, contains the proper aggregate alter return value
 *                              for the result.
 * @param componentAlterResult  The alter result of a component to incorporate.
 */
inline void AlterResult::incorporateAlterResult(float &aggregateAlterResult, float componentAlterResult)
{
	// If any component requests death, the aggregate will die.
	if (componentAlterResult == AlterResult::cms_kill) //lint !e777 // testing float for equality // This is okay, we're using consts for set and test.
	{
		aggregateAlterResult = AlterResult::cms_kill;
	}
	else if (aggregateAlterResult != AlterResult::cms_kill)
	{
		//-- The next alter time for the object is the min next alter time specified.
		//   We must check to make sure that the aggregateAlterResult of "cms_kill" is not overwritten.
		if (componentAlterResult < aggregateAlterResult)
		{
			aggregateAlterResult=componentAlterResult;
		}
	}
}

// ----------------------------------------------------------------------

/**
 * Incorporate the alter result of a component into an aggregate alter result. This
 * function differs from AlterResult::incorporateAlterResult() by insuring the largest 
 * alter time is used instead of the smallest.
 *
 * This function takes care of some special cases, like ensuring that an aggregate
 * return value of cms_kill does not get overwritten by another component that
 * requests an alter some time in the future or requests to not be killed.
 *
 * If any component desires to be killed, the aggregate alter result will return
 * cms_kill.
 *
 * @param aggregateAlterResult  On input, contains the current aggregate alter result.
 *                              This should be initialized by the caller to cms_alterNextFrame before
 *                              incorporating the results of any component alter results.
 *                              On output, contains the proper aggregate alter return value
 *                              for the result.
 * @param componentAlterResult  The alter result of a component to incorporate.
 */
inline void AlterResult::incorporateLongAlterResult(float &aggregateAlterResult, float componentAlterResult)
{
	// If any component requests death, the aggregate will die.
	if (componentAlterResult == AlterResult::cms_kill) //lint !e777 // testing float for equality // This is okay, we're using consts for set and test.
	{
		aggregateAlterResult = AlterResult::cms_kill;
	}
	else if (aggregateAlterResult != AlterResult::cms_kill)
	{
		//-- The next alter time for the object is the max next alter time specified.
		//   We must check to make sure that the aggregateAlterResult of "cms_kill" is not overwritten.
		if (componentAlterResult > aggregateAlterResult)
		{
			aggregateAlterResult=componentAlterResult;
		}
	}
}

// ----------------------------------------------------------------------

/**
 * Incorporate the alter result of a component into an aggregate alter result. This
 * function differs from AlterResult::incorporateAlterResult() by insuring that the
 * componentAlterResult will always replace the aggregateAlterResult unless the object
 * is to be killed.
 *
 * This function takes care of some special cases, like ensuring that an aggregate
 * return value of cms_kill does not get overwritten by another component that
 * requests an alter some time in the future or requests to not be killed.
 *
 * If any component desires to be killed, the aggregate alter result will return
 * cms_kill.
 *
 * @param aggregateAlterResult  On input, contains the current aggregate alter result.
 *                              On output, contains the proper aggregate alter return value
 *                              for the result.
 * @param componentAlterResult  The alter result of a component to incorporate.
 */
inline void AlterResult::incorporateExactAlterResult(float &aggregateAlterResult, float componentAlterResult)
{
	// If any component requests death, the aggregate will die.
	if (componentAlterResult == AlterResult::cms_kill) //lint !e777 // testing float for equality // This is okay, we're using consts for set and test.
	{
		aggregateAlterResult = AlterResult::cms_kill;
	}
	else if (aggregateAlterResult != AlterResult::cms_kill)
	{
		//-- The next alter time for the object is the next alter time specified.
		//   We must check to make sure that the aggregateAlterResult of "cms_kill" is not overwritten.
		aggregateAlterResult=componentAlterResult;
	}
}


// ======================================================================

#endif
