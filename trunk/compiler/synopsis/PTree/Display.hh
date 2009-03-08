//
// Copyright (C) 2004 Stefan Seefeld
// All rights reserved.
// Licensed to the public under the terms of the GNU LGPL (>= 2),
// see the file COPYING for details.
//
#ifndef Synopsis_PTree_Display_hh_
#define Synopsis_PTree_Display_hh_

#include <synopsis/PTree.hh>

namespace Synopsis
{
namespace PTree
{

//. The Display class provides an annotated view of the ptree,
//. for debugging purposes
class Display : private Visitor
{
public:
  Display(std::ostream &os, bool encoded);

  void display(Node const *);

  virtual void visit(Atom *);
  virtual void visit(List *);
  // atoms...
  virtual void visit(DupAtom *);
  // ...lists...
  virtual void visit(Brace *);
  virtual void visit(Block *b) { visit(static_cast<Brace *>(b));}
  virtual void visit(ClassBody *b) { visit(static_cast<Brace *>(b));}
  virtual void visit(Declarator *l) { print_encoded(l);}
  virtual void visit(Name *l) { print_encoded(l);}
  virtual void visit(FstyleCastExpr *l) { print_encoded(l);}
private:
  void newline();
  bool too_deep();
  void print_encoded(List *);

  std::ostream &my_os;
  size_t        my_indent;
  bool          my_encoded;
};

class RTTIDisplay : private Visitor
{
public:
  RTTIDisplay(std::ostream &os, bool encoded);

  void display(Node const *);

  virtual void visit(Atom *);
  virtual void visit(List *);
  virtual void visit(DupAtom *);
private:
  void newline();

  std::ostream &my_os;
  size_t        my_indent;
  bool          my_encoded;
};

class DotFileGenerator : public PTree::Visitor
{
public:
  DotFileGenerator(std::ostream &);
  void write(PTree::Node const *ptree);
private:
  virtual void visit(PTree::Atom *a);
  virtual void visit(PTree::List *l);

  std::ostream &my_os;
};

//. Display the given parse tree segment on the given output stream.
//. If 'encoded' is set to 'true', print encoded names / types
//. on appropriate nodes. If 'typeinfo' is set to 'true', print
//. the class names of the nodes.
inline void display(Node const *node, std::ostream &os,
		    bool encoded = false, bool typeinfo = false)
{
  if (typeinfo)
  {
    RTTIDisplay d(os, encoded);
    d.display(node);
  }
  else
  {
    Display d(os, encoded);
    d.display(node);
  }
}

//. Generate a dot file for the given parse tree segment.
inline void generate_dot_file(Node const *node, std::ostream &os)
{
  DotFileGenerator generator(os);
  generator.write(node);
}

}
}

#endif
