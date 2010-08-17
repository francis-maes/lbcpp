//
// Copyright (C) 2004 Stefan Seefeld
// All rights reserved.
// Licensed to the public under the terms of the GNU LGPL (>= 2),
// see the file COPYING for details.
//
#ifndef Synopsis_PTree_Writer_hh_
#define Synopsis_PTree_Writer_hh_

#include <synopsis/PTree.hh>
#include <sstream>

namespace Synopsis
{
namespace PTree
{

class Writer : private Visitor
{
public:
  Writer(std::ostream &os);

  unsigned long write(Node const *);

private:

  virtual void visit(Atom *);
  virtual void visit(List *);
  virtual void visit(Brace *);

  void newline();

  std::ostream &my_os;
  size_t        my_indent;
  unsigned long my_lines;
};

inline std::string reify(Node const *p)
{
  if (!p) return "";
  else if (p->is_atom()) return std::string(p->position(), p->length());

  std::ostringstream oss;
  Writer writer(oss);
  writer.write(p);
  return oss.str();
}

}
}

#endif
