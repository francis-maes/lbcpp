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

  virtual void computeAndAddGradient(double weight, const Variable& input, const Variable& supervision, const Variable& prediction, double& exampleLossValue, ObjectPtr* target)
  {
    const ScalarObjectFunctionPtr& lossFunction = supervision.getObjectAndCast<ScalarObjectFunction>();
    ObjectPtr lossGradient;
    lossFunction->compute(prediction.getObject(), &exampleLossValue, &lossGradient, 1.0);
    if (!lossGradient || !getPerception()->getNumOutputVariables())
      return; // when learning the perception, its number of output variables may be null at beginning

    bool isLocked = false;
    if (!target)
    {
      parametersLock.enterWrite();
      target = &getParameters()->getWeights();
      isLocked = true;
    }
    ObjectPtr& parameters = *target;
    if (!parameters)
      parameters = Object::create(getWeightsType(getPerception()->getOutputType()));
    
    size_t n = lossGradient->getNumVariables();
    for (size_t i = 0; i < n; ++i)
    {
      double w = lossGradient->getVariable(i).getDouble() * weight;
      if (w)
      {
        ObjectPtr object = parameters->getVariable(i).getObject();
        lbcpp::addWeighted(object, getPerception(), input, w);
        parameters->setVariable(i, object);
      }
    }

    if (isLocked)
      parametersLock.exitWrite();
  }

  virtual void updateParametersType()
  {
    const ObjectPtr& weights = getWeights();
    TypePtr weightsType = getWeightsType(getPerception()->getOutputType());
    if (weights->getClass() != weightsType)
      getParameters()->getWeights() = weights->cloneToNewType(weightsType);
  }

  virtual Variable predict(const Variable& input) const
  {
    ScopedReadLock _(parametersLock);
    const ObjectPtr& weights = getParameters()->getWeights();
    if (!weights)
      return Variable::missingValue(outputClass);
    ObjectPtr object = Object::create(outputClass);
    size_t n = object->getNumVariables();
    jassert(n == weights->getNumVariables());
    for (size_t i = 0; i < n; ++i)
    {
      ObjectPtr outputWeights = weights->getVariable(i).getObject();
      jassert(object->getVariableType(i)->inheritsFrom(doubleType));
      object->setVariable(i, lbcpp::dotProduct(outputWeights, getPerception(), input));
    }
    return object;
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
