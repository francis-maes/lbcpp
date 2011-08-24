/*-----------------------------------------.---------------------------------.
| Filename: EvaluateConstantsRewriter.h    | Evaluate Constants Rewriter     |
| Author  : Francis Maes                   |                                 |
| Started : 23/08/2011 12:02               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUA_EVALUATE_CONSTANTS_REWRITER_H_
# define LBCPP_LUA_EVALUATE_CONSTANTS_REWRITER_H_

# include "Rewriter.h"

namespace lbcpp {
namespace lua {

class EvaluateConstantsRewriter : public DefaultRewriter
{
public:
  EvaluateConstantsRewriter(ExecutionContextPtr context)
    : DefaultRewriter(context) {}

  virtual void visit(UnaryOperation& operation)
  {
    UnaryOp op = operation.getOp();
    accept(operation.getSubNode(0));
    const double* subNumber = getLiteralNumber(operation.getSubNode(0));

    if (op == unmOp && subNumber)
      setResult(new LiteralNumber(-(*subNumber)));
  }

  virtual void visit(BinaryOperation& operation)
  {
    BinaryOp op = operation.getOp();
    accept(operation.getSubNode(0));
    accept(operation.getSubNode(1));
    
    ExpressionPtr left = operation.getLeft();
    ExpressionPtr right = operation.getRight();
    const double* number1 = getLiteralNumber(left);
    const double* number2 = getLiteralNumber(right);

    switch (op)
    {
    case addOp:
      if (number1 && number2)
        setResult(new LiteralNumber(*number1 + *number2));
      else if (number1 && *number1 == 0)
        setResult(right); // 0 + x ==> x
      else if (number2 && *number2 == 0)
        setResult(left); // x + 0 ==> x
      break;
      
    case subOp:
      if (number1 && number2)
        setResult(new LiteralNumber(*number1 - *number2));
      else if (number1 && *number1 == 0)
        setResult(new UnaryOperation(unmOp, right));  // 0 - x ==> -x
      else if (number2 && *number2 == 0)
        setResult(left); // x - 0 ==> x
      break;
    
    case mulOp:
      if (number1 && number2)
        setResult(new LiteralNumber(*number1 * *number2));
      else if (number1 && *number1 == 1)
        setResult(right); // 1 * x ==> x
      else if (number2 && *number2 == 1)
        setResult(left);  // x * 1 ==> x
      break;
      
    case divOp:
      if (number1 && number2)
        setResult(new LiteralNumber(*number1 / *number2));
      else if (number2 && *number2 == 1)
        setResult(left); // x / 1 ==> x
      else if (number2 && *number2 == 0)
        setResult(new LiteralNumber(0.0 / *number2)); // x / 0 => nan
      else if (number1 && *number1 == 0)
        setResult(new LiteralNumber(0.0)); // 0 / x => 0
      break;
      
    default:
      break;
    }
  }

  virtual void visit(Parenthesis& parenthesis)
    {setResult(rewrite(parenthesis.getExpr()));}

  virtual void visit(Call& call)
  {
    acceptChildren(call);

    String fun = call.getFunction()->print();
    if (call.getNumArguments() == 1)
    {
      const double* number = getLiteralNumber(call.getArgument(0));
      if (number)
      {
        if (fun == T("math.exp"))
          setResult(new LiteralNumber(exp(*number)));
        else if (fun == T("math.log"))
          setResult(new LiteralNumber(log(*number)));
      }
    }
    else if (call.getNumArguments() == 2)
    {
      const double* number1 = getLiteralNumber(call.getArgument(0));
      const double* number2 = getLiteralNumber(call.getArgument(1));
      if (number1 && number2)
      {
        if (fun == T("math.max"))
          setResult(new LiteralNumber(juce::jmax(*number1, *number2)));
        else if (fun == T("math.min"))
          setResult(new LiteralNumber(juce::jmin(*number1, *number2)));
      }
    }
  }

protected:
  static const double* getLiteralNumber(const NodePtr& node)
  {
    LiteralNumberPtr number = node.dynamicCast<LiteralNumber>();
    return number ? &number->getValue() : NULL;
  }
};

}; /* namespace lua */
}; /* namespace lbcpp */

#endif // !LBCPP_LUA_EVALUATE_CONSTANTS_REWRITER_H_

