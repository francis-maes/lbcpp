//
// Copyright (C) 2004 Stefan Seefeld
// All rights reserved.
// Licensed to the public under the terms of the GNU LGPL (>= 2),
// see the file COPYING for details.
//
#ifndef Synopsis_PTree_Atoms_hh_
#define Synopsis_PTree_Atoms_hh_

#include <synopsis/PTree/NodesFwd.hh>
#include <synopsis/PTree/Node.hh>

namespace Synopsis
{
namespace PTree
{

class Literal : public Atom
{
public:
  Literal(Token const &tk) : Atom(tk), my_type(tk.type) {}
  virtual void accept(Visitor *visitor) { visitor->visit(this);}
  Token::Type type() const { return my_type;}
private:
  Token::Type my_type;
};

class CommentedAtom : public Atom
{
public:
  CommentedAtom(Token const &tk, Node *c = 0) : Atom(tk), my_comments(c) {}
  CommentedAtom(char const *p, size_t l, Node *c = 0) : Atom(p, l), my_comments(c) {}
  virtual void accept(Visitor *visitor) { visitor->visit(this);}

  Node *get_comments() { return my_comments;}
  void set_comments(Node *c) { my_comments = c;}
private:
  Node *my_comments;
};

// class DupLeaf is used by Ptree::Make() and QuoteClass (qMake()).
// The string given to the constructors are duplicated.

class DupAtom : public CommentedAtom 
{
public:
  DupAtom(char const *, size_t);
  DupAtom(char const *, size_t, char const *, size_t);
  virtual void accept(Visitor *visitor) { visitor->visit(this);}
};

class Identifier : public CommentedAtom
{
public:
  Identifier(Token const &t) : CommentedAtom(t) {}
  Identifier(char const *p, size_t l) : CommentedAtom(p, l) {}
  virtual void accept(Visitor *visitor) { visitor->visit(this);}
};

class Keyword : public CommentedAtom
{
public:
  Keyword(Token const &t) : CommentedAtom(t) {}
  Keyword(char const *str, int len) : CommentedAtom(str, len) {}
  virtual Token::Type token() const = 0;
  virtual void accept(Visitor *visitor) { visitor->visit(this);}
};

template <Token::Type t>
class KeywordT : public Keyword
{
public:
  KeywordT(Token const &tk) : Keyword(tk) {}
  KeywordT(char const *ptr, size_t length) : Keyword(ptr, length) {}
  virtual Token::Type token() const { return t;}
  virtual void accept(Visitor *visitor) { visitor->visit(this);}
};

class UserKeyword : public Keyword
{
public:
  UserKeyword(Token const &t) : Keyword(t), my_type(t.type) {}
  virtual Token::Type token() const { return my_type;}
  virtual void accept(Visitor *visitor) { visitor->visit(this);}
private:
  Token::Type my_type;
};

}
}

#endif
