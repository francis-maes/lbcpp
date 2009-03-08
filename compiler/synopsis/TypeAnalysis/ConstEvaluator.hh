//
// Copyright (C) 2004 Stefan Seefeld
// All rights reserved.
// Licensed to the public under the terms of the GNU LGPL (>= 2),
// see the file COPYING for details.
//
#ifndef Synopsis_TypeAnalysis_ConstEvaluator_hh_
#define Synopsis_TypeAnalysis_ConstEvaluator_hh_

#include <synopsis/PTree/Visitor.hh>
#include <synopsis/PTree/Atoms.hh>
#include <synopsis/PTree/Lists.hh>
#include <synopsis/SymbolLookup/Scope.hh>
#include <cassert>

namespace Synopsis
{
namespace TypeAnalysis
{

//. Evaluate the value of a constant expression.
class ConstEvaluator : private PTree::Visitor
{
public:
  ConstEvaluator(SymbolLookup::Scope const *s) : my_valid(false), my_scope(s) {}
  bool evaluate(PTree::Node const *node, long &value);

private:
  virtual void visit(PTree::Literal *);
  virtual void visit(PTree::Identifier *);
  virtual void visit(PTree::FstyleCastExpr *);
  virtual void visit(PTree::InfixExpr *);
  virtual void visit(PTree::SizeofExpr *);
  virtual void visit(PTree::UnaryExpr *);
  virtual void visit(PTree::CondExpr *);
  virtual void visit(PTree::ParenExpr *);
  
  bool                       my_valid;
  long                       my_value;
  SymbolLookup::Scope const *my_scope;
};

//. Evaluate the value of a constant expression.
//. TODO: This may also return the type of the expression...
inline bool evaluate_const(SymbolLookup::Scope const *scope,
			   PTree::Node const *node, long &value)
{
  if (!node) return false;
  ConstEvaluator e(scope);
  return e.evaluate(node, value);
}

}
}

#endif

