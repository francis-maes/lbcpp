/*-----------------------------------------.---------------------------------.
| Filename: Monomial.h                     | Monomial                        |
| Author  : Francis Maes                   |                                 |
| Started : 24/08/2011 19:55               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef INTELUA_ALGEBRA_MONOMIAL_H_
# define INTELUA_ALGEBRA_MONOMIAL_H_

# include "../Node.h"
# include "../Expression.h"
using namespace lbcpp::lua;

namespace algebra
{

class Monomial
{
public:
  Monomial(const IdentifierPtr& identifier);
  Monomial() {}

  bool operator ==(const Monomial& other) const
    {return m == other.m;}
  bool operator !=(const Monomial& other) const
    {return m != other.m;}
  bool operator <(const Monomial& other) const
    {return m < other.m;}
  
  // x^i y^j * x^k y^l = x^(i+k) y^(j+l)
  static Monomial mul(const Monomial& left, const Monomial& right);
  static bool isDivisible(const Monomial& left, const Monomial& right);
  static Monomial div(const Monomial& left, const Monomial& right);
  
  size_t getDegree() const;
  size_t getDegree(const String& identifier) const;
   
  ExpressionPtr toExpression() const;

protected:
  typedef std::map<String, size_t> Map;
  typedef Map::const_iterator const_iterator;
  Map m;
};

}; /* namespace algebra */

#endif // !INTELUA_ALGEBRA_MONOMIAL_H_
