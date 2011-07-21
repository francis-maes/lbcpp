/*-----------------------------------------.---------------------------------.
| Filename: DerivativeVisitor.h            | Derivative Visitor              |
| Author  : Francis Maes                   |                                 |
| Started : 20/07/2011 16:15               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUA_DERIVATIVE_VISITOR_H_
# define LBCPP_LUA_DERIVATIVE_VISITOR_H_

# include "Visitor.h"

namespace lbcpp {
namespace lua {

// compute the derivate of an expression w.r.t a variable
class ExpressionDerivateRewriter : public Rewriter
{
public:
  ExpressionDerivateRewriter(const IdentifierPtr& variable)
    : variable(variable) {}

  static ExpressionPtr computeWrtVariable(const ExpressionPtr& expr, const IdentifierPtr& variableentifier)
  {
    ExpressionDerivateRewriter visitor(variableentifier);
    return visitor.rewrite(expr).staticCast<Expression>();
  }

  virtual void visit(LiteralNumber& expression)
    {setResult(new LiteralNumber(0.0));}
 
  virtual void visit(Identifier& identifier)
  {
    if (identifier.getIdentifier() == variable->getIdentifier())
      setResult(new LiteralNumber(1.0)); // d(x)/dx = 1
    else
    {
      setResult(new LiteralNumber(0.0)); // bouh, pas bien. C'est ici que ca devient plus compliqué avec les statements ...
    }
  }

  virtual void visit(UnaryOperation& operation)
  {
    switch (operation.getOp())
    {
    case notOp:
    case lenOp:
    case unmOp:
      jassert(false); // not yet implemented
    }
  }

  virtual void visit(BinaryOperation& operation)
  {
    ExpressionPtr u = operation.getLeft();
    ExpressionPtr v = operation.getRight();
    ExpressionPtr uprime = rewrite(u);
    ExpressionPtr vprime = rewrite(v);

    switch (operation.getOp())
    {
    case addOp: setResult(add(uprime, vprime)); return;  // (u + v)' = u' + v'
    case subOp: setResult(sub(uprime, vprime)); return;  // (u - v)' = u' - v'
    case mulOp: setResult(add(multiply(uprime, v), multiply(u, vprime))); return; // (uv)' = u'v + uv'
    case divOp:
    case modOp:
    case powOp:
      {
        LiteralNumberPtr vnumber = v.dynamicCast<LiteralNumber>();
        if (vnumber)
        {
          // u ^ n = (n u^{n-1}) u'
          double n = vnumber->getValue();
          setResult(multiply(multiply(vnumber, pow(u, new LiteralNumber(n - 1.0))), uprime));
          return;
        }
        else
          jassert(false); // not yet implemented
        break;
      }

    case concatOp:
    case eqOp:
    case ltOp:
    case leOp:
    case andOp:
    case orOp:
      jassert(false); // not yet implemented
    }
  }
  
  virtual void visit(Parenthesis& p)
    {setResult(parenthesis(rewrite(p.getExpr())));}

protected:
  IdentifierPtr variable;
};

// compute the derivate of a function w.r.t one of its parameters
class FunctionDerivateRewriter : public DefaultRewriter
{
public:
  FunctionDerivateRewriter(const IdentifierPtr& variable)
    : variable(variable), expressionRewriter(variable) {}

  virtual void visit(Return& statement)
  {
    std::vector<ExpressionPtr> newExpressions(statement.getNumReturnValues());
    for (size_t i = 0; i < newExpressions.size(); ++i)
      newExpressions[i] = rewriteExpression(statement.getReturnValue(i));
    setResult(new Return(newExpressions));
  }

  ExpressionPtr rewriteExpression(const ExpressionPtr& expression)
    {return expressionRewriter.rewrite(expression).staticCast<Expression>();}

protected:
  IdentifierPtr variable;
  ExpressionDerivateRewriter expressionRewriter;
};

// Top-level rewriter : replace all occurences of (in a whole block)
// function (derivable x, y) ... end
//  ==>
/*
  setmetatable({
   f = function (x) return 2 * x end,
   dx = function (x) return 2 end
  }, DerivableFunction)
*/
class DerivableRewriter : public DefaultRewriter
{
public:
  static void applyExtension(BlockPtr& block)
    {DerivableRewriter().rewriteChildren((Node&)*block);}

  FunctionPtr computeFunctionDerivate(const FunctionPtr& function, const IdentifierPtr& variable) const
  {
    FunctionDerivateRewriter derivateFunction(variable);
    BlockPtr newBlock = derivateFunction.rewrite(function->getBlock()->cloneAndCast<Block>());
    return new Function(function->getPrototype(), newBlock);
  }

  ExpressionPtr transformFunction(Function& function, const std::vector<IdentifierPtr>& derivables) const
  {
    TablePtr table = new Table();
    
    table->append("f", &function);
    for (size_t i = 0; i < derivables.size(); ++i)
    {
      ExpressionPtr derivateFunction = computeFunctionDerivate(&function, derivables[i]);
      table->append("d" + derivables[i]->getIdentifier(), derivateFunction);
    }
    return new Call(new Identifier("setmetatable"), table, new Index(new Identifier("LuaChunk"), new Identifier("DerivableFunction")));
  }

  virtual void visit(Function& function)
  {
    DefaultRewriter::visit(function); // apply recursively first

    bool atLeastOneDerivable = false;
    std::vector<IdentifierPtr> derivables;
    size_t n = function.getNumParameters();
    for (size_t i = 0; i < n; ++i)
    {
      IdentifierPtr id = function.getParameterIdentifier(i);
      if (id->hasDerivableFlag())
      {
        derivables.push_back(id);
        function.setParameterIdentifier(i, new Identifier(id->getIdentifier())); // remove "derivable flag"
      }
    }
    if (derivables.size() > 0)
      setResult(transformFunction(function, derivables));
  }
};

}; /* namespace lua */
}; /* namespace lbcpp */

#endif // !LBCPP_LUA_DERIVATIVE_VISITOR_H_

