// ======================================================================
//
// CompositeExtent.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedCollision/FirstSharedCollision.h"
#include "sharedCollision/CompositeExtent.h"

#include "sharedCollision/Containment3d.h"
#include "sharedCollision/ExtentList.h"
#include "sharedFile/Iff.h"

#include <vector>

const Tag TAG_CPST = TAG(C,P,S,T);

// ----------------------------------------------------------------------

CompositeExtent::CompositeExtent( ExtentType const & type )
: Extent(type),
  m_extents( new ExtentVec() ),
  m_box()
{
}

CompositeExtent::~CompositeExtent()
{
	for( uint i = 0; i < m_extents->size(); i++ )
	{
		delete m_extents->at(i);

		m_extents->at(i) = nullptr;
	}

	delete m_extents;
	m_extents = nullptr;

}

// ----------------------------------------------------------------------

void CompositeExtent::load ( Iff & iff ) 
{
	iff.enterForm(TAG_CPST);

		switch (iff.getCurrentName())
		{
		case TAG_0000:
			load_0000(iff);
			break;

		default:
			FATAL (true, ("CompositeExtent::load - unsupported format"));
			break;
		}

	iff.exitForm(TAG_CPST);
	
}

// ----------

void CompositeExtent::write ( Iff & iff ) const
{
	iff.insertForm(TAG_CPST);

		iff.insertForm(TAG_0000);

			for(int i = 0; i < getExtentCount(); i++)
			{
				getExtent(i)->write(iff);
			}
			
		iff.exitForm(TAG_0000);

	iff.exitForm(TAG_CPST);
}

// ----------------------------------------------------------------------

int CompositeExtent::getExtentCount ( void ) const
{
	return static_cast<int>(m_extents->size());
}

// ----------

BaseExtent * CompositeExtent::getExtent ( int whichExtent ) 
{
	return m_extents->at( static_cast<uint>(whichExtent) );
}

// ----------

BaseExtent const * CompositeExtent::getExtent ( int whichExtent ) const
{
	return m_extents->at( static_cast<uint>(whichExtent) );
}

// ----------

void CompositeExtent::attachExtent ( BaseExtent * newExtent )
{
	m_extents->push_back( newExtent );

	updateBounds();
}

// ----------------------------------------------------------------------

void CompositeExtent::drawDebugShapes ( DebugShapeRenderer * renderer ) const
{
	UNREF(renderer);

#ifdef _DEBUG

	if(renderer == nullptr) return;

	for(int i = 0; i < getExtentCount(); i++)
	{
		getExtent(i)->drawDebugShapes(renderer);
	}

#endif
}

// ----------

void CompositeExtent::cloneChildren ( CompositeExtent * destExtent ) const
{
	for(int i = 0; i < getExtentCount(); i++)
	{
		destExtent->attachExtent( getExtent(i)->clone() );
	}
}

// ----------

void CompositeExtent::copy ( BaseExtent const * source )
{
	CompositeExtent const * compositeSource = safe_cast< CompositeExtent const * >(source);

	DEBUG_FATAL( getExtentCount() != compositeSource->getExtentCount(), ("CompositeExtent::copy - Source extent has a different number of children\n"));

	for(int i = 0; i < compositeSource->getExtentCount(); i++)
	{
		getExtent(i)->copy( compositeSource->getExtent(i) );
	}

	updateBounds();
}

// ----------

void CompositeExtent::transform	( BaseExtent const * source, Transform const & tform, float scale )
{
	CompositeExtent const * compositeSource = safe_cast< CompositeExtent const * >(source);

	DEBUG_FATAL( getExtentCount() != compositeSource->getExtentCount(), ("CompositeExtent::copy - Source extent has a different number of children\n"));

	for(int i = 0; i < compositeSource->getExtentCount(); i++)
	{
		getExtent(i)->transform( compositeSource->getExtent(i), tform, scale );
	}

	updateBounds();
}

// ----------

AxialBox CompositeExtent::getBoundingBox ( void ) const
{
	return m_box;
}

Sphere CompositeExtent::getBoundingSphere ( void ) const
{
	return m_sphere;
}

void CompositeExtent::updateBounds ( void )
{
	if(getExtentCount() == 0) 
	{
		m_sphere = Sphere::zero;
		m_box = AxialBox();
	}
	else
	{
		Sphere sphere = getExtent(0)->getBoundingSphere();
		AxialBox box = getExtent(0)->getBoundingBox();

		for(int i = 1; i < getExtentCount(); i++)
		{
			sphere = Containment3d::EncloseSphere( sphere, getExtent(i)->getBoundingSphere() );
			box = Containment3d::EncloseABox( box, getExtent(i)->getBoundingBox() );
		}

		m_sphere = sphere;
		m_box = box;
	}
}

// ----------------------------------------------------------------------

void CompositeExtent::load_0000 ( Iff & iff )
{
	iff.enterForm(TAG_0000);

		while(!iff.atEndOfForm())
		{
			Extent * pExtent = ExtentList::create(iff);

			attachExtent(pExtent);
		}
		
	iff.exitForm(TAG_0000);

	updateBounds();
}

// ======================================================================

