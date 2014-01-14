// ======================================================================
//
// ShapeUtils.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedMath/FirstSharedMath.h"
#include "sharedMath/ShapeUtils.h"

#include "sharedMath/MultiShape.h"
#include "sharedMath/Transform.h"
#include "sharedMath/Sphere.h"
#include "sharedMath/Cylinder.h"
#include "sharedMath/OrientedCylinder.h"
#include "sharedMath/AxialBox.h"
#include "sharedMath/YawedBox.h"
#include "sharedMath/OrientedBox.h"
#include "sharedMath/Segment3d.h"
#include "sharedMath/Line3d.h"

namespace ShapeUtils
{

// ----------------------------------------------------------------------

MultiShape	transform	( MultiShape const & shape, Transform const & tform, float scaleFactor )
{
	Vector center = tform.rotateTranslate_l2p( shape.getCenter() * scaleFactor );

	Vector axisX = tform.rotate_l2p( shape.getAxisX() );
	Vector axisY = tform.rotate_l2p( shape.getAxisY() );
	Vector axisZ = tform.rotate_l2p( shape.getAxisZ() );

    IGNORE_RETURN( axisX.normalize() );
    IGNORE_RETURN( axisY.normalize() );
    IGNORE_RETURN( axisZ.normalize() );

    // ----------

    return MultiShape( shape.getBaseType(), 
					   center, 
					   axisX, 
					   axisY, 
					   axisZ, 
					   shape.getExtentX() * scaleFactor, 
					   shape.getExtentY() * scaleFactor, 
					   shape.getExtentZ() * scaleFactor );
}

// ----------------------------------------------------------------------

Cylinder    transform   ( Cylinder const & shape, Transform const & tform, float scaleFactor )
{
    Cylinder temp = shape;

    temp = scale(temp,scaleFactor);

    temp = transform_yt(temp,tform);

    return temp;
}

OrientedCylinder transform ( OrientedCylinder const & shape, Transform const & tform, float scaleFactor )
{
	OrientedCylinder temp = shape;

	temp = scale(temp,scaleFactor);

	temp = transform_rt(temp,tform);

	return temp;
}

Sphere      transform   ( Sphere const & shape, Transform const & tform, float scaleFactor )
{
    Sphere temp = shape;

    temp = scale(temp,scaleFactor);

    temp = transform_rt(temp,tform);

    return temp;
}

AxialBox    transform   ( AxialBox const & shape, Transform const & tform, float scaleFactor )
{
    AxialBox temp = shape;

    temp = scale(temp,scaleFactor);

    temp = transform_t(temp,tform);

    return temp;
}

// ----------------------------------------------------------------------

Segment3d	transform_p2l	( Segment3d const & seg, Transform const & tform, float scaleFactor )
{
	Vector begin = tform.rotateTranslate_p2l(seg.getBegin());
	Vector end = tform.rotateTranslate_p2l(seg.getEnd());

	return Segment3d( begin / scaleFactor, end / scaleFactor );
}

Line3d		transform_p2l	( Line3d const & line, Transform const & tform, float scaleFactor )
{
	Vector point = tform.rotateTranslate_p2l(line.getPoint());
	Vector dir = tform.rotate_p2l(line.getNormal());

	return Line3d( point / scaleFactor, dir );
}

// ----------------------------------------------------------------------
// Transform-shape-to-world methods

// Translate methods

Sphere translate ( Sphere const & sphere, Vector const & t )
{
	Vector center = sphere.getCenter() + t;

	return Sphere(center, sphere.getRadius());
}

Cylinder translate ( Cylinder const & cyl, Vector const & t )
{
	Vector base = cyl.getBase() + t;
	float radius = cyl.getRadius();
	float height = cyl.getHeight();

	return Cylinder(base,radius,height);
}

AxialBox translate ( AxialBox const & box, Vector const & t )
{
	Vector min = box.getMin() + t;
	Vector max = box.getMax() + t;

	return AxialBox(min,max);
}

MultiShape translate ( MultiShape const & shape, Vector const & t )
{
    return MultiShape(  shape.getBaseType(),
                        shape.getShapeType(),
                        shape.getCenter() + t,
                        shape.getAxisX(),
                        shape.getAxisY(),
                        shape.getAxisZ(),
                        shape.getExtentX(),
                        shape.getExtentY(),
                        shape.getExtentZ() );
}

// ----------

Sphere transform_t ( Sphere const & sphere, Transform const & t )
{
	Vector center = t.rotateTranslate_l2p( sphere.getCenter() );

	return Sphere(center, sphere.getRadius());
}

Cylinder transform_t ( Cylinder const & cyl, Transform const & t )
{
	Vector base = cyl.getBase() + t.getPosition_p();
	float radius = cyl.getRadius();
	float height = cyl.getHeight();

	return Cylinder(base,radius,height);
}

AxialBox transform_t ( AxialBox const & box, Transform const & t )
{
	Vector min = box.getMin() + t.getPosition_p();
	Vector max = box.getMax() + t.getPosition_p();

	return AxialBox(min,max);
}

// ----------
// Yaw-translate methods

Sphere transform_yt ( Sphere const & sphere, Transform const & t )
{
	Vector center = t.rotateTranslate_l2p( sphere.getCenter() );

	return Sphere(center, sphere.getRadius());
}

Cylinder transform_yt ( Cylinder const & cyl, Transform const & t )
{
	Vector base = t.rotateTranslate_l2p( cyl.getBase() );

	return Cylinder( base, cyl.getRadius(), cyl.getHeight() );
}

YawedBox transform_yt ( AxialBox const & box, Transform const & t )
{
	real yaw = t.getLocalFrameK_p().theta();

	Vector center = t.rotateTranslate_l2p( box.getCenter() );

	Vector min = center - box.getDelta();
	Vector max = center + box.getDelta();
	
	AxialBox worldBox(min,max);

	return YawedBox(worldBox,yaw);
}

// ----------
// Rotate-translate methods

Sphere transform_rt ( Sphere const & sphere, Transform const & t )
{
	Vector center = t.rotateTranslate_l2p( sphere.getCenter() );

	return Sphere(center, sphere.getRadius());
}

OrientedCylinder transform_rt ( Cylinder const & cyl, Transform const & t )
{
	Vector base = t.rotateTranslate_l2p(cyl.getBase());
    Vector axis = t.rotate_l2p( Vector::unitY );

	return OrientedCylinder ( base, axis, cyl.getRadius(), cyl.getHeight() );
}

OrientedCylinder transform_rt ( OrientedCylinder const & cyl, Transform const & t )
{
	Vector base = t.rotateTranslate_l2p(cyl.getBase());
    Vector axis = t.rotate_l2p(cyl.getAxis());

	return OrientedCylinder ( base, axis, cyl.getRadius(), cyl.getHeight() );
}

OrientedBox		transform_rt ( AxialBox const & box, Transform const & t )
{
	Vector delta = box.getDelta();
	Vector center = t.rotateTranslate_l2p(box.getCenter());

    Vector xAxis = t.rotate_l2p( Vector::unitX );
    Vector yAxis = t.rotate_l2p( Vector::unitY );
    Vector zAxis = t.rotate_l2p( Vector::unitZ );

	Vector worldCenter = t.rotateTranslate_l2p( center );

	OrientedBox temp( worldCenter, xAxis, yAxis, zAxis, delta.x, delta.y, delta.z );

	return temp;
}

// ----------

Sphere scale ( Sphere const & sphere, float scaleFactor )
{
	return Sphere(	sphere.getCenter() * scaleFactor, 
					sphere.getRadius() * scaleFactor );
}

Cylinder scale ( Cylinder const & cylinder, float scaleFactor )
{
	return Cylinder(	cylinder.getBase() * scaleFactor, 
						cylinder.getRadius() * scaleFactor, 
						cylinder.getHeight() * scaleFactor );
}

OrientedCylinder scale ( OrientedCylinder const & cylinder, float scaleFactor )
{
	return OrientedCylinder(	cylinder.getBase() * scaleFactor, 
								cylinder.getAxis(), 
								cylinder.getRadius() * scaleFactor, 
								cylinder.getHeight() * scaleFactor );
}

AxialBox scale ( AxialBox const & box, float scaleFactor )
{
	return AxialBox(	box.getMin() * scaleFactor, 
						box.getMax() * scaleFactor );
}

YawedBox scale ( YawedBox const & box, float scaleFactor )
{
	return YawedBox(	box.getBase() * scaleFactor, 
						box.getAxisX(), 
						box.getAxisZ(), 
						box.getExtentX() * scaleFactor, 
						box.getExtentZ() * scaleFactor, 
						box.getHeight() * scaleFactor );
}

OrientedBox scale ( OrientedBox const & box, float scaleFactor )
{
	return OrientedBox(	box.getCenter() * scaleFactor, 
						box.getAxisX(), 
						box.getAxisY(), 
						box.getAxisZ(), 
						box.getExtentX() * scaleFactor, 
						box.getExtentY() * scaleFactor, 
						box.getExtentZ() * scaleFactor );
}

// ----------------------------------------------------------------------

AxialBox inflate ( AxialBox const & box, float amount )
{
	return AxialBox( box.getMax() + Vector(amount,amount,amount), box.getMin() - Vector(amount,amount,amount) );
}

Sphere inflate ( Sphere const & sphere, float amount )
{
	return Sphere( sphere.getCenter(), sphere.getRadius() + amount );
}

Cylinder inflate ( Cylinder const & cylinder, float amount )
{
	return Cylinder ( cylinder.getBase() - Vector(0.0f,amount,0.0f), cylinder.getRadius() + amount, cylinder.getHeight() + amount * 2.0f );
}
// ----------------------------------------------------------------------

} // namespace ShapeUtils



