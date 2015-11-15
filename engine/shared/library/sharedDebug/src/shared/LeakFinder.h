// ======================================================================
//
// LeakFinder.h
// bearhart
//
// Copyright 2005, Sony Online Entertainment
// All Rights Reserved
//
// ======================================================================

#ifndef INCLUDED_LeakFinder_H
#define INCLUDED_LeakFinder_H

// ======================================================================

#include "sharedDebug/CallStack.h"

#include <vector>
#include <list>
#include <unordered_map>

// ======================================================================

class LeakFinder
{
public:

	// --------------------------------------------------------

	LeakFinder();
	~LeakFinder();

	// --------------------------------------------------------

	void clear()                                             { liveObjects.clear(); }

	void onAllocate(void *object);
	void onFree(void *object);

	void onReference(void *object);
	void onDereference(void *object);

	// --------------------------------------------------------

	struct ReferenceCountChange
	{
		enum ChangeDirection { up, down };

		ReferenceCountChange() : dir(up) {}

		ChangeDirection dir;
		CallStack       callStack;
	};

	typedef std::list<ReferenceCountChange> ReferenceCountingData;

	struct LiveObject
	{
		void                                  *object;
		CallStack                              callStack;
		const ReferenceCountingData           *referenceData;
	};
	typedef std::vector<LiveObject> LiveObjectList;

	// --------------------------------------------------------

	bool empty()                                       const { return liveObjects.empty(); }
	int  size()                                        const { return liveObjects.size(); }
	void getCurrentObjects(LiveObjectList &o_objects)  const;

	// --------------------------------------------------------

	void debugPrint() const;

protected:

	void _printReferenceCountingData(const ReferenceCountingData &) const;

	struct ptr_hash {
		size_t operator()(void *p) const { return std::hash<unsigned long>()((unsigned long)p); }
	};

	struct ObjectData
	{
		ObjectData() : referenceData(0) { }
		~ObjectData() { if (referenceData) { delete referenceData; } }

		CallStack callStack;
		ReferenceCountingData *referenceData;
	};

	typedef std::unordered_map<void *, ObjectData, ptr_hash> ObjectMap;

	ObjectMap liveObjects;
};

// ======================================================================

#endif
