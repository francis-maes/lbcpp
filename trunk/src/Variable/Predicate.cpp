/*-----------------------------------------.---------------------------------.
| Filename: Predicate.cpp                  | Predicate : functions from      |
| Author  : Francis Maes                   |   Variables to booleans         |
| Started : 06/07/2010 15:14               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Object/Predicate.h>
using namespace lbcpp;

class BinaryPredicate : public Predicate
{
public:
  BinaryPredicate(PredicatePtr predicate1, PredicatePtr predicate2)
    : predicate1(predicate1), predicate2(predicate2) {}
  BinaryPredicate() {}

protected:
  PredicatePtr predicate1;
  PredicatePtr predicate2;
};

class LogicalOrPredicate : public BinaryPredicate
{
public:
  LogicalOrPredicate(PredicatePtr predicate1, PredicatePtr predicate2)
    : BinaryPredicate(predicate1, predicate2) {}
  LogicalOrPredicate() : BinaryPredicate() {}

  virtual String toString() const
    {return T("(") + predicate1->toString() + T(" v ") + predicate2->toString() + T(")");}

  virtual bool compute(const Variable& value) const
    {return predicate1->compute(value) || predicate2->compute(value);}
};

class LogicalAndPredicate : public BinaryPredicate
{
public:
  LogicalAndPredicate(PredicatePtr predicate1, PredicatePtr predicate2)
    : BinaryPredicate(predicate1, predicate2) {}
  LogicalAndPredicate() : BinaryPredicate() {}

  virtual String toString() const
    {return T("(") + predicate1->toString() + T(" ^ ") + predicate2->toString() + T(")");}

  virtual bool compute(const Variable& value) const
    {return predicate1->compute(value) && predicate2->compute(value);}
};

class UnaryComparisonPredicate : public Predicate
{
public:
  UnaryComparisonPredicate(const Variable& operand)
    : operand(operand) {}
  UnaryComparisonPredicate() {}

  virtual String operatorToString() const = 0;

  virtual String toString() const
    {return T("(x ") + operatorToString() + T(" ") + operand.toString() + T(")");}

protected:
  Variable operand;
};
  
class LessThanPredicate : public UnaryComparisonPredicate
{
public:
  LessThanPredicate(const Variable& operand)
    : UnaryComparisonPredicate(operand) {}
  LessThanPredicate() {}

  virtual String operatorToString() const
    {return T("<");}

  virtual bool compute(const Variable& value) const
    {return value < operand;}
};

class LessThanOrEqualToPredicate : public UnaryComparisonPredicate
{
public:
  LessThanOrEqualToPredicate(const Variable& operand)
    : UnaryComparisonPredicate(operand) {}
  LessThanOrEqualToPredicate() {}

  virtual String operatorToString() const
    {return T("<=");}

  virtual bool compute(const Variable& value) const
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

  virtual bool compute(const Variable& value) const
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

  virtual bool compute(const Variable& value) const
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

  virtual bool compute(const Variable& value) const
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

  virtual bool compute(const Variable& value) const
    {return value > operand;}
};

PredicatePtr lbcpp::logicalOr(PredicatePtr predicate1, PredicatePtr predicate2)
  {return new LogicalOrPredicate(predicate1, predicate2);}

PredicatePtr lbcpp::logicalAnd(PredicatePtr predicate1, PredicatePtr predicate2)
  {return new LogicalAndPredicate(predicate1, predicate2);}

PredicatePtr lbcpp::lessThanPredicate(const Variable& operand)
  {return new LessThanPredicate(operand);}

PredicatePtr lbcpp::lessThanOrEqualToPredicate(const Variable& operand)
  {return new LessThanOrEqualToPredicate(operand);}

PredicatePtr lbcpp::equalToPredicate(const Variable& operand)
  {return new EqualToPredicate(operand);}

PredicatePtr lbcpp::differentFromPredicate(const Variable& operand)
  {return new DifferentFromPredicate(operand);}

PredicatePtr lbcpp::greaterThanOrEqualToPredicate(const Variable& operand)
  {return new GreaterThanOrEqualToPredicate(operand);}

PredicatePtr lbcpp::greaterThanPredicate(const Variable& operand)
  {return new GreaterThanPredicate(operand);}

void declarePredicateClasses()
{
  LBCPP_DECLARE_ABSTRACT_CLASS(Predicate, Object);
    LBCPP_DECLARE_ABSTRACT_CLASS(BinaryPredicate, Predicate);
      LBCPP_DECLARE_CLASS(LogicalOrPredicate, BinaryPredicate);
      LBCPP_DECLARE_CLASS(LogicalAndPredicate, BinaryPredicate);

    LBCPP_DECLARE_ABSTRACT_CLASS(UnaryComparisonPredicate, Predicate);
      LBCPP_DECLARE_CLASS(LessThanPredicate, UnaryComparisonPredicate);
      LBCPP_DECLARE_CLASS(LessThanOrEqualToPredicate, UnaryComparisonPredicate);
      LBCPP_DECLARE_CLASS(EqualToPredicate, UnaryComparisonPredicate);
      LBCPP_DECLARE_CLASS(DifferentFromPredicate, UnaryComparisonPredicate);
      LBCPP_DECLARE_CLASS(GreaterThanOrEqualToPredicate, UnaryComparisonPredicate);
      LBCPP_DECLARE_CLASS(GreaterThanPredicate, UnaryComparisonPredicate);
}
