//
// Copyright (C) 2004 Stefan Seefeld
// All rights reserved.
// Licensed to the public under the terms of the GNU LGPL (>= 2),
// see the file COPYING for details.
//
#ifndef Synopsis_SymbolLookup_Walker_hh_
#define Synopsis_SymbolLookup_Walker_hh_

#include <synopsis/PTree.hh>
#include <synopsis/SymbolLookup/Scope.hh>
#include <stack>

namespace Synopsis
{
namespace SymbolLookup
{

//. This Walker adjusts the symbol lookup table while the parse tree
//. is being traversed such that symbols in the parse tree can be
//. looked up correctly in the right context.
class Walker : public PTree::Visitor
{
public:
  Walker(Scope *);
  virtual ~Walker();

  using PTree::Visitor::visit;
  virtual void visit(PTree::List *);
  virtual void visit(PTree::Block *);
  virtual void visit(PTree::TemplateDecl *);
  virtual void visit(PTree::NamespaceSpec *);
  virtual void visit(PTree::FunctionDefinition *);
  virtual void visit(PTree::ClassSpec *);
  virtual void visit(PTree::DotMemberExpr *);
  virtual void visit(PTree::ArrowMemberExpr *);

  //. Traverse the body of a namespace definition.
  void traverse_body(PTree::NamespaceSpec *);
  //. Traverse the body of the class definition.
  void traverse_body(PTree::ClassSpec *);
  //. Traverse the template parameter list of a template declaration.
  void traverse_parameters(PTree::TemplateDecl *);
  //. Traverse the body of the function definition.
  void traverse_body(PTree::FunctionDefinition *);

protected:
  Scope const *current_scope() { return my_scopes.top();}
  void leave_scope();
private:
  typedef std::stack<Scope *> Scopes;

  //. the virtual visit(Block) version above does scoping,
  //. which isn't what we want if traversing a function (FIXME: or is it ?)
  //. so the following factors out the common code.
  void visit_block(PTree::Block *);
  //. The symbol lookup table.
  Scopes  my_scopes;
};

}
}

#endif
