/*-----------------------------------------.---------------------------------.
| Filename: RationalFunction.h             | Rational Function               |
| Author  : Francis Maes                   |                                 |
| Started : 24/08/2011 19:53               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef INTELUA_ALGEBRA_RATIONAL_FUNCTION_H_
# define INTELUA_ALGEBRA_RATIONAL_FUNCTION_H_

# include "Polynomial.h"

namespace algebra
{

class RationalFunction
{
public:
  RationalFunction(const Polynomial& numerator, const Polynomial& denominator = Polynomial(1.0));
  RationalFunction(const IdentifierPtr& identifier);
  RationalFunction(const LiteralNumberPtr& number);
  RationalFunction() {}

  static RationalFunction fromExpression(const ExpressionPtr& expression);

  static RationalFunction add(const RationalFunction& left, const RationalFunction& right);
  static RationalFunction sub(const RationalFunction& left, const RationalFunction& right);

  static RationalFunction negate(const RationalFunction& fraction);
  static RationalFunction mul(const RationalFunction& left, const RationalFunction& right);
  static RationalFunction div(const RationalFunction& left, const RationalFunction& right);

  static RationalFunction invert(const RationalFunction& fraction);
  static RationalFunction square(const RationalFunction& fraction);

  static RationalFunction pow(const RationalFunction& expr, size_t power);
  static RationalFunction pow(const RationalFunction& expr, int power);

  void simplify();
  void normalizeConstants();

  ExpressionPtr toExpression() const;

protected:
  Polynomial numerator;
  Polynomial denominator;
};

}; /* namespace algebra */

#endif // !INTELUA_ALGEBRA_RATIONAL_FUNCTION_H_
