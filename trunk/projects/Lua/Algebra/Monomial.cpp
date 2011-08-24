/*-----------------------------------------.---------------------------------.
| Filename: Monomial.cpp                   | Monomial                        |
| Author  : Francis Maes                   |                                 |
| Started : 24/08/2011 19:56               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include "Monomial.h"
using namespace algebra;
using namespace lbcpp;
using namespace lbcpp::lua;

Monomial::Monomial(const IdentifierPtr& identifier)
{
  m[identifier->getIdentifier()] = 1;
}
  // x^i y^j * x^k y^l = x^(i+k) y^(j+l)
Monomial Monomial::mul(const Monomial& left, const Monomial& right)
{
  Monomial res;
  res.m = left.m;
  for (const_iterator it = right.m.begin(); it != right.m.end(); ++it)
    res.m[it->first] += it->second;
  return res;
}

bool Monomial::isDivisible(const Monomial& left, const Monomial& right)
{
  for (const_iterator it = right.m.begin(); it != right.m.end(); ++it)
    if (it->second > left.getDegree(it->first))
      return false;
  return true;
}

Monomial Monomial::div(const Monomial& left, const Monomial& right)
{
  Monomial res;
  res.m = left.m;
  for (const_iterator it = right.m.begin(); it != right.m.end(); ++it)
  {
    jassert(res.m[it->first] >= it->second);
    size_t degree = res.m[it->first] - it->second;
    if (degree > 0)
      res.m[it->first] = degree;
    else
      res.m.erase(it->first);
  }
  return res;
}

ExpressionPtr Monomial::toExpression() const
{
  std::vector<ExpressionPtr> factors;
  factors.reserve(m.size());
  for (const_iterator it = m.begin(); it != m.end(); ++it)
    factors.push_back(pow(new Identifier(it->first), new LiteralNumber((double)it->second)));
  if (factors.empty())
    return new LiteralNumber(1.0);
  ExpressionPtr res = factors[0];
  for (size_t i = 1; i < factors.size(); ++i)
    res = lua::mul(res, factors[i]);
  return res;
}

size_t Monomial::getDegree() const
{
  size_t res = 0;
  for (const_iterator it = m.begin(); it != m.end(); ++it)
    if (it->second > res)
      res = it->second;
  return res;
}

size_t Monomial::getDegree(const String& identifier) const
{
  const_iterator it = m.find(identifier);
  return it == m.end() ? 0 : it->second;
}
