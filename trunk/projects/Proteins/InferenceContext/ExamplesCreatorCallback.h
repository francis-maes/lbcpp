/*-----------------------------------------.---------------------------------.
| Filename: ExamplesCreatorCallback.h      | A callback that creates learning|
| Author  : Francis Maes                   |  examples                       |
| Started : 09/04/2010 15:55               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_CONTEXT_EXAMPLES_CREATOR_H_
# define LBCPP_INFERENCE_CONTEXT_EXAMPLES_CREATOR_H_

# include "InferenceCallback.h"
# include "InferenceStack.h"
# include "../InferenceStep/ClassificationInferenceStep.h"
# include "../InferenceStep/RegressionInferenceStep.h"
# include "../InferenceLearner/InferenceLearnerCallback.h"

namespace lbcpp
{

class ExamplesCreatorCallback : public InferenceCallback
{
public:
  ExamplesCreatorCallback(InferenceLearnerCallbackPtr learnerCallback)
    : learnerCallback(learnerCallback), enableExamplesCreation(true) {}

  virtual void classificationCallback(InferenceStackPtr stack, ClassifierPtr& classifier, ObjectPtr& input, ObjectPtr& supervision, ReturnCode& returnCode)
  {
    if (!classifier)
    {
      FeatureDictionaryPtr labels = stack->getCurrentInference().dynamicCast<ClassificationInferenceStep>()->getLabels();
      if (!labels)
      {
        returnCode = InferenceStep::errorReturnCode;
        return;
      }
      classifier = learnerCallback->createClassifier(stack->getCurrentInference(), labels);
    }

    if (supervision && enableExamplesCreation)
    {
      LabelPtr label = supervision.dynamicCast<Label>();
      jassert(classifier && label);
      addExample(classifier, new ClassificationExample(input, label->getIndex()));
    }
  }
  
  virtual void regressionCallback(InferenceStackPtr stack, RegressorPtr& regressor, ObjectPtr& input, ObjectPtr& supervision, ReturnCode& returnCode)
  {
    if (!regressor)
      regressor = learnerCallback->createRegressor(stack->getCurrentInference());
    if (supervision && enableExamplesCreation)
    {
      ScalarPtr scalar = supervision.dynamicCast<Scalar>();
      jassert(scalar);
      addExample(regressor, new RegressionExample(input, scalar->getValue()));
    }    
  }

  void trainStochasticIteration()
  {
    for (ExamplesMap::const_iterator it = examples.begin(); it != examples.end(); ++it)
    {
      LearningMachinePtr machine = it->first;
      ObjectContainerPtr trainingData = it->second->randomize();
      std::cout << "Training with " << trainingData->size() << " examples... " << std::flush;
      machine->trainStochastic(trainingData);
      std::cout << "ok." << std::endl;
      /*GradientBasedRegressorPtr regressor = machine.dynamicCast<GradientBasedRegressor>();
      if (regressor)
      {
        std::cout << "Regressor Num params = " << regressor->getParameters()->l0norm() << " Norm = " << regressor->getParameters()->l2norm() << std::endl;
      }
      GradientBasedClassifierPtr classifier = machine.dynamicCast<GradientBasedClassifier>();
      if (classifier)
      {
        std::cout << "Train accuracy: " << std::flush << classifier->evaluateAccuracy(trainingData) << " Num params = " << classifier->getParameters()->l0norm() << " Norm = " << classifier->getParameters()->l2norm() << std::endl;
      }*/
    }
  }

  void trainAndFlushExamples()
  {
    trainStochasticIteration();
    examples.clear();
  }

protected:
  InferenceLearnerCallbackPtr learnerCallback;
  bool enableExamplesCreation;

  typedef std::map<LearningMachinePtr, VectorObjectContainerPtr> ExamplesMap;
  ExamplesMap examples;

  void addExample(LearningMachinePtr learningMachine, ObjectPtr example)
  {
    VectorObjectContainerPtr& machineExamples = examples[learningMachine];
    if (!machineExamples)
      machineExamples = new VectorObjectContainer();
    machineExamples->append(example);
  }
};

typedef ReferenceCountedObjectPtr<ExamplesCreatorCallback> ExamplesCreatorCallbackPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_CONTEXT_EXAMPLES_CREATOR_H_
