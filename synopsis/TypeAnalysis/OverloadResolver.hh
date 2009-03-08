//
// Copyright (C) 2005 Stefan Seefeld
// All rights reserved.
// Licensed to the public under the terms of the GNU LGPL (>= 2),
// see the file COPYING for details.
//
#ifndef Synopsis_TypeAnalysis_OverloadResolver_hh_
#define Synopsis_TypeAnalysis_OverloadResolver_hh_

#include <Synopsis/PTree.hh>
#include <Synopsis/SymbolLookup.hh>

namespace Synopsis
{
namespace TypeAnalysis
{

//. Resolve a function call in the context of the given scope.
SymbolLookup::Symbol const *resolve_funcall(PTree::FuncallExpr const *funcall,
					    SymbolLookup::Scope const *);

}
}

#endif

