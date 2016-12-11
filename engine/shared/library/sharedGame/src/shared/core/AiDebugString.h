// ======================================================================
//
// AiDebugString.h
// Copyright Sony Online Entertainment, Inc.
//
// ======================================================================

#ifndef INCLUDED_AiDebugString_H
#define INCLUDED_AiDebugString_H

#ifdef _DEBUG

#include "sharedMath/PackedRgb.h"

class CachedNetworkId;
class Circle;
class NetworkId;
class Transform;
class Vector;

// ----------------------------------------------------------------------
class AiDebugString
{
public:

	typedef std::vector<Transform> TransformList;
	typedef std::vector<std::string> TextList;
	typedef std::vector<std::pair<CachedNetworkId, std::pair<Vector, PackedRgb> > > LineList;
	typedef std::vector<std::pair<TransformList, bool/*cyclic*/> > PathList;
	typedef std::vector<std::pair<CachedNetworkId, std::pair<Circle, PackedRgb> > > CircleList;
	typedef std::vector<std::pair<Transform, float/*axisLength*/> > AxisList;
	typedef std::vector<std::pair<float/*length*/, std::pair<float/*coneAngle*/, PackedRgb> > > ConeList;

	// Use to custom embed multiple colors in text

	static std::string const getColorCode(PackedRgb const & packedRgb);
	static char const * const getResetColorCode();

	static void setTextEnabled(bool const enabled);
	static bool isTextEnabled();

public:

	AiDebugString();
	explicit AiDebugString(std::string const & text);
	explicit AiDebugString(AiDebugString const & rhs);
	~AiDebugString();

	uint32 toString(std::string & text) const;
	std::string toString() const;

	void enableClearClientFlag();
	bool isClearClientFlagEnabled() const;

	// Text

	void addText(std::string const & text, PackedRgb const & color = PackedRgb::solidWhite);
	TextList const & getTextList() const;

	// Lines

	void addLineToObject(NetworkId const & target, PackedRgb const & color = PackedRgb::solidWhite);
	void addLineToObjectOffset(NetworkId const & target, Vector const & position_o, PackedRgb const & color = PackedRgb::solidWhite);
	void addLineToPosition(Vector const & position_w, PackedRgb const & color = PackedRgb::solidWhite);
	LineList const & getLineList() const;

	// Paths

	void addPath(TransformList const & transformList, bool const cyclic);
	PathList const & getPathList() const;

	// Circles

	void addCircle(Vector const & center_w, float const radius, PackedRgb const & color = PackedRgb::solidWhite);
	void addCircleAtObjectOffset(NetworkId const & target, Vector const & position_o, float const radius, PackedRgb const & color = PackedRgb::solidWhite);
	CircleList const & getCircleList() const;

	// Axes

	void addAxis(Transform const & transform, float const axisLength);
	AxisList const & getAxisList() const;

	// Cone (in the direction of the owner z-axis)

	void addCone(float const length, float const coneAngle, PackedRgb const & color = PackedRgb::solidWhite);
	ConeList const & getConeList() const;

private:

	int m_sequenceId;
	bool m_clearClientFlag;
	TextList * const m_textList;
	LineList * const m_lineList;
	PathList * const m_pathList;
	CircleList * const m_circleList;
	AxisList * const m_axisList;
	ConeList * const m_coneList;

	// Disabled

	AiDebugString const & operator =(AiDebugString const &);
};

// ======================================================================

#endif // _DEBUG
#endif // INCLUDED_AiDebugString_H
