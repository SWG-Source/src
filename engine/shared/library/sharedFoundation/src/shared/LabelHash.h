// ======================================================================
//
// LabelHash.h
//
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_LabelHash_H
#define INCLUDED_LabelHash_H

// ----------------------------------------------------------------------

class LabelHash
{
public:

	typedef uint32 Id;

	static void         addLabel    ( char const * const domain, char const * const label );

	static Id			hashLabel	( char const * const domain, char const * const label );

	static Id           checkLabel  ( char const * const domain, char const * const label, Id hashValue );

	static const char * findLabel   ( char const * const domain, Id hashValue );
};

// ----------

//#define LABEL_HASH(domain,label)		(LabelHash::hashLabel(#domain,#label))

// ----------

#if DEBUG_LEVEL == DEBUG_LEVEL_DEBUG

// the ##u is so that the compiler interprets the value as an unsigned integer constant

#define LABEL_HASH(domain,label,value)	(LabelHash::checkLabel(#domain,#label,value##u))

#else

#define LABEL_HASH(domain,label,value)	(value##u)

#endif

// ----------------------------------------------------------------------

#endif // #ifndef INCLUDED_LabelHash_H

