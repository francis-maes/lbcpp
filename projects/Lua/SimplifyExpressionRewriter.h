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

namespace lbcpp {
namespace lua {

class AlgebraNormalForm
{
public:
  static bool isNumberAlgebra(const ExpressionPtr& expression)
  {
    size_t n = expression->getNumSubNodes();
    for (size_t i = 0; i < n; ++i)
    {
      ExpressionPtr subNode = expression->getSubNode(i).dynamicCast<Expression>();
      if (!subNode || !isNumberAlgebra(subNode))
        return false;
    }
    if (expression.isInstanceOf<Identifier>())
      return true;
    if (expression.isInstanceOf<LiteralNumber>())
      return true;
    UnaryOperationPtr unaryOperation = expression.dynamicCast<UnaryOperation>();
    if (unaryOperation)
      return unaryOperation->getOp() == unmOp;
    BinaryOperationPtr binaryOperation = expression.dynamicCast<BinaryOperation>();
    if (binaryOperation)
      return binaryOperation->getOp() == addOp ||
             binaryOperation->getOp() == subOp ||
             binaryOperation->getOp() == mulOp ||
             binaryOperation->getOp() == divOp;
    return false;
  }

  struct Term
  {
    bool isNegative;
    std::vector<ExpressionPtr> numerator;    // identifiers or literal numbers
    std::vector<ExpressionPtr> denominator;
  };
  std::vector<Term> terms;
};

class SimplifyExpressionRewriter : public DefaultRewriter
{
public:
  SimplifyExpressionRewriter(ExecutionContextPtr context)
    : DefaultRewriter(context) {}

  ExpressionPtr simplify(UnaryOperation& operation)
  {
    UnaryOp op = operation.getOp();
    ExpressionPtr subExpression = rewrite(operation.getExpr()).staticCast<Expression>();
    const double* subNumber = getLiteralNumber(subExpression);

    if (op == unmOp)
    {
      // simplify constant expression
      if (subNumber)
        return new LiteralNumber(-(*subNumber));

      // simplify - - x into x
      if (getUnaryOp(subExpression) == unmOp)
        return subExpression->getSubNode(0);
    }
    
    return new UnaryOperation(op, subExpression);
  }

  ExpressionPtr simplify(BinaryOperation& operation)
  {
   /* if (AlgebraNormalForm::isNumberAlgebra(&operation))
    {
      AlgebraNormalForm normalForm(&operation);
      
    }

    BinaryOp op = operation.getOp();
    ExpressionPtr subExpression1 = rewrite(operation.getLeft()).staticCast<Expression>();
    ExpressionPtr subExpression2 = rewrite(operation.getRight()).staticCast<Expression>();
*/
    return &operation;
#if 0
        local op = AST.getBinaryOperationOp(formula)
    local subFormula1 = simplifyAndMakeFormulaUnique(formula:getSubNode(1))
    local subFormula2 = simplifyAndMakeFormulaUnique(formula:getSubNode(2))
    local subFormulaStr1 = subFormula1:print()
    local subFormulaStr2 = subFormula2:print()

    -- break commutativity
    if AST.isBinaryOperationCommutative(formula) and subFormulaStr1 > subFormulaStr2 then
      local tmp = subFormula1
      subFormula1 = subFormula2
      subFormula2 = tmp
      tmp = subFormulaStr1
      subFormulaStr1 = subFormulaStr2
      subFormulaStr2 = tmp
    end

    local number1 = getLiteralNumber(subFormula1)
    local number2 = getLiteralNumber(subFormula2)

    if op == "add" then
      -- simplify constant expression
      if number1 ~= nil and number2 ~= nil then
        return AST.literalNumber(number1 + number2)
      end
      -- Simplify 0 + x into x
      if number1 == 0 then
        return subFormula2
      end
    elseif op == "sub" then
      -- simplify constant expression
      if number1 ~= nil and number2 ~= nil then
        return AST.literalNumber(number1 - number2)
      end
      -- Simplify x - x into 0
      if subFormulaStr1 == subFormulaStr2 then  
        return AST.literalNumber(0)
      end
      -- Simplify x - 0 into x
      if number2 == 0 then
        return subFormula1
      end
    elseif op == "mul" then
      -- simplify constant expression
      if number1 ~= nil and number2 ~= nil then
        return AST.literalNumber(number1 * number2)
      end
      -- Simplify 1 * x into x
      if number1 == 1 then
        return subFormula2
      end      
    elseif op == "div" then
      -- simplify constant expression
      if number1 ~= nil and number2 ~= nil then
        return AST.literalNumber(number1 / number2)
      end
      -- Simplify x / 1 into x
      if number2 == 1 then
        return subFormula1
      end
      -- Simplify x / x into 1
      if subFormulaStr1 == subFormulaStr2 then  
        return AST.literalNumber(1)
      end
      -- Simplfy (a/b)/(c/d) into (a*d)/(b*c)
      if isDiv(subFormula1) and isDiv(subFormula2) then
        return simplifyAndMakeFormulaUnique(AST.binaryOperation("div",
                   AST.binaryOperation("mul", subFormula1:getSubNode(1), subFormula2:getSubNode(2)),
                   AST.binaryOperation("mul", subFormula1:getSubNode(2), subFormula2:getSubNode(1))))
      end
      -- Simplify (a/b)/c into a / (b*c)
      if isDiv(subFormula1) then
        return simplifyAndMakeFormulaUnique(AST.binaryOperation("div",
                   subFormula1:getSubNode(1),
                   AST.binaryOperation("mul", subFormula1:getSubNode(2), subFormula2)))
      end
      -- Simplify a / (b/c) into (a*c)/b
      if isDiv(subFormula2) then
        return simplifyAndMakeFormulaUnique(AST.binaryOperation("div",
                   AST.binaryOperation("mul", subFormula1, subFormula2:getSubNode(2)),
                   subFormula2:getSubNode(1)))
      end
    end    

    return AST.binaryOperation(op, subFormula1, subFormula2)
#endif
  }

  virtual void visit(UnaryOperation& operation)
    {setResult(simplify(operation));}

  virtual void visit(BinaryOperation& operation)
    {setResult(simplify(operation));}

  virtual void visit(Parenthesis& parenthesis)
    {setResult(rewrite(parenthesis.getExpr()));}

protected:
  static const double* getLiteralNumber(const ExpressionPtr& expression)
  {
    LiteralNumberPtr number = expression.dynamicCast<LiteralNumber>();
    return number ? &number->getValue() : NULL;
  }

  static UnaryOp getUnaryOp(const ExpressionPtr& expression)
  {
    UnaryOperationPtr operation = expression.dynamicCast<UnaryOperation>();
    return operation ? operation->getOp() : (UnaryOp)-1;
  }
};

}; /* namespace lua */
}; /* namespace lbcpp */

#endif // !LBCPP_LUA_REWRITER_SIMPLIFY_EXPRESSION_H_

