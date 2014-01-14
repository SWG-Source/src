// ======================================================================
//
// Bresenham.h
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_Bresenham_H
#define INCLUDED_Bresenham_H

// ======================================================================

class Bresenham
{
public:

	//integer point class for use as parameters, etc.
	struct Point3D
	{
		Point3D(int const xx, int const yy, int const zz);
		int x;
		int y;
		int z;
	};

	// functor interface class for allowing FindPointsAlong to convey
	// information back to the caller
	class PointSetter3d
	{
	public:
				PointSetter3d();
		virtual ~PointSetter3d();
		void set(Point3D const & point);

	private:
		PointSetter3d(PointSetter3d const & copy);
		PointSetter3d & operator=(PointSetter3d const & copy);
		virtual void realSet(Point3D const & point) = 0;
	};

	// FindPointsAlong will march from begin to end using a 3D Bresenham
	// line.  for each 'pixel' that the line passes over,
	// HowToSetPoint::set will be called with that pixels placement.

	// sloppyStep allows additional redundency whenever 'shifts' occur
	// while creating the line. example lines based of the value of
	// sloppyStep are below
	//
	// sloppyStep == false            sloppyStep == true
	//         .x....                         .x....
	//         .x....                         .xx...
	//         ..x...                         .xx...
	//         ..x...                         ..x...
	//         ..x...                         ..xx..
	//         ...x..                         ..xx..
	//         ...x..                         ...x..

	static void FindPointsAlong(Point3D const & begin, Point3D const & end, bool const sloppyStep, PointSetter3d & howTo);

private:
	Bresenham();
	Bresenham(Bresenham const & copy);
	Bresenham & operator=(Bresenham const & copy);
};


// ======================================================================

#endif
