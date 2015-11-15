//===================================================================
//
// Transform2d.cpp
// asommers
//
// copyright 2002, sony online entertainment
//
//===================================================================

#include "sharedMath/FirstSharedMath.h"
#include "sharedMath/Transform2d.h"
#include "sharedMath/Rectangle2d.h"

//===================================================================

const Transform2d Transform2d::identity;

//===================================================================

//-------------------------------------------------------------------

void Transform2d::yaw_l (float radians)
{
	const float sine   = sin (radians);
	const float cosine = cos (radians);
	const float a      = m_matrix [0][0];
	const float b      = m_matrix [0][1];
	const float c      = m_matrix [1][0];
	const float d      = m_matrix [1][1];

	m_matrix [0][0] = a * cosine + b *  -sine;
	m_matrix [0][1] = a *   sine + b * cosine;

	m_matrix [1][0] = c * cosine + d *  -sine;
	m_matrix [1][1] = c *   sine + d * cosine;
}

//-------------------------------------------------------------------

void Transform2d::rotateTranslate_p2l(Rectangle2d &o_result, const Rectangle2d &r) const
{
	const float x0 = r.x0 - m_matrix[0][2];
	const float y0 = r.y0 - m_matrix[1][2];
	const float x1 = r.x1 - m_matrix[0][2];
	const float y1 = r.y1 - m_matrix[1][2];

	float tx0, tx1, ty0, ty1;

	//-------------------------
	// calculate max and min x
	tx0=m_matrix[0][0]*x0;
	tx1=m_matrix[0][0]*x1;
	ty0=m_matrix[1][0]*y0;
	ty1=m_matrix[1][0]*y1;
	if (tx1>=tx0)
	{
		if (ty1>=ty0)
		{
			o_result.x1=tx1+ty1;
			o_result.x0=tx0+ty0;
		}
		else
		{
			o_result.x1=tx1+ty0;
			o_result.x0=tx0+ty1;
		}
	}
	else
	{
		if (ty1>=ty0)
		{
			o_result.x1=tx0+ty1;
			o_result.x0=tx1+ty0;
		}
		else
		{
			o_result.x1=tx0+ty0;
			o_result.x0=tx1+ty1;
		}
	}
	//-------------------------

	//-------------------------
	// calculate max and min y
	tx0=m_matrix[0][1]*x0;
	tx1=m_matrix[0][1]*x1;
	ty0=m_matrix[1][1]*y0;
	ty1=m_matrix[1][1]*y1;
	if (tx1>=tx0)
	{
		if (ty1>=ty0)
		{
			o_result.y1=tx1+ty1;
			o_result.y0=tx0+ty0;
		}
		else
		{
			o_result.y1=tx1+ty0;
			o_result.y0=tx0+ty1;
		}
	}
	else
	{
		if (ty1>=ty0)
		{
			o_result.y1=tx0+ty1;
			o_result.y0=tx1+ty0;
		}
		else
		{
			o_result.y1=tx0+ty0;
			o_result.y0=tx1+ty1;
		}
	}
	//-------------------------
}

//-------------------------------------------------------------------
