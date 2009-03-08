//
// Copyright (C) 2004 Stefan Seefeld
// All rights reserved.
// Licensed to the public under the terms of the GNU LGPL (>= 2),
// see the file COPYING for details.
//
#ifndef Synopsis_SymbolLookup_Scope_hh_
#define Synopsis_SymbolLookup_Scope_hh_

#include <synopsis/SymbolLookup/Symbol.hh>
#include <map>
#include <set>

namespace Synopsis
{
namespace SymbolLookup
{
struct TypeError : std::exception
{
  TypeError(PTree::Encoding const &n, PTree::Encoding const &t)
    : name(n), type(t) {}
  virtual ~TypeError() throw() {}
  virtual char const * what() const throw() { return "TypeError";}
  PTree::Encoding name;
  PTree::Encoding type;
};

struct Undefined : std::exception
{
  Undefined(PTree::Encoding const &n, PTree::Node const *ref = 0)
    : name(n), ptree(ref) {}
  virtual ~Undefined() throw() {}
  virtual char const * what() const throw() { return "Undefined";}
  PTree::Encoding     name;
  PTree::Node const * ptree;
};

struct MultiplyDefined : std::exception
{
  MultiplyDefined(PTree::Encoding const &n,
		  PTree::Node const *decl,
		  PTree::Node const *orig)
    : name(n), declaration(decl), original(orig) {}
  virtual ~MultiplyDefined() throw() {}
  virtual char const * what() const throw() { return "MultiplyDefined";}
  PTree::Encoding name;
  PTree::Node const * declaration;
  PTree::Node const * original;
};

class InternalError : public std::exception
{
public:
  InternalError(std::string const &what) : my_what(what) {}
  virtual ~InternalError() throw() {}
  virtual char const * what() const throw() { return my_what.c_str();}
private:
  std::string my_what;
};

typedef std::set<Symbol const *> SymbolSet;

class ScopeVisitor;

//. A Scope contains symbol definitions.
class Scope
{
protected:
  //. SymbolTable provides a mapping from (encoded) names to Symbols declared
  //. in this scope.
  typedef std::multimap<PTree::Encoding, Symbol const *> SymbolTable;
  //. ScopeTable provides a mapping from scope nodes to Scopes,
  //. which can be used to traverse the scope tree in parallel with
  //. the associated parse tree. As this traversal is also done
  //. during the parsing, the scopes can not be const.
  typedef std::map<PTree::Node const *, Scope *> ScopeTable;

public:
  typedef SymbolTable::const_iterator symbol_iterator;
  typedef ScopeTable::const_iterator scope_iterator;

  typedef unsigned int LookupContext;
  static LookupContext const DEFAULT = 0x0;
  static LookupContext const SCOPE = 0x1; // lookup a scope, see [basic.lookup.qual]
  static LookupContext const USING = 0x2; // lookup in the context of a using directive
  static LookupContext const ELABORATE = 0x4; // elaborate name lookup
  static LookupContext const DECLARATION = 0x8; // see 3.4.3.2/6 [namespace.qual]

  Scope() : my_refcount(1) {}
  Scope *ref() { ++my_refcount; return this;}
  Scope const *ref() const { ++my_refcount; return this;}
  void unref() const { if (!--my_refcount) delete this;}

  virtual Scope const *outer_scope() const = 0;
  Scope const *global_scope() const;

  virtual void accept(ScopeVisitor *v) = 0;

  symbol_iterator symbols_begin() const { return my_symbols.begin();}
  symbol_iterator symbols_end() const { return my_symbols.end();}

  scope_iterator scopes_begin() const { return my_scopes.begin();}
  scope_iterator scopes_end() const { return my_scopes.end();}

  //. declare the given symbol in the local scope 
  //. using the given encoded name.
  void declare(PTree::Encoding const &name, Symbol const *symbol);

  //. declare a nested scope
  void declare_scope(PTree::Node const *, Scope *);

  //. declare a 'using' directive.
  //. The default implementation raises an exception,
  //. as it is only well-formed when the current scope
  //. is a function scope or a namespace.
  virtual void use(PTree::UsingDirective const *);

  //. find a nested scope by declaration
  Scope *find_scope(PTree::Node const *) const;
  //. find a nested scope by symbol.
  //. The encoded name is provided for diagnostic purposes only.
  Scope *find_scope(PTree::Encoding const &, Symbol const *) const;
  //. Remove the given nested scope from the scope.
  void remove_scope(PTree::Node const*);
  //. find a nested scope by name
  //Scope *find_scope(PTree::Encoding const &) const;

  //. find the encoded name declared in this scope and 
  //. return a set of matching symbols.
  SymbolSet find(PTree::Encoding const &, LookupContext) const throw();
  //. Remove the given symbol from the scope.
  //. s shall not be used after its removal.
  void remove(Symbol const *s);

  //. look up the encoded name and return a set of matching symbols.
  SymbolSet lookup(PTree::Encoding const &, LookupContext = DEFAULT) const;

  virtual SymbolSet unqualified_lookup(PTree::Encoding const &,
				       LookupContext = DEFAULT) const = 0;
  virtual SymbolSet qualified_lookup(PTree::Encoding const &,
				     LookupContext = DEFAULT) const;

protected:

  //. Scopes are ref counted, and thus are deleted only by 'unref()'
  virtual ~Scope();

  SymbolTable    my_symbols;
  ScopeTable     my_scopes;
  mutable size_t my_refcount;
};

inline void Scope::declare_scope(PTree::Node const *node, Scope *scope)
{
  my_scopes[node] = scope->ref();
}

inline Scope *Scope::find_scope(PTree::Node const *node) const
{
  ScopeTable::const_iterator i = my_scopes.find(node);
  return i == my_scopes.end() ? 0 : i->second;
}

inline Scope const *Scope::global_scope() const
{
  Scope const *scope = this;
  while (Scope const *outer = scope->outer_scope())
    scope = outer;
  return scope;
}

}
}

#endif
