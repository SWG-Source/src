// ===============================================================
// VolumeContainer.h
// copyright 2000 Verant Interactive
// All rights reserved
// ===============================================================

#ifndef	INCLUDED_VolumeContainer_H
#define	INCLUDED_VolumeContainer_H

//-----------------------------------------------------------------------

#include "sharedObject/Container.h"
#include "Archive/AutoDeltaByteStream.h"

//-----------------------------------------------------------------------

class VolumeContainmentProperty;

//-----------------------------------------------------------------------

/** A volume container is a container that holds items, limited to a
 * maximum volume.  It represents things like chests, bags, etc.	
 */
class VolumeContainer:public Container
{

public:

	static PropertyId      getClassPropertyId();

public:

	VolumeContainer(Object& owner, int totalVolume);
	~VolumeContainer();

	bool                   add (Object& item, ContainerErrorCode& error, bool allowOverloaded = false);
	bool                   checkVolume(int addedVolume) const;
	int                    getCurrentVolume () const;                                  
	int                    getTotalVolume   () const;
	int                    getTotalVolumeLimitedByParents   () const;
	virtual bool           mayAdd (const Object& item, ContainerErrorCode& error) const;
	int                    recalculateVolume();
	virtual bool           remove (Object &item, ContainerErrorCode& error);
	virtual bool           remove (ContainerIterator &pos, ContainerErrorCode& error);

	virtual void           debugPrint(std::string &buffer) const;	

	// persistence interface:
	// @todo would be nice to protect this somehow.
	// but no good way now so DO NOT CALL THESE FUNCTIONS DIRECTLY
	virtual int            depersistContents(const Object& item);
	virtual bool           internalItemRemoved(const Object& item);

	int  debugDoNotUseSetCapacity(int newValue);
	
	enum 
	{
		VL_NoVolumeLimit = -1
	};
private:
	int    m_currentVolume; 
	int    m_totalVolume;   

public:
	/** Called by the loader to add an item.
	 */
	void addItemFromLoader(int objectId);

private:
	bool checkVolume(const VolumeContainmentProperty &item) const;

	bool  internalRemove(const Object & item, const VolumeContainmentProperty *itemProp = nullptr);
	void  insertNewItem(const Object & item, const VolumeContainmentProperty *itemProp = nullptr);

	void  childVolumeChanged(int volume, bool updateParent);

	VolumeContainer();
	VolumeContainer(const VolumeContainer&);
	VolumeContainer& operator=(const VolumeContainer&);
};

//-----------------------------------------------------------------------

inline int VolumeContainer::getCurrentVolume() const
{
	return m_currentVolume;
}

//-----------------------------------------------------------------------

inline int VolumeContainer::getTotalVolume() const
{
	return m_totalVolume;
}

//-----------------------------------------------------------------------

#endif
