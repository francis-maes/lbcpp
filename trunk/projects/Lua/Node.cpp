/*-----------------------------------------.---------------------------------.
| Filename: Node.cpp                       | Lua AST                         |
| Author  : Francis Maes                   |                                 |
| Started : 21/07/2011 15:31               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include "Node.h"
#include "Visitor.h"
#include "PrettyPrinterVisitor.h"
#include "SimplifyExpressionRewriter.h"
using namespace lbcpp::lua;
using lbcpp::LuaState;

String Node::print() const
  {return PrettyPrinterVisitor::print(*this);}

int Node::setLineInfo(LuaState& state)
{
  NodePtr node = state.checkObject(1, nodeClass).staticCast<Node>();
  if (!state.isNil(2))
    node->firstLineInfo = state.checkObject(2, lineInfoClass).staticCast<LineInfo>();
  if (!state.isNil(3))
    node->lastLineInfo = state.checkObject(3, lineInfoClass).staticCast<LineInfo>();
  return 0;
}

int Node::getNumSubNodes(LuaState& state)
{
  NodePtr node = state.checkObject(1, nodeClass).staticCast<Node>();
  state.pushNumber(node->getNumSubNodes());
  return 1;
}

int Node::getSubNode(LuaState& state)
{
  NodePtr node = state.checkObject(1, nodeClass).staticCast<Node>();
  int index = state.checkInteger(2);
  if (index <= 0 || index > (int)node->getNumSubNodes())
  {
    state.error("Invalid index " + String((int)index));
    return 0;
  }
  else
  {
    state.pushObject(node->getSubNode(index - 1));
    return 1;
  }
}

int Node::setSubNode(LuaState& state)
{
  NodePtr node = state.checkObject(1, nodeClass).staticCast<Node>();
  int index = state.checkInteger(2);
  if (index <= 0 || index > (int)node->getNumSubNodes())
    state.error("Invalid index " + String((int)index));
  else
    node->getSubNode(index - 1) = state.checkObject(3, nodeClass).staticCast<Node>();
  return 0;
}

int Node::print(LuaState& state)
{
  NodePtr node = state.checkObject(1, nodeClass).staticCast<Node>();
  state.pushString(node->print());
  return 1;
}

int Expression::simplify(LuaState& state)
{
  ExpressionPtr expr = state.checkObject(1, expressionClass).staticCast<Expression>();
  SimplifyExpressionRewriter rewriter(&state.getContext());
  state.pushObject(rewriter.rewrite(expr));
  return 1;
}

#define NODE_ACCEPT_FUNCTION(Class) \
  void Class ::accept(Visitor& visitor) \
    {return visitor.visit(*this);}

NODE_ACCEPT_FUNCTION(List)
NODE_ACCEPT_FUNCTION(Block)

// statements
NODE_ACCEPT_FUNCTION(Do)
NODE_ACCEPT_FUNCTION(Set)
NODE_ACCEPT_FUNCTION(While)
NODE_ACCEPT_FUNCTION(Repeat)
NODE_ACCEPT_FUNCTION(If)
NODE_ACCEPT_FUNCTION(ForNum)
NODE_ACCEPT_FUNCTION(ForIn)
NODE_ACCEPT_FUNCTION(Local)
NODE_ACCEPT_FUNCTION(Return)
NODE_ACCEPT_FUNCTION(Break)
NODE_ACCEPT_FUNCTION(ExpressionStatement)
NODE_ACCEPT_FUNCTION(Parameter)

// expressions
NODE_ACCEPT_FUNCTION(Nil)
NODE_ACCEPT_FUNCTION(Dots)
NODE_ACCEPT_FUNCTION(LiteralBoolean)
NODE_ACCEPT_FUNCTION(LiteralNumber)
NODE_ACCEPT_FUNCTION(LiteralString)
NODE_ACCEPT_FUNCTION(Function)
NODE_ACCEPT_FUNCTION(Pair)
NODE_ACCEPT_FUNCTION(Table)
NODE_ACCEPT_FUNCTION(UnaryOperation)
NODE_ACCEPT_FUNCTION(BinaryOperation)
NODE_ACCEPT_FUNCTION(Parenthesis)
NODE_ACCEPT_FUNCTION(Call)
NODE_ACCEPT_FUNCTION(Invoke)
NODE_ACCEPT_FUNCTION(Identifier)
NODE_ACCEPT_FUNCTION(Index)
NODE_ACCEPT_FUNCTION(Subspecified)

int UnaryOperation::getPrecendenceRank() const
  {return 7;}

int BinaryOperation::getPrecendenceRank() const
{
  if (op == powOp)
    return 8;
  if (op == mulOp || op == divOp || op == modOp)
    return 6;
  if (op == addOp || op == subOp)
    return 5;
  if (op == concatOp)
    return 4;
  if (op == ltOp || op == leOp || op == eqOp)
    return 3;
  if (op == andOp)
    return 2;
  if (op == orOp)
    return 1;
  jassert(false);
  return 0;
}

ExpressionPtr lbcpp::lua::multiply(const ExpressionPtr& left, const ExpressionPtr& right)
{
  LiteralNumberPtr leftNumber = left.dynamicCast<LiteralNumber>();
  LiteralNumberPtr rightNumber = right.dynamicCast<LiteralNumber>();

  if (leftNumber && rightNumber)
    return new LiteralNumber(leftNumber->getValue() * rightNumber->getValue());
  
  if ((leftNumber && !rightNumber) || (!leftNumber && rightNumber))
  {
    double number = (leftNumber ? leftNumber->getValue() : rightNumber->getValue());
    ExpressionPtr expr = (leftNumber ? right : left);
    if (number == 0.0)
      return new LiteralNumber(0.0);
    else if (number == 1.0)
      return expr;
    else if (number == -1.0)
      return new UnaryOperation(unmOp, expr);
  }

  return new BinaryOperation(mulOp, left, right);
}

ExpressionPtr lbcpp::lua::div(const ExpressionPtr& left, const ExpressionPtr& right)
{
  LiteralNumberPtr leftNumber = left.dynamicCast<LiteralNumber>();
  LiteralNumberPtr rightNumber = right.dynamicCast<LiteralNumber>();

  if (leftNumber && rightNumber)
    return new LiteralNumber(leftNumber->getValue() / rightNumber->getValue());
  
  // 0 / x = 0
  // x / 1 = x
  if (leftNumber && leftNumber->getValue() == 0.0 ||
      rightNumber && rightNumber->getValue() == 1.0)
    return left;

  return new BinaryOperation(divOp, left, right);
}

ExpressionPtr lbcpp::lua::add(const ExpressionPtr& left, const ExpressionPtr& right)
{
  LiteralNumberPtr leftNumber = left.dynamicCast<LiteralNumber>();
  LiteralNumberPtr rightNumber = right.dynamicCast<LiteralNumber>();

  if (leftNumber && rightNumber)
    return new LiteralNumber(leftNumber->getValue() + rightNumber->getValue());

  // x + 0 ==> x, 0 + x ==> x
  if ((leftNumber && !rightNumber) || (!leftNumber && rightNumber))
  {
    double number = (leftNumber ? leftNumber->getValue() : rightNumber->getValue());
    ExpressionPtr expr = (leftNumber ? right : left);
    if (number == 0.0)
      return expr;
  }

  // x + (-y) ==> x - y
  UnaryOperationPtr rightUnaryOp = right.dynamicCast<UnaryOperation>();
  if (rightUnaryOp && rightUnaryOp->getOp() == unmOp)
    return sub(left, right);

  return new BinaryOperation(addOp, left, right);
}

ExpressionPtr lbcpp::lua::sub(const ExpressionPtr& left, const ExpressionPtr& right)
{
  LiteralNumberPtr leftNumber = left.dynamicCast<LiteralNumber>();
  LiteralNumberPtr rightNumber = right.dynamicCast<LiteralNumber>();

  if (leftNumber && rightNumber)
    return new LiteralNumber(leftNumber->getValue() - rightNumber->getValue());

  // x - 0 = x
  if (rightNumber && rightNumber->getValue() == 0.0)
    return left;

  // 0 - x = -x
  if (leftNumber && leftNumber->getValue() == 0.0)
    return unm(right);

 // x - (-y) ==> x + y
  UnaryOperationPtr rightUnaryOp = right.dynamicCast<UnaryOperation>();
  if (rightUnaryOp && rightUnaryOp->getOp() == unmOp)
    return add(left, right);

  return new BinaryOperation(subOp, left, right);
}

ExpressionPtr lbcpp::lua::pow(const ExpressionPtr& left, const ExpressionPtr& right)
{
  LiteralNumberPtr leftNumber = left.dynamicCast<LiteralNumber>();
  LiteralNumberPtr rightNumber = right.dynamicCast<LiteralNumber>();

  if (leftNumber && rightNumber)
    return new LiteralNumber(std::pow(leftNumber->getValue(), rightNumber->getValue()));

  if (rightNumber)
  {
    double p = rightNumber->getValue();
    // x ^ 0 = 1
    if (p == 0.0)
      return new LiteralNumber(1.0);

    // x ^ 1 = x
    if (p == 1.0)
      return left;
  }

  // 0 ^ x = 0
  if (leftNumber && leftNumber->getValue() == 0.0)
    return new LiteralNumber(0.0);
    
  return new BinaryOperation(powOp, left, right);
}

ExpressionPtr lbcpp::lua::unm(const ExpressionPtr& expr)
{
  LiteralNumberPtr number = expr.dynamicCast<LiteralNumber>();
  if (number)
    return new LiteralNumber(-number->getValue());

  UnaryOperationPtr unaryOperation = expr.dynamicCast<UnaryOperation>();
  if (unaryOperation && unaryOperation->getOp() == unmOp)
    return unaryOperation->getExpr(); // - (- x) ==> x

  BinaryOperationPtr binaryOperation = expr.dynamicCast<BinaryOperation>();
  if (binaryOperation && binaryOperation->getOp() == subOp)
    return sub(binaryOperation->getRight(), binaryOperation->getLeft()); // -(x - y) ==> (y - x)

  return new UnaryOperation(unmOp, expr);
}

// todo: precompute constants
ExpressionPtr lbcpp::lua::lt(const ExpressionPtr& left, const ExpressionPtr& right)
  {return new BinaryOperation(ltOp, left, right);}

ExpressionPtr lbcpp::lua::le(const ExpressionPtr& left, const ExpressionPtr& right)
  {return new BinaryOperation(leOp, left, right);}

ExpressionPtr lbcpp::lua::notExpr(const ExpressionPtr& expr)
  {return new UnaryOperation(notOp, expr);}