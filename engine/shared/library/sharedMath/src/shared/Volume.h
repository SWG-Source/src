// ======================================================================
//
// Volume.h
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef VOLUME_H
#define VOLUME_H

// ======================================================================

class Plane;
class Sphere;
class Transform;
class Vector;

// ======================================================================

class Volume
{
public:

	explicit Volume(int numberOfPlanes);
	Volume(const Vector &originPoint, int numberOfVertices, const Vector *vertexList);
	Volume (const Volume &rhs, const Transform &transform);
	~Volume();
	
	int          getNumberOfPlanes() const;
	void         setPlane(int index, const Plane &plane);
	const Plane &getPlane(int index) const;

	bool contains(const Vector     &point) const;
	bool contains(const Sphere     &sphere) const;
	bool contains(const Vector     *pointCloud, int numberOfPoints) const;

	bool intersects(const Sphere   &sphere) const;
	bool intersects(Vector const & start, Vector const & end) const;
	bool fastConservativeExcludes(const Vector  *pointCloud, int numberOfPoints) const;

	void transform(const Transform &transform);
	void transform(const Volume &source, const Transform &transform);

private:
	
	// disabled
	Volume(const Volume &);
	Volume &operator =(const Volume &);

private:

	int     m_numberOfPlanes;
	Plane  *m_plane;
};

// ======================================================================
// Get the number of planes in the volume
//
// @return The number of planes in the volume.

inline int Volume::getNumberOfPlanes() const
{
	return m_numberOfPlanes;
}

// ======================================================================

#endif
