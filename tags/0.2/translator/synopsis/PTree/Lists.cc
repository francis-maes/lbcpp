//
// Copyright (C) 1997-2000 Shigeru Chiba
// Copyright (C) 2004 Stefan Seefeld
// All rights reserved.
// Licensed to the public under the terms of the GNU LGPL (>= 2),
// see the file COPYING for details.
//
#include <synopsis/PTree/Lists.hh>
#include <synopsis/PTree/operations.hh>
#include <synopsis/PTree/Encoding.hh>
#include <iostream>

using namespace Synopsis;
using namespace PTree;

Declarator::Declarator(Node *p, Node *q) // francis
  : List(p, q),
    my_declared_name(0),
    my_comments(0)
{
}

Declarator::Declarator(Node *n)
  : List(n ? n->car() : 0, n ? n->cdr() : 0),
    my_declared_name(0),
    my_comments(0)
{
}

Declarator::Declarator(Node *list, Encoding const &t, Encoding const &n, Node *dname)
  : List(list->car(), list->cdr()),
    my_type(t),
    my_name(n),
    my_declared_name(dname),
    my_comments(0)
{
}

Declarator::Declarator(Encoding const &t, Encoding const &n, Node *dname)
  : List(0, 0),
    my_type(t),
    my_name(n),
    my_declared_name(dname),
    my_comments(0)
{
}

Declarator::Declarator(Node *p, Node *q, Encoding const &t, Encoding const &n, Node *dname)
  : List(p, q),
    my_type(t),
    my_name(n),
    my_declared_name(dname),
    my_comments(0)
{
}

Declarator::Declarator(Node *list, Encoding const &t)
  : List(list->car(), list->cdr()),
    my_type(t),
    my_declared_name(0),
    my_comments(0)
{
}

Declarator::Declarator(Encoding const &t)
  : List(0, 0),
    my_type(t),
    my_declared_name(0),
    my_comments(0)
{
}

Declarator::Declarator(Declarator *decl, Node *p, Node *q)
  : List(p, q),
    my_type(decl->my_type),
    my_name(decl->my_name),
    my_declared_name(decl->my_declared_name),
    my_comments(0)
{
}

Node *Declarator::initializer()
{
  size_t l = PTree::length(this);
  if (l < 2) return 0;
  if (Node *assign = nth(this, l - 2))
    if (*assign == '=')
      return tail(this, l - 1); // initializer-clause
  if (Node *expr = nth(this, l - 1))
    if (!expr->is_atom() && first(expr) && *first(expr) == '(')
      return second(expr); // expression-list
  return 0;
}

Name::Name(Node *p, const Encoding &name)
  : List(p->car(), p->cdr()),
    my_name(name)
{
}

FstyleCastExpr::FstyleCastExpr(const Encoding &type, Node *p, Node *q)
  : List(p, q),
    my_type(type)
{
}

ClassSpec::ClassSpec(Node *p, Node *q, Node *c)
  : List(p, q),
    my_comments(c)
{
}

ClassSpec::ClassSpec(const Encoding &name, Node *car, Node *cdr, Node *c)
  : List(car, cdr),
    my_name(name),
    my_comments(c)
{
}

ClassBody *ClassSpec::body() 
{
  return dynamic_cast<ClassBody *>(PTree::nth(this, 3));
}
