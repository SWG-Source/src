// ======================================================================
//
// Bresenham.cpp
// copyright (c) 2004 Sony Online Entertainment
// tford
//
// ======================================================================

#include "sharedCollision/FirstSharedCollision.h"
#include "sharedCollision/Bresenham.h"

#include <vector>
#include <algorithm>

// ======================================================================

Bresenham::Point3D::Point3D(int const xx, int const yy, int const zz)
: x(xx)
, y(yy)
, z(zz)
{
}

// ======================================================================

Bresenham::PointSetter3d::PointSetter3d()
{
}

// ----------------------------------------------------------------------

Bresenham::PointSetter3d::~PointSetter3d()
{
}

// ----------------------------------------------------------------------

void Bresenham::PointSetter3d::set(Point3D const & point)
{
	realSet(point);
}

// ======================================================================

void Bresenham::FindPointsAlong(Point3D const & begin, Point3D const & end, bool const sloppyStep, PointSetter3d & howTo)
{
	int const deltaX = end.x - begin.x;
	int const deltaY = end.y - begin.y;
	int const deltaZ = end.z - begin.z;

	int const lengthX = abs(deltaX);
	int const lengthY = abs(deltaY);
	int const lengthZ = abs(deltaZ);

	int const lengthX2 = lengthX * 2;
	int const lengthY2 = lengthY * 2;
	int const lengthZ2 = lengthZ * 2;

	int const incrementX = (deltaX < 0) ? -1 : 1;
	int const incrementY = (deltaY < 0) ? -1 : 1;
	int const incrementZ = (deltaZ < 0) ? -1 : 1;

	int x = begin.x;
	int y = begin.y;
	int z = begin.z;

	// check x is dominant
	if (lengthX2 >= std::max(lengthY2, lengthZ2))
	{
		int errorY = lengthY2 - lengthX;
		int errorZ = lengthZ2 - lengthX;

		while (x != (end.x + incrementX))
		{
			howTo.set(Point3D(x, y, z));

			if (errorY >= 0)
			{
				if (sloppyStep)
				{
					howTo.set(Point3D(x + incrementX, y, z));
					howTo.set(Point3D(x, y + incrementY, z));
				}
				y += incrementY;
				errorY -= lengthX2;
			}

			if (errorZ >= 0)
			{
				if (sloppyStep)
				{
					howTo.set(Point3D(x + incrementX, y, z));
					howTo.set(Point3D(x, y, z + incrementZ));
				}
				z += incrementZ;
				errorZ -= lengthX2;
			}

			x += incrementX;
			errorY += lengthY2;
			errorZ += lengthZ2;
		}
	}
	// check y is dominant
	else if (lengthY2 >= std::max(lengthX2, lengthZ2))
	{
		int errorX = lengthX2 - lengthY;
		int errorZ = lengthZ2 - lengthY;

		while (y != (end.y + incrementY))
		{
			howTo.set(Point3D(x, y, z));
			if (errorX >= 0)
			{
				if (sloppyStep)
				{
					howTo.set(Point3D(x, y + incrementY, z));
					howTo.set(Point3D(x + incrementX, y, z));
				}
				x += incrementX;
				errorX -= lengthY2;
			}

			if (errorZ >= 0)
			{
				if (sloppyStep)
				{
					howTo.set(Point3D(x, y + incrementY, z));
					howTo.set(Point3D(x, y, z + incrementZ));
				}
				z += incrementZ;
				errorZ -= lengthY2;
			}

			y += incrementY;
			errorX += lengthX2;
			errorZ += lengthZ2;
		}
	}
	// check z is dominant
	else if (lengthZ2 >= std::max(lengthX2, lengthY2))
	{
		int errorX = lengthZ2 - lengthZ;
		int errorY = lengthZ2 - lengthZ;

		while (z != (end.z + incrementZ))
		{
			howTo.set(Point3D(x, y, z));
			if (errorX >= 0)
			{
				if (sloppyStep)
				{
					howTo.set(Point3D(x, y, z + incrementZ));
					howTo.set(Point3D(x + incrementX, y, z));
				}
				x += incrementX;
				errorX -= lengthY2;
			}

			if (errorY >= 0)
			{
				if (sloppyStep)
				{
					howTo.set(Point3D(x, y, z + incrementZ));
					howTo.set(Point3D(x, y + incrementY, z));
				}
				y += incrementY;
				errorY -= lengthX2;
			}

			z += incrementZ;
			errorX += lengthX2;
			errorY += lengthY2;
		}
	}
}

