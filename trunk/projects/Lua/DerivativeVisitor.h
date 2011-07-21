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

class ExpressionDerivativeRewriter : public RewriteVisitor
{
public:
  ExpressionDerivativeRewriter(const IdentifierPtr& variableId)
    : variableId(variableId) {}

  static ExpressionPtr computeWrtVariable(const ExpressionPtr& expr, const IdentifierPtr& variableIdentifier)
  {
    ExpressionDerivativeRewriter visitor(variableIdentifier);
    return visitor.rewrite(expr).staticCast<Expression>();
  }

  virtual void visit(LiteralNumber& expression)
    {setResult(new LiteralNumber(0.0));}
 
  virtual void visit(Identifier& identifier)
  {
    if (identifier.getIdentifier() == variableId->getIdentifier())
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
    case addOp: setResult(add(uprime, vprime)); return;
    case subOp: setResult(sub(uprime, vprime)); return;
    case mulOp: setResult(add(multiply(uprime, v), multiply(u, vprime))); return;
    case divOp:
    case modOp:
    case powOp:
    case concatOp:
    case eqOp:
    case ltOp:
    case leOp:
    case andOp:
    case orOp:
      jassert(false); // not yet implemented
    }
  }

protected:
  IdentifierPtr variableId;
};

// function f(x, y) ... end
//  ==>
// f = {
//   __call(x, y) ... end
//   dx(x, y) ... end
//   dy(x, y) ... end
// }
class DerivableFunctionDeclarationRewriter : public RewriteVisitor
{
public:
  virtual void visit(Set& statement)
  {
    if (statement.getLhs()->getNumSubNodes() != 1 &&
        statement.getExpr()->getNumSubNodes() != 1)
    {
      setResult(&statement);
      return;
    }

    FunctionPtr function = statement.getExpr()->getSubNode(0).dynamicCast<Function>();
    if (!function)
    {
      setResult(&statement);
      return;
    }
    
    // .. to be continued
  }
};


}; /* namespace lua */
}; /* namespace lbcpp */

#endif // !LBCPP_LUA_DERIVATIVE_VISITOR_H_

