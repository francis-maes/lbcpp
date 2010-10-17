/*-----------------------------------------.---------------------------------.
| Filename: NumericalInference.h           | Numerical Inference             |
| Author  : Francis Maes                   |   base class                    |
| Started : 27/05/2010 21:11               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_NUMERICAL_H_
# define LBCPP_INFERENCE_NUMERICAL_H_

# include <lbcpp/Inference/Inference.h>
# include <lbcpp/Perception/Perception.h>

namespace lbcpp
{

/*
** NumericalInference
*/
class NumericalInference : public Inference
{
public:
  NumericalInference(const String& name, PerceptionPtr perception);
  NumericalInference() {}

  virtual TypePtr getInputType() const
    {return perception->getInputType();}

  TypePtr getPerceptionOutputType() const
    {return perception->getOutputType();}

  virtual TypePtr getParametersType() const = 0;

  // if target == NULL, target is this parameters
  // supervision is the loss function
  //   ScalarFunction for single output machines
  //   ObjectScalarFunction for multiple output machines
  // parameters += weight * gradient(input, supervision=lossFunction, prediction)
  // exampleLossValue = loss(prediction) (supervision=lossFunction)
  virtual void computeAndAddGradient(double weight, const Variable& input, const Variable& supervision, const Variable& prediction, double& exampleLossValue, ObjectPtr* target) = 0;

  ObjectPtr getParametersCopy() const;
  void addWeightedToParameters(const ObjectPtr& value, double weight);
  void applyRegularizerToParameters(ScalarObjectFunctionPtr regularizer, double weight);
  void setParameters(ObjectPtr parameters);

  const PerceptionPtr& getPerception() const
    {return perception;}

  virtual void validateParametersChange() {}

  virtual void clone(ObjectPtr target) const;

protected:
  friend class NumericalInferenceClass;

  PerceptionPtr perception;
  juce::ReadWriteLock parametersLock;
  ObjectPtr parameters;
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_NUMERICAL_H_
