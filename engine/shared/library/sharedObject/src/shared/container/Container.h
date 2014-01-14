// ======================================================================
//
// Container.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef	INCLUDED_Container_H
#define	INCLUDED_Container_H

//-----------------------------------------------------------------------

#include "sharedObject/SlotIdArchive.h"
#include "sharedObject/CachedNetworkId.h"
#include "sharedObject/Property.h"

namespace Archive
{
	class AutoDeltaByteStream;
}

class Object;

//-----------------------------------------------------------------------

class ContainerIterator
{
	friend class Container;
public:
	ContainerIterator();
	ContainerIterator(Container &, const std::vector<CachedNetworkId>::iterator &);
	~ContainerIterator();
	ContainerIterator(const ContainerIterator &);
	ContainerIterator & operator= (const ContainerIterator &);

	ContainerIterator operator++(int);
	ContainerIterator& operator++();
	CachedNetworkId & operator*();
	bool operator==(const ContainerIterator &) const;
	bool operator!=(const ContainerIterator &) const;


private:
	std::vector<CachedNetworkId>::iterator m_iterator;
	Container *  m_owner;
};

//-----------------------------------------------------------------------

class ContainerConstIterator
{
	friend class Container;
public:
	ContainerConstIterator();
	ContainerConstIterator(const Container &, const std::vector<CachedNetworkId>::const_iterator &);
	~ContainerConstIterator();
	ContainerConstIterator(const ContainerConstIterator &);
	ContainerConstIterator & operator= (const ContainerConstIterator &);

	ContainerConstIterator operator++(int);
	ContainerConstIterator& operator++();
	const CachedNetworkId & operator*() const;
	bool operator==(const ContainerConstIterator &) const;
	bool operator!=(const ContainerConstIterator &) const;


private:
	std::vector<CachedNetworkId>::const_iterator m_iterator;
	const Container *  m_owner;
};


//-----------------------------------------------------------------------

/** Abstract base class for containers
 */
class Container : public Property
{
public:

	static PropertyId getClassPropertyId();

public:

	typedef CachedNetworkId ContainedItem;
	typedef std::vector<CachedNetworkId> Contents;

	//Error Codes
	enum ContainerErrorCode
	{
		CEC_Success = 0,
		CEC_Unknown,
		CEC_AddSelf,
		CEC_Full,
		CEC_SlotOccupied,
		CEC_NoSlot,
		CEC_InvalidArrangement,
		CEC_WrongType,
		CEC_NoPermission,
		CEC_OutOfRange,
		CEC_NotFound,
		CEC_AlreadyIn,
		CEC_TooLarge,
		CEC_HouseItemLimit,
		CEC_TooDeep,
		CEC_TryAgain,
		CEC_UnmovableType,
		CEC_Unmovable,
		CEC_CantSee,
		CEC_InventoryFull,
		CEC_TradeEquipped,
		CEC_HopperNotEmpty,
		CEC_VirtualContainerUnreachable,
		CEC_VirtualContainerUserUnreachable,
		CEC_VirtualContainerUserInvalid,
		CEC_BlockedByScript,
		CEC_BlockedByItemBeingTransferred,
		CEC_BlockedBySourceContainer,
		CEC_BlockedByDestinationContainer,
		CEC_NoContainer,
		CEC_SilentError,
		CEC_BioLinkedToOtherPlayer,
		CEC_Last //This must always be the last errorCode
	};

	
	//Iterator
	friend class ContainerIterator;
	friend class ContainerConstIterator;

	// game interface:
	virtual ~Container(void) = 0;

	virtual void     addToWorld();
	virtual void     removeFromWorld();

	virtual bool     isContentItemObservedWith(Object const &item) const;
	virtual bool     isContentItemExposedWith(Object const &item) const;
	virtual bool     canContentsBeObservedWith() const;

	virtual void     alter(real time);
	virtual void     conclude();

	int              getNumberOfItems (void) const;
	virtual bool     mayAdd (const Object& item, ContainerErrorCode& reason) const = 0;
	virtual bool     remove (Object &item, ContainerErrorCode& reason) = 0;
	virtual bool     remove (ContainerIterator &pos, ContainerErrorCode& reason) = 0;

	ContainerIterator           begin();
	ContainerIterator           end();
	ContainerConstIterator      begin() const;
	ContainerConstIterator      end() const;

	// persistence interface:
	// @todo would be nice to protect this somehow.
	// but no good way now so DO NOT CALL THESE FUNCTIONS DIRECTLY
	virtual int            depersistContents(const Object& item);
	virtual bool           internalItemRemoved(const Object& item);
	
public:
	bool             hasChanged(void) const;

	// debugging functions:
	virtual void     debugPrint(std::string &buffer) const =0;
	virtual void     debugLog() const;

protected:

  /** add an item to the contents.  Returns its new position. */
	int              addToContents(Object& item, ContainerErrorCode& error);
	void             clearChanged(void);

	int                    find (const NetworkId& item, ContainerErrorCode& error) const;
	const ContainedItem&   getContents (int position) const;

	Container(PropertyId propertyId, Object& owner);

private:
	bool             internalRemoveItem(const Object & item);
	int              insertNewItem(const ContainedItem & item);               
	  /**  This function is for use only by this class for debugging.
		*  You should use the containedBy pointer on the item to check for whether it is contained.
		*/
  	bool             isItemContained  (const NetworkId& item, ContainerErrorCode & error) const;

	bool                         m_changed;
	Contents                     m_contents; 
	int                          m_numContents;

	Container();
	Container(const Container&);
	Container& operator= (const Container&);
};

//-----------------------------------------------------------------------

#endif

