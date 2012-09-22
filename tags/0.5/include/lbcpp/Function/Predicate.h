/*-----------------------------------------.---------------------------------.
| Filename: Predicate.h                    | Predicate : functions from      |
| Author  : Francis Maes                   |   Variables to booleans         |
| Started : 06/07/2010 15:13               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OBJECT_PREDICATE_H_
# define LBCPP_OBJECT_PREDICATE_H_

# include "../Core/Function.h"

namespace lbcpp
{

class Predicate : public Function
{
public:
  virtual bool computePredicate(ExecutionContext& context, const Variable& input) const = 0;

  /*
  ** Function
  */
  virtual size_t getNumRequiredInputs() const
    {return 1;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {jassert(false); return anyType;}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return booleanType;}

protected:
  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
    {return computePredicate(context, input);}
};

typedef ReferenceCountedObjectPtr<Predicate> PredicatePtr;

/*
** BinaryPredicate
*/
class BinaryPredicate : public Predicate
{
public:
  BinaryPredicate(PredicatePtr predicate1, PredicatePtr predicate2)
    : predicate1(predicate1), predicate2(predicate2) {}
  BinaryPredicate() {}

  PredicatePtr getLeftPredicate() const
    {return predicate1;}

  PredicatePtr getRightPredicate() const
    {return predicate2;}

protected:
  friend class BinaryPredicateClass;

  PredicatePtr predicate1;
  PredicatePtr predicate2;
};

typedef ReferenceCountedObjectPtr<BinaryPredicate> BinaryPredicatePtr;

extern BinaryPredicatePtr logicalOrPredicate(PredicatePtr predicate1, PredicatePtr predicate2);
extern BinaryPredicatePtr logicalAndPredicate(PredicatePtr predicate1, PredicatePtr predicate2);

inline BinaryPredicatePtr logicalOr(PredicatePtr predicate1, PredicatePtr predicate2)
  {return logicalOrPredicate(predicate1, predicate2);}

inline BinaryPredicatePtr logicalAnd(PredicatePtr predicate1, PredicatePtr predicate2)
  {return logicalAndPredicate(predicate1, predicate2);}

/*
** UnaryComparisonPredicate 
*/
class UnaryComparisonPredicate : public Predicate
{
public:
  UnaryComparisonPredicate(const Variable& operand)
    : operand(operand) {}
  UnaryComparisonPredicate() {}

  virtual String operatorToString() const = 0;

  virtual String toString() const
    {return T("(x ") + operatorToString() + T(" ") + operand.toString() + T(")");}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return operand.getType();}

  Variable getOperand() const
    {return operand;}

protected:
  friend class UnaryComparisonPredicateClass;

  Variable operand;
};

typedef ReferenceCountedObjectPtr<UnaryComparisonPredicate> UnaryComparisonPredicatePtr;

extern UnaryComparisonPredicatePtr lessThanPredicate(const Variable& operand);
extern UnaryComparisonPredicatePtr lessThanOrEqualToPredicate(const Variable& operand);
extern UnaryComparisonPredicatePtr equalToPredicate(const Variable& operand);
extern UnaryComparisonPredicatePtr differentFromPredicate(const Variable& operand);
extern UnaryComparisonPredicatePtr greaterThanOrEqualToPredicate(const Variable& operand);
extern UnaryComparisonPredicatePtr greaterThanPredicate(const Variable& operand);

}; /* namespace lbcpp */

#endif // !LBCPP_OBJECT_PREDICATE_H_
