//
// Copyright (C) 2004 Stefan Seefeld
// All rights reserved.
// Licensed to the public under the terms of the GNU LGPL (>= 2),
// see the file COPYING for details.
//
#include <synopsis/PTree/Display.hh>
#include <synopsis/PTree/Writer.hh>
#include <synopsis/SymbolLookup/Scope.hh>
#include <synopsis/Trace.hh>
#include <functional>

using namespace Synopsis;
using namespace PTree;
using namespace SymbolLookup;

Scope::~Scope()
{
}

void Scope::declare(Encoding const &name, Symbol const *symbol)
{
  Trace trace("Scope::declare", Trace::SYMBOLLOOKUP);
  trace << name;
  my_symbols.insert(std::make_pair(name, symbol));
}

void Scope::use(PTree::UsingDirective const *)
{
  throw InternalError("Invalid use of using directive in this scope.");
}

Scope *Scope::find_scope(PTree::Encoding const &name, Symbol const *symbol) const
{
  PTree::Node const *decl = 0;
  if (NamespaceName const *ns = dynamic_cast<NamespaceName const *>(symbol))
    decl = ns->ptree();
  else if (TypeName const *tn = dynamic_cast<TypeName const *>(symbol))
  {
    decl = tn->ptree();
    // test that 'decl' is a ClassSpec
  }
  // TODO: test for ClassTemplateName ...
  if (!decl)
  {
    // the symbol was found but doesn't refer to a scope
    std::cerr << name << " neither refers to a namespace nor a type" << std::endl;
    throw TypeError(name, symbol->ptree()->encoded_type());
  }
  return find_scope(decl);
}

void Scope::remove_scope(PTree::Node const *decl)
{
  ScopeTable::iterator i = my_scopes.find(decl);
  if (i == my_scopes.end()) throw InternalError("Attempt to remove unknown scope !");
  my_scopes.erase(i);
}

SymbolSet Scope::find(Encoding const &name, LookupContext context) const throw()
{
  Trace trace("Scope::find", Trace::SYMBOLLOOKUP);
  trace << name;
  SymbolTable::const_iterator l = my_symbols.lower_bound(name);
  SymbolTable::const_iterator u = my_symbols.upper_bound(name);
  SymbolSet symbols;
  // [basic.lookup.qual]
  // During the lookup for a name preceding the :: scope resolution operator, 
  // object, function, and enumerator names are ignored.
  if (context & SCOPE)
    for (; l != u; ++l)
    {
      if ((!dynamic_cast<VariableName const *>(l->second)) &&
	  (!dynamic_cast<FunctionName const *>(l->second)))
	symbols.insert(l->second);
    }
  // [basic.lookup.elab]
  else if (context & ELABORATE)
    for (; l != u; ++l)
    {
      if ((dynamic_cast<ClassName const *>(l->second)) ||
	  (dynamic_cast<EnumName const *>(l->second)))
	symbols.insert(l->second);
    }
  // [basic.scope.hiding]
  else
  {
    // There is at most one type-name, which needs to be
    // hidden if any other symbol was found.
    TypeName const *type_name = 0;
    for (; l != u; ++l)
    {
      TypeName const *type = dynamic_cast<TypeName const *>(l->second);
      if (!type) symbols.insert(l->second);
      else type_name = type;
    }
    if (!symbols.size() && type_name) symbols.insert(type_name);
  }
  return symbols;
}

void Scope::remove(Symbol const *symbol)
{
  Trace trace("Scope::remove", Trace::SYMBOLLOOKUP);
  for (SymbolTable::iterator i = my_symbols.begin(); i != my_symbols.end(); ++i)
    if (i->second == symbol)
    {
      my_symbols.erase(i);
      delete symbol;
      return;
    }
  // FIXME: Calling 'remove' with an unknown symbol is an error.
  // Should we throw here ? 
}

SymbolSet 
Scope::lookup(PTree::Encoding const &name, LookupContext context) const
{
  Trace trace("Scope::lookup", Trace::SYMBOLLOOKUP);
  trace << name;
  // If the name is not qualified, start an unqualified lookup.
  if (!name.is_qualified())
    return unqualified_lookup(name, context);

  PTree::Encoding symbol_name = name.get_scope();
  PTree::Encoding remainder = name.get_symbol();
  
  // If the scope is the global scope, do a qualified lookup there.
  if (symbol_name.is_global_scope())
    return global_scope()->qualified_lookup(remainder, context);

  // Else do an unqualified lookup for the scope, followed by a
  // qualified lookup of the remainder in that scope.
  SymbolSet symbols = unqualified_lookup(symbol_name, context | SCOPE);
  if (symbols.empty())
    throw Undefined(symbol_name);
  else if (symbols.size() > 1)
    // If the name was found multiple times, it must refer to a function,
    // so throw a TypeError.
    throw TypeError(symbol_name, (*symbols.begin())->ptree()->encoded_type());

  // As scopes contain a table of nested scopes, accessible through their respective
  // declaration objects, we find the scope using the symbol's declaration as key
  // within its scope.
  Symbol const *symbol = *symbols.begin();
  Scope const *scope = symbol->scope()->find_scope(symbol->ptree());
  if (!scope) 
    throw InternalError("undeclared scope !");

  // Now do a qualified lookup of the remainder in the given scope.
  return scope->qualified_lookup(remainder, context);
}

SymbolSet 
Scope::qualified_lookup(PTree::Encoding const &name,
			LookupContext context) const
{
  Trace trace("Scope::qualified_lookup", Trace::SYMBOLLOOKUP);
  trace << name;
  PTree::Encoding symbol_name = name.get_scope();
  PTree::Encoding remainder = name.get_symbol();

  if (symbol_name.empty())
  {
    symbol_name = name;
    remainder.clear();
  }

  // find symbol locally
  SymbolSet symbols = find(symbol_name, context);
  if (symbols.empty()) return symbols; // nothing found

  // If the remainder is empty, just return the found symbol(s).
  else if (remainder.empty()) return symbols;

  // Having multiple symbols implies they are all overloaded functions.
  // That's a type error if the reminder is non-empty, as we are looking
  // for a scope.
  else if (symbols.size() > 1)
    throw TypeError(symbol_name, (*symbols.begin())->ptree()->encoded_type());

  // Find the scope the symbol refers to 
  // and look up the remainder there.

  // move into inner scope and start over the lookup
  Scope const *nested = find_scope(symbol_name, *symbols.begin());
  if (!nested) throw InternalError("undeclared scope !");

  return nested->qualified_lookup(remainder, context);
}
