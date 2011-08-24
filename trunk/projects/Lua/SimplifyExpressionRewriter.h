/*-----------------------------------------.---------------------------------.
| Filename: SimplifyExpressionRewriter.h   | Simplify Expression Rewriter    |
| Author  : Francis Maes                   |                                 |
| Started : 23/08/2011 12:02               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUA_REWRITER_SIMPLIFY_EXPRESSION_H_
# define LBCPP_LUA_REWRITER_SIMPLIFY_EXPRESSION_H_

# include "Rewriter.h"
# include "Algebra/RationalFunction.h"

namespace lbcpp {
namespace lua {

class MaxNormalForm
{
public:
  MaxNormalForm(Call& call)
    {build(&call);}
  
  void build(ExpressionPtr expr)
  {
    CallPtr call = expr.dynamicCast<Call>();
    if (call && call->getFunction()->print() == T("math.max"))
    {
      jassert(call->getNumArguments() == 2);
      build(call->getArgument(0));
      build(call->getArgument(1));      
    }
    else
    {
      LiteralNumberPtr number = expr.dynamicCast<LiteralNumber>();
      if (number)
      {
        if (this->number)
          this->number = new LiteralNumber(juce::jmax(number->getValue(), this->number->getValue()));
        else
          this->number = number;
      }
      else
        operands.insert(expr);
    }
  }
  
  ExpressionPtr toExpression() const
  {
    std::vector<ExpressionPtr> expressions;
    expressions.reserve(operands.size() + 1);
    if (number)
      expressions.push_back(number);
    for (OperandsSet::const_iterator it = operands.begin(); it != operands.end(); ++it)
      expressions.push_back(*it);
    jassert(expressions.size());
    ExpressionPtr res = expressions[0];
    IdentifierPtr id = new Identifier("math.max");
    for (size_t i = 1; i < expressions.size(); ++i)
      res = new Call(id, res, expressions[i]);
    return res;
  }

protected:
  struct NodeComparisonOperator
  {
    bool operator ()(NodePtr left, NodePtr right)
      {return left->print() < right->print();}
  };

  typedef std::set<ExpressionPtr, NodeComparisonOperator> OperandsSet;
  OperandsSet operands;
  LiteralNumberPtr number;
};

class CanonizeExpressionRewriter : public DefaultRewriter
{
public:
  CanonizeExpressionRewriter(ExecutionContextPtr context)
    : DefaultRewriter(context) {}
 
  virtual void visit(Parenthesis& parenthesis)
    {setResult(rewrite(parenthesis.getExpr()));}

  virtual void visit(UnaryOperation& operation)
  {
    accept(operation.getSubNode(0));
    simplifyNumberAlgebra(&operation);
  }

  virtual void visit(BinaryOperation& operation)
  {
    accept(operation.getSubNode(0));
    accept(operation.getSubNode(1));
    simplifyNumberAlgebra(&operation);
  }
    
  virtual void visit(Call& call)
  {
    for (size_t i = 0; i < call.getNumSubNodes(); ++i)
      accept(call.getSubNode(i));
    
    String function = call.getFunction()->print();
    if (function == T("math.min"))
    {
      jassert(call.getNumArguments() == 2);
      // min(a,b) = -max(-a,-b)
      setResult(rewrite(unm(new Call(new Identifier("math.max"), unm(call.getArgument(0)), unm(call.getArgument(1))))));
      return;
    }
    
    if (function == T("math.max"))
    {
      MaxNormalForm normalForm(call);
      setResult(normalForm.toExpression());
      return;
    }
    
    simplifyNumberAlgebra(&call);
  }

protected:
  void simplifyNumberAlgebra(const ExpressionPtr& expression)
  {
    if (isNumberAlgebra(expression))
    {
      algebra::RationalFunction fraction = algebra::RationalFunction::fromExpression(expression);
      fraction.simplify();
      fraction.normalizeConstants();
      setResult(fraction.toExpression());
    }
  }
  
  // returns true if the expression only contains literal numbers, identifiers, unm, add, sub, mul and div
  static bool isNumberAlgebra(const ExpressionPtr& expression)
  {
    if (expression.isInstanceOf<Identifier>())
      return true;
    if (expression.isInstanceOf<LiteralNumber>())
      return true;
    ParenthesisPtr parenthesis = expression.dynamicCast<Parenthesis>();
    if (parenthesis)
      return isNumberAlgebra(parenthesis->getExpr());
    UnaryOperationPtr unaryOperation = expression.dynamicCast<UnaryOperation>();
    if (unaryOperation)
      return unaryOperation->getOp() == unmOp && isNumberAlgebra(unaryOperation->getExpr());
    BinaryOperationPtr binaryOperation = expression.dynamicCast<BinaryOperation>();
    if (binaryOperation)
    {
      if (binaryOperation->getOp() == addOp ||
          binaryOperation->getOp() == subOp ||
          binaryOperation->getOp() == mulOp ||
          binaryOperation->getOp() == divOp)
        return isNumberAlgebra(binaryOperation->getLeft()) && isNumberAlgebra(binaryOperation->getRight());
      if (binaryOperation->getOp() == powOp)
        return isNumberAlgebra(binaryOperation->getLeft()) && isInteger(binaryOperation->getRight());
    }
    CallPtr call = expression.dynamicCast<Call>();
    if (call)
      return call->getFunction()->print() == T("math.inverse") &&
             call->getNumArguments() == 1 &&
             isNumberAlgebra(call->getArgument(0));
    return false;
  }
  
  static bool isInteger(ExpressionPtr expression)
  {
    LiteralNumberPtr number = expression.dynamicCast<LiteralNumber>();
    return number && number->getValue() == std::floor(number->getValue());
  }
};

}; /* namespace lua */
}; /* namespace lbcpp */

#endif // !LBCPP_LUA_REWRITER_SIMPLIFY_EXPRESSION_H_

