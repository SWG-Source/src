// ======================================================================
//
// DebugShapeRenderer.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedMath/FirstSharedMath.h"
#include "sharedMath/DebugShapeRenderer.h"

#include "sharedMath/Transform.h"

static DebugShapeRenderer::DebugShapeRendererFactory * gs_factory = nullptr;

// ----------------------------------------------------------------------

DebugShapeRenderer::DebugShapeRenderer()
: m_depthTest( true ),
  m_worldSpace( false ),
  m_applyScale( true ),
  m_color( VectorArgb::solidWhite ),
  m_overrideColor( VectorArgb::solidRed),
  m_override( false )
{
}

DebugShapeRenderer::~DebugShapeRenderer()
{
}

Vector DebugShapeRenderer::getScale ( void ) const
{
	return Vector::xyz111;
}

Transform DebugShapeRenderer::getTransform ( void ) const
{
	return Transform::identity;
}

void DebugShapeRenderer::setColor ( VectorArgb const & color )
{
	m_color = color;
}

void DebugShapeRenderer::setOverrideColor ( bool override, VectorArgb const & color )
{
	m_override = override;
	m_overrideColor = color;
}

VectorArgb DebugShapeRenderer::getColor ( void ) const
{
	if(m_override)
	{
		return m_overrideColor;
	}
	else
	{
		return m_color;
	}
}

void DebugShapeRenderer::setDepthTest ( bool enabled )
{
	m_depthTest = enabled;
}

void DebugShapeRenderer::setWorldSpace ( bool worldSpace )
{
	m_worldSpace = worldSpace;
}

void DebugShapeRenderer::setApplyScale ( bool applyScale )
{
	m_applyScale = applyScale;
}

void DebugShapeRenderer::drawFrame ( float )
{
}

void DebugShapeRenderer::drawSphere ( Sphere const &, int )
{
}

void DebugShapeRenderer::drawCylinder ( Cylinder const &, int )
{
}

void DebugShapeRenderer::drawOCylinder ( OrientedCylinder const &, int )
{
}

void DebugShapeRenderer::drawBox ( AxialBox const & )
{
}

void DebugShapeRenderer::drawYBox ( YawedBox const & )
{
}

void DebugShapeRenderer::drawOBox ( OrientedBox const & )
{
}

void DebugShapeRenderer::drawMesh ( IndexedTriangleList const * )
{
}

void DebugShapeRenderer::drawMeshNormals ( IndexedTriangleList const * )
{
}

void DebugShapeRenderer::drawLine ( Vector const & , Vector const & )
{
}

void DebugShapeRenderer::drawLineList ( VectorVec const & )
{
}

void DebugShapeRenderer::drawLineList ( VectorVec const & verts, VectorArgb const & color )
{
	setColor(color);
	drawLineList(verts);
}

void DebugShapeRenderer::drawPolygon ( VectorVec const & )
{
}

void DebugShapeRenderer::drawOctahedron ( Vector const &, float )
{
}

void DebugShapeRenderer::drawXZCircle ( Vector const &, float )
{
}

void DebugShapeRenderer::setFactory ( DebugShapeRenderer::DebugShapeRendererFactory * factory )
{
	gs_factory = factory;
}

DebugShapeRenderer * DebugShapeRenderer::create ( Object const * object )
{
	if(gs_factory == nullptr)
		return nullptr;

	return gs_factory(object);
}
