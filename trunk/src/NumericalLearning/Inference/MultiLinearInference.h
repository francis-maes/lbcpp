/*-----------------------------------------.---------------------------------.
| Filename: MultiLinearInference.h         | Multi-linear Scalar Inference   |
| Author  : Francis Maes                   |                                 |
| Started : 16/10/2010 12:37               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_NUMERICAL_LEARNING_INFERENCE_MULTI_LINEAR_H_
# define LBCPP_NUMERICAL_LEARNING_INFERENCE_MULTI_LINEAR_H_

# include <lbcpp/NumericalLearning/NumericalLearning.h>
# include <lbcpp/Function/ScalarObjectFunction.h>
# include "../../Core/Object/DenseObjectObject.h"
# include "../../Core/Object/DenseDoubleObject.h"

namespace lbcpp
{

// Input: Perception's input
// Output: outputClass parameter
// Supervision: ScalarObjectFunction
class MultiLinearInference : public NumericalInference
{
public:
  MultiLinearInference(const String& name, PerceptionPtr perception, ClassPtr outputClass)
    : NumericalInference(name, perception), outputClass(outputClass), numOutputs(outputClass->getObjectNumVariables())
  {
    parameters = new NumericalInferenceParameters(perception, getWeightsType(perception->getOutputType()));  
  }

  MultiLinearInference() {}

  virtual TypePtr getSupervisionType() const
    {return scalarObjectFunctionClass;}

  virtual TypePtr getOutputType(TypePtr ) const
    {return outputClass;}

  virtual TypePtr getWeightsType(TypePtr perceptionOutputType) const
    {return oneSubObjectPerInputVariableClass(outputClass, perceptionOutputType);}

  virtual void computeAndAddGradient(ExecutionContext& context, double weight, const Variable& input, const Variable& supervision, const Variable& prediction, double& exampleLossValue, ObjectPtr* target)
  {
    const NumericalInferenceParametersPtr& parameters = this->parameters.getObjectAndCast<NumericalInferenceParameters>(context);
    const PerceptionPtr& perception = parameters->getPerception();

    const MultiClassLossFunctionPtr& lossFunction = supervision.getObjectAndCast<MultiClassLossFunction>(context);
    std::vector<double> lossGradient;
    lossFunction->compute(context, prediction.getObject(), &exampleLossValue, &lossGradient, 1.0);
    if (lossGradient.empty() || !perception->getOutputType()->getObjectNumVariables())
      return; // when learning the perception, its number of output variables may be null at beginning

    bool isLocked = false;
    if (!target)
    {
      parametersLock.enterWrite();
      target = &parameters->getWeights();
      isLocked = true;
    }
    DenseObjectObjectPtr& weights = *(DenseObjectObjectPtr* )target;
    if (!weights)
      weights = Object::create(getWeightsType(perception->getOutputType())).staticCast<DenseObjectObject>();
    
    ObjectPtr perceivedInput;
    if (input.getType() == perception->getOutputType())
      perceivedInput = input.getObject();
    else
      perceivedInput = perception->computeFunction(context, input).getObject();

    for (size_t i = 0; i < lossGradient.size(); ++i)
    {
      double w = lossGradient[i] * weight;
      if (w)
        lbcpp::addWeighted(context, weights->getObjectReference(i), perceivedInput, w);
    }

    if (isLocked)
      parametersLock.exitWrite();
  }

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    ScopedReadLock _(parametersLock);
    const NumericalInferenceParametersPtr& parameters = this->parameters.getObjectAndCast<NumericalInferenceParameters>(context);
    const PerceptionPtr& perception = parameters->getPerception();
    const ObjectPtr& weights = parameters->getWeights();
    if (!weights)
      return Variable::missingValue(outputClass);

    const DenseObjectObjectPtr& denseWeights = weights.staticCast<DenseObjectObject>();
    DenseDoubleObjectPtr res = new DenseDoubleObject(outputClass.staticCast<DynamicClass>().get());
    std::vector<double>& outputs = res->getValues();
    size_t n = outputClass->getObjectNumVariables();
    outputs.resize(n);
    jassert(n == denseWeights->getNumObjects());

    ObjectPtr perceivedInput;
    if (input.getType() == perception->getOutputType())
      perceivedInput = input.getObject();
    else
      perceivedInput = perception->computeFunction(context, input).getObject();
    for (size_t i = 0; i < n; ++i)
      outputs[i] = lbcpp::dotProduct(context, denseWeights->getObject(i), perceivedInput);
    return res;
  }

  virtual bool loadFromXml(XmlImporter& importer)
  {
    if (!NumericalInference::loadFromXml(importer))
      return false;
    jassert(outputClass);
    numOutputs = outputClass->getObjectNumVariables();
    return true;
  }

private:
  friend class MultiLinearInferenceClass;
  ClassPtr outputClass;
  size_t numOutputs;
};

typedef ReferenceCountedObjectPtr<MultiLinearInference> MultiLinearInferencePtr;

}; /* namespace lbcpp */

#endif // !LBCPP_NUMERICAL_LEARNING_INFERENCE_MULTI_LINEAR_H_
