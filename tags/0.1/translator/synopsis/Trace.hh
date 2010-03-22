//
// Copyright (C) 2004 Stefan Seefeld
// All rights reserved.
// Licensed to the public under the terms of the GNU LGPL (>= 2),
// see the file COPYING for details.
//

#ifndef Synopsis_Trace_hh_
#define Synopsis_Trace_hh_

#include <iostream>
#include <string>

namespace Synopsis
{

class Trace
{
public:
  // This category list is incomplete and needs to be filled in as
  // more code is being written.
  enum Category { NONE=0x0,
		  PTREE=0x01,
		  SYMBOLLOOKUP=0x02,
		  PARSING=0x04,
		  TRANSLATION=0x08,
		  ALL=0xff};

  struct Entry
  {
    Entry(bool e);
    Entry(Entry const &e) : enabled(e.enabled) { e.enabled = false;}
    ~Entry();

    template <typename T> Entry const &operator<<(T const &t) const;

    mutable bool enabled;
  };
  friend struct Entry;
  Trace(std::string const &s, unsigned int c)
    : my_scope(s), my_visibility((my_mask & c) != 0)
  {
    if (!my_visibility) return;
    std::cout << indent() << "entering " << my_scope << std::endl;
    ++my_level;
  }
  template <typename T>
  Trace(std::string const &s, unsigned int c, T const &t)
    : my_scope(s), my_visibility((my_mask & c) != 0)
  {
    if (!my_visibility) return;
    std::cout << indent() << "entering " << my_scope << ' ' << t << std::endl;
    ++my_level;
  }
  ~Trace()
  {
    if (!my_visibility) return;
    --my_level;
    std::cout << indent() << "leaving " << my_scope << std::endl;
  }

  template <typename T>
  Entry operator<<(T const &t) const;

  static void enable(unsigned int mask = ALL) { my_mask = mask;}

private:
  static std::string indent() { return std::string(my_level, ' ');}

  static unsigned int my_mask;
  static size_t       my_level;
  std::string         my_scope;
  bool                my_visibility;
};

inline Trace::Entry::Entry(bool e) 
  : enabled(e)
{
  if (enabled)
    std::cout << Trace::indent();
}

inline Trace::Entry::~Entry() 
{
  if (enabled)
    std::cout << std::endl;
}

template <typename T> 
inline Trace::Entry const &Trace::Entry::operator<<(T const &t) const
{
  if (enabled)
    std::cout << t;
  return *this;
}

template <typename T>
inline Trace::Entry Trace::operator<<(T const &t) const
{
  Entry entry(my_visibility);
  return entry << t;
}

}

#endif
