// ======================================================================
//
// ArrangementDescriptorList.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#ifndef INCLUDED_ArrangementDescriptorList_H
#define INCLUDED_ArrangementDescriptorList_H

// ======================================================================

class ArrangementDescriptor;
class ConstCharCrcLowerString;
class CrcLowerString;
class LessPointerComparator;

// ======================================================================

class ArrangementDescriptorList
{
friend class ArrangementDescriptor;

public:

	static void install();

	static const ArrangementDescriptor *fetch(const CrcLowerString &filename);
	static const ArrangementDescriptor *fetch(const std::string &filename);

private:

	typedef std::map<const CrcLowerString*, ArrangementDescriptor*, LessPointerComparator>  DescriptorMap;

private:

	static void remove();
	static void stopTracking(const ArrangementDescriptor &descriptor);

private:

	static bool            ms_installed;
	static DescriptorMap  *ms_descriptors;

	static ConstCharCrcLowerString  ms_defaultFilename;

};

// ======================================================================

#endif
