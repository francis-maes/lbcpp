//
// Copyright (C) 1997-2000 Shigeru Chiba
// Copyright (C) 2000 Stefan Seefeld
// Copyright (C) 2000 Stephen Davies
// All rights reserved.
// Licensed to the public under the terms of the GNU LGPL (>= 2),
// see the file COPYING for details.
//

#include "synopsis/PTree/operations.hh"

namespace Synopsis
{
namespace PTree
{

bool operator == (const Node &p, char c)
{
  return p.is_atom() && p.length() == 1 && *p.position() == c;
}

bool operator == (const Node &n, const char *str)
{
  if (!n.is_atom()) return false;
  const char *p = n.position();
  size_t l = n.length();
  size_t i = 0;
  for(; i < l; ++i)
    if(p[i] != str[i] || str[i] == '\0')
      return false;
  return str[i] == '\0';
}

bool operator == (const Node &p, const Node &q)
{
  if(!p.is_atom() || !q.is_atom()) return false;

  size_t plen = p.length();
  size_t qlen = q.length();
  if(plen == qlen)
  {
    const char *pstr = p.position();
    const char *qstr = q.position();
    while(plen-- > 0)
      if(pstr[plen] != qstr[plen]) return false;
    return true;
  }
  else return false;
}

bool equal(const Node &n, const char *str, size_t len)
{
  if(!n.is_atom()) return false;
  const char *p = n.position();
  size_t l = n.length();
  if(l == len)
  {
    for(size_t i = 0; i < l; ++i)
      if(p[i] != str[i]) return false;
    return true;
  }
  else return false;
}

bool equal(const Node *p, const Node *q)
{
  if(p == q) return true;
  else if(p == 0 || q == 0) return false;
  else if(p->is_atom() || q->is_atom()) return *p == *q;
  else return equal(p->car(), q->car()) && equal(p->cdr(), q->cdr());
}

/*
  equiv() returns true even if p and q are lists and all the elements
  are equal respectively.
*/
bool equiv(const Node *p, const Node *q)
{
  if(p == q) return true;
  else if(p == 0 || q == 0) return false;
  else if(p->is_atom() || q->is_atom()) return *p == *q;
  else
  {
    while(p != 0 && q != 0)
      if(p->car() != q->car())
	return false;
      else
      {
	p = p->cdr();
	q = q->cdr();
      }
    return p == 0 && q == 0;
  }
}

const Node *last(const Node *p)
{
  if(!p) return 0;

  const Node *next;
  while((next = p->cdr())) p = next;
  return p;
}

Node *last(Node *p)
{
  if(!p) return 0;

  Node *next;
  while((next = p->cdr())) p = next;
  return p;
}

const Node *second(const Node *p)
{
  if(p)
  {
    p = p->cdr();
    if(p) return p->car();
  }
  return 0;
}

Node *second(Node *p)
{
  if(p)
  {
    p = p->cdr();
    if(p) return p->car();
  }
  return p;
}

const Node *third(const Node *p)
{
  if(p)
  {
    p = p->cdr();
    if(p)
    {
      p = p->cdr();
      if(p) return p->car();
    }
  }
  return p;
}

Node *third(Node *p)
{
  if(p)
  {
    p = p->cdr();
    if(p)
    {
      p = p->cdr();
      if(p) return p->car();
    }
  }
  return p;
}

const Node *ca_ar(const Node *p)
{
  while(p != 0 && !p->is_atom()) p = p->car();
  return p;
}

Node *ca_ar(Node *p)
{
  while(p != 0 && !p->is_atom()) p = p->car();
  return p;
}

/*
  length() returns a negative number if p is not a list.
*/
int length(const Node *p)
{
  int i = 0;
  if(p && p->is_atom()) return -2; /* p is not a pair */
  while(p)
  {
    ++i;
    if(p->is_atom()) return -1;	/* p is a pair, but not a list. */
    else p = p->cdr();
  }
  return i;
}

Node *cons(Node *p, Node *q)
{
  return new List(p, q);
}

List *list() 
{
  return 0;
}

List *list(Node *p)
{
  return new PTree::List(p, 0);
}

List *list(Node *p, Node *q)
{
  return new PTree::List(p, new PTree::List(q, 0));
}

List *list(Node *p1, Node *p2, Node *p3)
{
  return new PTree::List(p1, new PTree::List(p2, new PTree::List(p3, 0)));
}

List *list(Node *p1, Node *p2, Node *p3, Node *p4)
{
  return new List(p1, list(p2, p3, p4));
}

List *list(Node *p1, Node *p2, Node *p3, Node *p4, Node *p5)
{
  return nconc(list(p1, p2), list(p3, p4, p5));
}

List *list(Node *p1, Node *p2, Node *p3, Node *p4, Node *p5,
	   Node *p6)
{
  return nconc(list(p1, p2, p3), list(p4, p5, p6));
}

List *list(Node *p1, Node *p2, Node *p3, Node *p4, Node *p5,
	   Node *p6, Node *p7)
{
  return nconc(list(p1, p2, p3), list(p4, p5, p6, p7));
}

List *list(Node *p1, Node *p2, Node *p3, Node *p4, Node *p5,
	   Node *p6, Node *p7, Node *p8)
{
  return nconc(list(p1, p2, p3, p4), list(p5, p6, p7, p8));
}

Node *copy(Node *p)
{
  return append(p, 0);
}

//   q may be a leaf
//
Node *append(Node *p, Node *q)
{
  Node *result, *tail;
  if(!p)
  {
    if(q->is_atom())
      return cons(q, 0);
    else return q;
  }
  result = tail = cons(p->car(), 0);
  p = p->cdr();
  while(p != 0)
  {
    Node *cell = cons(p->car(), 0);
    tail->set_cdr(cell);
    tail = cell;
    p = p->cdr();
  }
  if(q != 0 && q->is_atom()) tail->set_cdr(cons(q, 0));
  else tail->set_cdr(q);
  return result;
}

/*
  replace_all() substitutes SUBST for all occurences of ORIG in LIST.
  It recursively searches LIST for ORIG.
*/
Node *replace_all(Node *list, Node *orig, Node *subst)
{
  if(list && orig && *list == *orig) return subst;
  else if(list == 0 || list->is_atom()) return list;
  else
  {
    Array newlist;
    bool changed = false;
    Node *rest = list;
    while(rest != 0)
    {
      Node *p = rest->car();
      Node *q = replace_all(p, orig, subst);
      newlist.append(q);
      if(p != q) changed = true;
      rest = rest->cdr();
    }

    if(changed) return newlist.all();
    else return list;
  }
}

Node *subst(Node *newone, Node *old, Node *tree)
{
  if(old == tree) return newone;
  else if(tree== 0 || tree->is_atom()) return tree;
  else
  {
    Node *head = tree->car();
    Node *head2 = subst(newone, old, head);
    Node *tail = tree->cdr();
    Node *tail2 = tail == 0 ? tail : subst(newone, old, tail);
    if(head == head2 && tail == tail2) return tree;
    else return cons(head2, tail2);
  }
}

Node *subst(Node *newone1, Node *old1, Node *newone2, Node *old2,
	    Node *tree)
{
  if(old1 == tree) return newone1;
  else if(old2 == tree) return newone2;
  else if(tree == 0 || tree->is_atom()) return tree;
  else
  {
    Node *head = tree->car();
    Node *head2 = subst(newone1, old1, newone2, old2, head);
    Node *tail = tree->cdr();
    Node *tail2 = tail == 0 ? tail : subst(newone1, old1, newone2, old2, tail);
    if(head == head2 && tail == tail2) return tree;
    else return cons(head2, tail2);
  }
}

Node *subst(Node *newone1, Node *old1, Node *newone2, Node *old2,
	    Node *newone3, Node *old3, Node *tree)
{
  if(old1 == tree) return newone1;
  else if(old2 == tree) return newone2;
  else if(old3 == tree) return newone3;
  else if(tree == 0 || tree->is_atom()) return tree;
  else
  {
    Node *head = tree->car();
    Node *head2 = subst(newone1, old1, newone2, old2,
			newone3, old3, head);
    Node *tail = tree->cdr();
    Node *tail2 = tail == 0 ? tail : subst(newone1, old1, newone2, old2,
					   newone3, old3, tail);
    if(head == head2 && tail == tail2) return tree;
    else return cons(head2, tail2);
  }
}

// shallow_subst() doesn't recursively apply substitution to a subtree.

Node *shallow_subst(Node *newone, Node *old, Node *tree)
{
  if(old == tree) return newone;
  else if(tree== 0 || tree->is_atom()) return tree;
  else
  {
    Node *head, *head2;
    head = tree->car();
    if(old == head) head2 = newone;
    else head2 = head;

    Node *tail = tree->cdr();
    Node *tail2 = (tail == 0) ? tail : shallow_subst(newone, old, tail);
    if(head == head2 && tail == tail2) return tree;
    else return cons(head2, tail2);
  }
}

Node *shallow_subst(Node *newone1, Node *old1,
		    Node *newone2, Node *old2, Node *tree)
{
  if(old1 == tree) return newone1;
  else if(old2 == tree) return newone2;
  else if(tree == 0 || tree->is_atom()) return tree;
  else
  {
    Node *head, *head2;
    head = tree->car();
    if(old1 == head) head2 = newone1;
    else if(old2 == head) head2 = newone2;
    else head2 = head;

    Node *tail = tree->cdr();
    Node *tail2 = tail == 0 ? tail : shallow_subst(newone1, old1, newone2, old2, tail);
    if(head == head2 && tail == tail2) return tree;
    else return cons(head2, tail2);
  }
}

Node *shallow_subst(Node *newone1, Node *old1,
		    Node *newone2, Node *old2,
		    Node *newone3, Node *old3, Node *tree)
{
  if(old1 == tree) return newone1;
  else if(old2 == tree) return newone2;
  else if(old3 == tree) return newone3;
  else if(tree == 0 || tree->is_atom()) return tree;
  else
  {
    Node *head, *head2;
    head = tree->car();
    if(old1 == head) head2 = newone1;
    else if(old2 == head) head2 = newone2;
    else if(old3 == head) head2 = newone3;
    else head2 = head;

    Node *tail = tree->cdr();
    Node *tail2 = tail == 0 ? tail : shallow_subst(newone1, old1, newone2, old2,
						   newone3, old3, tail);
    if(head == head2 && tail == tail2) return tree;
    else return cons(head2, tail2);
    }
}

Node *shallow_subst(Node *newone1, Node *old1,
		    Node *newone2, Node *old2,
		    Node *newone3, Node *old3,
		    Node *newone4, Node *old4, Node *tree)
{
  if(old1 == tree) return newone1;
  else if(old2 == tree) return newone2;
  else if(old3 == tree) return newone3;
  else if(old4 == tree) return newone4;
  else if(tree == 0 || tree->is_atom()) return tree;
  else
  {
    Node *head, *head2;
    head = tree->car();
    if(old1 == head) head2 = newone1;
    else if(old2 == head) head2 = newone2;
    else if(old3 == head) head2 = newone3;
    else if(old4 == head) head2 = newone4;
    else head2 = head;

    Node *tail = tree->cdr();
    Node *tail2 = tail == 0 ? tail : shallow_subst(newone1, old1, newone2, old2,
						   newone3, old3, newone4, old4, tail);
    if(head == head2 && tail == tail2) return tree;
    else return cons(head2, tail2);
  }
}

Node *subst_sublist(Node *newsub, Node *oldsub, Node *lst)
{
  if(lst == oldsub) return newsub;
  else return cons(lst->car(), subst_sublist(newsub, oldsub, lst->cdr()));
}

Node *snoc(Node *p, Node *q)
{
  return nconc(p, cons(q, 0));
}

/* nconc is desctructive append */

Node *nconc(Node *p, Node *q)
{
  if(!p) return q;
  else
  {
    last(p)->set_cdr(q);
    return p;
  }
}

Node *nconc(Node *p, Node *q, Node *r)
{
  return nconc(p, nconc(q, r));
}

}
}
