// ======================================================================
//
// SlotDescriptorList.h
// Copyright 2001 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_SlotDescriptorList_H
#define INCLUDED_SlotDescriptorList_H

// ======================================================================

class CrcLowerString;
class LessPointerComparator;
class SlotDescriptor;

// ======================================================================
/**
 * Manages the list of SlotDescriptor objects.
 *
 * This class provides an interface for creating SlotDescriptor
 * objects based on SlotDescriptor files.  So long as a single reference exists 
 * to a SlotDescriptor, that SlotDescriptor will stay loaded and will be returned 
 * to any caller that asks for it by the same filename.
 */

class SlotDescriptorList
{
friend class SlotDescriptor;

public:

	static void install();
	
	static const SlotDescriptor *fetch(const CrcLowerString &filename);
	static const SlotDescriptor *fetch(const std::string &filename);

private:

	typedef std::map<const CrcLowerString*, SlotDescriptor*, LessPointerComparator>  DescriptorMap;

private:

	static void remove();

	static void stopTracking(const SlotDescriptor &descriptor);

private:

	static bool           ms_installed;
	static DescriptorMap *ms_descriptors;

};

// ======================================================================

#endif
