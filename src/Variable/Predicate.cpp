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

void declarePredicateClasses()
{
  LBCPP_DECLARE_ABSTRACT_CLASS(Predicate, Object);
    LBCPP_DECLARE_ABSTRACT_CLASS(BinaryPredicate, Predicate);
      LBCPP_DECLARE_CLASS(LogicalOrPredicate, BinaryPredicate);
      LBCPP_DECLARE_CLASS(LogicalAndPredicate, BinaryPredicate);
}
