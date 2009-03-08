//
// Copyright (C) 1997-2000 Shigeru Chiba
// Copyright (C) 2000 Stefan Seefeld
// Copyright (C) 2000 Stephen Davies
// All rights reserved.
// Licensed to the public under the terms of the GNU LGPL (>= 2),
// see the file COPYING for details.
//
#include <synopsis/PTree/Node.hh>
#include <synopsis/PTree/Encoding.hh>
#include <synopsis/PTree/operations.hh>
#include <synopsis/Buffer.hh>
#include <iostream>
#include <sstream>
#include <cstring>
#include <cassert>
#include <stdexcept>

using namespace Synopsis;
using namespace PTree;

namespace
{
  //. Make sure the garbage collector is properly initialized.
  struct Initializer
  {
    Initializer() { init_gc();}
  } initializer;
}


Node::Node(const char *ptr, size_t len)
{
  my_data.leaf.position = ptr;
  my_data.leaf.length = len;
}

Node::Node(Node *p, Node *q)
{
  my_data.nonleaf.child = p;
  my_data.nonleaf.next = q;
}

const char *Node::begin() const
{
  if (is_atom()) return position();
  else
  {
    for (const Node *p = this; p; p = p->cdr())
    {
      const char *b = p->car() ? p->car()->begin() : 0;
      if (b) return b;
    }
    return 0;
  }
}

const char *Node::end() const
{
  if (is_atom()) return position() + length();
  else
  {
    int n = PTree::length(this);
    while(n > 0)
    {
      const char *e = PTree::nth(this, --n)->end();
      if (e) return e;
    }    
    return 0;
  }
}

Node *Iterator::pop()
{
  if(!ptree) return 0;
  else
  {
    Node *p = ptree->car();
    ptree = ptree->cdr();
    return p;
  }
}

bool Iterator::next(Node *& car)
{
  if(!ptree) return false;
  else
  {
    car = ptree->car();
    ptree = ptree->cdr();
    return true;
  }
}

Array::Array(size_t s)
{
  num = 0;
  if(s > 8)
  {
    size = s;
    array = new /*(GC)*/ Node *[s];
  }
  else
  {
    size = 8;
    array = default_buf;
  }
}

void Array::append(Node *p)
{
  if(num >= size)
  {
    size += 8;
    Node **a = new /*(GC)*/ Node *[size];
    memmove(a, array, size_t(num * sizeof(Node *)));
    array = a;
  }
  array[num++] = p;
}

Node *&Array::ref(size_t i)
{
  if(i < num) return array[i];
  else throw std::range_error("Array: out of range");
}

Node *Array::all()
{
  Node *lst = 0;
  for(int i = number() - 1; i >= 0; --i)
    lst = cons(ref(i), lst);
  return lst;
}
