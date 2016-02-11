// SphereGrid.h
// Copyright 2000-05, Sony Online Entertainment Inc., all rights reserved.
// Author: Neal Kettler

#ifndef	_INCLUDED_SphereGrid_H
#define	_INCLUDED_SphereGrid_H

//-----------------------------------------------------------------------

#include "sharedMath/Sphere.h"
#include "sharedMath/Capsule.h"
#include "sharedMath/SpatialSubdivision.h"
#include "sharedLog/Log.h"

#include <vector>
#include <map>
#include <set>

class Object;


#define INVALID_POB ((const Object*)0xFFFFFFFF)


//
// SphereGrid - Sorts spheres into a grid of GridSize x GridSize units.  This allows
//   fairly quick collision searches for things like trigger volumes.
//
// If an object is on a grid boundry it may reside in multiple grids at the same time.
// Objects are tracked in worldspace so portals are handled automatically.
//
template <class ObjectType, class Accessor>
class SphereGrid
{
public:
	SphereGrid(float GridSize = 100.0, float GridMaxDimension = 20000.0) :
		m_fGridSize( GridSize ),
		m_fGridMaxDimension( GridMaxDimension ),
		m_iGridSquareWidth( (int)((GridMaxDimension * 2.0f) / GridSize)  )
	{};


	void onObjectAdded( ObjectType object);
	void onObjectRemoved( ObjectType object);
	void onObjectMoved( ObjectType object );

	// search functions which check multiple pobs
	void findInRange( Vector const &center_w, float radius, std::set<ObjectType> &results);
	void findInRange(Vector const &center_w, float radius, SpatialSubdivisionFilter<ObjectType> const &filter, std::set<ObjectType> &results);
	void findInRange( Capsule const &queryCapsule_w, std::set<ObjectType> &results);
	void findInRange(Capsule const &queryCapsule_w, SpatialSubdivisionFilter<ObjectType> const &filter, std::set<ObjectType> &results);

	// search functions which check a single pob
	void findInRange(Object const *pob, Vector const &center_p, float radius, std::set<ObjectType> &results);
	void findInRange(Object const *pob, Vector const &center_p, float radius, SpatialSubdivisionFilter<ObjectType> const &filter, std::set<ObjectType> &results);

	//////////////void dumpSphereTree(std::vector<std::pair<ObjectType, Sphere> > &results) const;

private:

	SphereGrid(SphereGrid const &);
	SphereGrid &operator=(SphereGrid const &);

	void findInRangeSphere( Object const *pob, Vector const &center_w, float radius, SpatialSubdivisionFilter<ObjectType> const *filter, std::set<ObjectType> &results);
	void findInRangeCapsule( Object const *pob, Capsule const &queryCapsule_w, SpatialSubdivisionFilter<ObjectType> const *filter, std::set<ObjectType> &results);


	void    getContainingSquares(const Capsule & range, std::vector<int> & results) const;
	void	getContainingSquares( const Sphere& sphere, std::vector<int>& results) const;


	int	getSquare( const Vector& point ) const
	{
		int z = int((point.z + m_fGridMaxDimension) / m_fGridSize);
		int x = int((point.x + m_fGridMaxDimension) / m_fGridSize);
		int square = (m_iGridSquareWidth * z) + x;
		return square;
	}

	std::map< int , std::set< ObjectType > >	m_contents;		// map from square_id -> object list

	std::map< ObjectType, Vector >			m_locations;		// last location of known objects

	const float	m_fGridSize;
	const float	m_fGridMaxDimension;
	const int	m_iGridSquareWidth;

};



//
// Add object to grid squares
//
template<class ObjectType, class Accessor>
inline void SphereGrid<ObjectType,Accessor>::onObjectAdded(ObjectType object)
{
	const Sphere & sphere = Accessor::getExtent(object);
	if(sphere.getRadius() <= 0.0f)
		return;


	// Vector c = sphere.getCenter();
	// LOG("SphereGrid",( "Adding Object (%f %f %f) R = %f  O = %p", c.x, c.y, c.z, sphere.getRadius(), object ));

	m_locations[ object ] = sphere.getCenter();

	std::vector< int > squares;
	getContainingSquares(sphere, squares);


	std::vector< int >::iterator square_iter;
	for( square_iter = squares.begin(); square_iter != squares.end(); ++square_iter )
	{
		m_contents[ *square_iter ].insert( object );
	}
}



template<class ObjectType, class Accessor>
inline void SphereGrid< ObjectType,  Accessor>::onObjectRemoved(ObjectType object)
{
	int i_num_removed = 0;
	Sphere sphere = Accessor::getExtent(object);

	Vector& loc=m_locations[object];
	sphere.setCenter( loc );

	// LOG("SphereGrid", ("Removing object R = %f  O = %p", sphere.getRadius(), object  ) );

	std::vector< int > squares;
	getContainingSquares(sphere, squares);
	std::vector< int >::iterator square_iter;
	for( square_iter = squares.begin(); square_iter != squares.end(); ++square_iter )
	{
		std::set< ObjectType >& object_set = ( m_contents[ *square_iter ] ) ;
		typename std::set< ObjectType >::iterator iter = object_set.find( object );
		if ( iter != object_set.end() )
		{
	       		object_set.erase( iter );
			++i_num_removed;
		}
		else
		{
			DEBUG_FATAL(true, ("SphereGrid::onObjectRemoved, couldn't find object to remove! %p",object));
		}
	}

	// LOG("SphereGrid", ("Num removed  O = %p Num=%d", object, i_num_removed ) );

	m_locations.erase( object );
}



//
// Object moved - make sure it's in the proper grid squares now
//
template<class ObjectType, class Accessor>
inline void SphereGrid< ObjectType,  Accessor>::onObjectMoved( ObjectType object )
{

	Sphere end_sphere = Accessor::getExtent(object);
	Sphere start_sphere( end_sphere );

	Vector& start=m_locations[object];

	start_sphere.setCenter( start );

	std::vector< int > start_squares;
	std::vector< int > end_squares;

	getContainingSquares(start_sphere, start_squares);
	getContainingSquares(end_sphere, end_squares);

	if ( start_squares == end_squares )
		return;   // nothing to do...

	// Remove from old squares, then add to new squares
	std::vector< int >::iterator square_iter;

	for ( square_iter = start_squares.begin(); square_iter != start_squares.end(); ++square_iter )
	{
		std::set< ObjectType >& set = ( m_contents[ *square_iter ] );
		typename std::set< ObjectType >::iterator iter = set.find( object );
		if ( iter == set.end() )
		{
			DEBUG_FATAL(true, ("SphereGrid::onObjectMoved, couldn't find object to move!  %p",object));
		}
		else
		{
			set.erase( iter );
		}
	}
	for ( square_iter = end_squares.begin(); square_iter != end_squares.end(); ++square_iter )
	{
		std::set< ObjectType >& set = ( m_contents[ *square_iter ] );
		set.insert( object );
	}

	m_locations[object] = end_sphere.getCenter();
}


//
//  Get a list of all the squares needed to contain this capsule
//
template<class ObjectType, class Accessor>
inline void SphereGrid<ObjectType, Accessor>::getContainingSquares(const Capsule & range, std::vector<int> & results) const
{
	Sphere bounds = range.getBoundingSphere();
	Vector b_center = bounds.getCenter();
	float b_rad = bounds.getRadius();
	Vector b_min( b_center.x - b_rad, 0, b_center.z - b_rad);
	Vector b_max( b_center.x + b_rad, 0, b_center.z + b_rad);
	int squaremin = getSquare(b_min);
	int squaremax = getSquare(b_max);
	if ( squaremin == squaremax )
	{
		results.push_back(squaremin);
		return;
	}

	int xmin = int((b_min.x + m_fGridMaxDimension) / m_fGridSize);
	int xmax = int((b_max.x + m_fGridMaxDimension) / m_fGridSize);
	int zmin = int((b_min.z + m_fGridMaxDimension) / m_fGridSize);
	int zmax = int((b_max.z + m_fGridMaxDimension) / m_fGridSize);

	float sqrt_two = float(sqrt( 2.0 ));
	for ( int z = zmin; z <= zmax; ++z )
	{
		for ( int x = xmin; x <= xmax; ++x )
		{
			int square = int(m_iGridSquareWidth * z) + x;

			// Create a sphere that contains the square (makes collision simpler)
			Sphere squarebounds( float((x*m_fGridSize) - m_fGridMaxDimension) + float(m_fGridSize/2.0), b_center.y, float((z*m_fGridSize) - m_fGridMaxDimension) + float(m_fGridSize/2.0), float(m_fGridSize/2.0) * sqrt_two );
			if ( range.intersectsSphere( squarebounds ))   // range-->bounds
				results.push_back( square );
		}
	}
}



//
//  Get a list of all the squares needed to contain this sphere
//
template<class ObjectType, class Accessor>
inline void SphereGrid<ObjectType, Accessor>::getContainingSquares(const Sphere & bounds, std::vector<int> & results) const
{
	Vector b_center = bounds.getCenter();
	float b_rad = bounds.getRadius();
	Vector b_min( b_center.x - b_rad, 0, b_center.z - b_rad);
	Vector b_max( b_center.x + b_rad, 0, b_center.z + b_rad);
	int squaremin = getSquare(b_min);
	int squaremax = getSquare(b_max);
	if ( squaremin == squaremax )
	{
		results.push_back(squaremin);
		return;
	}

	int xmin = int((b_min.x + m_fGridMaxDimension) / m_fGridSize);
	int xmax = int((b_max.x + m_fGridMaxDimension) / m_fGridSize);
	int zmin = int((b_min.z + m_fGridMaxDimension) / m_fGridSize);
	int zmax = int((b_max.z + m_fGridMaxDimension) / m_fGridSize);

	float sqrt_two = float(sqrt( 2.0 ));
	for ( int z = zmin; z <= zmax; ++z )
	{
		for ( int x = xmin; x <= xmax; ++x )
		{
			int square = int(m_iGridSquareWidth * z) + x;

			// Create a sphere that contains the square (makes collision simpler)
			Sphere squarebounds( float((x*m_fGridSize) - m_fGridMaxDimension) + float(m_fGridSize/2.0), b_center.y, float((z*m_fGridSize) - m_fGridMaxDimension) + float(m_fGridSize/2.0), float(m_fGridSize/2.0) * sqrt_two );
			if ( bounds.intersectsSphere( squarebounds ))
				results.push_back( square );
		}
	}
}


//
// Common private method called by all findInRange() public methods using a sphere
//
template<class ObjectType, class ExtentAccessor>
inline void SphereGrid<ObjectType, ExtentAccessor>::findInRangeSphere( Object const *pob, Vector const &center_w, float radius,
	SpatialSubdivisionFilter<ObjectType> const *filter, std::set<ObjectType> &results)
{
	Sphere range(center_w, radius);
	std::vector< int > squares;
	getContainingSquares( range, squares );
	std::vector< int >::iterator citer;
	for ( citer = squares.begin(); citer != squares.end(); ++citer )
	{
		typename std::set< ObjectType >::const_iterator oiter;
		int square_id = *citer;
		std::set<  ObjectType >& objects = m_contents[ square_id ];   // objects in this square

		for ( oiter = objects.begin(); oiter != objects.end(); ++oiter )
		{
			Sphere sphere = ExtentAccessor::getExtent( *oiter );


			if ( range.intersectsSphere( sphere))
			{
	       			if ( pob != INVALID_POB )
				{
					Object const* thispob = ExtentAccessor::getCurrentPob( *oiter );
					if ( pob != thispob )
						continue;
				}
				if (( filter ) && ( ! (*filter)(*oiter) ))
				{
					continue;
				}


				results.insert(*oiter);
			}
		}
	}
}



//
// Common private method called by all findInRange() public methods using a capsule
//
template<class ObjectType, class ExtentAccessor>
inline void SphereGrid<ObjectType, ExtentAccessor>::findInRangeCapsule( Object const *pob, Capsule const &range,
	SpatialSubdivisionFilter<ObjectType> const *filter, std::set<ObjectType> &results)
{
	std::vector< int > squares;
	getContainingSquares( range, squares );
	std::vector< int >::iterator citer;
	for ( citer = squares.begin(); citer != squares.end(); ++citer )
	{
		typename std::set< ObjectType >::const_iterator oiter;
		int square_id = *citer;
		std::set<  ObjectType >& objects = m_contents[ square_id ];   // objects in this square

		for ( oiter = objects.begin(); oiter != objects.end(); ++oiter )
		{
			Sphere sphere = ExtentAccessor::getExtent( *oiter );


			if ( range.intersectsSphere( sphere))
			{

	       			if ( pob != INVALID_POB )
				{
					Object const* thispob = ExtentAccessor::getCurrentPob( *oiter );
					if ( pob != thispob )
						continue;
				}
				if (( filter ) && ( ! (*filter)(*oiter) ))
				{
					continue;
				}


				results.insert(*oiter);
			}
		}
	}
}


template<class ObjectType, class ExtentAccessor>
inline void SphereGrid<ObjectType, ExtentAccessor>::findInRange( const Capsule & range, std::set<ObjectType> & results)
{
	findInRangeCapsule( INVALID_POB, range, nullptr, results );
}


template<class ObjectType, class ExtentAccessor>
inline void SphereGrid<ObjectType, ExtentAccessor>::findInRange( Vector const &center_w, float radius, std::set<ObjectType> & results)
{
	findInRangeSphere( INVALID_POB, center_w, radius, nullptr, results );
}


template<class ObjectType, class ExtentAccessor>
inline void SphereGrid<ObjectType, ExtentAccessor>::findInRange( const Capsule & range, SpatialSubdivisionFilter<ObjectType> const &filter, std::set<ObjectType> & results)
{
	findInRangeCapsule( INVALID_POB, range, &filter, results );
}


template<class ObjectType, class ExtentAccessor>
inline void SphereGrid<ObjectType, ExtentAccessor>::findInRange( Vector const &center_w, float radius, SpatialSubdivisionFilter<ObjectType> const &filter, std::set<ObjectType> & results)
{
	findInRangeSphere( INVALID_POB, center_w, radius, &filter, results );
}


template<class ObjectType, class ExtentAccessor>
inline void SphereGrid<ObjectType, ExtentAccessor>::findInRange( Object const *pob, Vector const &center_w, float radius, std::set<ObjectType> & results)
{
	findInRangeSphere( pob, center_w, radius, nullptr, results );
}

template<class ObjectType, class ExtentAccessor>
inline void SphereGrid<ObjectType, ExtentAccessor>::findInRange( Object const *pob, Vector const &center_w, float radius, SpatialSubdivisionFilter<ObjectType> const &filter, std::set<ObjectType> & results)
{
	findInRangeSphere( pob, center_w, radius, &filter, results );
}



//
// DoubleSphereGrid - This class maintains two SphereGrid instances at different scales.
//   Certain objects have very large collision spheres that cover dozens of grid squares.  By using a seperate
//   sphere grid with a larger square size we can reduce the number of duplicate entries in the square contents tables.
//
template <class ObjectType, class Accessor>
class DoubleSphereGrid
{
public:
	DoubleSphereGrid() :
		m_largeGrid(250.0),   // native grid size in meters
		m_smallGrid(100.0),
		m_fMaxSmallSize( 150.0 )
	{}

	void onObjectAdded(ObjectType object);
	void onObjectRemoved(ObjectType object);
	void onObjectMoved(ObjectType object );

	// search functions which check multiple pobs
	void findInRange( Vector const &center_w, float radius, std::set<ObjectType> &results); // const;
	void findInRange(Vector const &center_w, float radius, SpatialSubdivisionFilter<ObjectType> const &filter, std::set<ObjectType> &results);
	void findInRange( Capsule const &queryCapsule_w, std::set<ObjectType> &results);
	void findInRange(Capsule const &queryCapsule_w, SpatialSubdivisionFilter<ObjectType> const &filter, std::set<ObjectType> &results);

	// search functions which check a single pob
	void findInRange(Object const *pob, Vector const &center_p, float radius, std::set<ObjectType> &results);
	void findInRange(Object const *pob, Vector const &center_p, float radius, SpatialSubdivisionFilter<ObjectType> const &filter, std::set<ObjectType> &results);

	//////////////void dumpSphereTree(std::vector<std::pair<ObjectType, Sphere> > &results) const;

private:

		DoubleSphereGrid(DoubleSphereGrid const &);
		DoubleSphereGrid &operator=(DoubleSphereGrid const &);

	SphereGrid< ObjectType, Accessor>       m_largeGrid;
	SphereGrid< ObjectType, Accessor>       m_smallGrid;
	const float			     m_fMaxSmallSize;
};





template<class ObjectType, class Accessor>
inline void DoubleSphereGrid<ObjectType, Accessor>::onObjectAdded(ObjectType object)
{
	const Sphere & sphere = Accessor::getExtent(object);
	if ( sphere.getRadius() >   m_fMaxSmallSize )
	{
		m_largeGrid.onObjectAdded( object );
	}
	else
	{
		m_smallGrid.onObjectAdded( object );
	}
}

template<class ObjectType, class Accessor>
inline void DoubleSphereGrid<ObjectType, Accessor>::onObjectRemoved(ObjectType object)
{
	const Sphere & sphere = Accessor::getExtent(object);
	if ( sphere.getRadius() >   m_fMaxSmallSize )
	{
		m_largeGrid.onObjectRemoved( object );
	}
	else
	{
		m_smallGrid.onObjectRemoved( object );
	}
}


template<class ObjectType, class Accessor>
inline void DoubleSphereGrid<ObjectType, Accessor>::onObjectMoved( ObjectType object )
{
	const Sphere & sphere = Accessor::getExtent(object);
	if ( sphere.getRadius() >   m_fMaxSmallSize )
	{
		m_largeGrid.onObjectMoved( object );
	}
	else
	{
		m_smallGrid.onObjectMoved( object );
	}
}



template<class ObjectType, class Accessor>
inline void DoubleSphereGrid<ObjectType, Accessor>::findInRange( const Capsule & range, std::set<ObjectType> & results)
{
	m_largeGrid.findInRange( range, results );
	m_smallGrid.findInRange( range, results );
}


template<class ObjectType, class Accessor>
inline void DoubleSphereGrid<ObjectType, Accessor>::findInRange(Vector const &center_w, float radius, std::set<ObjectType> & results)
{
	m_largeGrid.findInRange( center_w, radius, results);
	m_smallGrid.findInRange( center_w, radius, results);
}


template<class ObjectType, class Accessor>
inline void DoubleSphereGrid<ObjectType, Accessor>::findInRange( const Capsule & range, SpatialSubdivisionFilter<ObjectType> const &filter, std::set<ObjectType> & results)
{
	m_largeGrid.findInRange( range, filter, results );
	m_smallGrid.findInRange( range, filter, results );
}


template<class ObjectType, class Accessor>
inline void DoubleSphereGrid<ObjectType, Accessor>::findInRange(Vector const &center_w, float radius, SpatialSubdivisionFilter<ObjectType> const &filter, std::set<ObjectType> & results)
{
	m_largeGrid.findInRange( center_w, radius, filter, results);
	m_smallGrid.findInRange( center_w, radius, filter, results);
}


template<class ObjectType, class Accessor>
inline void DoubleSphereGrid<ObjectType, Accessor>::findInRange(Object const *pob, Vector const &center_w, float radius, std::set<ObjectType> & results)
{
	m_largeGrid.findInRange(pob, center_w, radius, results);
	m_smallGrid.findInRange(pob, center_w, radius, results);
}


template<class ObjectType, class Accessor>
inline void DoubleSphereGrid<ObjectType, Accessor>::findInRange(Object const *pob, Vector const &center_w, float radius, SpatialSubdivisionFilter<ObjectType> const &filter, std::set<ObjectType> & results)
{
	m_largeGrid.findInRange(pob, center_w, radius, filter, results);
	m_smallGrid.findInRange(pob, center_w, radius, filter, results);
}




#endif //	_INCLUDED_SphereGrid_H




