/*-----------------------------------------.---------------------------------.
| Filename: LossFunction.h                 | Base class for Loss Functions   |
| Author  : Francis Maes                   |                                 |
| Started : 17/02/2011 13:18               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LEARNING_LOSS_FUNCTION_H_
# define LBCPP_LEARNING_LOSS_FUNCTION_H_

# include "../Function/ScalarFunction.h"
# include "../Function/ScalarVectorFunction.h"

namespace lbcpp
{

/*
** FIXME: update-- Binary Classification Loss Functions
*/
class BinaryClassificationLossFunction : public ScalarFunction
{
public:
  BinaryClassificationLossFunction(bool isPositive) : isPositive(isPositive) {}
  BinaryClassificationLossFunction() {}

  virtual String toString() const;

  virtual void computePositive(double input, double* output, const double* derivativeDirection, double* derivative) const = 0;

  virtual void compute(double input, double* output, const double* derivativeDirection, double* derivative) const;

  bool getLabel() const
    {return isPositive;}

  lbcpp_UseDebuggingNewOperator

protected:
  friend class BinaryClassificationLossFunctionClass;

  bool isPositive;
};

typedef ReferenceCountedObjectPtr<BinaryClassificationLossFunction> BinaryClassificationLossFunctionPtr;

/*
** Multi Class Loss Functions
*/
class MultiClassLossFunction : public ScalarVectorFunction
{
public:
  size_t getNumClasses() const
    {return classes->getNumElements();}

  // Function
  virtual size_t getNumRequiredInputs() const
    {return 2;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index == 1 ? enumValueType : (TypePtr) denseDoubleVectorClass();}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName);

  lbcpp_UseDebuggingNewOperator

protected:
  friend class MultiClassLossFunctionClass;

  EnumerationPtr classes;

  size_t getCorrectClass(const Variable* otherInputs) const;
};

typedef ReferenceCountedObjectPtr<MultiClassLossFunction> MultiClassLossFunctionPtr;

extern MultiClassLossFunctionPtr oneAgainstAllMultiClassLossFunction(BinaryClassificationLossFunctionPtr binaryLossFunction);
extern MultiClassLossFunctionPtr mostViolatedMultiClassLossFunction(BinaryClassificationLossFunctionPtr binaryLossFunction);
extern MultiClassLossFunctionPtr logBinomialMultiClassLossFunction();

}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_LOSS_FUNCTION_H_
