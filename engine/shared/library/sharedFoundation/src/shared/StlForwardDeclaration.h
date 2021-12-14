// ======================================================================
//
// StlForwardDeclaration.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#ifndef INCLUDED_StlForwardDeclaration_H
#define INCLUDED_StlForwardDeclaration_H

#include <vector>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <queue>
#include <list>
#include <string>

namespace Unicode
{
	typedef unsigned short unicode_char_t;
	typedef std::basic_string<unicode_char_t, std::char_traits<unicode_char_t>, std::allocator<unicode_char_t> > String;
}

// ======================================================================

#endif
