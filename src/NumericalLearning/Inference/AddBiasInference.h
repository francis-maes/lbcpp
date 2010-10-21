/*-----------------------------------------.---------------------------------.
| Filename: AddBiasInference.h             | Add Bias Inference              |
| Author  : Francis Maes                   |                                 |
| Started : 21/10/2010 16:06               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_NUMERICAL_LEARNING_INFERENCE_ADD_BIAS_H_
# define LBCPP_NUMERICAL_LEARNING_INFERENCE_ADD_BIAS_H_

# include <lbcpp/Inference/DecoratorInference.h>
# include <lbcpp/NumericalLearning/NumericalLearning.h>

namespace lbcpp
{

class AddBiasInference : public StaticDecoratorInference
{
public:
  AddBiasInference(const String& name, InferencePtr numericalInference, double initialBias = 0.0)
    : StaticDecoratorInference(name, numericalInference), bias(initialBias)
    {addOnlineLearner(addBiasOnlineLearner(InferenceOnlineLearner::perPass));}

  AddBiasInference() {}

  virtual Variable finalizeInference(const InferenceContextPtr& context, const DecoratorInferenceStatePtr& finalState, ReturnCode& returnCode)
  {
    const Variable& subOutput = finalState->getSubOutput();
    return subOutput.exists() ? Variable(subOutput.getDouble() + bias, doubleType) : subOutput;
  }

  double getBias() const
    {return bias;}

  void setBias(double bias)
    {this->bias = bias;}
  
protected:
  friend class AddBiasInferenceClass;
  double bias;
};

typedef ReferenceCountedObjectPtr<AddBiasInference> AddBiasInferencePtr;

}; /* namespace lbcpp */

#endif // !LBCPP_NUMERICAL_LEARNING_INFERENCE_ADD_BIAS_H_
