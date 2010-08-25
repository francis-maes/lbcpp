/*-----------------------------------------.---------------------------------.
| Filename: BinaryPredicate.h              | Predicate : functions from      |
| Author  : Francis Maes                   |   Variables to booleans         |
| Started : 25/08/2010 22:18               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_PREDICATE_BINARY_H_
# define LBCPP_FUNCTION_PREDICATE_BINARY_H_

# include <lbcpp/Function/Predicate.h>

namespace lbcpp
{

class LogicalOrPredicate : public BinaryPredicate
{
public:
  LogicalOrPredicate(PredicatePtr predicate1, PredicatePtr predicate2)
    : BinaryPredicate(predicate1, predicate2) {}
  LogicalOrPredicate() : BinaryPredicate() {}

  virtual String toString() const
    {return T("(") + predicate1->toString() + T(" v ") + predicate2->toString() + T(")");}

  virtual bool computePredicate(const Variable& value, ErrorHandler& callback) const
    {return predicate1->computePredicate(value, callback) || predicate2->computePredicate(value, callback);}
};

class LogicalAndPredicate : public BinaryPredicate
{
public:
  LogicalAndPredicate(PredicatePtr predicate1, PredicatePtr predicate2)
    : BinaryPredicate(predicate1, predicate2) {}
  LogicalAndPredicate() : BinaryPredicate() {}

  virtual String toString() const
    {return T("(") + predicate1->toString() + T(" ^ ") + predicate2->toString() + T(")");}

  virtual bool computePredicate(const Variable& value, ErrorHandler& callback) const
    {return predicate1->computePredicate(value, callback) && predicate2->computePredicate(value, callback);}
};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_PREDICATE_BINARY_H_
