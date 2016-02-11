// ======================================================================
//
// AiDebugString.cpp
// Copyright Sony Online Entertainment, Inc.
//
// ======================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/AiDebugString.h"

#ifdef _DEBUG

#include "sharedFoundation/Crc.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedMath/Circle.h"
#include "sharedMath/Transform.h"
#include "sharedMath/Vector.h"
#include "sharedObject/CachedNetworkId.h"
#include "Unicode.h"
#include "UnicodeUtils.h"

#include <vector>

// ======================================================================
//
// AiDebugStringNamespace
//
// ======================================================================

namespace AiDebugStringNamespace
{
	char const * const s_resetColorCode = "\\#.";
	int s_sequenceId = 0;
	bool s_textEnabled = true;
}

using namespace AiDebugStringNamespace;

// ======================================================================
//
// AiDebugString.cpp
//
// ======================================================================

// ----------------------------------------------------------------------
AiDebugString::AiDebugString()
 : m_sequenceId(s_sequenceId++)
 , m_clearClientFlag(false)
 , m_textList(new TextList)
 , m_lineList(new LineList)
 , m_pathList(new PathList)
 , m_circleList(new CircleList)
 , m_axisList(new AxisList)
 , m_coneList(new ConeList)
{
}

// ----------------------------------------------------------------------
AiDebugString::AiDebugString(std::string const & text)
 : m_sequenceId(0)
 , m_clearClientFlag(false)
 , m_textList(new TextList)
 , m_lineList(new LineList)
 , m_pathList(new PathList)
 , m_circleList(new CircleList)
 , m_axisList(new AxisList)
 , m_coneList(new ConeList)
{
	Unicode::String const delimiters(Unicode::narrowToWide("`"));
	Unicode::UnicodeStringVector result;

	if (Unicode::tokenize(Unicode::narrowToWide(text), result, &delimiters, nullptr))
	{
		Unicode::UnicodeStringVector::const_iterator iterStringVector = result.begin();

		m_sequenceId = Unicode::toInt(*iterStringVector);
		++iterStringVector;
		m_clearClientFlag = (Unicode::toInt(*iterStringVector) != 0);
		++iterStringVector;

		// Extract the text

		{
			int const textCount = Unicode::toInt(*iterStringVector);
			//DEBUG_REPORT_LOG(true, ("textCount: %d\n", textCount));
			++iterStringVector;

			for (int count = 0; (iterStringVector != result.end()) && (count < textCount); ++count)
			{
				m_textList->push_back(Unicode::wideToNarrow(*iterStringVector));
				++iterStringVector;
			}
		}

		// Extract the lines to objects

		{
			int const lineCount = Unicode::toInt(*iterStringVector);
			//DEBUG_REPORT_LOG(true, ("lineCount: %d\n", lineCount));
			++iterStringVector;

			for (int count = 0; (iterStringVector != result.end()) && (count < lineCount); ++count)
			{
				NetworkId const networkId(Unicode::wideToNarrow(*iterStringVector));
				++iterStringVector;

				Vector position;
				position.x = Unicode::toFloat(*iterStringVector);
				++iterStringVector;

				position.y = Unicode::toFloat(*iterStringVector);
				++iterStringVector;

				position.z = Unicode::toFloat(*iterStringVector);
				++iterStringVector;

				PackedRgb color;
				color.r = static_cast<unsigned char>(Unicode::toInt(*iterStringVector));
				++iterStringVector;

				color.g = static_cast<unsigned char>(Unicode::toInt(*iterStringVector));
				++iterStringVector;

				color.b = static_cast<unsigned char>(Unicode::toInt(*iterStringVector));
				++iterStringVector;

				m_lineList->push_back(std::make_pair(CachedNetworkId(networkId), std::make_pair(position, PackedRgb(color.r, color.g, color.b))));
			}
		}

		// Extract the paths

		{
			int const pathCount = Unicode::toInt(*iterStringVector);
			//DEBUG_REPORT_LOG(true, ("pathCount: %d\n", pathCount));
			++iterStringVector;

			for (int count = 0; (iterStringVector != result.end()) && (count < pathCount); ++count)
			{
				bool const cyclic = Unicode::toInt(*iterStringVector) ? true : false;
				//DEBUG_REPORT_LOG(true, ("cyclic: %s\n", cyclic ? "yes" : "no"));
				++iterStringVector;

				int const pathSize = Unicode::toInt(*iterStringVector);
				//DEBUG_REPORT_LOG(true, ("pathSize: %d\n", pathSize));
				++iterStringVector;
				TransformList transformList;

				for (int pathIndex = 0; pathIndex < pathSize; ++pathIndex)
				{
					float const x = Unicode::toFloat(*iterStringVector);
					++iterStringVector;
					float const y = Unicode::toFloat(*iterStringVector);
					++iterStringVector;
					float const z = Unicode::toFloat(*iterStringVector);
					++iterStringVector;

					Transform transform;
					transform.setPosition_p(x, y, z);
					transformList.push_back(transform);
				}

				m_pathList->push_back(std::make_pair(transformList, cyclic));
			}
		}

		// Extract the circles

		{
			int const circleCount = Unicode::toInt(*iterStringVector);
			++iterStringVector;

			//DEBUG_REPORT_LOG(true, ("circleCount: %d\n", circleCount));

			for (int count = 0; (iterStringVector != result.end()) && (count < circleCount); ++count)
			{
				NetworkId const networkId(Unicode::wideToNarrow(*iterStringVector));
				++iterStringVector;

				float const x = Unicode::toFloat(*iterStringVector);
				++iterStringVector;

				float const y = Unicode::toFloat(*iterStringVector);
				++iterStringVector;

				float const z = Unicode::toFloat(*iterStringVector);
				++iterStringVector;

				float const radius = Unicode::toFloat(*iterStringVector);
				++iterStringVector;

				PackedRgb color;
				color.r = static_cast<unsigned char>(Unicode::toInt(*iterStringVector));
				++iterStringVector;

				color.g = static_cast<unsigned char>(Unicode::toInt(*iterStringVector));
				++iterStringVector;

				color.b = static_cast<unsigned char>(Unicode::toInt(*iterStringVector));
				++iterStringVector;

				m_circleList->push_back(std::make_pair(CachedNetworkId(networkId), std::make_pair(Circle(Vector(x, y, z), radius), color)));
			}
		}

		// Extract the axes

		{
			int const axisCount = Unicode::toInt(*iterStringVector);
			++iterStringVector;

			for (int count = 0; (iterStringVector != result.end()) && (count < axisCount); ++count)
			{
				float const ix = Unicode::toFloat(*iterStringVector);
				++iterStringVector;
				float const iy = Unicode::toFloat(*iterStringVector);
				++iterStringVector;
				float const iz = Unicode::toFloat(*iterStringVector);
				++iterStringVector;

				float const jx = Unicode::toFloat(*iterStringVector);
				++iterStringVector;
				float const jy = Unicode::toFloat(*iterStringVector);
				++iterStringVector;
				float const jz = Unicode::toFloat(*iterStringVector);
				++iterStringVector;

				float const kx = Unicode::toFloat(*iterStringVector);
				++iterStringVector;
				float const ky = Unicode::toFloat(*iterStringVector);
				++iterStringVector;
				float const kz = Unicode::toFloat(*iterStringVector);
				++iterStringVector;

				float const x = Unicode::toFloat(*iterStringVector);
				++iterStringVector;
				float const y = Unicode::toFloat(*iterStringVector);
				++iterStringVector;
				float const z = Unicode::toFloat(*iterStringVector);
				++iterStringVector;

				Transform transform;
				Vector i(ix, iy, iz);
				Vector j(jx, jy, jz);
				Vector k(kx, ky, kz);
				transform.setLocalFrameIJK_p(i, j, k);
				transform.setPosition_p(x, y, z);

				float const axisLength = Unicode::toFloat(*iterStringVector);
				++iterStringVector;

				m_axisList->push_back(std::make_pair(transform, axisLength));
			}
		}

		// Extract the cones

		{
			int const coneCount = Unicode::toInt(*iterStringVector);
			++iterStringVector;

			for (int count = 0; (iterStringVector != result.end()) && (count < coneCount); ++count)
			{
				float const length = Unicode::toFloat(*iterStringVector);
				++iterStringVector;

				float const angle = Unicode::toFloat(*iterStringVector);
				++iterStringVector;

				PackedRgb color;
				color.r = static_cast<unsigned char>(Unicode::toInt(*iterStringVector));
				++iterStringVector;

				color.g = static_cast<unsigned char>(Unicode::toInt(*iterStringVector));
				++iterStringVector;

				color.b = static_cast<unsigned char>(Unicode::toInt(*iterStringVector));
				++iterStringVector;

				m_coneList->push_back(std::make_pair(length, std::make_pair(angle, color)));
			}
		}
	}
}

// ----------------------------------------------------------------------
AiDebugString::AiDebugString(AiDebugString const & rhs)
 : m_sequenceId(rhs.m_sequenceId)
 , m_clearClientFlag(rhs.m_clearClientFlag)
 , m_textList(new TextList(*rhs.m_textList))
 , m_lineList(new LineList(*rhs.m_lineList))
 , m_pathList(new PathList(*rhs.m_pathList))
 , m_circleList(new CircleList(*rhs.m_circleList))
 , m_axisList(new AxisList(*rhs.m_axisList))
 , m_coneList(new ConeList(*rhs.m_coneList))
{
}

// ----------------------------------------------------------------------
AiDebugString::~AiDebugString()
{
	delete m_textList;
	delete m_lineList;
	delete m_pathList;
	delete m_circleList;
	delete m_axisList;
	delete m_coneList;
}

// ----------------------------------------------------------------------
void AiDebugString::addText(std::string const & text, PackedRgb const & color)
{
	std::string coloredText;
	coloredText += getColorCode(color);
	coloredText += text;
	coloredText += getResetColorCode();

	m_textList->push_back(coloredText);
}

// ----------------------------------------------------------------------
void AiDebugString::addLineToObject(NetworkId const & target, PackedRgb const & color)
{
	m_lineList->push_back(std::make_pair(CachedNetworkId(target), std::make_pair(Vector::zero, color)));
}

// ----------------------------------------------------------------------
void AiDebugString::addLineToObjectOffset(NetworkId const & target, Vector const & position_o, PackedRgb const & color)
{
	m_lineList->push_back(std::make_pair(CachedNetworkId(target), std::make_pair(position_o, color)));
}

// ----------------------------------------------------------------------
void AiDebugString::addLineToPosition(Vector const & position_w, PackedRgb const & color)
{
	m_lineList->push_back(std::make_pair(CachedNetworkId::cms_cachedInvalid, std::make_pair(position_w, color)));
}

// ----------------------------------------------------------------------
void AiDebugString::addPath(TransformList const & transformList, bool const cyclic)
{
	m_pathList->push_back(std::make_pair(transformList, cyclic));
}

// ----------------------------------------------------------------------
std::string AiDebugString::toString() const
{
	std::string text;

	IGNORE_RETURN(AiDebugString::toString(text));

	return text;
}

// ----------------------------------------------------------------------
uint32 AiDebugString::toString(std::string & text) const
{
	text.clear();

	FormattedString<4096> formattedString;

	//DEBUG_REPORT_LOG(true, ("m_sequenceId: %d\n", m_sequenceId));
	//DEBUG_REPORT_LOG(true, ("m_clearClientFlag: %s\n", m_clearClientFlag ? "yes" : "no"));

	text += formattedString.sprintf("%d`", m_sequenceId);
	text += m_clearClientFlag ? "1`" : "0`";

	if (s_textEnabled)
	{
		// Add the text

		text += formattedString.sprintf("%u`", m_textList->size());
		//DEBUG_REPORT_LOG(true, ("m_textList->size(): %u\n", m_textList->size()));

		TextList::const_iterator iterTextList = m_textList->begin();

		for (; iterTextList != m_textList->end(); ++iterTextList)
		{
			text += *iterTextList;
			text += "`";
		}
	}
	else
	{
		// No text

		text += "0`";
	}

	// Add the lines

	text += formattedString.sprintf("%u`", m_lineList->size());
	//DEBUG_REPORT_LOG(true, ("m_lineList->size(): %u\n", m_lineList->size()));

	LineList::const_iterator iterLineList = m_lineList->begin();

	for (; iterLineList != m_lineList->end(); ++iterLineList)
	{
		NetworkId const & target = iterLineList->first;
		Vector const & position = iterLineList->second.first;
		PackedRgb const & color = iterLineList->second.second;

		text += formattedString.sprintf("%s`%.1f`%.1f`%.1f`%d`%d`%d`", target.getValueString().c_str(), position.x, position.y, position.z, color.r, color.g, color.b);
	}

	// Add the paths

	text += formattedString.sprintf("%u`", m_pathList->size());
	//DEBUG_REPORT_LOG(true, ("m_pathList->size(): %u\n", m_pathList->size()));

	PathList::const_iterator iterPathList = m_pathList->begin();

	for (; iterPathList != m_pathList->end(); ++iterPathList)
	{
		TransformList const & path = iterPathList->first;
		bool const cyclic = iterPathList->second;
		TransformList::const_iterator iterPath = path.begin();
		//DEBUG_REPORT_LOG(true, ("cyclic: %s\n", cyclic ? "yes" : "no"));
		//DEBUG_REPORT_LOG(true, ("path.size(): %u\n", path.size()));
		text += formattedString.sprintf("%d`%u`", cyclic ? 1 : 0, path.size());

		for (; iterPath != path.end(); ++iterPath)
		{
			Transform const & transform = *iterPath;
			text += formattedString.sprintf("%.1f`%.1f`%.1f`", transform.getPosition_p().x, transform.getPosition_p().y, transform.getPosition_p().z);
		}
	}

	// Add the circles

	text += formattedString.sprintf("%u`", m_circleList->size());
	//DEBUG_REPORT_LOG(true, ("m_circleList->size(): %u\n", m_circleList->size()));

	CircleList::const_iterator iterCircleList = m_circleList->begin();

	for (; iterCircleList != m_circleList->end(); ++iterCircleList)
	{
		NetworkId const & target = iterCircleList->first;
		Circle const & circle = iterCircleList->second.first;
		PackedRgb const & color = iterCircleList->second.second;

		text += formattedString.sprintf("%s`%.1f`%.1f`%.1f`%.1f`%d`%d`%d`", target.getValueString().c_str(), circle.getCenter().x, circle.getCenter().y, circle.getCenter().z, circle.getRadius(), color.r, color.g, color.b);
	}

	// Add the axes

	text += formattedString.sprintf("%u`", m_axisList->size());

	AxisList::const_iterator iterAxisList = m_axisList->begin();

	for (; iterAxisList != m_axisList->end(); ++iterAxisList)
	{
		Transform const & transform = iterAxisList->first;
		float const axisLength = iterAxisList->second;
		Vector const i(transform.getLocalFrameI_p());
		Vector const j(transform.getLocalFrameJ_p());
		Vector const k(transform.getLocalFrameK_p());
		Vector const position(transform.getPosition_p());
		
		text += formattedString.sprintf("%.3f`%.3f`%.3f`%.3f`%.3f`%.3f`%.3f`%.3f`%.3f`%.1f`%.1f`%.1f`%.2f`", i.x, i.y, i.z, j.x, j.y, j.z, k.x, k.y, k.z, position.x, position.y, position.z, axisLength);
	}

	// Add the cones

	text += formattedString.sprintf("%u`", m_coneList->size());

	ConeList::const_iterator iterConeList = m_coneList->begin();

	for (; iterConeList != m_coneList->end(); ++iterConeList)
	{
		float const length = iterConeList->first;
		float const angle = iterConeList->second.first;
		PackedRgb const & color = iterConeList->second.second;
		
		text += formattedString.sprintf("%.2f`%.2f`%d`%d`%d`", length, angle, color.r, color.g, color.b);
	}

	// Skip the sequenceId during the Crc calculation

	uint32 crc = 0;
	std::string::const_iterator iterString = text.begin();

	for (; iterString != text.end(); ++iterString)
	{
		if (*iterString == '`')
		{
			crc = Crc::calculate(&*iterString);
			break;
		}
	}

	return crc;
}

// ----------------------------------------------------------------------
AiDebugString::TextList const & AiDebugString::getTextList() const
{
	return *m_textList;
}

// ----------------------------------------------------------------------
AiDebugString::LineList const & AiDebugString::getLineList() const
{
	return *m_lineList;
}

// ----------------------------------------------------------------------
AiDebugString::PathList const & AiDebugString::getPathList() const
{
	return *m_pathList;
}

// ----------------------------------------------------------------------
void AiDebugString::enableClearClientFlag()
{
	m_clearClientFlag = true;
}

// ----------------------------------------------------------------------
bool AiDebugString::isClearClientFlagEnabled() const
{
	return m_clearClientFlag;
}

// ----------------------------------------------------------------------
void AiDebugString::addCircle(Vector const & center_w, float const radius, PackedRgb const & color)
{
	// World-space circle

	if (radius > 0.0f)
	{
		m_circleList->push_back(std::make_pair(CachedNetworkId::cms_cachedInvalid, std::make_pair(Circle(center_w, radius), color)));
	}
	else
	{
		DEBUG_REPORT_LOG(true, ("AiDebugString::addCircle() Invalid radius(%f) specified. Must be > 0\n", radius));
	}
}

// ----------------------------------------------------------------------
void AiDebugString::addCircleAtObjectOffset(NetworkId const & target, Vector const & position_o, float const radius, PackedRgb const & color)
{
	// Object-space circle relative to the target

	if (radius > 0.0f)
	{	
		m_circleList->push_back(std::make_pair(CachedNetworkId(target), std::make_pair(Circle(position_o, radius), color)));
	}
	else
	{
		DEBUG_REPORT_LOG(true, ("AiDebugString::addCircleAtObjectOffset() Invalid radius(%f) specified. Must be > 0\n", radius));
	}
}

// ----------------------------------------------------------------------
AiDebugString::CircleList const & AiDebugString::getCircleList() const
{
	return *m_circleList;
}

// ----------------------------------------------------------------------------
void AiDebugString::addAxis(Transform const & transform, float const axisLength)
{
	m_axisList->push_back(std::make_pair(transform, axisLength));
}

// ----------------------------------------------------------------------------
AiDebugString::AxisList const & AiDebugString::getAxisList() const
{
	return *m_axisList;
}

// ----------------------------------------------------------------------------
void AiDebugString::addCone(float const length, float const angle, PackedRgb const & color)
{
	m_coneList->push_back(std::make_pair(length, std::make_pair(angle, color)));
}

// ----------------------------------------------------------------------------
AiDebugString::ConeList const & AiDebugString::getConeList() const
{
	return *m_coneList;
}

// ----------------------------------------------------------------------------
std::string const AiDebugString::getColorCode(PackedRgb const & packedRgb)
{
	unsigned const code = packedRgb.asUint32();
	unsigned const rgb = code & 0xFFFFFF;

	return FormattedString<32>().sprintf("\\#%06x", rgb);
}

// ----------------------------------------------------------------------------
char const * const AiDebugString::getResetColorCode()
{
	return s_resetColorCode;
}

// ----------------------------------------------------------------------------
void AiDebugString::setTextEnabled(bool const enabled)
{
	s_textEnabled = enabled;
}

bool AiDebugString::isTextEnabled()
{
	return s_textEnabled;
}

// ======================================================================

#endif // _DEBUG
