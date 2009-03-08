//
// Copyright (C) 2004 Stefan Seefeld
// All rights reserved.
// Licensed to the public under the terms of the GNU LGPL (>= 2),
// see the file COPYING for details.
//
#include <synopsis/SymbolLookup/Scopes.hh>
#include <synopsis/PTree/Writer.hh>
#include <synopsis/PTree/Display.hh>
#include <synopsis/Trace.hh>
#include <sstream>
#include <cassert>

using namespace Synopsis;
using namespace PTree;
using namespace SymbolLookup;

SymbolSet 
TemplateParameterScope::unqualified_lookup(Encoding const &name,
					   LookupContext context) const
{
  Trace trace("TemplateParameterScope::unqualified_lookup", Trace::SYMBOLLOOKUP);
  trace << name;
  SymbolSet symbols = find(name, context == SCOPE);
  return symbols.size() ? symbols : my_outer->unqualified_lookup(name, context);
}

SymbolSet 
LocalScope::unqualified_lookup(Encoding const &name,
			       LookupContext context) const
{
  Trace trace("LocalScope::unqualified_lookup", Trace::SYMBOLLOOKUP);
  trace << name;
  SymbolSet symbols = find(name, context == SCOPE);
  return symbols.size() ? symbols : my_outer->unqualified_lookup(name, context);
}

FunctionScope::FunctionScope(PTree::Declaration const *decl, 
			     PrototypeScope *proto,
			     Scope const *outer)
  : my_decl(decl), my_outer(outer->ref()),
    my_parameters(proto->parameters())
{
  for (SymbolTable::iterator i = proto->my_symbols.begin();
       i != proto->my_symbols.end();
       ++i)
  {
    Symbol const *symbol = i->second;
    my_symbols.insert(std::make_pair(i->first,
				     new VariableName(symbol->type(),
						      symbol->ptree(),
						      true, this)));
  }
  proto->unref();
}

void FunctionScope::use(PTree::UsingDirective const *udecl)
{
  if (*PTree::second(udecl) == "namespace") // using.dir
  {
    PTree::Encoding name = PTree::third(udecl)->encoded_name();
    SymbolSet symbols = lookup(name);
    // now get the namespace associated with that symbol:
    Symbol const *symbol = *symbols.begin();
    // FIXME: may this symbol-to-scope conversion be implemented
    //        by the appropriate Symbol subclass(es) ?
    Scope const *scope = symbol->scope()->find_scope(name, symbol);
    Namespace const *ns = dynamic_cast<Namespace const *>(scope);
    if (ns) my_using.insert(ns);
  }
  else
  {
    std::cout << "sorry, using declaration not supported yet" << std::endl;
  }
}

SymbolSet 
FunctionScope::unqualified_lookup(Encoding const &name,
				  LookupContext context) const
{
  Trace trace("FunctionScope::unqualified_lookup", Trace::SYMBOLLOOKUP);
  trace << name;
  SymbolSet symbols = find(name, context);
  if (my_parameters)
  {
    SymbolSet more = my_parameters->find(name, context);
    symbols.insert(more.begin(), more.end());
  }
  if (symbols.size()) return symbols;

  // see 7.3.4 [namespace.udir]
  for (Using::const_iterator i = my_using.begin(); i != my_using.end(); ++i)
  {
    SymbolSet more = (*i)->unqualified_lookup(name, context | USING);
    symbols.insert(more.begin(), more.end());
  }
  if (symbols.size() || !my_outer)
    return symbols;
  else
    return my_outer->unqualified_lookup(name, context);
}

std::string FunctionScope::name() const
{
  std::ostringstream oss;
  oss << PTree::reify(PTree::third(my_decl));
  return oss.str();
}

SymbolSet 
FunctionScope::qualified_lookup(PTree::Encoding const &name,
				LookupContext context) const
{
  Trace trace("FunctionScope::qualified_lookup", Trace::SYMBOLLOOKUP);
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
  // see 7.3.4 [namespace.udir]
  if (symbols.empty())
    for (Using::const_iterator i = my_using.begin(); i != my_using.end(); ++i)
    {
      SymbolSet more = (*i)->qualified_lookup(name, context);
      symbols.insert(more.begin(), more.end());
    }
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

SymbolSet 
PrototypeScope::unqualified_lookup(PTree::Encoding const &,
				   LookupContext) const
{
  Trace trace("PrototypeScope::unqualified_lookup", Trace::SYMBOLLOOKUP);
  return SymbolSet();
}

std::string PrototypeScope::name() const
{
  std::ostringstream oss;
  oss << PTree::reify(my_decl);
  return oss.str();
}

SymbolSet 
Class::unqualified_lookup(Encoding const &name,
			  LookupContext context) const
{
  Trace trace("Class::unqualified_lookup", Trace::SYMBOLLOOKUP);
  trace << name;
  SymbolSet symbols = find(name, context);
  for (Bases::const_iterator i = my_bases.begin(); i != my_bases.end(); ++i)
  {
    SymbolSet more = (*i)->find(name, context);
    symbols.insert(more.begin(), more.end());
  }
  if (my_parameters)
  {
    SymbolSet more = my_parameters->find(name, context);
    symbols.insert(more.begin(), more.end());
  }
  return symbols.size() ? symbols : my_outer->unqualified_lookup(name, context);
}

std::string Class::name() const
{
  PTree::Node const *name_spec = PTree::second(my_spec);
  // FIXME: why is name_spec for anonumous classes 'list(0, 0)' ?
  //        see Parser::class_spec()...
  if (name_spec && name_spec->is_atom())
    return std::string(name_spec->position(), name_spec->length());
  return "";
}

Namespace *Namespace::find_namespace(PTree::NamespaceSpec const *spec) const
{
  std::string name = "<anonymous>";
  PTree::Node const *ns_name = PTree::second(spec);
  if (ns_name)
    name.assign(ns_name->position(), ns_name->length());
  for (ScopeTable::const_iterator i = my_scopes.begin();
       i != my_scopes.end();
       ++i)
  {
    Namespace *ns = dynamic_cast<Namespace *>(i->second);
    if (ns && name == ns->name())
      return ns;
  }
  return 0;
}

void Namespace::use(PTree::UsingDirective const *udecl)
{
  if (*PTree::second(udecl) == "namespace") // using.dir
  {
    PTree::Encoding name = PTree::third(udecl)->encoded_name();
    SymbolSet symbols = lookup(name);
	if (!symbols.size()) return; // francis: faire le test au cas ou, il faudrait faire un warning 

    // now get the namespace associated with that symbol:
    Symbol const *symbol = *symbols.begin();
    // FIXME: may this symbol-to-scope conversion be implemented
    //        by the appropriate Symbol subclass(es) ?
    Scope const *scope = symbol->scope()->find_scope(name, symbol);
    Namespace const *ns = dynamic_cast<Namespace const *>(scope);
    if (ns) my_using.insert(ns);
  }
  else
  {
    std::cout << "sorry, using declaration not supported yet" << std::endl;
  }
}

SymbolSet 
Namespace::unqualified_lookup(Encoding const &name,
			      LookupContext context) const
{
  Using searched;
  return unqualified_lookup(name, context, searched);
}

SymbolSet 
Namespace::qualified_lookup(PTree::Encoding const &name,
			    LookupContext context) const
{
  Using searched;
  return qualified_lookup(name, context, searched);
}

std::string Namespace::name() const
{
  if (!my_spec) return "<global>";
  PTree::Node const *name_spec = PTree::second(my_spec);
  if (name_spec)
    return std::string(name_spec->position(), name_spec->length());
  else
    return "<anonymous>";
}

SymbolSet Namespace::unqualified_lookup(Encoding const &name,
					LookupContext context,
					Using &searched) const
{
  Trace trace("Namespace::unqualified_lookup", Trace::SYMBOLLOOKUP, this->name());
  trace << name;
  searched.insert(this);
  SymbolSet symbols = find(name, context);
  if (symbols.size()) return symbols;

  // see 7.3.4 [namespace.udir]
  for (Using::const_iterator i = my_using.begin(); i != my_using.end(); ++i)
    if (searched.find(*i) == searched.end())
    {
      SymbolSet more = (*i)->unqualified_lookup(name, context | USING, searched);
      symbols.insert(more.begin(), more.end());
    }
  if (symbols.size() ||
      context & USING ||
      !my_outer ||
      searched.find(my_outer) != searched.end())
    return symbols;
  else
    return my_outer->unqualified_lookup(name, context, searched);
}

SymbolSet Namespace::qualified_lookup(PTree::Encoding const &name,
				      LookupContext context,
				      Using &searched) const
{
  Trace trace("Namespace::qualified_lookup", Trace::SYMBOLLOOKUP, this->name());
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

  // see 3.4.3.2/2 and 3.4.3.2/6
  if (!symbols.size() && (context ^ DECLARATION || name.is_qualified()))
    for (Using::const_iterator i = my_using.begin(); i != my_using.end(); ++i)
      if (searched.find(*i) == searched.end())
      {
	SymbolSet more = (*i)->qualified_lookup(name, context, searched);
	symbols.insert(more.begin(), more.end());
      }
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

/*
** FRANCIS
*/
UserScope::UserScope(PTree::List const *node, Scope const *outer)
  : my_node(node), my_outer(outer->ref()), userName(PTree::reify(node))
{
}

void UserScope::setUserName(const std::string& userName)
  {this->userName = userName;}
  
void UserScope::setPrototype(SymbolLookup::PrototypeScope* prototype)
{
  for (SymbolTable::const_iterator i = prototype->symbols_begin();
       i != prototype->symbols_end();
       ++i)
  {
    Symbol const *symbol = i->second;
    my_symbols.insert(std::make_pair(i->first,
             new VariableName(symbol->type(),
                  symbol->ptree(),
                  true, this)));
  }
}

SymbolSet 
UserScope::unqualified_lookup(Encoding const &name,
			       LookupContext context) const
{
  Trace trace("UserScope::unqualified_lookup", Trace::SYMBOLLOOKUP);
  trace << name;
  SymbolSet symbols = find(name, context == SCOPE);
  return symbols.size() ? symbols : my_outer->unqualified_lookup(name, context);
}
