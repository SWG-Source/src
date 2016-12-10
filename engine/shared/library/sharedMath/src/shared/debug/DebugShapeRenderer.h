// ======================================================================
//
// DebugShapeRenderer.h
// copyright (c) 2001 Sony Online Entertainment
//
// ----------------------------------------------------------------------

#ifndef INCLUDED_DebugShapeRenderer_H
#define INCLUDED_DebugShapeRenderer_H

#include "sharedMath/VectorArgb.h"

class Sphere;
class Cylinder;
class OrientedCylinder;
class AxialBox;
class YawedBox;
class OrientedBox;
class IndexedTriangleList;
class Vector;
class Object;
class Transform;

typedef std::vector<Vector> VectorVec;

class DebugShapeRenderer
{
public:

	DebugShapeRenderer();
	virtual ~DebugShapeRenderer();
	
	virtual Vector      getScale        ( void ) const;
	virtual Transform   getTransform    ( void ) const;
	
	virtual void        setColor        ( VectorArgb const & color );
	virtual void        setOverrideColor( bool override, VectorArgb const & color = VectorArgb::solidRed );
	
	virtual VectorArgb  getColor        ( void ) const;
	
	virtual void        setDepthTest    ( bool enabled );
	virtual void        setWorldSpace   ( bool worldSpace );
	virtual void        setApplyScale   ( bool applyScale ); // if true, parent object's scale factor will be applied to debug shapes before rendering
	
	virtual void        drawFrame       ( float radius );
	
	virtual void        drawSphere      ( Sphere const & sphere, int tess = 16 );
	virtual void        drawCylinder    ( Cylinder const & cylinder, int tess = 16 );
	virtual void        drawOCylinder   ( OrientedCylinder const & cylinder, int tess = 16 );
	virtual void        drawBox         ( AxialBox const & box );
	virtual void        drawYBox        ( YawedBox const & box );
	virtual void        drawOBox        ( OrientedBox const & box );
	
	virtual void        drawMesh        ( IndexedTriangleList const * mesh );
	virtual void        drawMeshNormals ( IndexedTriangleList const * mesh );
	virtual void        drawLine        ( Vector const & begin, Vector const & end );
	virtual void        drawLineList    ( VectorVec const & verts );
	virtual void        drawLineList    ( VectorVec const & verts, VectorArgb const & color );
	virtual void        drawPolygon     ( VectorVec const & verts );
	virtual void        drawOctahedron  ( Vector const & center, float radius );
	virtual void        drawXZCircle    ( Vector const & center, float radius );
	
	// ----------
	
	void draw ( Sphere const & s );
	void draw ( Cylinder const & s );
	void draw ( OrientedCylinder const & s );
	void draw ( AxialBox const & s );
	void draw ( YawedBox const & s );
	void draw ( OrientedBox const & s );
	
	// ----------
	
	typedef DebugShapeRenderer * (DebugShapeRendererFactory)( Object const * object );
	
	static void setFactory( DebugShapeRendererFactory * factory );
	
	static DebugShapeRenderer * create( Object const * object );

protected:

	bool        m_depthTest;
	bool        m_worldSpace;
	bool        m_applyScale;

private:

	VectorArgb  m_color;
	VectorArgb  m_overrideColor;
	bool        m_override;

};

// ----------------------------------------------------------------------

inline void DebugShapeRenderer::draw ( Sphere const & s )           { drawSphere(s); }
inline void DebugShapeRenderer::draw ( Cylinder const & s )         { drawCylinder(s); }
inline void DebugShapeRenderer::draw ( OrientedCylinder const & s ) { drawOCylinder(s); }
inline void DebugShapeRenderer::draw ( AxialBox const & s )         { drawBox(s); }
inline void DebugShapeRenderer::draw ( YawedBox const & s )         { drawYBox(s); }
inline void DebugShapeRenderer::draw ( OrientedBox const & s )      { drawOBox(s); }

// ----------------------------------------------------------------------

#endif
