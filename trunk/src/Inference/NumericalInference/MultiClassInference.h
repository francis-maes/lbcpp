/*-----------------------------------------.---------------------------------.
| Filename: MultiClassInference.h          | Multi Class Inference classes   |
| Author  : Francis Maes                   |                                 |
| Started : 16/10/2010 14:20               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_NUMERICAL_MULTI_CLASS_H_
# define LBCPP_INFERENCE_NUMERICAL_MULTI_CLASS_H_

# include <lbcpp/Inference/DecoratorInference.h>
# include <lbcpp/Inference/SequentialInference.h>
# include <lbcpp/Function/ScalarObjectFunction.h>
# include "../../Data/Object/DenseDoubleObject.h"

namespace lbcpp
{

class MultiClassInference : public StaticDecoratorInference
{
public:
  MultiClassInference(const String& name, EnumerationPtr classes, InferencePtr scoresInference)
    : StaticDecoratorInference(name, scoresInference), classes(classes)
    {setBatchLearner(onlineToBatchInferenceLearner());}

  MultiClassInference() {}

  virtual MultiClassLossFunctionPtr createLossFunction(size_t correctClass) const = 0;

  virtual TypePtr getSupervisionType() const
    {return classes;}

  virtual TypePtr getOutputType(TypePtr ) const
    {return classes;}

  virtual void setName(const String& name)
    {DecoratorInference::setName(name); decorated->setName(name + T(" scores"));}

  virtual DecoratorInferenceStatePtr prepareInference(const InferenceContextPtr& context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
  {
    DecoratorInferenceStatePtr res = new DecoratorInferenceState(input, supervision);
    ScalarObjectFunctionPtr lossFunction;
    if (supervision.exists())
    {
      size_t correctClass = (size_t)supervision.getInteger();
      jassert(correctClass < lossFunctions.size());
      lossFunction = lossFunctions[correctClass];
    }
    res->setSubInference(decorated, input, lossFunction);
    return res;
  }

  // todo: return probability distribution
  virtual Variable finalizeInference(const InferenceContextPtr& context, const DecoratorInferenceStatePtr& finalState, ReturnCode& returnCode)
  {
    Variable subInferenceOutput = finalState->getSubOutput();
    if (!subInferenceOutput.exists())
      return Variable();

    DenseDoubleObjectPtr scores = subInferenceOutput.dynamicCast<DenseDoubleObject>();
    if (scores)
    {
      size_t n = classes->getNumElements();
      double bestScore = -DBL_MAX;
      size_t res = n;
      for (size_t i = 0; i < n; ++i)
      {
        double score = scores->getValue(i);
        if (score > bestScore)
          bestScore = score, res = i;
      }
      return Variable(res, classes);    
    }
    else
    {
      jassert(false); // not implemented
      return Variable();
    }
  }

  virtual bool loadFromXml(XmlImporter& importer)
  {
    if (!StaticDecoratorInference::loadFromXml(importer))
      return false;
    createPerClassLossFunctions();
    return true;
  }

protected:
  friend class MultiClassInferenceClass;

  EnumerationPtr classes;
  std::vector<MultiClassLossFunctionPtr> lossFunctions;

  void createPerClassLossFunctions()
  {
    lossFunctions.resize(classes->getNumElements());
    for (size_t i = 0; i < lossFunctions.size(); ++i)
      lossFunctions[i] = createLossFunction(i);
  }
};

class MultiClassLinearSVMInference : public MultiClassInference
{
public:
  MultiClassLinearSVMInference(PerceptionPtr perception, EnumerationPtr classes, InferenceOnlineLearnerPtr learner, const String& name)
    : MultiClassInference(name, classes, multiLinearInference(name, perception, enumBasedDoubleVectorClass(classes)))
  {
    decorated->setOnlineLearner(learner);
    createPerClassLossFunctions();
  }

  MultiClassLinearSVMInference() {}

  virtual MultiClassLossFunctionPtr createLossFunction(size_t correctClass) const
    {return oneAgainstAllMultiClassLossFunction(hingeLossFunction(true), classes, correctClass);}
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_NUMERICAL_MULTI_CLASS_H_
