//===================================================================
//
// LotManager.h
// asommers
//
// copyright 2002, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_LotManager_H
#define INCLUDED_LotManager_H

//===================================================================

class BoxExtent;
class Object;
class Rectangle2d;
class SpatialSubdivisionHandle;
class StructureFootprint;

#include "sharedMath/SphereTree.h"
#include "sharedMath/Vector.h"
#include "sharedObject/LotType.h"

//===================================================================

class LotManager
{
public:

	LotManager (float mapWidthInMeters, float chunkWidthInMeters);
	~LotManager ();

	void     addNoBuildEntry (const Object& object, float noBuildRadius);
	void     removeNoBuildEntry (const Object& object);
	int      getNumberOfNoBuildEntries () const;

	void     addStructureFootprintEntry (const Object& object, const StructureFootprint& structureFootprint, int x, int z, RotationType rotation);
	void     removeStructureFootprintEntry (const Object& object);
	int      getNumberOfStructureFootprintEntries () const;

	bool     canPlace (const StructureFootprint* structureFootprint, int x, int z, RotationType rotation, float& height_w, bool forceChunkCreation=true) const;
	bool     canPlace (Rectangle2d const & rectangle2d) const;

	//-- tool interface
	int      getWidth () const;
	LotType  getLotType (int x, int z) const;

private:

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

	class NoBuildEntry
	{
	public:

		NoBuildEntry (Vector const & position_w, float radius);
		~NoBuildEntry ();

		Sphere const     getSphere () const;
		SpatialSubdivisionHandle * getSpatialSubdivisionHandle ();
		void             setSpatialSubdivisionHandle (SpatialSubdivisionHandle * spatialSubdivisionHandle);

	private:

		NoBuildEntry (NoBuildEntry const & rhs);
		NoBuildEntry & operator = (NoBuildEntry const & rhs);

	private:

		Vector const m_position_w;
		float const  m_radius;

		SpatialSubdivisionHandle * m_spatialSubdivisionHandle;
	};

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

	class NoBuildEntrySphereExtentAccessor: public BaseSphereTreeAccessor<NoBuildEntry const *, NoBuildEntrySphereExtentAccessor>
	{
	public:

		static Sphere const getExtent(NoBuildEntry const * const noBuildEntry)
		{
			return noBuildEntry ? noBuildEntry->getSphere () : Sphere::zero;
		}

		static char const *getDebugName(NoBuildEntry const * const /*noBuildEntry*/)
		{
			return 0;
		}

	};

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

	class StructureFootprintEntry
	{
	public:

		const StructureFootprint* m_structureFootprint;
		int                       m_x;
		int                       m_z;
		RotationType              m_rotation;
	};

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

private:

	bool canPlace (BoxExtent& boxExtent, int x, int z, const StructureFootprint* structureFootprint, int structureX, int structureZ, bool forceChunkCreation) const;

private:

	LotManager ();
	LotManager (const LotManager&);
	LotManager& operator= (const LotManager&);

private:

	const float      m_mapWidthInMeters;
	const float      m_chunkWidthInMeters;
	const int        m_width;
	const int        m_width_2;

	typedef std::map<const Object*, NoBuildEntry*> NoBuildEntryMap;
	NoBuildEntryMap* const m_noBuildEntryMap;

	typedef std::map<const Object*, StructureFootprintEntry> StructureFootprintEntryMap;
	StructureFootprintEntryMap* const m_structureFootprintEntryMap;

	SphereTree<const NoBuildEntry*, NoBuildEntrySphereExtentAccessor> m_sphereTree;

};

//===================================================================

#endif
