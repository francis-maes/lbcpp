/*-----------------------------------------.---------------------------------.
| Filename: NumericalInference.h           | Base Class for Numerical        |
| Author  : Francis Maes                   |                     Inferences  |
| Started : 21/10/2010 18:56               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_NUMERICAL_LEARNING_INFERENCE_H_
# define LBCPP_NUMERICAL_LEARNING_INFERENCE_H_

# include "../Perception/Perception.h"
# include "../Inference/Inference.h"
# include "../Data/DoubleVector.h"
# include "../FeatureGenerator/FeatureGenerator.h"

namespace lbcpp
{

extern ClassPtr numericalInferenceClass;
extern ClassPtr numericalInferenceParametersClass(TypePtr weightsType);

class NumericalInferenceParameters;
typedef ReferenceCountedObjectPtr<NumericalInferenceParameters> NumericalInferenceParametersPtr;

class NumericalInferenceParameters : public Object
{
public:
  NumericalInferenceParameters(const PerceptionPtr& perception, TypePtr weightsType);
  NumericalInferenceParameters() {}

  const PerceptionPtr& getPerception() const
    {return perception;}

  const ObjectPtr& getWeights() const
    {return weights;}

  ObjectPtr& getWeights()
    {return weights;}

  virtual void clone(ExecutionContext& context, const ObjectPtr& t) const;

private:
  friend class NumericalInferenceParametersClass;

  PerceptionPtr perception;
  ObjectPtr weights;
};

class NumericalInference : public Inference
{
public:
  NumericalInference(const String& name, PerceptionPtr perception);
  NumericalInference() {}

  //virtual TypePtr getInputType() const
  //  {return getPerception()->getInputType();}

  virtual TypePtr getWeightsType(TypePtr perceptionOutputType) const = 0;

  virtual TypePtr getParametersType() const
    {return parameters.exists() ? parameters.getType() : (TypePtr)numericalInferenceParametersClass(getWeightsType(getPerception()->getOutputType()));}

  const NumericalInferenceParametersPtr& getParameters() const;
  
  const PerceptionPtr& getPerception() const;

  ObjectPtr getWeightsCopy(ExecutionContext& context) const;
  const ObjectPtr& getWeights() const;
  void setWeights(const ObjectPtr& newWeights);
  
  // if target == NULL, target is this parameters
  // supervision is the loss function
  //   ScalarFunction for single output machines
  //   ObjectScalarFunction for multiple output machines
  // parameters += weight * gradient(input, supervision=lossFunction, prediction)
  // exampleLossValue = loss(prediction) (supervision=lossFunction)
  virtual void computeAndAddGradient(ExecutionContext& context, double weight, const Variable& input, const Variable& supervision, const Variable& prediction, double& exampleLossValue, ObjectPtr* target) = 0;

  void addWeightedToParameters(ExecutionContext& context, const ObjectPtr& value, double weight);
  void addWeightedToParameters(ExecutionContext& context, const PerceptionPtr& perception, const Variable& input, double weight);
  void applyRegularizerToParameters(ExecutionContext& context, ScalarVectorFunctionPtr regularizer, double weight);
  void updateParametersType(ExecutionContext& context); // this function is called when the type of Perception changes

protected:
  virtual Variable computeInference(ExecutionContext& context, const Variable& input, const Variable& supervision) const
    {return computeFunction(context, input);}
};

typedef ReferenceCountedObjectPtr<NumericalInference> NumericalInferencePtr;

extern NumericalInferencePtr linearInference(const String& name, PerceptionPtr perception);
extern NumericalInferencePtr multiLinearInference(const String& name, PerceptionPtr perception, ClassPtr outputClass);

}; /* namespace lbcpp */

#endif //!LBCPP_NUMERICAL_LEARNING_INFERENCE_H_
