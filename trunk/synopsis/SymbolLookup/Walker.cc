//
// Copyright (C) 2004 Stefan Seefeld
// All rights reserved.
// Licensed to the public under the terms of the GNU LGPL (>= 2),
// see the file COPYING for details.
//
#include <Synopsis/SymbolLookup/Walker.hh>
#include <Synopsis/PTree/Lists.hh>
#include <Synopsis/PTree/TypeVisitor.hh>
#include <Synopsis/Trace.hh>
#include <Synopsis/PTree/Display.hh>
#include <Synopsis/SymbolLookup/Scopes.hh>

using namespace Synopsis;
using namespace SymbolLookup;

Walker::Walker(Scope *scope)
{
  Trace trace("Walker::Walker", Trace::SYMBOLLOOKUP);
  my_scopes.push(scope->ref());
}

Walker::~Walker() 
{
  Trace trace("Walker::~Walker", Trace::SYMBOLLOOKUP);
  Scope *scope = my_scopes.top();
  scope->unref();
  my_scopes.pop();
}

void Walker::visit(PTree::List *node)
{
  Trace trace("Walker::visit(List)", Trace::SYMBOLLOOKUP);
  if (node->car()) node->car()->accept(this);
  if (node->cdr()) node->cdr()->accept(this);
}

void Walker::visit(PTree::Block *node)
{
  Trace trace("Walker::visit(Block)", Trace::SYMBOLLOOKUP);
  Scope *scope = my_scopes.top()->find_scope(node);
  if (!scope)
  {
    // Not all Blocks represent a scope...
    visit_block(node);
  }
  else
  {
    scope->ref();
    my_scopes.push(scope);
    visit_block(node);
    leave_scope();
  }  
}

void Walker::visit(PTree::TemplateDecl *tdecl)
{
  Trace trace("Walker::visit(TemplateDecl)", Trace::SYMBOLLOOKUP);
  traverse_parameters(tdecl);
  // If we are in a template template parameter, the following
  // is just the 'class' keyword.
  // Else it is a Declaration, which we want to traverse.
  PTree::Node *decl = PTree::nth(tdecl, 4);
  if (!decl->is_atom()) decl->accept(this);
  else
  {
    std::cout << "length " << PTree::length(tdecl) << std::endl;
  }
}

void Walker::visit(PTree::NamespaceSpec *spec)
{
  Trace trace("Walker::visit(NamespaceSpec)", Trace::SYMBOLLOOKUP);
  traverse_body(spec);
}

void Walker::visit(PTree::FunctionDefinition *def)
{
  Trace trace("Walker::visit(FunctionDefinition)", Trace::SYMBOLLOOKUP);
  PTree::Node *decl = PTree::third(def);
  visit(static_cast<PTree::Declarator *>(decl)); // visit the declarator
  traverse_body(def);
}

void Walker::visit(PTree::ClassSpec *spec)
{
  Trace trace("Walker::visit(ClassSpec)", Trace::SYMBOLLOOKUP);
  traverse_body(spec);
}

void Walker::visit(PTree::DotMemberExpr *)
{
  Trace trace("Walker::visit(DotMemberExpr)", Trace::SYMBOLLOOKUP);
  std::cout << "Sorry: dot member expression (<postfix>.<name>) not yet supported" << std::endl;
}

void Walker::visit(PTree::ArrowMemberExpr *)
{
  Trace trace("Walker::visit(ArrowMemberExpr)", Trace::SYMBOLLOOKUP);
  std::cout << "Sorry: arrow member expression (<postfix>-><name>) not yet supported" << std::endl;
}

void Walker::traverse_body(PTree::NamespaceSpec *spec)
{
  Trace trace("Walker::traverse_body(NamespaceSpec)", Trace::SYMBOLLOOKUP);
  Scope *scope = my_scopes.top()->find_scope(spec);
  assert(scope);
  scope->ref();
  my_scopes.push(scope);
  PTree::tail(spec, 2)->car()->accept(this);
  leave_scope();
}

void Walker::traverse_body(PTree::ClassSpec *spec)
{
  Trace trace("Walker::traverse_body(ClassSpec)", Trace::SYMBOLLOOKUP);
  if (PTree::ClassBody *body = spec->body())
  {
    Scope *scope = my_scopes.top()->find_scope(spec);
    assert(scope);
    scope->ref();
    my_scopes.push(scope);
    body->accept(this);
    leave_scope();
  }
}

void Walker::traverse_parameters(PTree::TemplateDecl *decl)
{
  Trace trace("Walker::traverse_body(TemplateDecl)", Trace::SYMBOLLOOKUP);
  Scope *scope = my_scopes.top()->find_scope(decl);
  scope->ref();
  my_scopes.push(scope);
  // list of template parameters (TypeParameter or ParameterDeclaration)
  PTree::third(decl)->accept(this);
  leave_scope();
}

void Walker::traverse_body(PTree::FunctionDefinition *def)
{
  Trace trace("Walker::traverse_body(FunctionDefinition)", Trace::SYMBOLLOOKUP);
  PTree::Node *decl = PTree::third(def);

  Scope *scope = my_scopes.top();
  PTree::Encoding name = decl->encoded_name();
  if (name.is_qualified())
  {
    SymbolSet symbols = scope->lookup(name, Scope::DECLARATION);
    assert(!symbols.empty());
    // FIXME: We need type analysis / overload resolution
    //        here to take the right symbol / scope.
    FunctionName const *symbol = dynamic_cast<FunctionName const *>(*symbols.begin());
    assert(symbol);
    scope = symbol->as_scope();
  }
  else
    scope = my_scopes.top()->find_scope(def);
  assert(scope);
  scope->ref();
  my_scopes.push(scope);
  visit_block(static_cast<PTree::Block *>(PTree::nth(def, 3)));
  leave_scope();
}

void Walker::visit_block(PTree::Block *node)
{
  Trace trace("Walker::visit_block(Block)", Trace::SYMBOLLOOKUP);
  visit(static_cast<PTree::List *>(node));
}

void Walker::leave_scope()
{
  Trace trace("Walker::leave_scope", Trace::SYMBOLLOOKUP);
  Scope *top = my_scopes.top();
  my_scopes.pop();
  top->unref();
}
