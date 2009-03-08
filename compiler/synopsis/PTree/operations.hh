//
// Copyright (C) 2004 Stefan Seefeld
// All rights reserved.
// Licensed to the public under the terms of the GNU LGPL (>= 2),
// see the file COPYING for details.
//
#ifndef Synopsis_PTree_operations_hh_
#define Synopsis_PTree_operations_hh_

#include <synopsis/PTree/Node.hh>
#include <cassert>

namespace Synopsis
{
namespace PTree
{
bool operator == (const Node &p, char c);
inline bool operator != (const Node &p, char c) { return !operator == (p, c);}
bool operator == (const Node &p, const char *str);
inline bool operator != (const Node &p, const char *str) { return !operator == (p, str);}
bool operator == (const Node &p, const Node &q);
inline bool operator != (const Node &p, const Node &q) { return !operator == (p, q);}
bool equal(const Node &p, const char *str, size_t len);
bool equal(const Node *p, const Node *q);
bool equiv(const Node *p, const Node *q);

//. Return the last cons cell.
const Node *last(const Node *);
//. Return the last cons cell.
Node *last(Node *);
inline const Node *first(const Node *p) { return p ? p->car() : 0;}
inline Node *first(Node *p) { return p ? p->car() : 0;}
inline const Node *rest(const Node *p) { return p ? p->cdr() : 0;}
inline Node *rest(Node *p) { return p ? p->cdr() : 0;}
inline const Node *nth(const Node *p, size_t n)
{
  while(p && n-- > 0) p = p->cdr();
  return p ? p->car() : 0;
}
inline Node *nth(Node *p, size_t n)
{
  while(p && n-- > 0) p = p->cdr();
  return p ? p->car() : 0;
}
inline const Node *tail(const Node *p, size_t k)
{
  while(p && k-- > 0) p = p->cdr();
  return p;
}
inline Node *tail(Node *p, size_t k)
{
  while(p && k-- > 0) p = p->cdr();
  return p;
}

const Node *second(const Node *);
Node *second(Node *);
const Node *third(const Node *);
Node *third(Node *);
int length(const Node *);

inline const Node *cadr(const Node *p) { return p->cdr()->car();}
inline Node *cadr(Node *p) { return p->cdr()->car();}
inline const Node *cddr(const Node *p) { return p->cdr()->cdr();}
inline Node *cddr(Node *p) { return p->cdr()->cdr();}
//. compute Caa..ar
const Node *ca_ar(const Node *);
Node *ca_ar(Node *);

Node *cons(Node *, Node *);
List *list();
List *list(Node *);
List *list(Node *, Node *);
List *list(Node *, Node *, Node *);
List *list(Node *, Node *, Node *, Node *);
List *list(Node *, Node *, Node *, Node *, Node *);
List *list(Node *, Node *, Node *, Node *, Node *, Node *);
List *list(Node *, Node *, Node *, Node *, Node *, Node *,
	   Node *);
List *list(Node *, Node *, Node *, Node *, Node *, Node *,
	   Node *, Node *);
Node *copy(Node *);
Node *append(Node *, Node *);
Node *replace_all(Node *, Node *, Node *);
Node *subst(Node *, Node *, Node *);
Node *subst(Node *, Node *, Node *, Node *, Node *);
Node *subst(Node *, Node *, Node *, Node *,
	    Node *, Node *, Node *);
Node *shallow_subst(Node *, Node *, Node *);
Node *shallow_subst(Node *, Node *, Node *, Node *, Node *);
Node *shallow_subst(Node *, Node *, Node *, Node *,
		    Node *, Node *, Node *);
Node *shallow_subst(Node *, Node *, Node *, Node *,
		    Node *, Node *, Node *, Node *, Node *);
Node *subst_sublist(Node *, Node *, Node *);

/* they cause side-effect */
Node *nconc(Node *, Node *);
Node *nconc(Node *, Node *, Node *);
template <typename N> N *nconc(N *p, Node *q) 
{
  assert(p);
  last(p)->set_cdr(q);
  return p;
}

Node *snoc(Node *, Node *);
template <typename N> N *snoc(N *p, Node *q)
{
  return nconc(p, cons(q, 0));
}

}
}

#endif
