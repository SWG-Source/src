// ======================================================================
//
// LeakFinder.cpp
// bearhart
//
// Copyright 2005, Sony Online Entertainment
// All Rights Reserved
//
// ======================================================================

#include "sharedDebug/FirstSharedDebug.h"
#include "sharedDebug/LeakFinder.h"

#include <map>

// ======================================================================

LeakFinder::LeakFinder()
{
}

// ----------------------------------------------------------------------

LeakFinder::~LeakFinder()
{
}

// ----------------------------------------------------------------------

void LeakFinder::onAllocate(void *object)
{
	ObjectData od;
	od.callStack.sample();
	liveObjects[object]=od;
}

// ----------------------------------------------------------------------

void LeakFinder::onFree(void *object)
{
	liveObjects.erase(object);
}

// ----------------------------------------------------------------------

void LeakFinder::onReference(void *object)
{
	ObjectMap::iterator oi = liveObjects.find(object);
	if (oi!=liveObjects.end())
	{
		ObjectData &od = oi->second;
		if (!od.referenceData)
		{
			od.referenceData = new ReferenceCountingData;
		}


		ReferenceCountChange rc;
		rc.dir=ReferenceCountChange::up;
		rc.callStack.sample();
		od.referenceData->push_back(rc);
	}
}

// ----------------------------------------------------------------------

void LeakFinder::onDereference(void *object)
{
	ObjectMap::iterator oi = liveObjects.find(object);
	if (oi!=liveObjects.end())
	{
		ObjectData &od = oi->second;
		if (!od.referenceData)
		{
			od.referenceData = new ReferenceCountingData;
		}

		ReferenceCountChange rc;
		rc.dir=ReferenceCountChange::down;
		rc.callStack.sample();
		od.referenceData->push_back(rc);
	}
}

// ----------------------------------------------------------------------

void LeakFinder::getCurrentObjects(LiveObjectList &o_objects) const
{
	o_objects.reserve(o_objects.size() + liveObjects.size());
	for (ObjectMap::const_iterator oi=liveObjects.begin();oi!=liveObjects.end();++oi)
	{
		LiveObject lo;

		lo.object=oi->first;
		lo.callStack=oi->second.callStack;
		lo.referenceData = oi->second.referenceData;
		o_objects.push_back(lo);
	}
}

// ----------------------------------------------------------------------

void LeakFinder::debugPrint() const
{
	LeakFinder::LiveObjectList leaks;
	getCurrentObjects(leaks);

	if (!leaks.empty())
	{
		std::map<CallStack, std::vector<const LiveObject *> > uniques;
		std::map<CallStack, std::vector<const LiveObject *> >::iterator ui;

		LeakFinder::LiveObjectList::iterator li;

		for (li=leaks.begin();li!=leaks.end();++li)
		{
			uniques[li->callStack].push_back(&(*li));
		}

		for (ui=uniques.begin();ui!=uniques.end();++ui)
		{
			bool printedRefs=false;
			REPORT_PRINT(true, ("------------------------------------------------------------------------------\n"));
			REPORT_PRINT(true, ("leaked objects:\n"));

         size_t i;
			for (i=0;i<ui->second.size();++i)
			{
				const ReferenceCountingData *referenceData = ui->second[i]->referenceData;

				const bool printRefs = referenceData && !printedRefs;

				if (i>0 && (i%10 == 0) || printRefs)
				{
					REPORT_PRINT(true, ("\n"));
				}
				REPORT_PRINT(true, ("(0x%08X) ", reinterpret_cast<int>(ui->second[i]->object)));
				if (printRefs)
				{
					printedRefs=true;
					REPORT_PRINT(true, ("\n"));
					_printReferenceCountingData(*referenceData);
				}
			}
			if (i%10!=0)
			{
				REPORT_PRINT(true, ("\n"));
			}

			REPORT_PRINT(true, ("from:\n"));
			ui->first.debugPrint();
			REPORT_PRINT(true, ("------------------------------------------------------------------------------\n"));
		}
	}
}

// ----------------------------------------------------------------------

void LeakFinder::_printReferenceCountingData(const ReferenceCountingData &history) const
{
	if (history.empty())
	{
		REPORT_PRINT(true, ("No reference counting history.\n"));
		return;
	}

	std::map<CallStack, int> refs;
	std::map<CallStack, int> derefs;

	std::list<ReferenceCountChange>::const_iterator hi;
	for (hi=history.begin();hi!=history.end();++hi)
	{
		const ReferenceCountChange &rc = *hi;
		if (rc.dir==ReferenceCountChange::up)
		{
			refs[rc.callStack]++;
		}
		else
		{
			derefs[rc.callStack]++;
		}
	}

	std::map<CallStack, int>::iterator ri;
	for (ri=refs.begin();ri!=refs.end();++ri)
	{
		REPORT_PRINT(true, ("%d Refs:\n", ri->second));
		ri->first.debugPrint();
	}
	for (ri=derefs.begin();ri!=derefs.end();++ri)
	{
		REPORT_PRINT(true, ("%d De-Refs:\n", ri->second));
		ri->first.debugPrint();
	}
}

// ======================================================================
