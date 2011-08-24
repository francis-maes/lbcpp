/*-----------------------------------------.---------------------------------.
| Filename: Polynomial.h                   | Polynomial                      |
| Author  : Francis Maes                   |                                 |
| Started : 24/08/2011 19:54               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef INTELUA_ALGEBRA_POLYNOMIAL_H_
# define INTELUA_ALGEBRA_POLYNOMIAL_H_

# include "Monomial.h"

namespace algebra
{

class Polynomial
{
public:
  Polynomial(const IdentifierPtr& identifier);
  Polynomial(const LiteralNumberPtr& number);
  Polynomial(double number);
  Polynomial() {}

  static Polynomial zero();
  static Polynomial one();

  static Polynomial add(const Polynomial& left, const Polynomial& right);
  static Polynomial sub(const Polynomial& left, const Polynomial& right);
  static Polynomial mul(const Polynomial& left, const Polynomial& right);
  static Polynomial mul(const Polynomial& left, const Monomial& right, double weight);

  static Polynomial square(const Polynomial& polynomial);
  static Polynomial negate(const Polynomial& polynomial);
  
  // returns quotient and remainder of a/b
  static std::pair<Polynomial, Polynomial> div(const Polynomial& a, const Polynomial& b);

  ExpressionPtr toExpression() const;
  bool getHighestDegreeMonomial(Monomial& res, double& weight) const;
  bool isZero() const;
  bool isOne() const;

  bool areConstantsIntegers() const;
  double getConstantsL2Norm() const;
  void multiplyByScalar(double scalar);
   
protected:
  typedef std::map<Monomial, double> MonomialMap;
  typedef MonomialMap::iterator iterator;
  typedef MonomialMap::const_iterator const_iterator;

  MonomialMap monomials;    // a polynomial is a weighted sum of monomials

  void addMonomial(const Monomial& monomial, double weight = 1.0);
  bool getBestDivisorMonomial(Monomial dividend, Monomial& res, double& weight) const;
};

}; /* namespace algebra */

#endif // !INTELUA_ALGEBRA_POLYNOMIAL_H_
