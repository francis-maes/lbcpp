//
// Copyright (C) 2005 Stefan Seefeld
// All rights reserved.
// Licensed to the public under the terms of the GNU LGPL (>= 2),
// see the file COPYING for details.
//

#include <Synopsis/PTree/Visitor.hh>
#include <Synopsis/TypeAnalysis/TypeEvaluator.hh>
#include <Synopsis/Trace.hh>
#include "OverloadResolver.hh"
#include <vector>
#include <typeinfo>

namespace Synopsis
{

using namespace SymbolLookup;

namespace TypeAnalysis
{

// SymbolSet match_arguments(SymbolSet const &functions, std::vector<Type> const &types)
// {
//   Trace trace("match_arguments");
//   std::cout << "available functions:" << std::endl;
//   for (SymbolSet::iterator i = functions.begin(); i != functions.end(); ++i)
//     std::cout << (*i)->type() << std::endl;
//   std::cout << "provided argument types:" << std::endl;
//   for (std::vector<Type>::const_iterator i = types.begin(); i != types.end(); ++i)
//     std::cout << *i << std::endl;
//   SymbolSet viable;
//   // FIXME: for now ignore ellipses and default arguments
//   for (SymbolSet::iterator i = functions.begin(); i != functions.end(); ++i)
//   {
//     Type function((*i)->type(), (*i)->scope());
//     if (function.num_of_arguments() == types.size())
//       viable.insert(*i);
//   }
//   std::cout << "viable functions" << std::endl;
//   for (SymbolSet::iterator i = viable.begin(); i != viable.end(); ++i)
//     std::cout << (*i)->type() << std::endl;
//   SymbolSet matches;
//   return matches;
// }

// Symbol const *resolve_funcall(PTree::FuncallExpr const *funcall, Scope const *scope)
// {
//   Trace trace("resolve_funcall");
//   // overload resolution is done in multiple steps:
//   //
//   // o determine a set of symbols that match the name of the function being called,
//   //   including objects
//   // o determine the types associated with the parameter expressions
//   // o determine the set of viable functions (13.3.2 [over.match.viable])
//   // o rank the set of viable functions and select the best viable function
//   //   (13.3.3 [over.match.best])
//   PTree::Node const *function = PTree::first(funcall);
//   SymbolSet functions;
//   PTree::Encoding function_name;

//   // if this is an identifier or a name, we can look it up in the symbol table
//   // (possibly getting a set of overloaded functions)
//   if (function->is_atom()) // identifier
//   {
//     function_name = PTree::Encoding(function->position(), function->length());
//     functions = scope->lookup(function_name);
//   }
//   else if (PTree::type_of(function) == Token::ntName)
//   {
//     function_name = function->encoded_name();
//     functions = scope->lookup(function_name);
//   }
//   // else if it is a primary expression, we use type_of to determine the function
//   // signature. Here we can't get more than one matches (see [13.4])
//   else
//   {
//     std::cout << "<expression in overload resolution: not yet implemented>" << std::endl;
//     return 0;
//   }
//   // now find number and types of the arguments.
//   PTree::Node const *arguments = PTree::second(PTree::rest(funcall));
//   std::vector<Type> types;
//   for (size_t i = 0; i < PTree::length(arguments); ++i)
//   {
//     types.push_back(type_of(PTree::nth(arguments, i), scope));
//     ++i; // skip comma
//   }
//   SymbolSet matches = match_arguments(functions, types);
//   if (matches.size() == 1)
//     return *matches.begin();

//   return 0;
// }

}
}
