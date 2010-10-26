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

# include <lbcpp/Data/RandomGenerator.h> // tmp

namespace lbcpp
{

class AddBiasInference : public Inference
{
public:
  AddBiasInference(const String& name, double initialBias = 0.0)
    : Inference(name)
  {
    parameters = initialBias;
    addOnlineLearner(addBiasOnlineLearner(InferenceOnlineLearner::perPass));
  }

  AddBiasInference() {}

  virtual TypePtr getInputType() const
    {return doubleType;}

  virtual TypePtr getSupervisionType() const
    {return anyType;}

  virtual TypePtr getOutputType(TypePtr inputType) const
    {return inputType;}

  virtual TypePtr getParametersType() const
    {return doubleType;}

  double getBias() const
    {return getParameters().getDouble();}

  void setBias(double bias)
    {setParameters(bias);}

protected:
  virtual Variable run(InferenceContextWeakPtr context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
    {return Variable(input.getDouble() + getBias(), input.getType());}
};

typedef ReferenceCountedObjectPtr<AddBiasInference> AddBiasInferencePtr;

}; /* namespace lbcpp */

#endif // !LBCPP_NUMERICAL_LEARNING_INFERENCE_ADD_BIAS_H_
