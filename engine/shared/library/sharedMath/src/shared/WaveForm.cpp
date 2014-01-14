// ============================================================================
//
// WaveForm.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "sharedMath/FirstSharedMath.h"
#include "sharedMath/WaveForm.h"

#include "sharedFile/Iff.h"
#include "sharedMath/CatmullRomSpline.h"
#include "sharedRandom/Random.h"

// ============================================================================
//
// WaveFormNamespace
//
// ============================================================================

namespace WaveFormNamespace
{
#ifdef _DEBUG
	int s_singleValueMiss = 0;
	int s_singleValueHit = 0;
	int s_splineCalculationCount = 0;
	int s_linearCalculationCount = 0;
#endif // _DEBUG
};

using namespace WaveFormNamespace;

// ============================================================================
//
// WaveFormControlPointIter
//
// ============================================================================

//-----------------------------------------------------------------------------
WaveFormControlPointIter::WaveFormControlPointIter()
 : m_iter()
 , m_initialPercent(0.0f)
{
}

//-----------------------------------------------------------------------------
void WaveFormControlPointIter::reset(WaveForm::ControlPointList::const_iterator const &iter)
{
	m_iter = iter;
	m_initialPercent = Random::randomReal(0.0f, 1.0f);
}

// ============================================================================
//
// WaveFormControlPoint
//
// ============================================================================

float const WaveFormControlPoint::m_theValueMax = 10000;
float const WaveFormControlPoint::m_theValueMin = -10000;

//-----------------------------------------------------------------------------
WaveFormControlPoint::WaveFormControlPoint()
 : m_percent(0.0f)
 , m_value(0.0f)
 , m_randomMax(0.0f)
 , m_randomMin(0.0f)
{
}

//-----------------------------------------------------------------------------
WaveFormControlPoint::WaveFormControlPoint(float const percent, float const value, float const randomMax, float const randomMin)
 : m_percent(percent)
 , m_value(value)
 , m_randomMax(randomMax)
 , m_randomMin(randomMin)
{
	DEBUG_FATAL(percent < 0.0f, ("WaveFormControlPoint::WaveFormControlPoint() - percent (%.3f) must be >= 0", percent));
	DEBUG_FATAL(percent > 1.0f, ("WaveFormControlPoint::WaveFormControlPoint() - percent (%.3f) must be <= 1", percent));
	DEBUG_FATAL(randomMin < 0.0f, ("WaveFormControlPoint::WaveFormControlPoint() - randomMax (%.3f) must be >= 0", randomMin));
	DEBUG_FATAL(randomMax < 0.0f, ("WaveFormControlPoint::WaveFormControlPoint() - randomMax (%.3f) must be >= 0", randomMax));

	// Clamp the value to the absolute min and max value to prevent weirdness

	m_value = clamp(m_theValueMin, m_value, m_theValueMax);
}

//-----------------------------------------------------------------------------
float WaveFormControlPoint::getPercent() const
{
	return m_percent;
}

//-----------------------------------------------------------------------------
float WaveFormControlPoint::getValue() const
{
	return m_value;
}

//-----------------------------------------------------------------------------
float WaveFormControlPoint::getRandomMin() const
{
	return m_randomMin;
}

//-----------------------------------------------------------------------------
float WaveFormControlPoint::getRandomMax() const
{
	return m_randomMax;
}

//-----------------------------------------------------------------------------
void WaveFormControlPoint::setValue(float const value)
{
	m_value = value;
}

//-----------------------------------------------------------------------------
void WaveFormControlPoint::setRandomMin(float const randomMin)
{
	m_randomMin = randomMin;
}

//-----------------------------------------------------------------------------
void WaveFormControlPoint::setRandomMax(float const randomMax)
{
	m_randomMax = randomMax;
}

// ============================================================================
//
// WaveForm
//
// ============================================================================

#define TAG_WVFM TAG(W,V,F,M)

//-----------------------------------------------------------------------------
WaveForm::WaveForm()
 : m_controlPointList()
 , m_interpolationType(IT_linear)
 , m_sampleType(ST_continuous)
 , m_valueMax(10000.0f)
 , m_valueMin(-10000.0f)
 , m_singleValue(false)
#ifdef _DEBUG
 , m_name("")
 , m_controlPointCount(0)
#endif // _DEBUG
{
}

//-----------------------------------------------------------------------------
WaveForm::WaveForm(WaveForm const &rhs)
 : m_controlPointList(rhs.m_controlPointList)
 , m_interpolationType(rhs.m_interpolationType)
 , m_sampleType(rhs.m_sampleType)
 , m_valueMax(rhs.m_valueMax)
 , m_valueMin(rhs.m_valueMin)
 , m_singleValue(rhs.m_singleValue)
#ifdef _DEBUG
 , m_name(rhs.m_name)
 , m_controlPointCount(static_cast<int>(m_controlPointList.size()))
#endif // _DEBUG
{
}

//-----------------------------------------------------------------------------
WaveForm::~WaveForm()
{
}

//-----------------------------------------------------------------------------
WaveForm &WaveForm::operator =(WaveForm const &rhs)
{
	if (this == &rhs)
	{
		return *this;
	}

	m_controlPointList = rhs.m_controlPointList;

#ifdef _DEBUG
	m_controlPointCount = static_cast<int>(m_controlPointList.size());
	m_name = rhs.m_name;
#endif // _DEBUG

	m_interpolationType = rhs.m_interpolationType;
	m_sampleType = rhs.m_sampleType;
	m_valueMax = rhs.m_valueMax;
	m_valueMin = rhs.m_valueMin;
	m_singleValue = rhs.m_singleValue;

	return *this;
}

#ifdef _DEBUG
//-----------------------------------------------------------------------------
int WaveForm::getSingleValueHit()
{
	return s_singleValueHit;
}

//-----------------------------------------------------------------------------
int WaveForm::getSingleValueMiss()
{
	return s_singleValueMiss;
}

//-----------------------------------------------------------------------------
int WaveForm::getSplineCalculationCount()
{
	return s_splineCalculationCount;
}

//-----------------------------------------------------------------------------
int WaveForm::getLinearCalculationCount()
{
	return s_linearCalculationCount;
}

#endif // _DEBUG

//-----------------------------------------------------------------------------
void WaveForm::setValueMax(float const max)
{
	// Check for ridiculous values

	DEBUG_FATAL(max >= 64000.0f, ("WaveForm::setValueMax() - Ridiculous max value set (%.3f)", max));

	m_valueMax = max;
}

//-----------------------------------------------------------------------------
void WaveForm::setValueMin(float const min)
{
	// Check for ridiculous values

	DEBUG_FATAL(min <= -64000.0f, ("WaveForm::setValueMax() - Ridiculous minvalue set (%.3f)", min));

	m_valueMin = min;
}

//-----------------------------------------------------------------------------
float const WaveForm::getValueMax() const
{
	return m_valueMax;
}

//-----------------------------------------------------------------------------
float const WaveForm::getValueMin() const
{
	return m_valueMin;
}

//-----------------------------------------------------------------------------
void WaveForm::calculateMinMax(float &max, float &min)
{
	// Recalculate the min and max values

	max = m_controlPointList.begin()->getValue();
	min = m_controlPointList.begin()->getValue();

	for (ControlPointList::const_iterator current = m_controlPointList.begin(); current != m_controlPointList.end(); ++current)
	{
		float const valuePlusRandom = current->getValue() + current->getRandomMax();
		float const valueMinusRandom = current->getValue() - current->getRandomMin();

		if (valuePlusRandom > m_valueMax)
		{
			max = m_valueMax;
		}
		else if (max < valuePlusRandom)
		{
			max = valuePlusRandom;
		}

		if (valueMinusRandom < m_valueMin)
		{
			min = m_valueMin;
		}
		else if (min > valueMinusRandom)
		{
			min = valueMinusRandom;
		}
	}

	DEBUG_FATAL(m_valueMin > m_valueMax, ("WaveForm::calculateMinMax() - m_valueMin (%.3f) > m_valueMax (%.3f)", m_valueMin, m_valueMax));

	// Make sure to prevent wackiness by clamping the minimum space represented
	// between the min and max values

	float const epsilon = 0.1f;
	float foo = 0.0f;

	if ((max - min) <= epsilon)
	{
		foo = (epsilon - (max - min)) / 2.0f;
	}

	// Center the values in the waveform

	min -= foo;
	max += foo;

	DEBUG_FATAL(m_valueMin > (m_valueMax - epsilon), ("WaveForm::calculateMinMax() - m_valueMin (%.3f) <= (m_valueMax (%.3f) - epsilon (%.3f))", m_valueMin, m_valueMax, epsilon));

	// Make sure the min and max are still within bounds

	if (min < m_valueMin)
	{
		min += foo;
		max += foo;
	}
	else if (max > m_valueMax)
	{
		min -= foo;
		max -= foo;
	}

	// Make sure the values are legit

	DEBUG_FATAL(min > max, ("WaveForm::calculateMinMax() - min (%.3f) > max (%.3f)", min, max));
	DEBUG_FATAL(min < m_valueMin, ("WaveForm::calculateMinMax() - min (%.3f) < m_valueMin (%.3f)", m_valueMin, max));
	DEBUG_FATAL(max > m_valueMax, ("WaveForm::calculateMinMax() - max (%.3f) > m_valueMax (%.3f)", max, m_valueMax));
}

//-----------------------------------------------------------------------------
void WaveForm::copyControlPoints(WaveForm const &waveForm)
{
	m_interpolationType = waveForm.m_interpolationType;
	m_sampleType = waveForm.m_sampleType;

	// Remove all the current control points

	clear();

	// Insert the control points from the specified waveform

	ControlPointList::const_iterator current = waveForm.m_controlPointList.begin();

	for (; current != waveForm.m_controlPointList.end(); ++current)
	{
		insert(*current);
	}
}

//-----------------------------------------------------------------------------
void WaveForm::insert(WaveFormControlPoint const &controlPoint)
{
	WaveFormControlPoint newControlPoint(controlPoint);
	
	// Make sure the value is in a valid range

	clampValue(newControlPoint);

	// Insert the control point in the correct place in the list based on the
	// percent value

	bool inserted = false;
	ControlPointList::iterator current = m_controlPointList.begin();

	for (; current != m_controlPointList.end(); ++current)
	{
		if (newControlPoint.getPercent() < current->getPercent())
		{
			IGNORE_RETURN(m_controlPointList.insert(current, newControlPoint));
			inserted = true;
			break;
		}
	}

	// This handles inserting in to the list if there is nothing in the list
	// or if the item needs to be at the end of the list

	if (!inserted)
	{
		IGNORE_RETURN(m_controlPointList.insert(m_controlPointList.end(), newControlPoint));
	}

#ifdef _DEBUG
	m_controlPointCount = static_cast<int>(m_controlPointList.size());
#endif // _DEBUG
}

//-----------------------------------------------------------------------------
void WaveForm::remove(ControlPointList::iterator &iter)
{
	DEBUG_FATAL(!verifyIteratorValid(iter), ("WaveForm::remove() - Invalid iterator"));

	iter = m_controlPointList.erase(iter);

#ifdef _DEBUG
	m_controlPointCount = static_cast<int>(m_controlPointList.size());
#endif // _DEBUG
}

//-----------------------------------------------------------------------------
void WaveForm::setControlPoint(ControlPointList::const_iterator const &iter, WaveFormControlPoint const &controlPoint)
{
	DEBUG_FATAL(!verifyIteratorValid(iter), ("WaveForm::setControlPoint() - Invalid iterator"));

	for (ControlPointList::iterator current = m_controlPointList.begin(); current != m_controlPointList.end(); ++current)
	{
		if (current == iter)
		{
			// Set control point

			*current = controlPoint;

			// Clamp the value to the max and min value

			clampValue(*current);

			// Clamp the percent to the neighboring control points

			clampPercent(current);

			break;
		}
	}
}

//-----------------------------------------------------------------------------
void WaveForm::clampValue(WaveFormControlPoint &controlPoint) const
{
	float const percent = controlPoint.getPercent();
	float value = controlPoint.getValue();
	float randomMax = controlPoint.getRandomMax();
	float randomMin = controlPoint.getRandomMin();

	// Check the value

	if (value > m_valueMax)
	{
		value = m_valueMax;
	}
	else if (value < m_valueMin)
	{
		value = m_valueMin;
	}

	// Check the random max

	if (value + randomMax > m_valueMax)
	{
		randomMax = m_valueMax - value;
	}

	// Check the random min

	if (value - randomMin < m_valueMin)
	{
		randomMin = -(m_valueMin - value);
	}

	controlPoint = WaveFormControlPoint(percent, value, randomMax, randomMin);
}

//-----------------------------------------------------------------------------
void WaveForm::clampPercent(ControlPointList::iterator const &iter)
{
	DEBUG_FATAL(!verifyIteratorValid(iter), ("WaveForm::clampPercent() - Invalid iterator"));

	// Make sure the control point values are in the correct range

	float percent = iter->getPercent();

	// Find the previous and next iterators so we can clamp this control point
	// inside them

	ControlPointList::iterator current = m_controlPointList.begin();
	ControlPointList::iterator prev = current;
	ControlPointList::iterator next = current;
	int index = 1;
	
	for (; current != m_controlPointList.end(); ++current)
	{
		if (current == iter)
		{
			next = current;
			++next;
			break;
		}

		prev = current;
		++index;
	}

	// Clamp the percent to previous percent

	if (iter == m_controlPointList.begin())
	{
		percent = 0.0f;
	}
	else if (percent < prev->m_percent)
	{
		percent = prev->m_percent;
	}

	// Clamp the percent to next percent

	if (index == static_cast<int>(m_controlPointList.size()))
	{
		percent = 1.0f;
	}
	else if ((next != m_controlPointList.end()) && (percent > next->m_percent))
	{
		percent = next->m_percent;
	}

	// Set the new clamped values back into the iterator

	*iter = WaveFormControlPoint(percent, iter->getValue(), iter->getRandomMax(), iter->getRandomMin());
}

//-----------------------------------------------------------------------------
void WaveForm::clear()
{
	m_singleValue = false;
	m_controlPointList.clear();

#ifdef _DEBUG
	m_controlPointCount = 0;
#endif // _DEBUG
}

//-----------------------------------------------------------------------------
WaveForm::InterpolationType WaveForm::getInterpolationType() const
{
	return m_interpolationType;
}

//-----------------------------------------------------------------------------
WaveForm::SampleType WaveForm::getSampleType() const
{
	return m_sampleType;
}

//-----------------------------------------------------------------------------
void WaveForm::setInterpolationType(InterpolationType const interpolationMode)
{
	m_interpolationType = interpolationMode;
}

//-----------------------------------------------------------------------------
void WaveForm::setSampleType(SampleType const sampleMode)
{
	m_sampleType = sampleMode;
}

#ifdef _DEBUG
//-----------------------------------------------------------------------------
std::string const &WaveForm::getName() const
{
	return m_name;
}

//-----------------------------------------------------------------------------
void WaveForm::setName(std::string const &name)
{
	m_name = name;
}
#endif // _DEBUG

//-----------------------------------------------------------------------------
void WaveForm::randomize(float const maxValue, float const minValue)
{
	// Remove all the control points

	clear();

	// Random interpolation mode

	setInterpolationType((rand() % 2) ? IT_linear : IT_spline);
	
	// Random control positions

	int const count = rand() % 7 + 2;
	float const delta = 1.0f / static_cast<float>(count + 1);

	// Start

	insert(WaveFormControlPoint(0.0f, 0.0f, 0.0f, 0.0f));

	// Inner positions

	for (int i = 0; i < count; ++i)
	{
		float const fi = static_cast<float>(i);

		float const difference = maxValue - minValue;
		float const percent = static_cast<float>((static_cast<int>((delta + delta * fi) * 100.0f)) / 100.0f);
		float const value = minValue + (Random::randomReal(0.0f, 1.0f) * difference);
		float const randomMax = 0.0f;
		float const randomMin = 0.0f;

		insert(WaveFormControlPoint(percent, value, randomMax, randomMin));
	}

	// End

	insert(WaveFormControlPoint(1.0f, 0.0f, 0.0f, 0.0f));
}

//-----------------------------------------------------------------------------
float WaveForm::getValue(WaveFormControlPointIter &iter, float const percent) const
{
	if (m_singleValue &&
	    (m_controlPointList.size() > 0))
	{
#ifdef _DEBUG
		++s_singleValueHit;
#endif // _DEBUG

		return (m_controlPointList.begin()->getValue());
	}

#ifdef _DEBUG
	++s_singleValueMiss;
#endif // _DEBUG

#ifdef _DEBUG
	float const iterPercent = iter.m_iter->m_percent;
	UNREF(iterPercent);
#endif // _DEBUG

	DEBUG_FATAL(m_controlPointList.size() < 2, ("WaveForm::getValue() - There must be at least 2 control points defined."));
	DEBUG_FATAL(!verifyIteratorValid(iter.m_iter), ("WaveForm::getValue() - Invalid iterator"));
	DEBUG_FATAL(percent < 0.0f, ("WaveForm::getValue() - percent is < 0"));
	DEBUG_FATAL(percent > 1.0f, ("WaveForm::getValue() - percent is > 1"));
	DEBUG_FATAL(iter.m_iter == m_controlPointList.end(), ("WaveForm::getValue() - iterator == m_controlPointList.end()"));
	DEBUG_FATAL(percent < iter.m_iter->m_percent, ("WaveForm::getValue() - percent %f < iter->m_percent %f", percent, iter.m_iter->m_percent));

	ControlPointList::const_iterator next = iter.m_iter;
	++next;

	if (next != m_controlPointList.end())
	{
		float nextPercent = next->m_percent;

		while (percent > nextPercent)
		{
			++iter.m_iter;
			++next;

			nextPercent = next->m_percent;
		}
	}
	else
	{
		next = iter.m_iter;
	}

	float const previousPercent = iter.m_iter->m_percent;
	float const nextPercent = next->m_percent;

	DEBUG_FATAL((previousPercent > 1.0f), ("previousPercent(%f) > 1", previousPercent));
	DEBUG_FATAL((previousPercent < 0.0f), ("previousPercent(%f) < 0", previousPercent));
	DEBUG_FATAL((nextPercent > 1.0f), ("nextPercent(%f) > 1", nextPercent));
	DEBUG_FATAL((nextPercent < 0.0f), ("nextPercent(%f) < 0", nextPercent));

	float const previousValue = iter.m_iter->m_value;
	float const nextValue = next->m_value;

	float value = previousValue;
	
	if ((nextPercent - previousPercent) > 0.0f)
	{
		switch (m_interpolationType)
		{
			case IT_linear:
				{
#ifdef _DEBUG
					++s_linearCalculationCount;
#endif // _DEBUG

					float currentPercent = clamp(0.0f, (percent - previousPercent) / (nextPercent - previousPercent), 1.0f);

					DEBUG_FATAL((currentPercent > 1.0f), ("currentPercent(%f) > 1", currentPercent));
					DEBUG_FATAL((currentPercent < 0.0f), ("currentPercent(%f) < 0", currentPercent));

					float const valueDifference = (nextValue - previousValue) * currentPercent;

					float const previousRandomMaxValue = iter.m_iter->m_randomMax;
					float const previousRandomMinValue = iter.m_iter->m_randomMin;

					float const nextRandomMaxValue = next->m_randomMax;
					float const nextRandomMinValue = next->m_randomMin;

					float randomMaxValueDifference = previousRandomMaxValue + (nextRandomMaxValue - previousRandomMaxValue) * currentPercent;
					float randomMinValueDifference = previousRandomMinValue + (nextRandomMinValue - previousRandomMinValue) * currentPercent;

					if (m_sampleType == ST_initial)
					{
						float const randomDifference = randomMaxValueDifference + randomMinValueDifference;

						value += valueDifference - randomMinValueDifference + iter.m_initialPercent * randomDifference;
					}
					else if (m_sampleType == ST_continuous)
					{
						value += valueDifference + Random::randomReal(-randomMinValueDifference, randomMaxValueDifference);
					}
				}
				break;
			case IT_spline:
				{
#ifdef _DEBUG
					++s_splineCalculationCount;
#endif // _DEBUG

					// We have to find the four neighboring control points

					ControlPointList::const_iterator c1 = m_controlPointList.begin();
					ControlPointList::const_iterator c2 = m_controlPointList.begin();
					ControlPointList::const_iterator c3 = m_controlPointList.begin();
					ControlPointList::const_iterator c4 = m_controlPointList.begin();

					if (m_controlPointList.size() == 2)
					{
						// Handling 2 controls points is easy

						++c3;
						++c4;
					}
					else
					{
						// Find the correct positions of the control points

						int index = 0;

						for (; c3 != m_controlPointList.end();)
						{
							++c3;
							c4 = c3;
							++c4;

							if (c4 == m_controlPointList.end())
							{
								c4 = c3;
							}

							float const previousPercent = c2->getPercent();
							float const currentPercent = c3->getPercent();

							DEBUG_FATAL((previousPercent > currentPercent), ("WaveForm::getColorAtPercent() - previousPercent(%.3f) > currentPercent(%.3f)", previousPercent, currentPercent));

							if ((percent >= previousPercent) &&
								(percent <= currentPercent))
							{
								break;
							}
							else
							{
								++c2;

								if (index > 0)
								{
									++c1;
								}
							}

							++index;
						}
					}

					// Find the position between the second and third control points

					float const difference = c3->getPercent() - c2->getPercent();
					float const t = (difference > 0.0f) ? ((percent - c2->getPercent()) / difference) : 0.0f;

					DEBUG_FATAL((t < 0.0f), ("WaveForm::getColorAtPercent() - The spline position t(%f) is < 0", t));
					DEBUG_FATAL((t > 1.0f), ("WaveForm::getColorAtPercent() - The spline position t(%f) is > 1", t));

					// Now that we have found the correct position of the control points,
					// we can find the individual color components

					float const c1x = c1->getPercent();
					float const c2x = c2->getPercent();
					float const c3x = c3->getPercent();
					float const c4x = c4->getPercent();

					// Max value
						
					float const c1yMax = c1->getValue() + c1->getRandomMax();
					float const c2yMax = c2->getValue() + c2->getRandomMax();
					float const c3yMax = c3->getValue() + c3->getRandomMax();
					float const c4yMax = c4->getValue() + c4->getRandomMax();

					float resultX;
					float maxResultY;

					CatmullRomSpline::getCatmullRomSplinePoint(c1x, c1yMax, c2x, c2yMax, c3x, c3yMax, c4x, c4yMax, t, resultX, maxResultY);

					// Min value
						
					float const c1yMin = c1->getValue() - c1->getRandomMin();
					float const c2yMin = c2->getValue() - c2->getRandomMin();
					float const c3yMin = c3->getValue() - c3->getRandomMin();
					float const c4yMin = c4->getValue() - c4->getRandomMin();

					float minResultY;

					CatmullRomSpline::getCatmullRomSplinePoint(c1x, c1yMin, c2x, c2yMin, c3x, c3yMin, c4x, c4yMin, t, resultX, minResultY);

					// Make sure the max is the largest value

					if (minResultY > maxResultY)
					{
						std::swap(minResultY, maxResultY);
					}

					// Get the final value

					if (m_sampleType == ST_initial)
					{
						value = minResultY + iter.m_initialPercent * (maxResultY - minResultY);
					}
					else if (m_sampleType == ST_continuous)
					{
						value = Random::randomReal(minResultY, maxResultY);
					}
				}
				break;
			default:
				{
					DEBUG_FATAL(true, ("Unexpected interpolation mode."));
				}
				break;
		}
	}

	return clamp(m_valueMin, value, m_valueMax);
}

//-----------------------------------------------------------------------------
void WaveForm::write(Iff &iff) const
{
	iff.insertForm(TAG_WVFM);
	{
		iff.insertChunk(TAG_0002);
		{
			iff.insertChunkData(static_cast<int32>(m_interpolationType));
			iff.insertChunkData(static_cast<int32>(m_sampleType));
			iff.insertChunkData(m_valueMin);
			iff.insertChunkData(m_valueMax);

			// Control points
			
			int32 const controlPointCount = static_cast<int32>(m_controlPointList.size());

			DEBUG_FATAL(controlPointCount < 2, ("There should be 2 or more control points to write."));

			iff.insertChunkData(controlPointCount);

			ControlPointList::const_iterator current = m_controlPointList.begin();

			for (; current != m_controlPointList.end(); ++current)
			{
				float const percent = current->getPercent();
				float const value = current->getValue();
				float const randomMin = current->getRandomMin();
				float const randomMax = current->getRandomMax();

				iff.insertChunkData(percent);
				iff.insertChunkData(value);
				iff.insertChunkData(randomMin);
				iff.insertChunkData(randomMax);
			}
		}
		iff.exitChunk(TAG_0002);
	}
	iff.exitForm(TAG_WVFM);
}

//-----------------------------------------------------------------------------
void WaveForm::load(Iff &iff)
{	
	iff.enterForm(TAG_WVFM);
	{
		switch (iff.getCurrentName())
		{
			case TAG_0000: { load_0000(iff); } break;
			case TAG_0001: { load_0001(iff); } break;
			case TAG_0002: { load_0002(iff); } break;
			default:
				{
					char currentTagName[5];
					ConvertTagToString(iff.getCurrentName(), currentTagName);
		
					FATAL(true, ("Unsupported data version: %s", currentTagName));
				}
		}
	}
	iff.exitForm(TAG_WVFM);

	DEBUG_FATAL((m_controlPointList.size() < 2), ("Waveforms require at least two control points to be defined. Control points defined: (%d).", m_controlPointList.size()));

#ifdef _DEBUG
		ControlPointList::const_iterator iterControlPointList = m_controlPointList.begin();

		for (; iterControlPointList != m_controlPointList.end(); ++iterControlPointList)
		{
			float const percent = iterControlPointList->getPercent();

			DEBUG_WARNING((percent > 1.0f), ("percent(%f) > 1", percent));
			DEBUG_WARNING((percent < 0.0f), ("percent(%f) < 0", percent));
		}
#endif // _DEBUG

	// Check to see if all the waveform values are just the same, if so, we can make
	// some optimazations when querying for waveform values
	
	m_singleValue = true;

	if (m_controlPointList.size() >= 2)
	{
		float firstValue = m_controlPointList.begin()->getValue();

		ControlPointList::const_iterator iterControlPointList = m_controlPointList.begin();
		++iterControlPointList;

		for (; iterControlPointList != m_controlPointList.end(); ++iterControlPointList)
		{
			float const currentValue = iterControlPointList->getValue();
			float const randomMin = iterControlPointList->getRandomMin();
			float const randomMax = iterControlPointList->getRandomMax();

			if ((currentValue != firstValue) ||
			    ((currentValue == firstValue) &&
			     ((randomMin != 0.0f) ||
			      (randomMax != 0.0f))))
			{
				m_singleValue = false;
				break;
			}
		}
	}
}

//-----------------------------------------------------------------------------
void WaveForm::load_0000(Iff &iff)
{
	m_controlPointList.clear();
	
	iff.enterChunk(TAG_0000);
	{
		// Interpolation mode
	
		m_interpolationType = static_cast<InterpolationType>(iff.read_int32());
	
		// Control points
		
		int const controlPointCount = iff.read_int32();
	
		for (int i = 0; i < controlPointCount; ++i)
		{
			float const percent = iff.read_float();
			float const value = iff.read_float();
			float const randomMax = iff.read_float();
			float const randomMin = iff.read_float();
	
			WaveFormControlPoint controlPoint(percent, value, randomMax, randomMin);
	
			insert(controlPoint);
		}
	}
	iff.exitChunk(TAG_0000);
}

//-----------------------------------------------------------------------------
void WaveForm::load_0001(Iff &iff)
{
	m_controlPointList.clear();
	
	iff.enterChunk(TAG_0001);
	{
		// Interpolation mode
	
		m_interpolationType = static_cast<InterpolationType>(iff.read_int32());

		// Sample mode
	
		m_sampleType = static_cast<SampleType>(iff.read_int32());
	
		// Control points
		
		int const controlPointCount = iff.read_int32();
	
		for (int i = 0; i < controlPointCount; ++i)
		{
			float const percent = iff.read_float();
			float const value = iff.read_float();
			float const randomMax = iff.read_float();
			float const randomMin = iff.read_float();
	
			WaveFormControlPoint controlPoint(percent, value, randomMax, randomMin);
	
			insert(controlPoint);
		}
	}
	iff.exitChunk(TAG_0001);
}

//-----------------------------------------------------------------------------
void WaveForm::load_0002(Iff & iff)
{
	m_controlPointList.clear();
	
	iff.enterChunk(TAG_0002);
	{
		m_interpolationType = static_cast<InterpolationType>(iff.read_int32());
		m_sampleType = static_cast<SampleType>(iff.read_int32());
		m_valueMin = iff.read_float();
		m_valueMax = iff.read_float();
	
		// Control points
		
		int const controlPointCount = iff.read_int32();
	
		for (int i = 0; i < controlPointCount; ++i)
		{
			float const percent = iff.read_float();
			float const value = iff.read_float();
			float const randomMin = iff.read_float();
			float const randomMax = iff.read_float();
	
			WaveFormControlPoint controlPoint(percent, value, randomMax, randomMin);
	
			insert(controlPoint);
		}
	}
	iff.exitChunk(TAG_0002);
}

//-----------------------------------------------------------------------------
WaveForm::ControlPointList::iterator WaveForm::getIteratorBegin()
{
	return m_controlPointList.begin();
}

//-----------------------------------------------------------------------------
WaveForm::ControlPointList::iterator WaveForm::getIteratorEnd()
{
	return m_controlPointList.end();
}

//-----------------------------------------------------------------------------
WaveForm::ControlPointList::const_iterator WaveForm::getIteratorBegin() const
{
	return m_controlPointList.begin();
}

//-----------------------------------------------------------------------------
WaveForm::ControlPointList::const_iterator WaveForm::getIteratorEnd() const
{
	return m_controlPointList.end();
}

//-----------------------------------------------------------------------------
int WaveForm::getControlPointCount() const
{
	return static_cast<int>(m_controlPointList.size());
}

//-----------------------------------------------------------------------------
void WaveForm::clampAll(float const min, float const max)
{
	setValueMin(min);
	setValueMax(max);

	ControlPointList::iterator current = m_controlPointList.begin();

	for (; current != m_controlPointList.end(); ++current)
	{
		clampValue(*current);
	}
}

//-----------------------------------------------------------------------------
void WaveForm::scaleAll(float const percent)
{
	ControlPointList::iterator current = m_controlPointList.begin();

	for (; current != m_controlPointList.end(); ++current)
	{
		float const newPercent = current->getPercent();
		float const newValue = current->getValue() * percent;
		float const newRandomMin = current->getRandomMin() * percent;
		float const newRandomMax = current->getRandomMax() * percent;

		*current = WaveFormControlPoint(newPercent, newValue, newRandomMax, newRandomMin);
	}
}

#ifdef _DEBUG
//-----------------------------------------------------------------------------
bool WaveForm::verifyIteratorValid(ControlPointList::const_iterator const &iter) const
{
	bool result = false;
	ControlPointList::const_iterator current = m_controlPointList.begin();

	for (; current != m_controlPointList.end(); ++current)
	{
		if (current == iter)
		{
			result = true;
			break;
		}
	}

	return result;
}
#endif // _DEBUG

//-----------------------------------------------------------------------------

bool WaveForm::isConstantValue(float const val)
{
	if(!m_singleValue)
		return false;
	if(m_controlPointList.empty())
		return true;
	WaveFormControlPoint &firstPoint = *(m_controlPointList.begin());
	return (WithinEpsilonInclusive(firstPoint.m_value, val, 0.00001f) &&
			WithinEpsilonInclusive(firstPoint.m_randomMin, 0.0f, 0.00001f) &&
			WithinEpsilonInclusive(firstPoint.m_randomMax, 0.0f, 0.00001f));
}

// ============================================================================
