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
      {
        operands.insert(expr);

        // exp(x) > x
        if (call && call->getFunction()->print() == T("math.exp"))
          operands.erase(call->getArgument(0));

        // x > log(x)
        operands.erase(new Call("math.log", expr));
      }
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
  typedef std::set<ExpressionPtr, ObjectComparator> OperandsSet;
  OperandsSet operands;
  LiteralNumberPtr number;
};

class TemporaryIdentifierMap
{
public:
  IdentifierPtr makeIdentifier(const ExpressionPtr& expression)
  {
    ExpressionToIdentifierMap::const_iterator it = ei.find(expression);
    if (it == ei.end())
    {
      IdentifierPtr identifier = newIdentifier();
      ei[expression] = identifier;
      ie[identifier] = expression;
      return identifier;
    }
    else
      return it->second;
  }

  ExpressionPtr getExpression(const IdentifierPtr& identifier) const
  {
    IdentifierToExpressionMap::const_iterator it = ie.find(identifier);
    return it == ie.end() ? ExpressionPtr() : it->second;
  }

protected:
  typedef std::map<ExpressionPtr, IdentifierPtr, ObjectComparator> ExpressionToIdentifierMap;
  ExpressionToIdentifierMap ei;

  typedef std::map<IdentifierPtr, ExpressionPtr, ObjectComparator> IdentifierToExpressionMap;
  IdentifierToExpressionMap ie;

  IdentifierPtr newIdentifier() const
  {
    static int counter = 1;
    String id = T("__temp") + String(counter++);
    return new Identifier(id);
  }
};

class ReplaceNonRationalsByTemporariesRewriter : public DefaultRewriter
{
public:
  ReplaceNonRationalsByTemporariesRewriter(ExecutionContextPtr context, TemporaryIdentifierMap& temporaries)
    : DefaultRewriter(context), temporaries(temporaries) {}

  virtual void visit(UnaryOperation& operation)
  {
    if (!algebra::RationalFunction::isRationalFunctionRoot(&operation))
      setResult(temporaries.makeIdentifier(&operation));
    else
      acceptChildren(operation);
  }

  virtual void visit(BinaryOperation& operation)
  {
    if (!algebra::RationalFunction::isRationalFunctionRoot(&operation))
      setResult(temporaries.makeIdentifier(&operation));
    else
      acceptChildren(operation);
  }
    
  virtual void visit(Call& call)
  {
    if (!algebra::RationalFunction::isRationalFunctionRoot(&call))
      setResult(temporaries.makeIdentifier(&call));
    else
      acceptChildren(call);
  }

  virtual void visit(Index& index)
    {setResult(temporaries.makeIdentifier(&index));}
  
protected:
  TemporaryIdentifierMap& temporaries;
};

class ReplaceTemporariesByExpressionsRewriter : public DefaultRewriter
{
public:
  ReplaceTemporariesByExpressionsRewriter(ExecutionContextPtr context, const TemporaryIdentifierMap& temporaries)
    : DefaultRewriter(context), temporaries(temporaries) {}
  
  virtual void visit(Identifier& identifier)
  {
    ExpressionPtr expression = temporaries.getExpression(&identifier);
    if (expression)
      setResult(expression);
  }

protected:
  const TemporaryIdentifierMap& temporaries;
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

    BinaryOp op = operation.getOp();
    ExpressionPtr left = operation.getLeft();
    CallPtr rightCall = operation.getRight().dynamicCast<Call>();

    // x / exp(y) ==> x * exp(-y)
    if (op == divOp && rightCall && rightCall->getFunction()->print() == T("math.exp"))
    {
      ExpressionPtr expArgument = rightCall->getArgument(0);
      setResult(rewrite(mul(left, new Call(rightCall->getFunction(), unm(expArgument)))));
      return;
    }

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
      // min(a,b) ==> -max(-a,-b)
      setResult(rewrite(unm(new Call(new Identifier("math.max"), unm(call.getArgument(0)), unm(call.getArgument(1))))));
      return;
    }
    
    if (function == T("math.max"))
    {
      MaxNormalForm normalForm(call);
      setResult(normalForm.toExpression());
      return;
    }
    
    if (function == T("math.log"))
    {
      jassert(call.getNumArguments() == 1);
      CallPtr subCall = call.getArgument(0).dynamicCast<Call>();
      BinaryOperationPtr binaryOperation = call.getArgument(0).dynamicCast<BinaryOperation>();

      // log(exp(x)) ==> x
      if (subCall && subCall->getFunction()->print() == T("math.exp"))
      {
        jassert(subCall->getNumArguments() == 1);
        setResult(subCall->getArgument(0));
        return;
      }

      // log(a/b) ==> log(a) - log(b)
      if (binaryOperation && binaryOperation->getOp() == divOp)
      {
        setResult(sub(new Call(call.getFunction(), binaryOperation->getLeft()),
                      new Call(call.getFunction(), binaryOperation->getRight())));
        return;
      }
    }

    simplifyNumberAlgebra(&call);
  }

protected:
  void simplifyNumberAlgebra(const ExpressionPtr& expression)
  {
    if (algebra::RationalFunction::isRationalFunctionRoot(expression))
    {
      ExpressionPtr expr = expression->cloneAndCast<Expression>();
      TemporaryIdentifierMap temporaries;
      expr = ReplaceNonRationalsByTemporariesRewriter(context, temporaries).rewrite(expr).staticCast<Expression>();
      expr = algebra::RationalFunction::canonize(expr);
      expr = ReplaceTemporariesByExpressionsRewriter(context, temporaries).rewrite(expr);
      setResult(expr);
    }
  }
};

}; /* namespace lua */
}; /* namespace lbcpp */

#endif // !LBCPP_LUA_REWRITER_SIMPLIFY_EXPRESSION_H_

