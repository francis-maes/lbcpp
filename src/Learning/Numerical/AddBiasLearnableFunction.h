/*-----------------------------------------.---------------------------------.
| Filename: AddBiasLearnableFunction.h     | AddBias Learnable Function      |
| Author  : Francis Maes                   |                                 |
| Started : 28/02/2011 20:25               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LEARNING_NUMERICAL_ADD_BIAS_LEARNABLE_FUNCTION_H_
# define LBCPP_LEARNING_NUMERICAL_ADD_BIAS_LEARNABLE_FUNCTION_H_

# include <lbcpp/Learning/Numerical.h>

namespace lbcpp
{

class AddBiasLearnableFunction : public Function
{
public:
  AddBiasLearnableFunction(BinaryClassificationScore scoreToOptimize = binaryClassificationAccuracyScore, double bias = 0.0)
    : scoreToOptimize(scoreToOptimize), bias(bias) {}

  virtual size_t getNumRequiredInputs() const
    {return 2;}
  
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index ? anyType : doubleType;}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {
    outputName = T("biased");
    outputShortName = T("b");
    setBatchLearner(addBiasBatchLearner(scoreToOptimize));
    return doubleType;
  }

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    const Variable& input = inputs[0];
    return input.exists() ? Variable(input.getDouble() + bias) : input;
  }

  double getBias() const
    {return bias;}

  void setBias(double bias)
    {this->bias = bias;}

protected:
  friend class AddBiasLearnableFunctionClass;

  BinaryClassificationScore scoreToOptimize;
  double bias;
};

typedef ReferenceCountedObjectPtr<AddBiasLearnableFunction> AddBiasLearnableFunctionPtr;
extern ClassPtr addBiasLearnableFunctionClass;

}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_NUMERICAL_ADD_BIAS_LEARNABLE_FUNCTION_H_
