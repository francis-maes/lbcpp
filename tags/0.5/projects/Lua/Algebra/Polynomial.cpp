/*-----------------------------------------.---------------------------------.
| Filename: Polynomial.cpp                 | Polynomial                      |
| Author  : Francis Maes                   |                                 |
| Started : 24/08/2011 19:57               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include "Polynomial.h"
using namespace algebra;
using namespace lbcpp;
using namespace lbcpp::lua;

Polynomial::Polynomial(const IdentifierPtr& identifier)
{
  addMonomial(Monomial(identifier));
}

Polynomial::Polynomial(const LiteralNumberPtr& number)
{
  if (number->getValue())
    addMonomial(Monomial(), number->getValue());
}

Polynomial::Polynomial(double number)
{
  if (number)
    addMonomial(Monomial(), number);
}

Polynomial Polynomial::zero()
  {return Polynomial();}
  
Polynomial Polynomial::one()
  {return Polynomial(1.0);}

Polynomial Polynomial::add(const Polynomial& left, const Polynomial& right)
{
  Polynomial res = left;
  for (const_iterator it = right.monomials.begin(); it != right.monomials.end(); ++it)
    res.addMonomial(it->first, it->second);
  return res;
}

Polynomial Polynomial::sub(const Polynomial& left, const Polynomial& right)
{
  Polynomial res = left;
  for (const_iterator it = right.monomials.begin(); it != right.monomials.end(); ++it)
    res.addMonomial(it->first, -it->second);
  return res;
}

Polynomial Polynomial::negate(const Polynomial& polynomial)
{
  Polynomial res = polynomial;
  for (iterator it = res.monomials.begin(); it != res.monomials.end(); ++it)
  {
    jassert(it->second);
    it->second *= -1;
  }
  return res;
}

Polynomial Polynomial::mul(const Polynomial& left, const Polynomial& right)
{
  Polynomial res;
  for (const_iterator it1 = left.monomials.begin(); it1 != left.monomials.end(); ++it1)
    for (const_iterator it2 = right.monomials.begin(); it2 != right.monomials.end(); ++it2)
    {
      jassert(it1->second);
      jassert(it2->second);
      double weight = it1->second * it2->second;
      jassert(weight);
      res.addMonomial(Monomial::mul(it1->first, it2->first), weight);
    }
  return res;
}

Polynomial Polynomial::square(const Polynomial& polynomial)
  {return mul(polynomial, polynomial);}
  
Polynomial Polynomial::mul(const Polynomial& left, const Monomial& right, double weight)
{
  jassert(weight);
  Polynomial res;
  for (const_iterator it = left.monomials.begin(); it != left.monomials.end(); ++it)
  {
    jassert(it->second);
    res.addMonomial(Monomial::mul(it->first, right), it->second * weight);
  }
  return res;
}

// returns quotient and remainder of a/b
std::pair<Polynomial, Polynomial> Polynomial::div(const Polynomial& a, const Polynomial& b)
{
  Polynomial q;
  Polynomial r = a;

  //std::cout << "Division: A = " << a.toExpression()->print() << " B = " << b.toExpression()->print() << std::endl;
  size_t iteration = 1;
  std::set<String> allValuesOfR;
  while (r.getDegree() >= b.getDegree())
  {
    if (iteration > 1000)
    {
      //std::cout << "Max iterations reached" << std::endl;
      jassert(false);
      break;
    }
    
    //std::cout << "Division iteration " << iteration++ << " Q = " << q.toExpression()->print() << " R = " << r.toExpression()->print() << std::endl;
    
    Monomial mr;
    double mrw;
    if (!r.getHighestDegreeMonomial(mr, mrw))
    {
      //std::cout << "Stopping: no more monomes in remainder" << std::endl;
      break;
    }
    //std::cout << " --> MR: " << mr.toExpression()->print() << std::endl;
    
    Monomial mb;
    double mbw;
    if (!b.getBestDivisorMonomial(mr, mb, mbw))
    {
      //std::cout << "Stopping: could not find a divisor monome" << std::endl;
      break;
    }
    jassert(mbw);
  
    jassert(Monomial::isDivisible(mr, mb));
    //std::cout << " --> MB: " << mb.toExpression()->print() << std::endl;
    
    // mq = mr / mb
    Monomial mq = Monomial::div(mr, mb);
    //std::cout << " --> MQ: " << mq.toExpression()->print() << std::endl;
    double mqw = mrw / mbw;
    q.addMonomial(mq, mqw);
    
    // r <- r - b * mq
    r = Polynomial::sub(r, Polynomial::mul(b, mq, mqw));
    String str = r.toExpression()->print();
    if (allValuesOfR.find(str) == allValuesOfR.end())
      allValuesOfR.insert(str);
    else
    {
      //std::cout << "Stopping: cycle" << std::endl;
      break;
    }
  }
  
  // a = bq + r
#ifdef JUCE_DEBUG
  Polynomial dbg = Polynomial::sub(a, Polynomial::add(Polynomial::mul(b, q), r));
  if (!dbg.isZero())
  {
    std::cerr << "Division was wrong!!! A = " << a.toExpression()->print() << " B = " << b.toExpression()->print() << " Q = " << q.toExpression()->print() << " R = " << r.toExpression()->print() << std::endl;
    jassert(false);
  }
  jassert(dbg.isZero());
#endif    
  return std::make_pair(q, r);
}

ExpressionPtr Polynomial::toExpression() const
{
  std::vector<ExpressionPtr> terms;
  terms.reserve(monomials.size());
  for (const_iterator it = monomials.begin(); it != monomials.end(); ++it)
    terms.push_back(lua::mul(new LiteralNumber(it->second), it->first.toExpression()));
  if (terms.empty())
    return new LiteralNumber(0.0);
  ExpressionPtr res = terms[0];
  for (size_t i = 1; i < terms.size(); ++i)
    res = lua::add(res, terms[i]);
  return res;
}

bool Polynomial::getHighestDegreeMonomial(Monomial& res, double& weight) const
{
  size_t highestDegree = (size_t)-1;
  for (const_iterator it = monomials.begin(); it != monomials.end(); ++it)
  {
    size_t degree = it->first.getDegree();
    if (highestDegree == (size_t)-1 || degree > highestDegree)
    {
      highestDegree = degree;
      res = it->first;
      weight = it->second;
    }
  }
  return highestDegree != (size_t)-1;
}

size_t Polynomial::getDegree() const
{
  size_t highestDegree = 0;
  for (const_iterator it = monomials.begin(); it != monomials.end(); ++it)
  {
    size_t degree = it->first.getDegree();
    if (degree > highestDegree)
      highestDegree = degree;
  }
  return highestDegree;
}

bool Polynomial::isZero() const
  {return monomials.empty();}
  
bool Polynomial::isOne() const
{
  return monomials.size() == 1 &&
      monomials.begin()->first == Monomial() &&
      monomials.begin()->second == 1.0;
}
 
bool Polynomial::areConstantsIntegers() const
{
  for (const_iterator it = monomials.begin(); it != monomials.end(); ++it)
  {
    double weight = it->second;
    if (weight != std::floor(weight))
      return false;
  }
  return true;
}
 
double Polynomial::getConstantsL2Norm() const
{
  double res = 0.0;
  for (const_iterator it = monomials.begin(); it != monomials.end(); ++it)
    res += it->second * it->second;
  return sqrt(res);
}
  
void Polynomial::multiplyByScalar(double scalar)
{
  if (scalar == 0.0)
    monomials.clear();
  else if (scalar != 1.0)
  {
    for (iterator it = monomials.begin(); it != monomials.end(); ++it)
      it->second *= scalar;
  }
}

void Polynomial::addMonomial(const Monomial& monomial, double weight)
{
  double& w = monomials[monomial];
  w += weight;
  if (!w)
    monomials.erase(monomial);
}

bool Polynomial::getBestDivisorMonomial(Monomial dividend, Monomial& res, double& weight) const
{
  size_t bestDegree = (size_t)-1;
  for (const_iterator it = monomials.begin(); it != monomials.end(); ++it)
    if (Monomial::isDivisible(dividend, it->first))
    {
      size_t degree = Monomial::div(dividend, it->first).getDegree();
      if (degree < bestDegree)
      {
        bestDegree = degree;
        res = it->first;
        weight = it->second;
      }
    }
  return bestDegree != (size_t)-1;
}  
