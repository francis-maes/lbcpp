/*-----------------------------------------.---------------------------------.
| Filename: UnaryComparisonPredicate.h     | Predicate : functions from      |
| Author  : Francis Maes                   |   Variables to booleans         |
| Started : 25/08/2010 22:18               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_PREDICATE_UNARY_COMPARISON_H_
# define LBCPP_FUNCTION_PREDICATE_UNARY_COMPARISON_H_

# include <lbcpp/Function/Predicate.h>

namespace lbcpp
{

class LessThanPredicate : public UnaryComparisonPredicate
{
public:
  LessThanPredicate(const Variable& operand)
    : UnaryComparisonPredicate(operand) {}
  LessThanPredicate() {}

  virtual String operatorToString() const
    {return T("<");}

  virtual bool computePredicate(const Variable& value, MessageCallback& callback) const
    {return value < operand;}
};

class LessThanOrEqualToPredicate : public UnaryComparisonPredicate
{
public:
  LessThanOrEqualToPredicate(const Variable& operand)
    : UnaryComparisonPredicate(operand) {jassert(operand);}
  LessThanOrEqualToPredicate() {}

  virtual String operatorToString() const
    {return T("<=");}

  virtual bool computePredicate(const Variable& value, MessageCallback& callback) const
    {return value <= operand;}
};

class EqualToPredicate : public UnaryComparisonPredicate
{
public:
  EqualToPredicate(const Variable& operand)
    : UnaryComparisonPredicate(operand) {}
  EqualToPredicate() {}

  virtual String operatorToString() const
    {return T("==");}

  virtual bool computePredicate(const Variable& value, MessageCallback& callback) const
    {return value == operand;}
};

class DifferentFromPredicate : public UnaryComparisonPredicate
{
public:
  DifferentFromPredicate(const Variable& operand)
    : UnaryComparisonPredicate(operand) {}
  DifferentFromPredicate() {}

  virtual String operatorToString() const
    {return T("!=");}

  virtual bool computePredicate(const Variable& value, MessageCallback& callback) const
    {return value != operand;}
};

class GreaterThanOrEqualToPredicate : public UnaryComparisonPredicate
{
public:
  GreaterThanOrEqualToPredicate(const Variable& operand)
    : UnaryComparisonPredicate(operand) {}
  GreaterThanOrEqualToPredicate() {}

  virtual String operatorToString() const
    {return T(">=");}

  virtual bool computePredicate(const Variable& value, MessageCallback& callback) const
    {return value >= operand;}
};

class GreaterThanPredicate : public UnaryComparisonPredicate
{
public:
  GreaterThanPredicate(const Variable& operand)
    : UnaryComparisonPredicate(operand) {}
  GreaterThanPredicate() {}

  virtual String operatorToString() const
    {return T(">");}

  virtual bool computePredicate(const Variable& value, MessageCallback& callback) const
    {return value > operand;}
};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_PREDICATE_UNARY_COMPARISON_H_
