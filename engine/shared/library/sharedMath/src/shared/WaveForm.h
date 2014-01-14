// ============================================================================
//
// WaveForm.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_WaveForm_H
#define INCLUDED_WaveForm_H

#include <list>
#include <string>

class WaveForm;
class WaveFormControlPointIter; // Defined below
class WaveFormEdit;
class Iff;

//-----------------------------------------------------------------------------
class WaveFormControlPoint
{
friend class WaveForm;

public:

	WaveFormControlPoint();
	WaveFormControlPoint(float const percent, float const value, float const randomMax = 0.0f, float const randomMin = 0.0f);

	void  setValue(float const value);
	void  setRandomMin(float const randomMin);
	void  setRandomMax(float const randomMax);

	float getPercent() const;
	float getValue() const;
	float getRandomMin() const;
	float getRandomMax() const;

private:

	float              m_percent;     // [0..1]
	float              m_value;       // [-n..n]
	float              m_randomMax;   // [0..n]
	float              m_randomMin;   // [0..n]
	static float const m_theValueMax;
	static float const m_theValueMin;
};

//-----------------------------------------------------------------------------
class WaveForm
{
public:

	typedef std::list<WaveFormControlPoint> ControlPointList;

	enum InterpolationType
	{
		IT_linear,
		IT_spline,
	};

	enum SampleType
	{
		ST_initial,
		ST_continuous,
	};

#ifdef _DEBUG
	static int getSingleValueHit();
	static int getSingleValueMiss();
	static int getSplineCalculationCount();
	static int getLinearCalculationCount();
#endif // _DEBUG

public:

	WaveForm();
	WaveForm(WaveForm const &rhs);
	~WaveForm();

	WaveForm & operator =(WaveForm const &rhs);

#ifdef _DEBUG
	// Set/get the name

	std::string const &getName() const;
	void               setName(std::string const &name);
#endif // _DEBUG

	// Get the interpolated value

	float       getValue(WaveFormControlPointIter &iter, float const percent) const;

	// Set/Get the max and min values

	void        setValueMax(float const max);
	void        setValueMin(float const min);
	float const getValueMax() const;
	float const getValueMin() const;

	// Copies all the data except the min and max, so the inserted data is clamped
	// to the current min and max

	void        copyControlPoints(WaveForm const &waveForm);

	// Calculates the min and max control point value

	void        calculateMinMax(float &max, float &min);

	// Adds the control point into the correct place in the list based on the percent

	void        insert(WaveFormControlPoint const &controlPoint);

	// Remove a control point from the waveform

	void        remove(ControlPointList::iterator &iter);

	// Changes the values of the control point at the iterator
	
	void        setControlPoint(ControlPointList::const_iterator const &iter, WaveFormControlPoint const &controlPoint);

	// Scales all the control points by the specified percent

	void        scaleAll(float const percent);

	// Change the interpolation type

	void        setInterpolationType(InterpolationType const interpolationType);

	// Change the sample type

	void        setSampleType(SampleType const sampleType);

	// Remove all the control points

	void        clear();

	// Set some random values at a random number of control points

	void        randomize(float const maxValue = 1.0f, float const minValue = 0.0f);

	// Load the waveform from an iff

	void        load(Iff &iff);

	// Write the waveform to an iff

	void        write(Iff &iff) const;

	// Get the begin and end iterator from the control point list

	ControlPointList::iterator getIteratorBegin();
	ControlPointList::iterator getIteratorEnd();
	ControlPointList::const_iterator getIteratorBegin() const;
	ControlPointList::const_iterator getIteratorEnd() const;

	// Get the number of control points in the list

	int getControlPointCount() const;

	// Get the interpolation type

	InterpolationType  getInterpolationType() const;

	// Get the sample type

	SampleType         getSampleType() const;

	void clampAll(float const min, float const max);

	bool isConstantValue(float const val);
private:

	ControlPointList  m_controlPointList;
	InterpolationType m_interpolationType;
	SampleType        m_sampleType;
	float             m_valueMax;
	float             m_valueMin;
	bool              m_singleValue;

	// Assures the value is between the maxValue and the minValue

	void         clampValue(WaveFormControlPoint &controlPoint) const;

	// Assures the percent value of the control point is valid, relative to
	// the neighboring control points

	void         clampPercent(ControlPointList::iterator const &iter);

	void         load_0000(Iff &iff);
	void         load_0001(Iff &iff);
	void load_0002(Iff & iff);

#ifdef _DEBUG
	std::string m_name;
	int         m_controlPointCount;

	// Verifies the iterator belongs to this waveform

	bool verifyIteratorValid(ControlPointList::const_iterator const &iter) const;
#endif // _DEBUG
};

//-----------------------------------------------------------------------------
class WaveFormControlPointIter
{
public:

	WaveFormControlPointIter();

	void reset(WaveForm::ControlPointList::const_iterator const &iter);

	WaveForm::ControlPointList::const_iterator m_iter;
	float m_initialPercent; // [0..1]
};

// ============================================================================

#endif // INCLUDED_WaveForm_H
