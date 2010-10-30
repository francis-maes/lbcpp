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
# include "../../Data/Object/DenseObjectObject.h"
# include "../../Data/Object/DenseDoubleObject.h"

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
    const MultiClassLossFunctionPtr& lossFunction = supervision.getObjectAndCast<MultiClassLossFunction>();
    std::vector<double> lossGradient;
    lossFunction->compute(prediction.getObject(), &exampleLossValue, &lossGradient, 1.0);
    if (lossGradient.empty() || !getPerception()->getOutputType()->getObjectNumVariables())
      return; // when learning the perception, its number of output variables may be null at beginning
    const PerceptionPtr& perception = getPerception();

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
    
    for (size_t i = 0; i < lossGradient.size(); ++i)
    {
      double w = lossGradient[i] * weight;
      if (w)
      {
        ObjectPtr object = parameters->getVariable(i).getObject();
        if (input.getType() == perception->getOutputType())
          lbcpp::addWeighted(object, input.getObject(), w);
        else
          lbcpp::addWeighted(object, perception, input, w);
        parameters->setVariable(i, object);
      }
    }

    if (isLocked)
      parametersLock.exitWrite();
  }

  virtual Variable predict(const Variable& input) const
  {
    ScopedReadLock _(parametersLock);
    const NumericalInferenceParametersPtr& parameters = this->parameters.getObjectAndCast<NumericalInferenceParameters>();
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
    if (input.getType() == perception->getOutputType())
    {
      // perception as already been applied
      for (size_t i = 0; i < n; ++i)
        outputs[i] = lbcpp::dotProduct(denseWeights->getObject(i), input.getObject());
    }
    else
    {
      // default case: simultaneous perception computation and dot-product computation
      for (size_t i = 0; i < n; ++i)
        outputs[i] = lbcpp::dotProduct(denseWeights->getObject(i), perception, input);
    }
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
