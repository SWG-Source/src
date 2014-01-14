// ======================================================================
//
// ShapeUtils.h
// copyright (c) 2001 Sony Online Entertainment
//
// ----------------------------------------------------------------------

#ifndef INCLUDED_ShapeUtils_H
#define INCLUDED_ShapeUtils_H

class Transform;
class Vector;
class MultiShape;
class Sphere;
class Cylinder;
class OrientedCylinder;
class AxialBox;
class YawedBox;
class OrientedBox;
class Segment3d;
class Line3d;

// ----------------------------------------------------------------------

namespace ShapeUtils
{

// ----------
// Transform the given shape to parent space and enclose it with another shape

AxialBox	EncloseTransform_ABox	( AxialBox const & box, Transform const & tform );


// ----------


MultiShape       transform  ( MultiShape const & shape,
                              Transform const & tform,
                              float scaleFactor = 1.0f );

Sphere           transform  ( Sphere const & sphere,
                              Transform const & tform,
                              float scaleFactor = 1.0f );

Cylinder         transform  ( Cylinder const & cylinder,
                              Transform const & tform,
                              float scaleFactor = 1.0f );

OrientedCylinder transform  ( OrientedCylinder const & cylinder,
                              Transform const & tform,
                              float scaleFactor = 1.0f );

// This will produce the smallest axis-aligned bounding box containing the transformed box

AxialBox    transform   ( AxialBox const & box,
                          Transform const & tform,
                          float scaleFactor = 1.0f );

// ----------------------------------------------------------------------

Segment3d   transform_p2l   ( Segment3d const & seg,
                              Transform const & tform,
                              float scaleFactor = 1.0f );

Line3d      transform_p2l   ( Line3d const & line,
                              Transform const & tform,
                              float scaleFactor = 1.0f );

// ----------------------------------------------------------------------
// Transform-shape-to-world methods

// Translate-only methods

Sphere              translate   ( Sphere const & sphere, Vector const & t );
Cylinder            translate   ( Cylinder const & cyl, Vector const & t );
AxialBox            translate   ( AxialBox const & box, Vector const & t );
MultiShape          translate   ( MultiShape const & shape, Vector const & t );

Sphere              transform_t ( Sphere const & sphere, Transform const & t );
Cylinder            transform_t ( Cylinder const & cyl, Transform const & t );
AxialBox            transform_t ( AxialBox const & box, Transform const & t );

// ----------
// Yaw-translate methods

Sphere              transform_yt ( Sphere const & sphere, Transform const & t );
Cylinder            transform_yt ( Cylinder const & cyl, Transform const & t );
YawedBox            transform_yt ( AxialBox const & box, Transform const & t );

// ----------
// Rotate-translate methods

Sphere              transform_rt ( Sphere const & sphere, Transform const & t );
OrientedCylinder    transform_rt ( Cylinder const & cyl, Transform const & t );
OrientedCylinder    transform_rt ( OrientedCylinder const & cyl, Transform const & t );
OrientedBox		    transform_rt ( AxialBox const & box, Transform const & t );

// ----------

Sphere              scale ( Sphere const & sphere, float scaleFactor );
Cylinder            scale ( Cylinder const & cylinder, float scaleFactor );
OrientedCylinder    scale ( OrientedCylinder const & cylinder, float scaleFactor );
AxialBox            scale ( AxialBox const & box, float scaleFactor );
YawedBox            scale ( YawedBox const & box, float scaleFactor );
OrientedBox         scale ( OrientedBox const & box, float scaleFactor );

// ----------
// Other useful methods

AxialBox            inflate ( AxialBox const & B, float amount );
Sphere              inflate ( Sphere const & S, float amount );
Cylinder            inflate ( Cylinder const & C, float amount );

}; // namespace ShapeUtils

// ======================================================================

#endif	// #ifndef INCLUDED_ShapeUtils_H

