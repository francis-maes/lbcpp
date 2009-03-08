//
// Copyright (C) 2004 Stefan Seefeld
// All rights reserved.
// Licensed to the public under the terms of the GNU LGPL (>= 2),
// see the file COPYING for details.
//

#include <Synopsis/PTree/Writer.hh>
#include <stdexcept>

using namespace Synopsis;
using namespace PTree;

Writer::Writer(std::ostream &os)
  : my_os(os),
    my_indent(0),
    my_lines(0)
{
}

unsigned long Writer::write(Node const *n)
{
  const_cast<Node *>(n)->accept(this);
  unsigned long lines = my_lines;
  my_lines = 0;
  return lines;
}

void Writer::visit(Atom *a)
{
  const char *ptr = a->position();
  size_t len = a->length();
  while(len-- > 0)
  {
    char c = *ptr++;
    if(c == '\n') newline();
    else my_os.put(c);
  }
}

void Writer::visit(List *l) 
{
  Node *p = l;
  while(true)
  {
    Node *head = p->car();
    if(head != 0) head->accept(this);
    p = p->cdr();
    if(!p) break;
    else if(p->is_atom())
      throw std::runtime_error("Writer::visit(List *): not list");
    else my_os.put(' ');
  }
}

void Writer::visit(Brace *l)
{
  my_os << '{';
  ++my_indent;
  if (l->cdr()) // francis
  {
    Node *p = cadr(l);
    while(p)
    {
      if(p->is_atom())
        throw std::runtime_error("Writer::visit(Brace *): non list");
      else
      {
        newline();
        Node *q = p->car();
        p = p->cdr();
        if(q) q->accept(this);
      }
    }
  }
  else
  {
    my_os << " <NULL> ";
  }
  --my_indent;
  newline();
  my_os << '}';
  newline();
}

void Writer::newline()
{
  my_os.put('\n');
  for(size_t i = 0; i != my_indent; ++i) my_os << "  ";
  ++my_lines;
}

