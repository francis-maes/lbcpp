/*-----------------------------------------.---------------------------------.
| Filename: RationalFunction.cpp           | Rational Function               |
| Author  : Francis Maes                   |                                 |
| Started : 24/08/2011 20:01               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include "RationalFunction.h"
using namespace algebra;
using namespace lbcpp;
using namespace lbcpp::lua;

RationalFunction::RationalFunction(const Polynomial& numerator, const Polynomial& denominator)
  : numerator(numerator), denominator(denominator) {}

RationalFunction::RationalFunction(const IdentifierPtr& identifier)
  : numerator(identifier), denominator(1.0) {}

RationalFunction::RationalFunction(const LiteralNumberPtr& number)
  : numerator(number), denominator(1.0) {}

bool RationalFunction::isRationalFunctionRoot(const ExpressionPtr& expression)
{
  if (expression.isInstanceOf<Identifier>() || expression.isInstanceOf<LiteralNumber>())
    return true;
  ParenthesisPtr parenthesis = expression.dynamicCast<Parenthesis>();
  if (parenthesis)
    return isRationalFunctionRoot(parenthesis->getExpr());
  UnaryOperationPtr unaryOperation = expression.dynamicCast<UnaryOperation>();
  if (unaryOperation)
    return unaryOperation->getOp() == unmOp;
  BinaryOperationPtr binaryOperation = expression.dynamicCast<BinaryOperation>();
  if (binaryOperation)
  {
    if (binaryOperation->getOp() == addOp ||
        binaryOperation->getOp() == subOp ||
        binaryOperation->getOp() == mulOp ||
        binaryOperation->getOp() == divOp)
      return true;
    if (binaryOperation->getOp() == powOp)
    {
      LiteralNumberPtr rightNumber = binaryOperation->getRight().dynamicCast<LiteralNumber>();
      return rightNumber && rightNumber->getValue() == std::floor(rightNumber->getValue());
    }
  }
  CallPtr call = expression.dynamicCast<Call>();
  if (call)
    return call->getFunction()->print() == T("math.inverse") &&
           call->getNumArguments() == 1;
  return false;
}

RationalFunction RationalFunction::fromExpression(const ExpressionPtr& expression)
{
  IdentifierPtr identifier = expression.dynamicCast<Identifier>();
  if (identifier)
    return RationalFunction(identifier);
  
  LiteralNumberPtr literalNumber = expression.dynamicCast<LiteralNumber>();
  if (literalNumber)
    return RationalFunction(literalNumber);

  ParenthesisPtr parenthesis = expression.dynamicCast<Parenthesis>();
  if (parenthesis)
    return fromExpression(parenthesis->getExpr());
        
  UnaryOperationPtr unaryOperation = expression.dynamicCast<UnaryOperation>();
  if (unaryOperation && unaryOperation->getOp() == unmOp)
    return negate(fromExpression(unaryOperation->getExpr()));
    
  BinaryOperationPtr binaryOperation = expression.dynamicCast<BinaryOperation>();
  if (binaryOperation)
  {
    RationalFunction left = fromExpression(binaryOperation->getLeft());
    RationalFunction right = fromExpression(binaryOperation->getRight());
    
    BinaryOp op = binaryOperation->getOp();
    if (op == addOp)
      return add(left, right);
    else if (op == subOp)
      return sub(left, right);
    else if (op == mulOp)
      return mul(left, right);
    else if (op == divOp)
      return div(left, right);
    else if (op == powOp)
    {
      LiteralNumberPtr power = binaryOperation->getRight().dynamicCast<LiteralNumber>();
      jassert(power && power->getValue() == std::floor(power->getValue()));
      return pow(left, (int)power->getValue());
    }
  }
    
  CallPtr call = expression.dynamicCast<Call>();
  if (call)
  {
    if (call->getFunction()->print() == T("math.inverse") && call->getNumArguments() == 1)
      return invert(fromExpression(call->getArgument(0)));
  }

  jassert(false); // this kind of expressions cannot be converted into a RationalFunction 
  return RationalFunction();
}

ExpressionPtr RationalFunction::canonize(const ExpressionPtr& expression)
{
  RationalFunction fraction = fromExpression(expression);
  fraction.simplify();
  fraction.normalizeConstants();
  return fraction.toExpression();
}

RationalFunction RationalFunction::add(const RationalFunction& left, const RationalFunction& right)
{
  RationalFunction res;
  res.numerator = Polynomial::add(
    Polynomial::mul(left.numerator, right.denominator),
    Polynomial::mul(left.denominator, right.numerator));
  res.denominator = Polynomial::mul(left.denominator, right.denominator);
  return res;
}

RationalFunction RationalFunction::sub(const RationalFunction& left, const RationalFunction& right)
{
  RationalFunction res;
  res.numerator = Polynomial::sub(
    Polynomial::mul(left.numerator, right.denominator),
    Polynomial::mul(left.denominator, right.numerator));
  res.denominator = Polynomial::mul(left.denominator, right.denominator);
  return res;
}

RationalFunction RationalFunction::negate(const RationalFunction& fraction)
  {return RationalFunction(Polynomial::negate(fraction.numerator), fraction.denominator);}

RationalFunction RationalFunction::mul(const RationalFunction& left, const RationalFunction& right)
  {return RationalFunction(Polynomial::mul(left.numerator, right.numerator), Polynomial::mul(left.denominator, right.denominator));}

RationalFunction RationalFunction::div(const RationalFunction& left, const RationalFunction& right)
  {return RationalFunction(Polynomial::mul(left.numerator, right.denominator), Polynomial::mul(left.denominator, right.numerator));}

RationalFunction RationalFunction::invert(const RationalFunction& fraction)
  {return RationalFunction(fraction.denominator, fraction.numerator);}

RationalFunction RationalFunction::square(const RationalFunction& fraction)
  {return RationalFunction(Polynomial::square(fraction.numerator), Polynomial::square(fraction.denominator));}

RationalFunction RationalFunction::pow(const RationalFunction& expr, size_t power)
{
  jassert(power != 0);
  if (power == 1)
    return expr;
  if (power % 2 == 0)
    return square(pow(expr, power / 2));
  else
    return mul(expr, square(pow(expr, power / 2)));
}

RationalFunction RationalFunction::pow(const RationalFunction& expr, int power)
{
  if (power == 0) // todo: check that this polynomial is not zero
    return RationalFunction(Polynomial::one(), Polynomial::one());
  else if (power > 0)
    return pow(expr, (size_t)power);
  else
    return pow(invert(expr), (size_t)(-power));
}

ExpressionPtr RationalFunction::toExpression() const
  {return lua::div(numerator.toExpression(), denominator.toExpression());}

void RationalFunction::simplify()
{
  if (denominator.isZero()) // invalid rational function
    return;
  if (numerator.isOne() || denominator.isOne()) // already maximally simplified
    return;
    
  std::pair<Polynomial, Polynomial> qr = Polynomial::div(numerator, denominator);
  if (qr.second.isZero())
  {
    numerator = qr.first;
    denominator = Polynomial::one();
    return;
  }
  
  qr = Polynomial::div(denominator, numerator);
  if (qr.second.isZero())
  {
    numerator = Polynomial::one();
    denominator = qr.first;
    return;
  }
  
  // todo: factorization
}

void RationalFunction::normalizeConstants()
{
  // ensure that the denominator highest degree monomial has a positive constant
  Monomial monomial;
  double weight = 1.0;
  denominator.getHighestDegreeMonomial(monomial, weight);
  double invZ = (weight >= 0 ? 1.0 : -1.0);
/*
  if (numerator.areConstantsIntegers() && denominator.areConstantsIntegers())
  {
    // todo: find highest common divisor of all constants
  }
  else
  {    
    double l2norm = denominator.getConstantsL2Norm();
    if (!l2norm)
      return; // invalid rational function
    invZ /= l2norm;
  }*/

  numerator.multiplyByScalar(invZ);
  denominator.multiplyByScalar(invZ);      
}
