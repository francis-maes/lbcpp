//
// Copyright (C) 2004 Stefan Seefeld
// All rights reserved.
// Licensed to the public under the terms of the GNU LGPL (>= 2),
// see the file COPYING for details.
//
#ifndef Synopsis_PTree_GC_hh_
#define Synopsis_PTree_GC_hh_

#include <cstring> // for size_t

// define dummy replacements for GC allocator operators

namespace Synopsis
{
namespace PTree
{

enum GCPlacement {GC, NoGC};
class LightObject {};
class Object {};
inline void init_gc() {}
inline void cleanup_gc() {}

}
}
/*
inline void *operator new(size_t size, Synopsis::PTree::GCPlacement)
{
  return ::operator new(size);
}

inline void *operator new [](size_t size, Synopsis::PTree::GCPlacement)
{
  return ::operator new [](size);
}
*/
#endif
