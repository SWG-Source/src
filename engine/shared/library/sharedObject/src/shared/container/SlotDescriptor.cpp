// ======================================================================
//
// SlotDescriptor.cpp
// Copyright 2001 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedObject/FirstSharedObject.h"
#include "sharedObject/SlotDescriptor.h"

#include "sharedFile/Iff.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedObject/SlotDescriptorList.h"
#include "sharedObject/SlotId.h"
#include "sharedObject/SlotIdManager.h"

#include <vector>

// ======================================================================

const Tag TAG_SLTD = TAG(S,L,T,D);

// ======================================================================

SlotDescriptor::SlotDescriptor(Iff &iff, const CrcLowerString &name)
:	m_name(new CrcLowerString(name)),
	m_slots(new SlotIdVector()),
	m_referenceCount(0)
{
	// load object from Iff
	iff.enterForm(TAG_SLTD);

		const Tag version = iff.getCurrentName();
		if (version == TAG_0000)
			load_0000(iff);
		else
		{
			char buffer[5];

			ConvertTagToString(version, buffer);
			DEBUG_FATAL(true, ("unsupported SlotDescriptor version [%s]\n", buffer));
		}

	iff.exitForm(TAG_SLTD);
}

// ----------------------------------------------------------------------

const CrcLowerString &SlotDescriptor::getName() const
{
	return *m_name;
}

// ----------------------------------------------------------------------

const SlotDescriptor::SlotIdVector &SlotDescriptor::getSlots() const
{
	return *m_slots;
}

// ----------------------------------------------------------------------

void SlotDescriptor::fetch() const
{
	++m_referenceCount;
}

// ----------------------------------------------------------------------

void SlotDescriptor::release() const
{
	--m_referenceCount;
	if (m_referenceCount < 1)
	{
		// time to delete

		//-- make sure SlotDescriptorList stops tracking this instance
		SlotDescriptorList::stopTracking(*this);

		//-- delete the instance
		delete const_cast<SlotDescriptor*>(this);
	}
}

// ----------------------------------------------------------------------

int SlotDescriptor::getReferenceCount() const
{
	return m_referenceCount;
}

// ======================================================================

SlotDescriptor::~SlotDescriptor()
{
	// if you get this warning, check if you are trying to delete the
	// object when you should be fetch()ing and release()ing.  Call
	// release() instead.
	DEBUG_WARNING(m_referenceCount != 0, ("SlotDescriptor %s destroyed with non-zero reference count [%d]", m_name->getString(), m_referenceCount));

	delete m_slots;
	delete m_name;
}

// ----------------------------------------------------------------------

void SlotDescriptor::load_0000(Iff &iff)
{
	iff.enterForm(TAG_0000);
		iff.enterChunk(TAG_DATA);
		{
			char            buffer[1024];	
			CrcLowerString  slotName("");

			while (iff.getChunkLengthLeft())
			{
				//-- load the slot name
				iff.read_string(buffer, sizeof(buffer)-1);
				slotName.setString(buffer);

				//-- convert to slot id
				const SlotId slotId = SlotIdManager::findSlotId(slotName);
				if (slotId == SlotId::invalid)
					WARNING(true, ("SlotDescriptor [%s] specified invalid slot [%s], ignoring", m_name->getString(), slotName.getString()));
				else
					m_slots->push_back(slotId);
			}
		}
		iff.exitChunk(TAG_DATA);
	iff.exitForm(TAG_0000);
}

// ======================================================================
