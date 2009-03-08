//
// Copyright (C) 2004 Stefan Seefeld
// All rights reserved.
// Licensed to the public under the terms of the GNU LGPL (>= 2),
// see the file COPYING for details.
//
#ifndef Synopsis_SymbolLookup_Scopes_hh_
#define Synopsis_SymbolLookup_Scopes_hh_

#include <synopsis/SymbolLookup/Scope.hh>
#include <string>
#include <vector>
#include <list>

namespace Synopsis
{
namespace SymbolLookup
{

class TemplateParameterScope;
class LocalScope;
class PrototypeScope;
class FunctionScope;
class Class;
class Namespace;
class UserScope; // francis

//. A Visitor for Scopes.
//. The default implementation does nothing, so
//. users only need to implement the ones they need.
class ScopeVisitor
{
public:
  virtual ~ScopeVisitor() {}

  virtual void visit(TemplateParameterScope *) {}
  virtual void visit(LocalScope *) {}
  virtual void visit(PrototypeScope *) {}
  virtual void visit(FunctionScope *) {}
  virtual void visit(Class *) {}
  virtual void visit(Namespace *) {}
  virtual void visit(UserScope *) {} // francis
};

class TemplateParameterScope : public Scope
{
public:
  TemplateParameterScope(PTree::List const *node, Scope const *outer)
    : my_node(node), my_outer(outer->ref()) {}

  virtual SymbolSet
  unqualified_lookup(PTree::Encoding const &, LookupContext) const;

  virtual Scope const *outer_scope() const { return my_outer;}
  virtual void accept(ScopeVisitor *v) { v->visit(this);}

protected:
  ~TemplateParameterScope() { my_outer->unref();}

private:
  PTree::List const *my_node;
  Scope       const *my_outer;
};

class LocalScope : public Scope
{
public:
  LocalScope(PTree::List const *node, Scope const *outer)
    : my_node(node), my_outer(outer->ref()) {}

  virtual Scope const *outer_scope() const { return my_outer;}

  virtual SymbolSet 
  unqualified_lookup(PTree::Encoding const &, LookupContext) const;

  virtual void accept(ScopeVisitor *v) { v->visit(this);}

protected:
  ~LocalScope() { my_outer->unref();}

private:
  PTree::List const *my_node;
  Scope       const *my_outer;
};

class FunctionScope : public Scope
{
public:
  FunctionScope(PTree::Declaration const *, PrototypeScope *, Scope const *);

  virtual void use(PTree::UsingDirective const *);
  virtual Scope const *outer_scope() const { return my_outer;}
  virtual SymbolSet 
  unqualified_lookup(PTree::Encoding const &, LookupContext) const;
  virtual SymbolSet 
  qualified_lookup(PTree::Encoding const &, LookupContext) const;

  // FIXME: what is 'name' ? (template parameters...)
  std::string name() const;

  virtual void accept(ScopeVisitor *v) { v->visit(this);}

protected:
  ~FunctionScope() { my_outer->unref();}

private:
  typedef std::set<Namespace const *> Using;

  PTree::Declaration const *    my_decl;
  Scope const *                 my_outer;
  Class const *                 my_class;
  TemplateParameterScope const *my_parameters;
  Using                         my_using;
};

class PrototypeScope : public Scope
{
  friend class FunctionScope;
public:
  PrototypeScope(PTree::Node const *decl, Scope const *outer,
		 TemplateParameterScope const *params)
    : my_decl(decl), my_outer(outer->ref()), my_parameters(params) {}

  virtual Scope const *outer_scope() const { return my_outer;}
  virtual SymbolSet 
  unqualified_lookup(PTree::Encoding const &, LookupContext) const;

  PTree::Node const *declaration() const { return my_decl;}
  TemplateParameterScope const *parameters() const { return my_parameters;}

  std::string name() const;

  virtual void accept(ScopeVisitor *v) { v->visit(this);}

protected:
  ~PrototypeScope() { my_outer->unref();}

private:
  PTree::Node const *           my_decl;
  Scope const *                 my_outer;
  TemplateParameterScope const *my_parameters;
};

class Class : public Scope
{
public:
  typedef std::vector<Class const *> Bases;

  Class(PTree::ClassSpec const *spec, Scope const *outer,
	Bases const &bases, TemplateParameterScope const *params)
    : my_spec(spec), my_outer(outer->ref()), my_bases(bases), my_parameters(params)
  {
  }

  virtual Scope const *outer_scope() const { return my_outer;}
  virtual SymbolSet 
  unqualified_lookup(PTree::Encoding const &, LookupContext) const;

  // FIXME: what is 'name' ? (template parameters...)
  std::string name() const;

  virtual void accept(ScopeVisitor *v) { v->visit(this);}

protected:
  ~Class() { my_outer->unref();}

private:
  PTree::ClassSpec       const *my_spec;
  Scope                  const *my_outer;
  Bases                         my_bases;
  TemplateParameterScope const *my_parameters;
};

class Namespace : public Scope
{
public:
  Namespace(PTree::NamespaceSpec const *spec, Namespace const *outer)
    : my_spec(spec),
      my_outer(outer ? static_cast<Namespace const *>(outer->ref()) : 0)
  {
  }
  //. Find a nested namespace.
  Namespace *find_namespace(PTree::NamespaceSpec const *name) const;

  virtual void use(PTree::UsingDirective const *);
  virtual Scope const *outer_scope() const { return my_outer;}
  virtual SymbolSet 
  unqualified_lookup(PTree::Encoding const &, LookupContext) const;
  virtual SymbolSet 
  qualified_lookup(PTree::Encoding const &, LookupContext) const;

  // FIXME: should that really be a string ? It may be better to be conform with
  // Class::name, which, if the class is a template, can't be a string (or can it ?)
  std::string name() const;

  virtual void accept(ScopeVisitor *v) { v->visit(this);}

protected:
  ~Namespace() { if (my_outer) my_outer->unref();}

private:
  typedef std::set<Namespace const *> Using;

  SymbolSet 
  unqualified_lookup(PTree::Encoding const &, LookupContext, Using &) const;
  SymbolSet 
  qualified_lookup(PTree::Encoding const &, LookupContext, Using &) const;

  PTree::NamespaceSpec const *my_spec;
  Namespace const *           my_outer;
  Using                       my_using;
};

// francis
class UserScope : public Scope
{
public:
  UserScope(PTree::List const *node, Scope const *outer);

  virtual void accept(ScopeVisitor *v) { v->visit(this);}

  std::string name() const
    {return userName;}
    
  void setUserName(const std::string& userName);
  void setPrototype(SymbolLookup::PrototypeScope* prototype);

  virtual Scope const *outer_scope() const { return my_outer;}

  virtual SymbolSet 
  unqualified_lookup(PTree::Encoding const &, LookupContext) const;

protected:
  ~UserScope() { my_outer->unref();}

private:
  PTree::List const *my_node;
  Scope       const *my_outer;
  std::string userName;
};

}
}

#endif
