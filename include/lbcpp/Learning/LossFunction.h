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
** RegressionLossFunction
*/
class RegressionLossFunction : public ScalarFunction
{
public:
  virtual void computeRegressionLoss(double input, double target, double* output, double* derivative) const = 0;

  virtual size_t getNumRequiredInputs() const;
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const;
  virtual String getOutputPostFix() const;
  virtual void computeScalarFunction(double input, const Variable* otherInputs, double* output, double* derivative) const;

  lbcpp_UseDebuggingNewOperator
};

typedef ReferenceCountedObjectPtr<RegressionLossFunction> RegressionLossFunctionPtr;

extern RegressionLossFunctionPtr squareRegressionLossFunction();

/*
** DiscriminativeLossFunction
*/
class DiscriminativeLossFunction : public ScalarFunction
{
public:
  virtual void computeDiscriminativeLoss(double score, double* output, double* derivative) const = 0;

  virtual size_t getNumRequiredInputs() const;
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const;
  virtual String getOutputPostFix() const;
  virtual void computeScalarFunction(double input, const Variable* otherInputs, double* output, double* derivative) const;

  lbcpp_UseDebuggingNewOperator
};

typedef ReferenceCountedObjectPtr<DiscriminativeLossFunction> DiscriminativeLossFunctionPtr;

extern DiscriminativeLossFunctionPtr hingeDiscriminativeLossFunction(double margin = 1);
extern DiscriminativeLossFunctionPtr logBinomialDiscriminativeLossFunction();

/*
** MultiClassLossFunction
*/
class MultiClassLossFunction : public ScalarVectorFunction
{
public:
  virtual void computeMultiClassLoss(const DenseDoubleVectorPtr& scores, size_t correctClass, size_t numClasses, double* output, DenseDoubleVectorPtr* gradientTarget, double gradientWeight) const = 0;

  virtual size_t getNumRequiredInputs() const;
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const;
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName);
  virtual void computeScalarVectorFunction(const DenseDoubleVectorPtr& input, const Variable* otherInputs, double* output, DenseDoubleVectorPtr* gradientTarget, double gradientWeight) const;

  lbcpp_UseDebuggingNewOperator

protected:
  friend class MultiClassLossFunctionClass;

  EnumerationPtr classes;
};

typedef ReferenceCountedObjectPtr<MultiClassLossFunction> MultiClassLossFunctionPtr;

extern MultiClassLossFunctionPtr oneAgainstAllMultiClassLossFunction(DiscriminativeLossFunctionPtr binaryLossFunction);
extern MultiClassLossFunctionPtr mostViolatedMultiClassLossFunction(DiscriminativeLossFunctionPtr binaryLossFunction);
extern MultiClassLossFunctionPtr logBinomialMultiClassLossFunction();

}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_LOSS_FUNCTION_H_
