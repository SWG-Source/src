// ======================================================================
//
// AutoDeltaObserverOps.h
// Copyright 2001, 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
// 
// ======================================================================

#ifndef	INCLUDED_AutoDeltaObserverOps_H
#define	INCLUDED_AutoDeltaObserverOps_H

//-----------------------------------------------------------------------

namespace Archive {

// enum list of every operation that can be performed on an observed object
enum AutoDeltaObserverOp
{
	// base observer ops
	ADOO_generic,
	ADOO_clear,
	ADOO_erase,
	ADOO_pop_back,
	ADOO_push_back,
	ADOO_insert,
	ADOO_set,
	ADOO_setAll,
	ADOO_unpack,
	ADOO_unpackDelta,
};


}//namespace Archive

#endif	// INCLUDED_AutoDeltaObserverOps_H
