// ======================================================================
//
// StlForwardDeclaration.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#ifndef INCLUDED_StlForwardDeclaration_H
#define INCLUDED_StlForwardDeclaration_H

// ======================================================================

// Setup handling for template value defaults that require accessing the
// {template type}::fwd value.  gcc 3.2 requires typename here, gcc 2.95
// optionally accepts typename here, msvc 6 will not accept it here.

#if defined(PLATFORM_WIN32) || defined(WIN32)

#define FORWARD_TYPENAME

#elif defined(PLATFORM_LINUX)

#define FORWARD_TYPENAME  typename

#else

#error unsupported platform

#endif

// ======================================================================

// grab stlport configuration
//#include "stl/config/features.h" // uncomment for newer stlport versions
//#include "stl/_config.h"

namespace std
{
	template <class _Tp>                                                                struct less;
	template <class _Tp>                                                                struct equal_to;
	template <class _T1, class _T2>                                                     struct pair;
	template <class _T1>                                                                struct hash;
	template <class _Tp>                                                                class  allocator;
	template <class _CharT, class _Traits, class _Alloc>                                class  basic_string;
	template <class _CharT>                                                             struct  char_traits;

	template <size_t _Nb>                                                               class  bitset;
	template <class _Tp, class _Alloc>                                                  class  deque;
	template <class _Tp, class _Alloc>                                                  class  list;
	template <class _Tp, class _Alloc>                                                  class  queue;
	template <class _Tp, class _Alloc>                                                  class  vector;
	template <class _Tp, class _Sequence>                                               class  stack;
	template <class _Tp, class _Container, class _Compare>                              class  priority_queue;
	template <class _Key, class _Tp, class _Compare, class _Alloc>                      class  map;
	//template <class _Key, class _Tp, class _HashFcn, class _Compare, class _Alloc>      class  hash_map;
	template <class _Key, class _Tp, class _Compare, class _Alloc>                      class  multimap;
	template <class _Key, class _Compare, class _Alloc>                                 class  set;
	//template <class _Key, class _HashFcn, class _Compare, class _Alloc>                 class  hash_set;
	template <class _Key, class _Compare, class _Alloc>                                 class  multiset;

	typedef basic_string<char, char_traits<char>, allocator<char> >                     string;

	template <class _Key, class _Tp, class _Hash, class _Compare, class _Alloc> 		class unordered_map;
	template <class _Key, class _Hash, class _Compare, class _Alloc> 					class unordered_set;
}

template <class _Tp, class _Alloc = std::allocator<_Tp> > struct stddeque
{
	typedef std::deque<_Tp, _Alloc> fwd;
};

template <class _Tp, class _Alloc = std::allocator<_Tp> > struct stdlist
{
	typedef std::list<_Tp, _Alloc> fwd;
};

template <class _Key, class _Tp, class _Compare = std::less<_Key>, class _Alloc = std::allocator< std::pair <const _Key, _Tp> > > struct stdmap
{
	typedef std::map<_Key, _Tp, _Compare, _Alloc> fwd;
};

//template <class _Key, class _Tp, class _HashFcn = std::hash<_Key>, class _Compare = std::equal_to<_Key>, class _Alloc = std::allocator< std::pair <const _Key, _Tp> > > struct stdhash_map
//{
//	typedef std::hash_map<_Key, _Tp, _HashFcn, _Compare, _Alloc> fwd;
//};

template <class _Key, class _Tp, class _HashFcn = std::hash<_Key>, class _Compare = std::equal_to<_Key>, class _Alloc = std::allocator< std::pair <const _Key, _Tp> > > struct stdunordered_map
{
	typedef std::unordered_map<_Key, _Tp, _HashFcn, _Compare, _Alloc> fwd;
};

template <class _Key, class _Tp, class _Compare = std::less<_Key>, class _Alloc = std::allocator< std::pair <const _Key, _Tp> > > struct stdmultimap
{
	typedef std::multimap<_Key, _Tp, _Compare, _Alloc> fwd;
};

template <class _Key, class _Compare = std::less<_Key>, class _Alloc = std::allocator<_Key> > struct stdset
{
	typedef std::set<_Key, _Compare, _Alloc> fwd;
};

//template <class _Key, class _HashFcn = std::hash<_Key>, class _Compare = std::equal_to<_Key>, class _Alloc = std::allocator<_Key> > struct stdhash_set
//{
//	typedef std::hash_set<_Key, _HashFcn, _Compare, _Alloc> fwd;
//};

template <class _Key, class _HashFcn = std::hash<_Key>, class _Compare = std::equal_to<_Key>, class _Alloc = std::allocator<_Key> > struct stdunordered_set
{
	typedef std::unordered_set<_Key, _HashFcn, _Compare, _Alloc> fwd;
};

template <class _Key, class _Compare = std::less<_Key>, class _Alloc = std::allocator<_Key> > struct stdmultiset
{
	typedef std::multiset<_Key, _Compare, _Alloc> fwd;
};

template <class _Tp, class _Alloc = std::allocator<_Tp> > struct stdvector
{
	typedef std::vector<_Tp, _Alloc> fwd;
};

template <class _Tp, class _Container = FORWARD_TYPENAME stdvector<_Tp>::fwd, class _Compare = std::less<typename _Container::value_type> > struct stdpriority_queue
{
	typedef std::priority_queue<_Tp, _Container, _Compare> fwd;
};

template <class _Tp, class _Sequence = FORWARD_TYPENAME stddeque<_Tp>::fwd > struct stdqueue
{
	typedef std::queue<_Tp, _Sequence> fwd;
};

template <class _Tp, class _Sequence = FORWARD_TYPENAME stddeque<_Tp>::fwd > struct stdstack
{
	typedef std::stack<_Tp, _Sequence> fwd;
};

namespace Unicode
{
	typedef unsigned short unicode_char_t;
	typedef std::basic_string<unicode_char_t, std::char_traits<unicode_char_t>, std::allocator<unicode_char_t> > String;
}

// ======================================================================

#endif
