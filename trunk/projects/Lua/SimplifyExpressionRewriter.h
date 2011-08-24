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

class Monomial
{
public:
  Monomial(const IdentifierPtr& identifier)
    {m[identifier->getIdentifier()] = 1;}
  Monomial() {}

  bool operator ==(const Monomial& other) const
    {return m == other.m;}
  bool operator !=(const Monomial& other) const
    {return m != other.m;}
  bool operator <(const Monomial& other) const
    {return m < other.m;}
  
  // x^i y^j * x^k y^l = x^(i+k) y^(j+l)
  static Monomial mul(const Monomial& left, const Monomial& right)
  {
    Monomial res;
    res.m = left.m;
    for (const_iterator it = right.m.begin(); it != right.m.end(); ++it)
      res.m[it->first] += it->second;
    return res;
  }
  
  static bool isDivisible(const Monomial& left, const Monomial& right)
  {
    for (const_iterator it = right.m.begin(); it != right.m.end(); ++it)
      if (it->second > left.getDegree(it->first))
        return false;
    return true;
  }
  
  static Monomial div(const Monomial& left, const Monomial& right)
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
  
  ExpressionPtr toExpression() const
  {
    std::vector<ExpressionPtr> factors;
    factors.reserve(m.size());
    for (const_iterator it = m.begin(); it != m.end(); ++it)
      factors.push_back(pow(new Identifier(it->first), new LiteralNumber((double)it->second)));
    if (factors.empty())
      return new LiteralNumber(1.0);
    ExpressionPtr res = factors[0];
    for (size_t i = 1; i < factors.size(); ++i)
      res = multiply(res, factors[i]);
    return res;
  }
  
  size_t getDegree() const
  {
    size_t res = 0;
    for (const_iterator it = m.begin(); it != m.end(); ++it)
      if (it->second > res)
        res = it->second;
    return res;
  }
  
  size_t getDegree(const String& identifier) const
  {
    const_iterator it = m.find(identifier);
    return it == m.end() ? 0 : it->second;
  }
  
protected:
  typedef std::map<String, size_t> Map;
  typedef Map::const_iterator const_iterator;
  Map m;
};

class Polynomial
{
public:
  Polynomial(const IdentifierPtr& identifier)
    {addMonomial(Monomial(identifier));}
  Polynomial(const LiteralNumberPtr& number)
    {if (number->getValue()) addMonomial(Monomial(), number->getValue());}
  Polynomial(double number)
    {if (number) addMonomial(Monomial(), number);}
  Polynomial() {}

  static Polynomial zero()
    {return Polynomial();}
    
  static Polynomial one()
    {return Polynomial(1.0);}

  static Polynomial add(const Polynomial& left, const Polynomial& right)
  {
    Polynomial res = left;
    for (const_iterator it = right.monomials.begin(); it != right.monomials.end(); ++it)
      res.addMonomial(it->first, it->second);
    return res;
  }
  
  static Polynomial sub(const Polynomial& left, const Polynomial& right)
  {
    Polynomial res = left;
    for (const_iterator it = right.monomials.begin(); it != right.monomials.end(); ++it)
      res.addMonomial(it->first, -it->second);
    return res;
  }
  
  static Polynomial negate(const Polynomial& polynomial)
  {
    Polynomial res = polynomial;
    for (iterator it = res.monomials.begin(); it != res.monomials.end(); ++it)
    {
      jassert(it->second);
      it->second *= -1;
    }
    return res;
  }

  static Polynomial mul(const Polynomial& left, const Polynomial& right)
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

  static Polynomial square(const Polynomial& polynomial)
    {return mul(polynomial, polynomial);}
    
  static Polynomial mul(const Polynomial& left, const Monomial& right, double weight)
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
  static std::pair<Polynomial, Polynomial> div(const Polynomial& a, const Polynomial& b)
  {
    Polynomial q;
    Polynomial r = a;
   
    //std::cout << "Division: A = " << a.toExpression()->print() << " B = " << b.toExpression()->print() << std::endl;
    size_t iteration = 1;
    std::set<String> allValuesOfR;
    while (true)
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

  ExpressionPtr toExpression() const
  {
    std::vector<ExpressionPtr> terms;
    terms.reserve(monomials.size());
    for (const_iterator it = monomials.begin(); it != monomials.end(); ++it)
      terms.push_back(multiply(new LiteralNumber(it->second), it->first.toExpression()));
    if (terms.empty())
      return new LiteralNumber(0.0);
    ExpressionPtr res = terms[0];
    for (size_t i = 1; i < terms.size(); ++i)
      res = lua::add(res, terms[i]);
    return res;
  }
  
  bool getHighestDegreeMonomial(Monomial& res, double& weight) const
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
  
  bool isZero() const
    {return monomials.empty();}
    
  bool isOne() const
  {
    return monomials.size() == 1 &&
        monomials.begin()->first == Monomial() &&
        monomials.begin()->second == 1.0;
  }
   
  bool areConstantsIntegers() const
  {
    for (const_iterator it = monomials.begin(); it != monomials.end(); ++it)
    {
      double weight = it->second;
      if (weight != std::floor(weight))
        return false;
    }
    return true;
  }
   
  double getConstantsL2Norm() const
  {
    double res = 0.0;
    for (const_iterator it = monomials.begin(); it != monomials.end(); ++it)
      res += it->second * it->second;
    return sqrt(res);
  }
    
  void multiplyByScalar(double scalar)
  {
    if (scalar == 0.0)
      monomials.clear();
    else if (scalar != 1.0)
    {
      for (iterator it = monomials.begin(); it != monomials.end(); ++it)
        it->second *= scalar;
    }
  }
   
protected:
  typedef std::map<Monomial, double> MonomialMap;
  typedef MonomialMap::iterator iterator;
  typedef MonomialMap::const_iterator const_iterator;

  MonomialMap monomials;    // a polynomial is a weighted sum of monomials

  void addMonomial(const Monomial& monomial, double weight = 1.0)
  {
    double& w = monomials[monomial];
    w += weight;
    if (!w)
      monomials.erase(monomial);
  }

  bool getBestDivisorMonomial(Monomial dividend, Monomial& res, double& weight) const
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
};

class RationalFunction
{
public:
  RationalFunction(const Polynomial& numerator, const Polynomial& denominator = Polynomial(1.0))
    : numerator(numerator), denominator(denominator) {}
  RationalFunction(const IdentifierPtr& identifier)
    : numerator(identifier), denominator(1.0) {}
  RationalFunction(const LiteralNumberPtr& number)
    : numerator(number), denominator(1.0) {}
  RationalFunction() {}

  static RationalFunction fromExpression(const ExpressionPtr& expression)
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

  static RationalFunction add(const RationalFunction& left, const RationalFunction& right)
  {
    RationalFunction res;
    res.numerator = Polynomial::add(
      Polynomial::mul(left.numerator, right.denominator),
      Polynomial::mul(left.denominator, right.numerator));
    res.denominator = Polynomial::mul(left.denominator, right.denominator);
    return res;
  }
  
  static RationalFunction sub(const RationalFunction& left, const RationalFunction& right)
  {
    RationalFunction res;
    res.numerator = Polynomial::sub(
      Polynomial::mul(left.numerator, right.denominator),
      Polynomial::mul(left.denominator, right.numerator));
    res.denominator = Polynomial::mul(left.denominator, right.denominator);
    return res;
  }

  static RationalFunction negate(const RationalFunction& fraction)
    {return RationalFunction(Polynomial::negate(fraction.numerator), fraction.denominator);}
  
  static RationalFunction mul(const RationalFunction& left, const RationalFunction& right)
    {return RationalFunction(Polynomial::mul(left.numerator, right.numerator), Polynomial::mul(left.denominator, right.denominator));}

  static RationalFunction div(const RationalFunction& left, const RationalFunction& right)
    {return RationalFunction(Polynomial::mul(left.numerator, right.denominator), Polynomial::mul(left.denominator, right.numerator));}

  static RationalFunction invert(const RationalFunction& fraction)
    {return RationalFunction(fraction.denominator, fraction.numerator);}

  static RationalFunction square(const RationalFunction& fraction)
    {return RationalFunction(Polynomial::square(fraction.numerator), Polynomial::square(fraction.denominator));}

  static RationalFunction pow(const RationalFunction& expr, size_t power)
  {
    jassert(power != 0);
    if (power == 1)
      return expr;
    if (power % 2 == 0)
      return square(pow(expr, power / 2));
    else
      return mul(expr, square(pow(expr, power / 2)));
  }

  static RationalFunction pow(const RationalFunction& expr, int power)
  {
    if (power == 0) // todo: check that this polynomial is not zero
      return RationalFunction(Polynomial::one(), Polynomial::one());
    else if (power > 0)
      return pow(expr, (size_t)power);
    else
      return pow(invert(expr), (size_t)(-power));
  }

  ExpressionPtr toExpression() const
    {return lua::div(numerator.toExpression(), denominator.toExpression());}

  void simplify()
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
  
  void normalizeConstants()
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

protected:
  Polynomial numerator;
  Polynomial denominator;
};

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
        operands.insert(expr);
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
  struct NodeComparisonOperator
  {
    bool operator ()(NodePtr left, NodePtr right)
      {return left->print() < right->print();}
  };

  typedef std::set<ExpressionPtr, NodeComparisonOperator> OperandsSet;
  OperandsSet operands;
  LiteralNumberPtr number;
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
      // min(a,b) = -max(-a,-b)
      setResult(rewrite(unm(new Call(new Identifier("math.max"), unm(call.getArgument(0)), unm(call.getArgument(1))))));
      return;
    }
    
    if (function == T("math.max"))
    {
      MaxNormalForm normalForm(call);
      setResult(normalForm.toExpression());
      return;
    }
    
    simplifyNumberAlgebra(&call);
  }

protected:
  void simplifyNumberAlgebra(const ExpressionPtr& expression)
  {
    if (isNumberAlgebra(expression))
    {
      RationalFunction fraction = RationalFunction::fromExpression(expression);
      fraction.simplify();
      fraction.normalizeConstants();
      setResult(fraction.toExpression());
    }
  }
  
  // returns true if the expression only contains literal numbers, identifiers, unm, add, sub, mul and div
  static bool isNumberAlgebra(const ExpressionPtr& expression)
  {
    if (expression.isInstanceOf<Identifier>())
      return true;
    if (expression.isInstanceOf<LiteralNumber>())
      return true;
    ParenthesisPtr parenthesis = expression.dynamicCast<Parenthesis>();
    if (parenthesis)
      return isNumberAlgebra(parenthesis->getExpr());
    UnaryOperationPtr unaryOperation = expression.dynamicCast<UnaryOperation>();
    if (unaryOperation)
      return unaryOperation->getOp() == unmOp && isNumberAlgebra(unaryOperation->getExpr());
    BinaryOperationPtr binaryOperation = expression.dynamicCast<BinaryOperation>();
    if (binaryOperation)
    {
      if (binaryOperation->getOp() == addOp ||
          binaryOperation->getOp() == subOp ||
          binaryOperation->getOp() == mulOp ||
          binaryOperation->getOp() == divOp)
        return isNumberAlgebra(binaryOperation->getLeft()) && isNumberAlgebra(binaryOperation->getRight());
      if (binaryOperation->getOp() == powOp)
        return isNumberAlgebra(binaryOperation->getLeft()) && isInteger(binaryOperation->getRight());
    }
    CallPtr call = expression.dynamicCast<Call>();
    if (call)
      return call->getFunction()->print() == T("math.inverse") &&
             call->getNumArguments() == 1 &&
             isNumberAlgebra(call->getArgument(0));
    return false;
  }
  
  static bool isInteger(ExpressionPtr expression)
  {
    LiteralNumberPtr number = expression.dynamicCast<LiteralNumber>();
    return number && number->getValue() == std::floor(number->getValue());
  }
};

}; /* namespace lua */
}; /* namespace lbcpp */

#endif // !LBCPP_LUA_REWRITER_SIMPLIFY_EXPRESSION_H_

