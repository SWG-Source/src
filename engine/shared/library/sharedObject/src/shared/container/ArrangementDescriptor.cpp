// ======================================================================
//
// ArrangementDescriptor.cpp
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#include "sharedObject/FirstSharedObject.h"
#include "sharedObject/ArrangementDescriptor.h"

#include "sharedFile/Iff.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedObject/ArrangementDescriptorList.h"
#include "sharedObject/SlotId.h"
#include "sharedObject/SlotIdManager.h"

#include <vector>

// ======================================================================

const Tag TAG_ARG  = TAG3(A,R,G);
const Tag TAG_ARGD = TAG(A,R,G,D);

// ======================================================================

ArrangementDescriptor::ArrangementDescriptor(Iff &iff, const CrcLowerString &name)
:	m_name(new CrcLowerString(name)),
	m_referenceCount(0),
	m_arrangements(new ArrangementVector())
{
	// load instance data from iff
	iff.enterForm(TAG_ARGD);

		// handle version loading
		const Tag version = iff.getCurrentName();
		if (version == TAG_0000)
			load_0000(iff);
		else
		{
			char buffer[5];

			ConvertTagToString(version, buffer);
			FATAL(true, ("unsupported ArrangementDescriptor version [%s]", buffer));
		}

	iff.exitForm(TAG_ARGD);
}

// ----------------------------------------------------------------------

const CrcLowerString &ArrangementDescriptor::getName() const
{
	return *m_name;
}

// ----------------------------------------------------------------------

int ArrangementDescriptor::getReferenceCount() const
{
	return m_referenceCount;
}

// ----------------------------------------------------------------------

void ArrangementDescriptor::fetch() const
{
	++m_referenceCount;
}

// ----------------------------------------------------------------------

void ArrangementDescriptor::release() const
{
	--m_referenceCount;

	// check if we need to delete this one
	if (m_referenceCount < 1)
	{
		ArrangementDescriptorList::stopTracking(*this);
		delete const_cast<ArrangementDescriptor*>(this);
	}
}

// ----------------------------------------------------------------------

int ArrangementDescriptor::getArrangementCount() const
{
	return static_cast<int>(m_arrangements->size());
}

// ----------------------------------------------------------------------

const ArrangementDescriptor::SlotIdVector &ArrangementDescriptor::getArrangement(int index) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, static_cast<int>(m_arrangements->size()));

	return (*m_arrangements)[static_cast<size_t>(index)];
}

// ======================================================================

ArrangementDescriptor::~ArrangementDescriptor()
{
	DEBUG_WARNING(m_referenceCount != 0, ("non-zero reference count upon deletion: bad reference handling (%d)", m_referenceCount));

	delete m_arrangements;
	delete m_name;
}

// ----------------------------------------------------------------------

void ArrangementDescriptor::load_0000(Iff &iff)
{
	iff.enterForm(TAG_0000);
	{
		CrcLowerString  slotName("");
		char            buffer[1024];

		SlotIdVector    arrangement;

		// load the arrangement list
		while (!iff.atEndOfForm())
		{
			iff.enterChunk(TAG_ARG);
			{
				// load the arrangement
				while (iff.getChunkLengthLeft())
				{
					// load slot name
					iff.read_string(buffer, sizeof(buffer) - 1);
					slotName.setString(buffer);

					// convert slot name to SlotId
					const SlotId slotId = SlotIdManager::findSlotId(slotName);
					if (slotId == SlotId::invalid)
						DEBUG_WARNING(true, ("ArrangementDescriptor [%s] specified invalid slot name [%s], ignoring", m_name->getString(), slotName.getString()));
					else
						arrangement.push_back(slotId);
				}
			}
			iff.exitChunk(TAG_ARG);

			// add arrangement to list of arrangements
			m_arrangements->push_back(arrangement);

			// cleanup temp vector
			arrangement.clear();
		}
	}
	iff.exitForm(TAG_0000);
}

// ======================================================================
