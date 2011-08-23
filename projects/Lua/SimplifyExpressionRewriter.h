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

  static Polynomial add(const Polynomial& left, const Polynomial& right)
  {
    Polynomial res = left;
    for (const_iterator it = right.monomials.begin(); it != right.monomials.end(); ++it)
      res.addMonomial(it->first, it->second);
    res.pruneEmptyMonomials();
    return res;
  }
  
  static Polynomial sub(const Polynomial& left, const Polynomial& right)
  {
    Polynomial res = left;
    for (const_iterator it = right.monomials.begin(); it != right.monomials.end(); ++it)
      res.addMonomial(it->first, -it->second);
    res.pruneEmptyMonomials();
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
        double weight = it1->second * it2->second;
        jassert(weight);
        res.addMonomial(Monomial::mul(it1->first, it2->first), weight);
      }
    return res;
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
  
protected:
  typedef std::map<Monomial, double> MonomialMap;
  typedef MonomialMap::iterator iterator;
  typedef MonomialMap::const_iterator const_iterator;

  MonomialMap monomials;    // a polynomial is a weighted sum of monomials

  void addMonomial(const Monomial& monomial, double weight = 1.0)
    {monomials[monomial] += weight;}
    
  void pruneEmptyMonomials()
  {
    iterator it, nxt;
    for (it = monomials.begin(); it != monomials.end(); it = nxt)
    {
      nxt = it; ++nxt;
      if (it->second == 0.0)
        monomials.erase(it);
    }
  }
};

class PolynomialFraction
{
public:
  PolynomialFraction(const Polynomial& numerator, const Polynomial& denominator = Polynomial(1.0))
    : numerator(numerator), denominator(denominator) {}
  PolynomialFraction(const IdentifierPtr& identifier)
    : numerator(identifier), denominator(1.0) {}
  PolynomialFraction(const LiteralNumberPtr& number)
    : numerator(number), denominator(1.0) {}
  PolynomialFraction() {}

  static PolynomialFraction fromExpression(const ExpressionPtr& expression)
  {
    IdentifierPtr identifier = expression.dynamicCast<Identifier>();
    if (identifier)
      return PolynomialFraction(identifier);
    
    LiteralNumberPtr literalNumber = expression.dynamicCast<LiteralNumber>();
    if (literalNumber)
      return PolynomialFraction(literalNumber);
    
    UnaryOperationPtr unaryOperation = expression.dynamicCast<UnaryOperation>();
    if (unaryOperation && unaryOperation->getOp() == unmOp)
      return negate(fromExpression(unaryOperation->getExpr()));
      
    BinaryOperationPtr binaryOperation = expression.dynamicCast<BinaryOperation>();
    if (binaryOperation)
    {
      PolynomialFraction left = fromExpression(binaryOperation->getLeft());
      PolynomialFraction right = fromExpression(binaryOperation->getRight());
      
      BinaryOp op = binaryOperation->getOp();
      if (op == addOp)
        return add(left, right);
      else if (op == subOp)
        return sub(left, right);
      else if (op == mulOp)
        return mul(left, right);
      else if (op == divOp)
        return div(left, right);
    }

    jassert(false); // this kind of expressions cannot be converted into a PolynomialFraction 
    return PolynomialFraction();
  }

  static PolynomialFraction add(const PolynomialFraction& left, const PolynomialFraction& right)
  {
    PolynomialFraction res;
    res.numerator = Polynomial::add(
      Polynomial::mul(left.numerator, right.denominator),
      Polynomial::mul(left.denominator, right.numerator));
    res.denominator = Polynomial::mul(left.denominator, right.denominator);
    return res;
  }
  
  static PolynomialFraction sub(const PolynomialFraction& left, const PolynomialFraction& right)
  {
    PolynomialFraction res;
    res.numerator = Polynomial::sub(
      Polynomial::mul(left.numerator, right.denominator),
      Polynomial::mul(left.denominator, right.numerator));
    res.denominator = Polynomial::mul(left.denominator, right.denominator);
    return res;
  }

  static PolynomialFraction negate(const PolynomialFraction& fraction)
    {return PolynomialFraction(Polynomial::negate(fraction.numerator), fraction.denominator);}
  
  static PolynomialFraction mul(const PolynomialFraction& left, const PolynomialFraction& right)
    {return PolynomialFraction(Polynomial::mul(left.numerator, right.numerator), Polynomial::mul(left.denominator, right.denominator));}

  static PolynomialFraction div(const PolynomialFraction& left, const PolynomialFraction& right)
    {return PolynomialFraction(Polynomial::mul(left.numerator, right.denominator), Polynomial::mul(left.denominator, right.numerator));}

  static PolynomialFraction invert(const PolynomialFraction& fraction)
    {return PolynomialFraction(fraction.denominator, fraction.numerator);}

  ExpressionPtr toExpression() const
    {return lua::div(numerator.toExpression(), denominator.toExpression());}

protected:
  Polynomial numerator;
  Polynomial denominator;
};

#if 0
class AlgebraTerm
{
public:
  AlgebraTerm(bool isNegative, const std::vector<ExpressionPtr>& numerator, const std::vector<ExpressionPtr>& denominator)
    : isNegative(isNegative), numerator(numerator), denominator(denominator) {}
  AlgebraTerm(ExpressionPtr numerator)
    : isNegative(false), numerator(1, numerator) {}
  AlgebraTerm() : isNegative(false) {}
  
  static AlgebraTerm negate(const AlgebraTerm& term)
    {return AlgebraTerm(!term.isNegative, term.numerator, term.denominator);}
  
  static AlgebraTerm invert(const AlgebraTerm& term)
    {return AlgebraTerm(term.isNegative, term.denominator, term.numerator);}
  
  static AlgebraTerm mul(const AlgebraTerm& left, const AlgebraTerm& right)
  {
    AlgebraTerm res;
    res.isNegative = (left.isNegative != right.isNegative);
    append(res.numerator, left.numerator);
    append(res.numerator, right.numerator);
    append(res.denominator, left.denominator);
    append(res.denominator, right.denominator);
    return res;
  }
  
  static AlgebraTerm div(const AlgebraTerm& left, const AlgebraTerm& right)
    {return mul(left, invert(right));}
  
protected:
  bool isNegative;
  std::vector<ExpressionPtr> numerator;    // identifiers or literal numbers
  std::vector<ExpressionPtr> denominator;
  
  static void append(std::vector<ExpressionPtr>& dest, const std::vector<ExpressionPtr>& source)
  {
    size_t n = source.size();
    dest.reserve(dest.size() + n);
    for (size_t i = 0; i < n; ++i)
      dest.push_back(source[i]);
  }
};

class AlgebraNormalForm
{
public:
  AlgebraNormalForm(const ExpressionPtr& expression)
  {
    terms = makeTerms(expression);
  }


protected:
  std::vector<AlgebraTerm> makeTerms(const ExpressionPtr& expression)
  {
    std::vector<AlgebraTerm> res;
    if (expression.isInstanceOf<Identifier>() || expression.isInstanceOf<LiteralNumber>())
    {
      res.push_back(AlgebraTerm(expression));
    }
    else if (expression.isInstanceOf<UnaryOperation>())
    {
      UnaryOperationPtr operation = expression.staticCast<UnaryOperation>();
      if (operation->getOp() == unmOp)
      {
        res = makeTerms(operation->getExpr());
        for (size_t i = 0; i < res.size(); ++i)
          res[i] = AlgebraTerm::negate(res[i]);
      }
      else
        jassert(false);
    }
    else if (expression.isInstanceOf<BinaryOperation>())
    {
      BinaryOperationPtr operation = expression.staticCast<BinaryOperation>();
      std::vector<AlgebraTerm> leftTerms = makeTerms(operation->getLeft());
      std::vector<AlgebraTerm> rightTerms = makeTerms(operation->getRight());
      
      if (operation->getOp() == addOp)
      {
        res = leftTerms;
        res.reserve(res.size() + rightTerms.size());
        for (size_t i = 0; i < rightTerms.size(); ++i)
          res.push_back(rightTerms[i]);
      }
      else if (operation->getOp() == subOp)
      {
        res = leftTerms;
        res.reserve(res.size() + rightTerms.size());
        for (size_t i = 0; i < rightTerms.size(); ++i)
          res.push_back(AlgebraTerm::negate(rightTerms[i]));
      }
      else if (operation->getOp() == mulOp)
      {
        res.reserve(leftTerms.size() * rightTerms.size());
        for (size_t i = 0; i < leftTerms.size(); ++i)
          for (size_t j = 0; j < rightTerms.size(); ++j)
            res.push_back(AlgebraTerm::mul(leftTerms[i], rightTerms[j]));
      }
      else if (operation->getOp() == divOp)
      {
        if (rightTerms.size() == 1)
        {
          res.reserve(leftTerms.size());
          for (size_t i = 0; i < leftTerms.size(); ++i)
            res.push_back(AlgebraTerm::div(leftTerms[i], rightTerms[0]));
        }
        else
        {
          jassert(false);
          std::cerr << "Prout" << std::endl;
        }
      }
      else
        jassert(false);
    }
    else
      jassert(false);
      
    return res;
  }

  std::vector<AlgebraTerm> terms;
};
#endif // 0

#if 0
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
    
    const double* number1 = getLiteralNumber(operation.getLeft());
    const double* number2 = getLiteralNumber(operation.getRight());

    switch (op)
    {
    case addOp:
      if (number1 && number2)
        setResult(new LiteralNumber(*number1 + *number2));
      else if (number1 && *number1 == 0)
        setResult(operation.getRight());
      else if (number2 && *number2 == 0)
        setResult(operation.getLeft());
      break;
      
    default:
      break;
    }
  }

  virtual void visit(Parenthesis& parenthesis)
    {setResult(rewrite(parenthesis.getExpr()));}

protected:
  static const double* getLiteralNumber(const NodePtr& node)
  {
    LiteralNumberPtr number = node.dynamicCast<LiteralNumber>();
    return number ? &number->getValue() : NULL;
  }
};
#endif // 0

class SimplifyExpressionRewriter : public DefaultRewriter
{
public:
  SimplifyExpressionRewriter(ExecutionContextPtr context)
    : DefaultRewriter(context) {}
 
  virtual void visit(Parenthesis& parenthesis)
    {setResult(rewrite(parenthesis.getExpr()));}

  virtual void visit(UnaryOperation& operation)
    {simplifyNumberAlgebra(&operation);}

  virtual void visit(BinaryOperation& operation)
    {simplifyNumberAlgebra(&operation);}

protected:
  void simplifyNumberAlgebra(const ExpressionPtr& expression)
  {
    if (isNumberAlgebra(expression))
    {
      PolynomialFraction fraction = PolynomialFraction::fromExpression(expression);
      // fraction->simplify()
      setResult(fraction.toExpression());
    }
  }
  
  // returns true if the expression only contains literal numbers, identifiers, unm, add, sub, mul and div
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
};

}; /* namespace lua */
}; /* namespace lbcpp */

#endif // !LBCPP_LUA_REWRITER_SIMPLIFY_EXPRESSION_H_

